#include "Kyoto/Graphics/CCubeMaterial.hpp"

#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGX_Impl.hpp" // IWYU pragma: keep
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "MetaRender/CCubeRenderer.hpp"
#include <dolphin/mtx.h>

#include "Kyoto/MemoryCopy.hpp"

// Material data is big-endian; keep direct loads on big-endian targets.
#if TARGET_LITTLE_ENDIAN
#define SBig(x) CBasics::SwapBytes(x)
#else
#define SBig(x) x
#endif

static const float gkEpsilon32 = FLT_EPSILON;

static CVector3f sPlayerPosition(0.f, 0.f, 0.f);
CVector3f CCubeMaterial::sViewingFrom(0.f, 0.f, 0.f);
static CTransform4f sTextureProjectionTransform(CTransform4f::Identity());
int sLastMaterialUnique = -1;
static int sMaterialCachedState = 0;
const CCubeModel* CCubeMaterial::sLastModelCached = nullptr;
const CCubeModel* CCubeMaterial::sRenderingModel = nullptr;
const uchar* sLastMaterialCached = nullptr;
static CTexture* spShadowTexture = NULL;
static float sReflectionAlpha = 0.f;
static float sLastTime = 0.f;
static float sThrobX = 1.f;
static float sThrobY = 1.f;
static int sReflectionStage = -1;
static bool sbRenderModelBlack = false;
static bool lbl_805A9559 = false;
static bool sbRenderModelShadow = false;
static uchar sChannel0DisableLightMask = 0;
static uchar sChannel1EnableLightMask = 0;
static const GXColor sGXBlack = {0, 0, 0, 255};
static const GXColor sGXWhite = {0xFF, 0xFF, 0xFF, 0xFF};

void CCubeMaterial::SetupBlendMode(const uint blendFactors, const CModelFlags& flags,
                                   bool alphaTest) {
  GXBlendFactor newSrcFactor = static_cast< GXBlendFactor >(blendFactors & 0xFFFF);
  GXBlendFactor newDstFactor = static_cast< GXBlendFactor >(blendFactors >> 0x10);
  CModelFlags::ETrans blendMode = flags.GetTrans();

  if (alphaTest) {
    CGX::SetAlphaCompare(GX_GEQUAL, 64, GX_AOP_OR, GX_NEVER, 0);
    newSrcFactor = GX_BL_ONE;
    newDstFactor = GX_BL_ZERO;
  } else {
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  }
  // TODO: how to check
  if (blendMode > CModelFlags::kT_Four && newSrcFactor == GX_BL_ONE) {
    newSrcFactor = GX_BL_SRCALPHA;
    if (newDstFactor == GX_BL_ZERO) {
      newDstFactor = blendMode > 6 ? GX_BL_ONE : GX_BL_INVSRCALPHA;
    }
  }
  CGX::SetBlendMode(GX_BM_BLEND, newSrcFactor, newDstFactor, GX_LO_CLEAR);
}

static void HandleTev(int tevCur, const uint* materialDataCur, const uint* texMapTexCoordFlags,
                      bool shadowMapsEnabled) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCur);
  const uint colorArgs = shadowMapsEnabled ? 0x7a04f : SBig(materialDataCur[0]);
  const uint alphaArgs = SBig(materialDataCur[1]);
  const uint colorOps = SBig(materialDataCur[2]);
  const uint alphaOps = SBig(materialDataCur[3]);

  CGX::SetStandardDirectTev_Compressed(stage, colorArgs, alphaArgs, colorOps, alphaOps);

  uint tmtcFlags = SBig(*texMapTexCoordFlags);
  uint matFlags = SBig(materialDataCur[4]);
  CGX::SetTevOrder(stage, static_cast< GXTexCoordID >(tmtcFlags & 0xFF),
                   static_cast< GXTexMapID >(tmtcFlags >> 8 & 0xFF),
                   static_cast< GXChannelID >(matFlags & 0xFF));
  CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(matFlags >> 0x8 & 0xFF));
  CGX::SetTevKAlphaSel(stage, static_cast< GXTevKAlphaSel >(matFlags >> 0x10 & 0xFF));
}

