#include "GameVersions.h"

#include "dolphin/base/PPCArch.h"
#include "dolphin/gx.h"
#include "dolphin/hw_regs.h"
#include "dolphin/mtx.h"
#include "dolphin/os.h"
#include "dolphin/vi.h"

#include "dolphin/gx/GXPriv.h"

static GXData gxData;
static GXFifoObj FifoObj;

GXData *const __GXData = &gxData;

#if VERSION < VERSION_GM8E_02
char *__GXVersion = "<< Dolphin SDK - GX\trelease build: Sep  5 2002 05:33:28 (0x2301) >>";
#else
char *__GXVersion = "<< Dolphin SDK - GX	release build: Nov  7 2002 05:47:57 (0x2301) >>";
#endif

void *__piReg = NULL;
void *__cpReg = NULL;
void *__peReg = NULL;
void *__memReg = NULL;

static GXVtxAttrFmtList GXDefaultVATList[] = {
    {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
    {GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0},
    {GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0},
    {GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0},
    {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX1, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX2, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX3, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX4, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX5, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX6, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX7, GX_TEX_ST, GX_F32, 0},
    {GX_VA_NULL, 0, 0, 0},
};

static f32 GXDefaultProjData[] = {1.0f, 0.0f, 1.0f, 0.0f, -1.0f, -2.0f, 0.0f};

static asm BOOL IsWriteGatherBufferEmpty(void) {
  nofralloc;
  sync;
  mfspr r3, WPAR;
  andi.r3, r3, 1;
  blr;
}

static void EnableWriteGatherPipe(void) {
  u32 hid2;
  hid2 = PPCMfhid2();
  PPCMtwpar(OSUncachedToPhysical((void *)GXFIFO_ADDR));
  hid2 |= 0x40000000;
  PPCMthid2(hid2);
}

static GXTexRegion *__GXDefaultTexRegionCallback(const GXTexObj *obj, GXTexMapID id) {
  GXTexFmt format;

  format = GXGetTexObjFmt(obj);

  if (format != GX_TF_C4 && format != GX_TF_C8 && format != GX_TF_C14X2) {
    return &__GXData->TexRegions[__GXData->nextTexRgn++ & 7];
  } else {
    return &__GXData->TexRegionsCI[__GXData->nextTexRgnCI++ & 3];
  }
}

static GXTlutRegion *__GXDefaultTlutRegionCallback(u32 tlut) {
  if (tlut >= 20) {
    return NULL;
  }

  return &__GXData->TlutRegions[tlut];
}

GXFifoObj *GXInit(void *base, u32 size) {
  u32 i;
  u32 freqBase;
  u32 texRefresh;
  u32 peRefresh;

  OSRegisterVersion(__GXVersion);
  __GXData->inDispList = GX_FALSE;
  __GXData->dlSaveContext = GX_TRUE;
  __GXData->tcsManEnab = 0;
  __GXData->tevTcEnab = 0;

  GXSetMisc(GX_MT_XF_FLUSH, 0);

  __piReg = OSPhysicalToUncached(0x0C003000);
  __cpReg = OSPhysicalToUncached(0x0C000000);
  __peReg = OSPhysicalToUncached(0x0C001000);
  __memReg = OSPhysicalToUncached(0x0C004000);

  __GXFifoInit();

  GXInitFifoBase(&FifoObj, base, size);
  GXSetCPUFifo(&FifoObj);
  GXSetGPFifo(&FifoObj);

  __GXPEInit();
  EnableWriteGatherPipe();

  __GXData->genMode = 0;
  SET_REG_FIELD(__GXData->genMode, 8, 24, 0);
  __GXData->bpMask = 255;
  SET_REG_FIELD(__GXData->bpMask, 8, 24, 0x0F);
  __GXData->lpSize = 0;
  SET_REG_FIELD(__GXData->lpSize, 8, 24, 0x22);

  for (i = 0; i < GX_MAX_TEVSTAGE; i++) {
    __GXData->tevc[i] = 0;
    __GXData->teva[i] = 0;
    __GXData->tref[i / 2] = 0;
    __GXData->texmapId[i] = GX_TEXMAP_NULL;

    SET_REG_FIELD(__GXData->tevc[i], 8, 24, 0xC0 + i * 2);
    SET_REG_FIELD(__GXData->teva[i], 8, 24, 0xC1 + i * 2);
    SET_REG_FIELD(__GXData->tevKsel[i / 2], 8, 24, 0xF6 + i / 2);
    SET_REG_FIELD(__GXData->tref[i / 2], 8, 24, 0x28 + i / 2);
  }

  __GXData->iref = 0;
  SET_REG_FIELD(__GXData->iref, 8, 24, 0x27);

  for (i = 0; i < GX_MAX_TEXCOORD; i++) {
    __GXData->suTs0[i] = 0;
    __GXData->suTs1[i] = 0;

    SET_REG_FIELD(__GXData->suTs0[i], 8, 24, 0x30 + i * 2);
    SET_REG_FIELD(__GXData->suTs1[i], 8, 24, 0x31 + i * 2);
  }

  SET_REG_FIELD(__GXData->suScis0, 8, 24, 0x20);
  SET_REG_FIELD(__GXData->suScis1, 8, 24, 0x21);

  SET_REG_FIELD(__GXData->cmode0, 8, 24, 0x41);
  SET_REG_FIELD(__GXData->cmode1, 8, 24, 0x42);

  SET_REG_FIELD(__GXData->zmode, 8, 24, 0x40);
  SET_REG_FIELD(__GXData->peCtrl, 8, 24, 0x43);

  SET_REG_FIELD(__GXData->cpTex, 2, 7, 0);

  __GXData->dirtyState = 0;
  __GXData->dirtyVAT = 0;

  freqBase = OS_BUS_CLOCK / 500;

  __GXFlushTextureState();
  texRefresh = (freqBase / 2048) | 0x400 | 0x69000000;
  GX_WRITE_RAS_REG(texRefresh);

  __GXFlushTextureState();
  peRefresh = (freqBase / 4224) | 0x200 | 0x46000000;
  GX_WRITE_RAS_REG(peRefresh);

  for (i = 0; i < 8; i++) {
    SET_REG_FIELD(__GXData->vatA[i], 1, 30, 1);
    SET_REG_FIELD(__GXData->vatB[i], 1, 31, 1);
    do {
      s32 regAddr;
      GX_WRITE_U8(0x8);
      GX_WRITE_U8(i | 0x80);
      GX_WRITE_U32(__GXData->vatB[i]);
      regAddr = i - 12;
    } while (0);
  }

  {
    u32 reg1 = 0;
    u32 reg2 = 0;

    SET_REG_FIELD(reg1, 1, 0, 1);
    SET_REG_FIELD(reg1, 1, 1, 1);
    SET_REG_FIELD(reg1, 1, 2, 1);
    SET_REG_FIELD(reg1, 1, 3, 1);
    SET_REG_FIELD(reg1, 1, 4, 1);
    SET_REG_FIELD(reg1, 1, 5, 1);
    GX_WRITE_XF_REG(0, reg1);

    SET_REG_FIELD(reg2, 1, 0, 1);
    GX_WRITE_XF_REG(0x12, reg2);
  }

  {
    u32 reg = 0;
    SET_REG_FIELD(reg, 1, 0, 1);
    SET_REG_FIELD(reg, 1, 1, 1);
    SET_REG_FIELD(reg, 1, 2, 1);
    SET_REG_FIELD(reg, 1, 3, 1);
    SET_REG_FIELD(reg, 8, 24, 0x58);

    GX_WRITE_RAS_REG(reg);
  }

  for (i = 0; i < 8; i++) {
    GXInitTexCacheRegion(&__GXData->TexRegions[i], 0, i * 0x8000, 0, 0x80000 + i * 0x8000, 0);
  }

  for (i = 0; i < 4; i++) {
    GXInitTexCacheRegion(&__GXData->TexRegionsCI[i], 0, (i * 2 + 8) * 0x8000, 0,
                         (i * 2 + 9) * 0x8000, 0);
  }

  for (i = 0; i < 16; i++) {
    GXInitTlutRegion(&__GXData->TlutRegions[i], 0xC0000 + 0x2000 * i, GX_TLUT_256);
  }

  for (i = 0; i < 4; i++) {
    GXInitTlutRegion(&__GXData->TlutRegions[i + 16], 0xE0000 + 0x8000 * i, GX_TLUT_1K);
  }

  {
    u32 reg = 0;

    GX_SET_CP_REG(3, reg);

    SET_REG_FIELD(__GXData->perfSel, 4, 4, 0);
    GX_WRITE_U8(0x8);
    GX_WRITE_U8(0x20);
    GX_WRITE_U32(__GXData->perfSel);

    reg = 0;
    GX_WRITE_XF_REG(6, reg);

    reg = 0x23000000;
    GX_WRITE_RAS_REG(reg);

    reg = 0x24000000;
    GX_WRITE_RAS_REG(reg);

    reg = 0x67000000;
    GX_WRITE_RAS_REG(reg);
  }

  __GXSetTmemConfig(0);

  __GXInitGX();

  return &FifoObj;
}

void __GXInitGX(void) {
  GXRenderModeObj *renderObj;

  Mtx ident;
  GXColor clearColor = {64, 64, 64, 255};
  GXColor ambColor = {0, 0, 0, 0};
  GXColor matColor = {255, 255, 255, 255};
  u32 i;

  switch (VIGetTvFormat()) {
  case VI_NTSC:
    renderObj = &GXNtsc480IntDf;
    break;

  case VI_PAL:
    renderObj = &GXPal528IntDf;
    break;

  case VI_EURGB60:
    renderObj = &GXEurgb60Hz480IntDf;
    break;

  case VI_MPAL:
    renderObj = &GXMpal480IntDf;
    break;

  default:
    renderObj = &GXNtsc480IntDf;
    break;
  }

  GXSetCopyClear(clearColor, 0xFFFFFF);

  GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX3, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  GXSetTexCoordGen2(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX4, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  GXSetTexCoordGen2(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX5, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  GXSetTexCoordGen2(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX6, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  GXSetTexCoordGen2(GX_TEXCOORD7, GX_TG_MTX2x4, GX_TG_TEX7, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

  GXSetNumTexGens(1);
  GXClearVtxDesc();
  GXInvalidateVtxCache();

  for (i = GX_VA_POS; i <= GX_LIGHT_ARRAY; i++) {
    GXSetArray((GXAttr)i, __GXData, 0);
  }

  for (i = 0; i < GX_MAX_VTXFMT; i++) {
    GXSetVtxAttrFmtv((GXVtxFmt)i, GXDefaultVATList);
  }

  GXSetLineWidth(6, GX_TO_ZERO);
  GXSetPointSize(6, GX_TO_ZERO);
  GXEnableTexOffsets(GX_TEXCOORD0, GX_FALSE, GX_FALSE);
  GXEnableTexOffsets(GX_TEXCOORD1, GX_FALSE, GX_FALSE);
  GXEnableTexOffsets(GX_TEXCOORD2, GX_FALSE, GX_FALSE);
  GXEnableTexOffsets(GX_TEXCOORD3, GX_FALSE, GX_FALSE);
  GXEnableTexOffsets(GX_TEXCOORD4, GX_FALSE, GX_FALSE);
  GXEnableTexOffsets(GX_TEXCOORD5, GX_FALSE, GX_FALSE);
  GXEnableTexOffsets(GX_TEXCOORD6, GX_FALSE, GX_FALSE);
  GXEnableTexOffsets(GX_TEXCOORD7, GX_FALSE, GX_FALSE);

  ident[0][0] = 1.0f;
  ident[0][1] = 0.0f;
  ident[0][2] = 0.0f;
  ident[0][3] = 0.0f;

  ident[1][0] = 0.0f;
  ident[1][1] = 1.0f;
  ident[1][2] = 0.0f;
  ident[1][3] = 0.0f;

  ident[2][0] = 0.0f;
  ident[2][1] = 0.0f;
  ident[2][2] = 1.0f;
  ident[2][3] = 0.0f;

  GXLoadPosMtxImm(ident, GX_PNMTX0);
  GXLoadNrmMtxImm(ident, GX_PNMTX0);
  GXSetCurrentMtx(GX_PNMTX0);

  GXLoadTexMtxImm(ident, GX_IDENTITY, GX_MTX3x4);
  GXLoadTexMtxImm(ident, GX_PTIDENTITY, GX_MTX3x4);

  GXSetViewport(0.0f, 0.0f, renderObj->fbWidth, renderObj->xfbHeight, 0.0f, 1.0f);

  GXSetProjectionv(GXDefaultProjData);

  GXSetCoPlanar(GX_FALSE);
  GXSetCullMode(GX_CULL_BACK);
  GXSetClipMode(GX_CLIP_ENABLE);

  GXSetScissor(0, 0, renderObj->fbWidth, renderObj->efbHeight);
  GXSetScissorBoxOffset(0, 0);

  GXSetNumChans(0);
  GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
  GXSetChanAmbColor(GX_COLOR0A0, ambColor);
  GXSetChanMatColor(GX_COLOR0A0, matColor);

  GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
  GXSetChanAmbColor(GX_COLOR1A1, ambColor);
  GXSetChanMatColor(GX_COLOR1A1, matColor);

  GXInvalidateTexAll();

  __GXData->nextTexRgn = 0;
  __GXData->nextTexRgnCI = 0;

  GXSetTexRegionCallback(__GXDefaultTexRegionCallback);
  GXSetTlutRegionCallback(__GXDefaultTlutRegionCallback);

  GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
  GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0);
  GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP3, GX_COLOR0A0);
  GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD4, GX_TEXMAP4, GX_COLOR0A0);
  GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD5, GX_TEXMAP5, GX_COLOR0A0);
  GXSetTevOrder(GX_TEVSTAGE6, GX_TEXCOORD6, GX_TEXMAP6, GX_COLOR0A0);
  GXSetTevOrder(GX_TEVSTAGE7, GX_TEXCOORD7, GX_TEXMAP7, GX_COLOR0A0);

  GXSetTevOrder(GX_TEVSTAGE8, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  GXSetTevOrder(GX_TEVSTAGE9, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  GXSetTevOrder(GX_TEVSTAGE10, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  GXSetTevOrder(GX_TEVSTAGE11, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  GXSetTevOrder(GX_TEVSTAGE12, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  GXSetTevOrder(GX_TEVSTAGE13, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  GXSetTevOrder(GX_TEVSTAGE14, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  GXSetTevOrder(GX_TEVSTAGE15, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);

  GXSetNumTevStages(1);
  GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

  GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

  GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);

  for (i = 0; i < GX_MAX_TEVSTAGE; i++) {
    GXSetTevKColorSel((GXTevStageID)i, GX_TEV_KCSEL_1_4);
    GXSetTevKAlphaSel((GXTevStageID)i, GX_TEV_KASEL_1);
    GXSetTevSwapMode((GXTevStageID)i, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }

  GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
  GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
  GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
  GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);

  for (i = 0; i < GX_MAX_TEVSTAGE; i++) {
    GXSetTevDirect((GXTevStageID)i);
  }

  GXSetNumIndStages(0);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE2, GX_ITS_1, GX_ITS_1);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE3, GX_ITS_1, GX_ITS_1);

  GXSetFog(GX_FOG_NONE, 0.0f, 1.0f, 0.1f, 1.0f, ambColor);
  GXSetFogRangeAdj(GX_FALSE, 0, NULL);

  GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

  GXSetColorUpdate(GX_TRUE);
  GXSetAlphaUpdate(GX_TRUE);

  GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
  GXSetZCompLoc(GX_TRUE);

  GXSetDither(GX_TRUE);

  GXSetDstAlpha(GX_FALSE, 0);
  GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

  GXSetFieldMask(GX_TRUE, GX_TRUE);
  GXSetFieldMode((GXBool)renderObj->field_rendering,
                 (GXBool)((renderObj->viHeight == 2 * renderObj->xfbHeight) ? GX_TRUE : GX_FALSE));

  GXSetDispCopySrc(0, 0, renderObj->fbWidth, renderObj->efbHeight);
  GXSetDispCopyDst(renderObj->fbWidth, renderObj->efbHeight);
  GXSetDispCopyYScale((f32)renderObj->xfbHeight / (f32)renderObj->efbHeight);
  GXSetCopyClamp((GXFBClamp)(GX_CLAMP_TOP | GX_CLAMP_BOTTOM));

  GXSetCopyFilter(renderObj->aa, renderObj->sample_pattern, GX_TRUE, renderObj->vfilter);
  GXSetDispCopyGamma(GX_GM_1_0);
  GXSetDispCopyFrame2Field(GX_COPY_PROGRESSIVE);
  GXClearBoundingBox();

  GXPokeColorUpdate(GX_TRUE);
  GXPokeAlphaUpdate(GX_TRUE);
  GXPokeDither(GX_FALSE);
  GXPokeBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ONE, GX_LO_SET);
  GXPokeAlphaMode(GX_ALWAYS, 0);
  GXPokeAlphaRead(GX_READ_FF);
  GXPokeDstAlpha(GX_FALSE, 0);
  GXPokeZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
  GXSetGPMetric(GX_PERF0_NONE, GX_PERF1_NONE);
  GXClearGPMetric();
}