static uint HandleAnimatedUV(const uint* uvAnim, GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  static const Mtx postMtx = {
      {0.5f, 0.0f, 0.0f, 0.5f},
      {0.0f, 0.0f, 0.5f, 0.5f},
      {0.0f, 0.0f, 0.0f, 1.0f},
  };
  static Mtx texMtx1 = {
      {1.f, 0.f, 0.f, 0.f},
      {0.f, 1.f, 0.f, 0.f},
      {0.f, 0.f, 1.f, 0.f},
  };
  uint type = SBig(*uvAnim);
  const float* params = reinterpret_cast< const float* >(uvAnim + 1);
  switch (type) {
  case 0: {
    CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse().MultiplyIgnoreTranslation(
        CGraphics::GetModelMatrix());
    xf.SetTranslation(CVector3f::Zero());
    CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
    CGX::LoadTexMtxImm(postMtx, ptTexMtx, GX_MTX3x4);
    return 1;
  }
  case 1: {
    CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse() * CGraphics::GetModelMatrix();
    CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
    CGX::LoadTexMtxImm(postMtx, ptTexMtx, GX_MTX3x4);
    return 1;
  }
  case 2: {
    const float f1 = SBig(params[0]);
    const float f2 = SBig(params[1]);
    const float f3 = SBig(params[2]);
    const float f4 = SBig(params[3]);
    const float seconds = CGraphics::GetSecondsMod900();
    texMtx1[0][3] = f1 + seconds * f3;
    texMtx1[1][3] = f2 + seconds * f4;
    CGX::LoadTexMtxImm(texMtx1, texMtx, GX_MTX3x4);
    return 5;
  }
  case 3: {
    const float f1 = SBig(params[0]);
    const float f2 = SBig(params[1]);
    const float seconds = CGraphics::GetSecondsMod900();
    const float angle = f1 + seconds * f2;
    const float asin = CMath::FastSinR(angle);
    const float acos = CMath::FastCosR(angle);
    Mtx mtx = {
        {acos, -asin, 0.f, 0.5f * (1.f - (acos - asin))},
        {asin, acos, 0.f, 0.5f * (1.f - (asin + acos))},
        {0.f, 0.f, 1.f, 0.f},
    };
    CGX::LoadTexMtxImm(mtx, texMtx, GX_MTX3x4);
    return 3;
  }
  case 4:
  case 5: {
    const float f1 = SBig(params[0]);
    const float f2 = SBig(params[1]);
    const float f3 = SBig(params[2]);
    const float f4 = SBig(params[3]);
    const float value = (f4 + CGraphics::GetSecondsMod900()) * f1 * f3;
    const float fmod = CMath::FastFmod(value, 1.f);
    const float fs = CCast::FtoS(fmod * f2);
    const float v2 = fs * f3;
    if (type == 4) {
      texMtx1[0][3] = v2;
      texMtx1[1][3] = 0.f;
    } else {
      texMtx1[0][3] = 0.f;
      texMtx1[1][3] = v2;
    }
    CGX::LoadTexMtxImm(texMtx1, texMtx, GX_MTX3x4);
    return 5;
  }
  case 6: {
    static const Mtx sTexMtx = {
        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 0.f},
    };
    static const Mtx sPtMtx = {
        {0.5f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.5f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    };
    const CTransform4f& mm = CGraphics::GetModelMatrix();
    const CVector3f& rowX = mm.GetRow(kDX);
    const CVector3f& rowY = mm.GetRow(kDY);
    const CVector3f& rowZ = mm.GetRow(kDZ);
    Mtx tmpTexMtx;
    memcpy(&tmpTexMtx, &sTexMtx, sizeof(Mtx));
    tmpTexMtx[0][0] = rowX[kDX];
    tmpTexMtx[0][1] = rowX[kDY];
    tmpTexMtx[0][2] = rowX[kDZ];
    tmpTexMtx[1][0] = rowY[kDX];
    tmpTexMtx[1][1] = rowY[kDY];
    tmpTexMtx[1][2] = rowY[kDZ];
    tmpTexMtx[2][0] = rowZ[kDX];
    tmpTexMtx[2][1] = rowZ[kDY];
    tmpTexMtx[2][2] = rowZ[kDZ];
    Mtx tmpPtMtx;
    memcpy(&tmpPtMtx, &sPtMtx, sizeof(Mtx));
    tmpPtMtx[0][3] = mm.Get03() * 0.05f;
    tmpPtMtx[1][3] = mm.Get13() * 0.05f;
    CGX::LoadTexMtxImm(tmpTexMtx, texMtx, GX_MTX3x4);
    CGX::LoadTexMtxImm(tmpPtMtx, ptTexMtx, GX_MTX3x4);
    return 1;
  }
  case 7: {
    static const Mtx sPtMtx = {
        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    };
    const CTransform4f& vm = CGraphics::GetViewMatrix();
    CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse().MultiplyIgnoreTranslation(
        CGraphics::GetModelMatrix());
    xf.SetTranslation(CVector3f::Zero());
    Mtx tmpPtMtx;
    memcpy(&tmpPtMtx, &sPtMtx, sizeof(Mtx));
    float scale = SBig(params[0]);
    scale = 0.5f * scale;
    tmpPtMtx[0][0] = scale;
    tmpPtMtx[0][3] = CMath::FastFmod(0.025f * (vm.Get03() + vm.Get13()) * SBig(params[1]), 1.f);
    tmpPtMtx[1][2] = scale;
    tmpPtMtx[1][3] = CMath::FastFmod(0.05f * vm.Get23() * SBig(params[1]), 1.f);
    CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
    CGX::LoadTexMtxImm(tmpPtMtx, ptTexMtx, GX_MTX3x4);
    return 3;
  }
  default:
    return 0;
  }
}

static void HandleTransparency(uint& finalTevCount, uint& finalKColorCount,
                               const CModelFlags& modelFlags, uint blendFactors, uint& finalCCFlags,
                               uint& finalACFlags) {
  const CModelFlags::ETrans blendMode = modelFlags.GetTrans();
  const CColor& color = modelFlags.GetColorRef();

  if (blendMode == 2) {
    if (static_cast< GXBlendFactor >(blendFactors >> 16) == 1) {
      return;
    }
  }

  if (blendMode == 3) {
    const uint stage = finalTevCount;
    const uint stage2 = stage + 1;
    CGX::SetTevColorIn(static_cast< GXTevStageID >(stage), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                       GX_CC_KONST);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(stage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_APREV);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVREG0);
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0_A));
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVPREV);
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    CGX::SetTevDirect(static_cast< GXTevStageID >(stage));

    CGX::SetTevColorIn(static_cast< GXTevStageID >(stage2), GX_CC_CPREV, GX_CC_C0, GX_CC_KONST,
                       GX_CC_ZERO);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(stage2), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_APREV);
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(stage2));
    CGX::SetTevDirect(static_cast< GXTevStageID >(stage2));
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage2), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color.GetGXColor());

    finalKColorCount += 1;
    finalTevCount += 2;
  } else {
    uint alphaArgs = 0x380c7;
    if (blendMode == 8) {
      alphaArgs = 0x31ce7;
    }

    uint colorArgs = 0x781cf;
    if (blendMode == 2) {
      colorArgs = 0x7018f;
    }

    const uint stage = finalTevCount;
    CGX::SetTevColorIn_Compressed(static_cast< GXTevStageID >(stage), colorArgs);
    CGX::SetTevAlphaIn_Compressed(static_cast< GXTevStageID >(stage), alphaArgs);
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(stage));

    finalCCFlags = 0x100;
    finalACFlags = 0x100;

    CGX::SetTevDirect(static_cast< GXTevStageID >(stage));
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color.GetGXColor());
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKAlphaSel >(finalKColorCount + GX_TEV_KASEL_K0_A));

    finalTevCount += 1;
    finalKColorCount += 1;
  }
}

static void DoModelShadow(uint texCount, uint tcgCount) {
  static const Mtx identity2D = {
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  };

  spShadowTexture->Load(static_cast< GXTexMapID >(texCount), CTexture::kCM_Repeat);

  Mtx mtx;
  memcpy(&mtx, &identity2D, sizeof(Mtx));
  mtx[0][0] = sTextureProjectionTransform.Get00();
  mtx[0][1] = sTextureProjectionTransform.Get01();
  mtx[0][2] = sTextureProjectionTransform.Get02();
  mtx[0][3] = sTextureProjectionTransform.Get03();
  mtx[1][0] = sTextureProjectionTransform.Get20();
  mtx[1][1] = sTextureProjectionTransform.Get21();
  mtx[1][2] = sTextureProjectionTransform.Get22();
  mtx[1][3] = sTextureProjectionTransform.Get23();
  CGX::LoadTexMtxImm(mtx, GX_TEXMTX5, GX_MTX3x4);

  CGX::SetTexCoordGen(static_cast< GXTexCoordID >(tcgCount), GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX5,
                      GX_FALSE, GX_PTIDENTITY);

  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
  CGX::SetTevOrder(GX_TEVSTAGE0, static_cast< GXTexCoordID >(tcgCount),
                   static_cast< GXTexMapID >(texCount), GX_COLOR1A1);

  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_C0);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_A0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_1);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
}

uint CCubeMaterial::HandleReflection(bool usesTevReg2, const GXTexMapID indTexSlot,
                                     const int indMtxScaleExp, const uint tevCount,
                                     const uint texCount, const uint tcgCount,
                                     const uint kColorCount, uint& finalCCFlags,
                                     uint& finalACFlags) {
  uint out = 0;
  GXTevStageID finalTevCount = static_cast< GXTevStageID >(tevCount);
  GXTevColorArg colorArg = GX_CC_CPREV;
  const GXTevKColorID finalKColorCount = static_cast< GXTevKColorID >(kColorCount);
  const GXTexMapID texMap = static_cast< GXTexMapID >(texCount);
  if (usesTevReg2) {
    colorArg = GX_CC_C2;
    CGX::SetTevColorIn(finalTevCount, GX_CC_ZERO, GX_CC_C2, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(finalTevCount, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
    CGX::SetTevColorOp(finalTevCount, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevAlphaOp(finalTevCount, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevOrder(finalTevCount, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_ZERO);
    out = 1;
  } else {
    colorArg = GX_CC_KONST;
  }

  CColor reflectionColor;
  reflectionColor.Set(sReflectionAlpha, sReflectionAlpha, sReflectionAlpha, sReflectionAlpha);
  CGX::SetTevKColor(finalKColorCount, reflectionColor.GetGXColor());
  CGX::SetTevKColorSel(finalTevCount,
                       static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));

  finalTevCount = static_cast< GXTevStageID >(finalTevCount + out);

  CCubeRenderer::That()->GetRealReflection()->Load(texMap, CTexture::kCM_Clamp);

  GXTexCoordID texCoord;
  if (indTexSlot != GX_TEXMAP_NULL) {
    texCoord = static_cast< GXTexCoordID >(tcgCount + 1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, static_cast< GXTexCoordID >(tcgCount), indTexSlot);
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(tcgCount), GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX6,
                        GX_TRUE, GX_PTTEXMTX6);
    CGX::SetTevIndWarp(finalTevCount, GX_INDTEXSTAGE0, 1, 0, GX_ITM_0);
    CGX::SetIndTexMtxSTPointFive(GX_ITM_0, static_cast< s8 >(indMtxScaleExp));
    if (sReflectionStage > static_cast< int >(finalTevCount)) {
      CGX::SetTevDirect(static_cast< GXTevStageID >(sReflectionStage));
    }
    sReflectionStage = finalTevCount;
  } else {
    texCoord = static_cast< GXTexCoordID >(tcgCount);
  }

  CGX::SetTexCoordGen(texCoord, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX7, GX_FALSE, GX_PTIDENTITY);

  CGX::SetTevColorIn(finalTevCount, GX_CC_ZERO, colorArg, GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetTevAlphaIn(finalTevCount, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(finalTevCount);

  finalACFlags = 0;
  finalCCFlags = 0;
  CGX::SetTevOrder(finalTevCount, texCoord, texMap, GX_COLOR_NULL);
  return out + 1;
}

void CCubeMaterial::EnsureTevsDirect() {
  if (sReflectionStage == -1) {
    return;
  }

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(static_cast< GXTevStageID >(sReflectionStage));
  sReflectionStage = -1;
}

void CCubeMaterial::SetCurrentBlack() const {
  const uint* data = reinterpret_cast< const uint* >(mData);
  const uint texCount = CBasics::SwapBytes(data[1]);
  const uint flags = CBasics::SwapBytes(data[0]);
  const uint vertexDesc = CBasics::SwapBytes(data[texCount + 2]);

  if ((flags & (kStateFlag_DepthSorting | kStateFlag_AlphaTest)) != 0) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  }

  CGX::SetVtxDescv_Compressed(vertexDesc);

  GXTevColorArg colorArg = GX_CC_ZERO;
  if (lbl_805A9559) {
    colorArg = GX_CC_ONE;
  }
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, colorArg);

  GXTevAlphaArg alphaArg = GX_CA_ZERO;
  if (lbl_805A9559) {
    alphaArg = GX_CA_KONST;
  }
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, alphaArg);

  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetNumTevStages(1);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumIndStages(0);
}

static uint HandleColorChannels(uint chanCount, uint firstChan) {
  if (sbRenderModelShadow) {
    if (chanCount != 0) {
      CGX::SetChanAmbColor(CGX::Channel1, sGXBlack);
      CGX::SetChanMatColor(CGX::Channel1, sGXWhite);
      CGX::SetChanCtrl(CGX::Channel1, true, GX_SRC_REG, GX_SRC_REG,
                       static_cast< GXLightID >(sChannel1EnableLightMask), GX_DF_CLAMP, GX_AF_SPOT);

      const uchar chan0Lights = CGraphics::GetLightMask() & ~sChannel0DisableLightMask;
      CGX::SetChanCtrl_Compressed(CGX::Channel0, static_cast< GXLightID >(chan0Lights), firstChan);
      if (chan0Lights != 0) {
        CGX::SetChanMatColor(CGX::Channel0, sGXWhite);
      } else {
        CGX::SetChanMatColor(CGX::Channel0, CGX::GetChanAmbColor(CGX::Channel0));
      }
    }
    return 2;
  }

  if (chanCount == 2) {
    CGX::SetChanAmbColor(CGX::Channel1, sGXBlack);
    CGX::SetChanMatColor(CGX::Channel1, sGXWhite);
  } else {
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  }

  if (chanCount >= 1) {
    const uchar lightMask = CGraphics::GetLightMask();
    CGX::SetChanCtrl_Compressed(CGX::Channel0, static_cast< GXLightID >(lightMask), firstChan);
    if (lightMask != 0) {
      CGX::SetChanMatColor(CGX::Channel0, sGXWhite);
    } else {
      CGX::SetChanMatColor(CGX::Channel0, CGX::GetChanAmbColor(CGX::Channel0));
    }
  } else {
    CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  }

  return chanCount;
}

static void HandleDepth(uint modelFlags, uint matFlags) {
  GXCompare func = GX_NEVER;
  if ((modelFlags & CModelFlags::kF_DepthCompare) == 0) {
    func = GX_ALWAYS;
  } else if ((modelFlags & CModelFlags::kF_DepthGreater) != 0) {
    if ((modelFlags & CModelFlags::kF_DepthNonInclusive) != 0) {
      func = GX_GREATER;
    } else {
      func = GX_GEQUAL;
    }
  } else if ((modelFlags & CModelFlags::kF_DepthNonInclusive) != 0) {
    func = GX_LESS;
  } else {
    func = GX_LEQUAL;
  }
  CGX::SetZMode(true, func,
                (modelFlags & CModelFlags::kF_DepthUpdate) == CModelFlags::kF_DepthUpdate &&
                    (matFlags & kStateFlag_DepthWrite) != 0);
}

static void DoPassthru(const uint finalTevCount) {
  const GXTevStageID stage = static_cast< GXTevStageID >(finalTevCount);
  CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevDirect(stage);
  CGX::SetStandardTevColorAlphaOp(stage);
}

union scanner_t {
  const uint* words;
  const uchar* bytes;
};

void CCubeMaterial::SetCurrent(const CModelFlags& flags, const CCubeSurface& surface,
                               const CCubeModel& model) const {
  if (mData == sLastMaterialCached) {
    switch (sMaterialCachedState) {
    case 1:
      if (sLastModelCached == sRenderingModel) {
        return;
      }
      break;
    case 2:
      break;
    default:
      return;
    }
  }

  if (sbRenderModelBlack) {
    SetCurrentBlack();
    return;
  }

  uint numIndStages = 0;
  GXTexMapID indTexSlot = GX_TEXMAP0;
  scanner_t materialDataCur;
  materialDataCur.bytes = GetData();
  const uint matFlags = SBig(materialDataCur.words[0]);
  uint texCount = SBig(materialDataCur.words[1]);
  const bool reflection =
      (matFlags & (kStateFlag_Reflection | kStateFlag_ReflectionSurfaceEye)) != 0;
  if (reflection) {
    sLastMaterialCached = materialDataCur.bytes;
    sRenderingModel = &model;
    EnsureViewDepStateCached((matFlags & kStateFlag_ReflectionSurfaceEye) != 0 ? &surface
                                                                               : nullptr);
  }

  sRenderingModel = &model;
  sLastMaterialCached = GetData();
  if ((flags.GetOtherFlags() & CModelFlags::kF_NoTextureLock) == 0) {
    const rstl::vector< TCachedToken< CTexture > >& textures = model.GetTextures();
    materialDataCur.words += 2;
    for (uint i = 0; i < texCount; ++i) {
      textures[SBig(*materialDataCur.words)].GetObject()->Load(static_cast< GXTexMapID >(i),
                                                               CTexture::kCM_Repeat);
      ++materialDataCur.words;
    }
  } else {
    materialDataCur.words += texCount + 2;
  }

  const int groupIdx = static_cast< int >(SBig(materialDataCur.words[1]));
  if (sLastMaterialUnique != -1 && sLastMaterialUnique == groupIdx && sMaterialCachedState == 0) {
    return;
  }
  sLastMaterialUnique = groupIdx;

  CGX::SetVtxDescv_Compressed(SBig(materialDataCur.words[0]));
  materialDataCur.words += 2;

  const bool packedLightmaps = (matFlags & kStateFlag_LightmapUvArray) != 0;
  if (packedLightmaps != CCubeModel::IsUsingPackedLightmaps()) {
    model.SetUsingPackedLightmaps(packedLightmaps);
  }

  uint finalKColorCount = 0;
  if ((matFlags & kStateFlag_KonstValues) != 0) {
    finalKColorCount = SBig(materialDataCur.words[0]);
    for (uint i = 0; i < finalKColorCount; ++i) {
      CGX::SetTevKColor(static_cast< GXTevKColorID >(i),
                        CColor::ToGX(SBig(materialDataCur.words[i + 1])));
    }
    materialDataCur.words += finalKColorCount + 1;
  }

  const uint blendFactors = SBig(*materialDataCur.words);
  if (CCubeRenderer::That()->GetInAreaDraw()) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    SetupBlendMode(blendFactors, flags, (matFlags & kStateFlag_AlphaTest) != 0);
  }

  ++materialDataCur.words;
  const bool indTex = (matFlags & kStateFlag_ReflectionIndirectTexture) != 0;
  if (indTex) {
    indTexSlot = static_cast< GXTexMapID >(SBig(*materialDataCur.words++));
  }

  HandleDepth(flags.GetOtherFlags(), matFlags);

  const uint chanCount = SBig(materialDataCur.words[0]);
  const uint firstChan = SBig(materialDataCur.words[1]);
  materialDataCur.words += chanCount + 1;
  const uint finalNumColorChans = HandleColorChannels(chanCount, firstChan);

  uint matTevCount;
  uint firstTev = 0;
  if (sbRenderModelShadow) {
    firstTev = 2;
  }

  matTevCount = SBig(*materialDataCur.words++);
  uint finalTevCount = matTevCount;
  uint finalCCFlags = 0;
  uint finalACFlags = 0;
  scanner_t texMapTexCoordFlags;
  texMapTexCoordFlags = materialDataCur;
  texMapTexCoordFlags.bytes += matTevCount * 20;
  bool usesTevReg2 = false;

  const bool thermal = CCubeRenderer::That()->GetThermal();
  if (thermal) {
    scanner_t savedTexMapTexCoordFlags;
    savedTexMapTexCoordFlags = texMapTexCoordFlags;
    finalTevCount = firstTev + 1;
    finalCCFlags = SBig(materialDataCur.words[2]);
    if ((finalCCFlags >> 9 & 3) == GX_TEVREG0) {
      materialDataCur.words += 5;
      texMapTexCoordFlags.words += 1;
      const GXColor thermalColor = {0xc0, 0xc0, 0xc0, 0xc0};
      GXSetTevColor(GX_TEVREG0, thermalColor);
      finalCCFlags = SBig(materialDataCur.words[2]);
    }
    finalACFlags = SBig(materialDataCur.words[3]);
    HandleTev(firstTev, materialDataCur.words, texMapTexCoordFlags.words, sbRenderModelShadow);
    usesTevReg2 = false;
    texMapTexCoordFlags.words = savedTexMapTexCoordFlags.words + matTevCount;
  } else {
    finalTevCount = matTevCount + firstTev;
    for (uint i = firstTev; i < finalTevCount; ++i) {
      HandleTev(i, materialDataCur.words, texMapTexCoordFlags.words,
                sbRenderModelShadow && i == firstTev);
      finalCCFlags = SBig(materialDataCur.words[2]);
      finalACFlags = SBig(materialDataCur.words[3]);
      if ((finalCCFlags >> 9 & 3) == GX_TEVREG2) {
        usesTevReg2 = true;
      }
      materialDataCur.words += 5;
      texMapTexCoordFlags.words += 1;
    }
  }

  scanner_t uvAnim;
  uvAnim = texMapTexCoordFlags;
  uint tcgCount = 0;
  if (thermal) {
    const uint fullTcgCount = SBig(uvAnim.words[0]);
    tcgCount = fullTcgCount <= 2 ? fullTcgCount : 2;
    for (uint i = 0; i < tcgCount; ++i) {
      CGX::SetTexCoordGen_Compressed(static_cast< GXTexCoordID >(i), SBig(uvAnim.words[i + 1]));
    }
    uvAnim.words += fullTcgCount + 1;
  } else {
    tcgCount = SBig(uvAnim.words[0]);
    for (uint i = 0; i < tcgCount; ++i) {
      CGX::SetTexCoordGen_Compressed(static_cast< GXTexCoordID >(i), SBig(uvAnim.words[i + 1]));
    }
    uvAnim.words += tcgCount + 1;
  }

  const uint animCount = SBig(uvAnim.words[1]);
  uvAnim.words += 2;
  uint animIdx = 0;
  GXTexMtx texMtx = GX_TEXMTX0;
  GXPTTexMtx ptTexMtx = GX_PTTEXMTX0;
  for (; animIdx < animCount;) {
    const int size = HandleAnimatedUV(uvAnim.words, texMtx, ptTexMtx);
    if (size == 0) {
      break;
    }
    ++animIdx;
    texMtx = static_cast< GXTexMtx >(texMtx + 3);
    ptTexMtx = static_cast< GXPTTexMtx >(ptTexMtx + 3);
    uvAnim.words += size;
  }

  if (flags.GetTrans() != CModelFlags::kT_Opaque) {
    HandleTransparency(finalTevCount, finalKColorCount, flags, blendFactors, finalCCFlags,
                       finalACFlags);
  }

  if (reflection) {
    if (sReflectionAlpha > 0.f) {
      uint addedTevs = 0;
      if (indTex) {
        addedTevs = HandleReflection(usesTevReg2, static_cast< GXTexMapID >(indTexSlot & 7), 0,
                                     finalTevCount, texCount, tcgCount, finalKColorCount,
                                     finalCCFlags, finalACFlags);
        numIndStages = 1;
        tcgCount += 2;
      } else {
        addedTevs = HandleReflection(usesTevReg2, GX_TEXMAP_NULL, 0, finalTevCount, texCount,
                                     tcgCount, finalKColorCount, finalCCFlags, finalACFlags);
        tcgCount += 1;
      }
      texCount += 1;
      finalTevCount += addedTevs;
      finalKColorCount += 1;
    } else if ((finalCCFlags >> 9 & 3) != 0) {
      DoPassthru(finalTevCount);
      finalTevCount += 1;
    }
  }

  if (sbRenderModelShadow) {
    DoModelShadow(texCount, tcgCount);
    tcgCount += 1;
  }

  CGX::SetNumIndStages(numIndStages);
  CGX::SetNumTevStages(finalTevCount);
  CGX::SetNumTexGens(tcgCount);
  CGX::SetNumChans(finalNumColorChans);
}

void CCubeMaterial::EnsureViewDepStateCached(const CCubeSurface* surface) {
  static const Mtx texMtx1 = {
      {0.5f, 0.f, 0.f, 0.5f},
      {0.f, 0.f, 0.5f, 0.5f},
      {0.f, 0.f, 0.f, 1.f},
  };
  static const Mtx texMtx2 = {
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  };

  if ((surface == NULL && sLastModelCached == sRenderingModel) || sRenderingModel == NULL) {
    return;
  }

  const CTransform4f& modelMtx = CGraphics::GetModelMatrix();
  const CVector3f& playerPos =
      modelMtx.TransposeRotate(sPlayerPosition - modelMtx.GetTranslation());
  CVector3f points[2];
  points[1] = playerPos;
  sLastModelCached = sRenderingModel;
  points[0] = CVector3f::Zero();
  CVector3f& modelPoint = points[0];
  CVector3f& playerPoint = points[1];
  float radius = 0.f;

  if (surface != NULL) {
    sMaterialCachedState = 2;

    const CPlane plane(surface->GetCenter(), surface->GetNormalHint());
    modelPoint = playerPoint - plane.GetHeight(playerPoint) * plane.GetNormal();
  } else {
    sMaterialCachedState = 1;

    const CAABox& bounds = sRenderingModel->GetBoundingBox();
    modelPoint = bounds.GetCenterPoint();
    modelPoint.SetZ(playerPoint.GetZ());
    radius = 0.5f * (bounds.GetWidth() + bounds.GetHeight());
  }

  CCubeRenderer* renderer = CCubeRenderer::That();
  if (renderer->GetReflectionFlag()) {
    const CVector3f& oldDelta = sViewingFrom - sPlayerPosition;
    const CVector3f& newDelta = modelPoint - sPlayerPosition;
    const float oldMag = oldDelta.MagSquared();
    const float newMag = newDelta.MagSquared();
    if (newMag < oldMag) {
      sViewingFrom = modelPoint;
    }
  } else {
    sViewingFrom = modelPoint;
    renderer->SetReflectionFlag();
  }

  const CVector3f distVec = modelPoint - playerPoint;
  const float dist = distVec.Magnitude();
  const float reflDist = CMath::Max(gkEpsilon32, dist - 0.5f * radius);

  if (reflDist >= 5.f) {
    sReflectionAlpha = 0.f;
    return;
  }

  sReflectionAlpha = (5.f - reflDist) / 5.f;

  CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse() * CGraphics::GetModelMatrix();
  CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), GX_TEXMTX6, GX_MTX3x4);
  CGX::LoadTexMtxImm(texMtx1, GX_PTTEXMTX6, GX_MTX3x4);

  CVector3f right = CVector3f::Cross(distVec / reflDist, CVector3f(0.f, 0.f, 1.f));
  float xScale = 0.32258067f;
  float yScale = 0.32258067f;
  if (right.CanBeNormalized()) {
    right.Normalize();
  } else {
    right = CVector3f::Right();
  }

  const float scale = 0.02f * reflDist + 1.f;
  xScale *= scale * sThrobX;
  yScale *= scale * sThrobY;

  Mtx texMtx;
  memcpy(&texMtx, &texMtx2, sizeof(Mtx));
  texMtx[0][0] = xScale * right.GetX();
  texMtx[0][1] = xScale * right.GetY();
  texMtx[0][3] = -CVector3f::Dot(modelPoint, right) * xScale + 0.5f;
  texMtx[1][2] = yScale;
  texMtx[1][3] = -playerPoint.GetZ() * yScale;
  CGX::LoadTexMtxImm(texMtx, GX_TEXMTX7, GX_MTX3x4);
}

void CCubeMaterial::KillCachedViewDepState() { sLastModelCached = NULL; }

void CCubeMaterial::ResetCachedMaterials() {
  KillCachedViewDepState();
  sRenderingModel = NULL;
  sLastMaterialCached = NULL;
  sLastMaterialUnique = -1;
}

void CCubeModel::SetNewPlayerPositionAndTime(const CVector3f& pos, const CStopwatch& stopwatch) {
  sPlayerPosition = pos;
  CCubeMaterial::KillCachedViewDepState();

  s64 millis = stopwatch.GetCurrMicros() / 1000;

  float frequency = 1.5f;
  float timeWrapScale = 100000.f;
  float period = M_2PIF / frequency;
  const float time = static_cast< float >(static_cast< uint >(
                         millis % static_cast< uint >(timeWrapScale * period))) /
                     1000.f;
  sLastTime = time;

  float throbAmplitudeX = 0.05f;
  float throbAmplitudeY = 0.015f;
  float phaseX = 0.f;
  float phaseY = 1.f;
  sThrobX = 1. / (1. - throbAmplitudeX * sin(time * frequency + phaseX));
  sThrobY = 1. / (1. - throbAmplitudeY * sin(sLastTime * frequency + phaseY));
}

void CCubeModel::SetRenderModelBlack(bool v) {
  sbRenderModelBlack = v;
  lbl_805A9559 = false;
}

void CCubeModel::EnableShadowMaps(const CTexture* shadowTex, const CTransform4f& textureProjXf,
                                  unsigned char chan0DisableMask,
                                  unsigned char chan1EnableLightMask) {
  sbRenderModelShadow = true;
  spShadowTexture = const_cast< CTexture* >(shadowTex);
  sTextureProjectionTransform = textureProjXf;
  sChannel0DisableLightMask = chan0DisableMask;
  sChannel1EnableLightMask = chan1EnableLightMask;
}

void CCubeModel::DisableShadowMaps() { sbRenderModelShadow = false; }

uint CCubeMaterial::GetCompressedBlend() const {
  const uint* ptr = reinterpret_cast< const uint* >(mData);
  const uint flags = CBasics::SwapBytes(ptr[0]);
  const uint texCount = CBasics::SwapBytes(ptr[1]);
  const uint* blend = ptr + texCount + 4;
  if ((flags & kStateFlag_KonstValues) != 0) {
    blend += CBasics::SwapBytes(*blend) + 1;
  }
  return CBasics::SwapBytes(*blend);
}
