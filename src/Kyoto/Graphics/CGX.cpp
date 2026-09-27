#include "Kyoto/Graphics/CGX.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CTexture.hpp"

#include "dolphin/gx/GXTexture.h"

#include <limits.h>

CGX::SGXState CGX::sGXState;

#if NONMATCHING
// Doesn't need to be so big
static GXVtxDescList sVtxDescList[12];
#else
static GXVtxDescList sVtxDescList[GX_MAX_VTXDESCLIST_SZ];
#endif

#define CGX_INLINE
#define CGX_CPP
#include "Kyoto/Graphics/CGX_Impl.hpp" // IWYU pragma: keep

CGX::SGXState::SGXState()
: mDescList(0)
, mPrevNumChans(2)
, mNumChans(0)
, mNumTexGens(UCHAR_MAX)
, mNumTevStages(UCHAR_MAX)
, mNumIndStages(UCHAR_MAX)
, mZmode(UCHAR_MAX)
, mFogType(0)
, mLineWidthAndOffset(USHRT_MAX)
, mBlendMode(USHRT_MAX)
, mAlphaCompare(UINT_MAX) {
  const GXColor sGXClear = {0, 0, 0, 0};
  const GXColor sGXWhite = {255, 255, 255, 255};
  for (int i = 0; i < 2; ++i) {
    mPrevChanCtrls[i] = USHRT_MAX;
    mChanCtrls[i] = USHRT_MAX;
    mChanAmbColors[i] = sGXClear;
    mChanMatColors[i] = sGXWhite;
  }
  for (int i = 0; i < 4; ++i) {
    mKColors[i] = sGXClear;
  }
  SetChanCtrl(Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  SetChanCtrl(Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
}

void CGX::ResetGXStates() {
  sGXState.mDescList = 0;
  GXClearVtxDesc();
  for (int i = 0; i < 12; i++) {
    sGXState.mArrayPtrs[i] = reinterpret_cast< const void* >(1);
  }
  for (int i = 0; i < 8; i++) {
    CTexture::InvalidateTexmap(static_cast< GXTexMapID >(i));
  }
  for (int i = 0; i < 4; i++) {
    GXSetTevKColor(static_cast< GXTevKColorID >(i), sGXState.mKColors[i]);
  }
  GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_RED);
  GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_GREEN);
  GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_BLUE);
  SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetCurrentMtx(GX_PNMTX0);
  SetNumIndStages(0);
  for (int i = 0; i < 4; i++) {
    GXSetIndTexCoordScale(static_cast< GXIndTexStageID >(i), GX_ITS_1, GX_ITS_1);
  }
  for (int i = 0; i < 16; i++) {
    SetTevDirect(static_cast< GXTevStageID >(i));
  }
  for (int i = 0; i < 8; i++) {
    GXSetTexCoordScaleManually(static_cast< GXTexCoordID >(i), false, 0, 0);
  }
  GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
}

void CGX::ResetGXStatesFull() {
  new (&sGXState) SGXState();
  ResetGXStates();
}

static inline void __SetChanCtrlCompressed(GXChannelID channel, uint flags) {
  GXBool enable = CGX::ShiftRightAndMask(flags, 1, 0);
  GXColorSrc ambSrc = static_cast< GXColorSrc >(CGX::ShiftRightAndMask(flags, 1, 1));
  GXColorSrc matSrc = static_cast< GXColorSrc >(CGX::ShiftRightAndMask(flags, 1, 2));
  GXLightID lightMask = static_cast< GXLightID >(CGX::ShiftRightAndMask(flags, 0xFF, 3));
  GXDiffuseFn diffFn = static_cast< GXDiffuseFn >(CGX::ShiftRightAndMask(flags, 3, 11));
  GXAttnFn attnFn = static_cast< GXAttnFn >(CGX::ShiftRightAndMask(flags, 3, 13));

  GXSetChanCtrl(channel, enable, ambSrc, matSrc, lightMask, diffFn, attnFn);
}

void CGX::FlushState() {
  if (sGXState.mChanFlags & 1) {
    GXSetNumChans(sGXState.mNumChans);
    sGXState.mPrevNumChans = sGXState.mNumChans;
  }
  if (sGXState.mChanFlags & 2) {
    __SetChanCtrlCompressed(GX_COLOR0, sGXState.mChanCtrls[0]);
    sGXState.mPrevChanCtrls[0] = sGXState.mChanCtrls[0];
  }
  if (sGXState.mChanFlags & 4) {
    __SetChanCtrlCompressed(GX_COLOR1, sGXState.mChanCtrls[1]);
    sGXState.mPrevChanCtrls[1] = sGXState.mChanCtrls[1];
  }
  sGXState.mChanFlags = 0;
}

void CGX::SetIndTexMtxSTPointFive(GXIndTexMtxID id, s8 scaleExp) {
  static const float indMtx[2][3] = {
      {0.5f, 0.f, 0.f},
      {0.f, 0.5f, 0.f},
  };
  GXSetIndTexMtx(id, const_cast< float (*)[3] >(indMtx), scaleExp);
}

void CGX::SetVtxDescv_Compressed(uint flags) {
  if (flags == sGXState.mDescList) {
    return;
  }
  GXVtxDescList* list = sVtxDescList;
  for (uint idx = 0; idx < 11; ++idx) {
    uint shift = idx * 2;
    if ((flags & 3 << shift) == (sGXState.mDescList & 3 << shift)) {
      continue;
    }
    list->attr = static_cast< GXAttr >(GX_VA_POS + idx);
    list->type = static_cast< GXAttrType >(flags >> shift & 3);
    ++list;
  }
  list->attr = GX_VA_NULL;
  list->type = GX_NONE;
  GXSetVtxDescv(sVtxDescList);
  sGXState.mDescList = flags;
}

void CGX::SetVtxDesc(GXAttr attr, GXAttrType type) {
  uint lshift = (attr - GX_VA_POS) * 2;
  uint rshift = 3 << lshift;
  uint flags = type << lshift;
  if (flags != (sGXState.mDescList & rshift)) {
    sGXState.mDescList = flags | (sGXState.mDescList & ~rshift);
    GXSetVtxDesc(attr, type);
  }
}

void CGX::ResetVtxDescv() {
  static const GXVtxDescList vtxDescList[2] = {
      {GX_VA_POS, GX_INDEX16},
      {GX_VA_NULL, GX_NONE},
  };
  SetVtxDescv(vtxDescList);
}

void CGX::SetVtxDescv(const GXVtxDescList* list) {
  uint flags = 0;
  for (; list->attr != GX_VA_NULL; ++list) {
    flags |= (list->type & 3) << (list->attr - GX_VA_POS) * 2;
  }
  SetVtxDescv_Compressed(flags);
}

void CGX::SetStandardDirectTev_Compressed(GXTevStageID stageId, uint colorArgs, uint alphaArgs,
                                          uint colorOps, uint alphaOps) {
  STevState& state = sGXState.mTevStates[stageId];
  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
  if (state.mColorInArgs != colorArgs) {
    state.mColorInArgs = colorArgs;
    GXSetTevColorIn(stageId, static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 0)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 5)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 10)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 15)));
  }
  if (state.mAlphaInArgs != alphaArgs) {
    state.mAlphaInArgs = alphaArgs;
    GXSetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 0)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 5)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 10)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 15)));
  }
  if (colorOps != alphaOps || (colorOps & 0x1FF) != 0x100) {
    SetTevColorOp_Compressed(stageId, colorOps);
    SetTevAlphaOp_Compressed(stageId, alphaOps);
  } else if (colorOps != state.mColorOps || colorOps != state.mAlphaOps) {
    // Fast path for GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true
    state.mAlphaOps = colorOps;
    state.mColorOps = colorOps;
    GXTevRegID outReg = static_cast< GXTevRegID >(ShiftRightAndMask(colorOps, 3, 9));
    GXSetTevColorOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, outReg);
    GXSetTevAlphaOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, outReg);
  }
}

void CGX::SetStandardTevColorAlphaOp(GXTevStageID stageId) {
  STevState& state = sGXState.mTevStates[stageId];
  if (state.mColorOps != 0x100 || state.mAlphaOps != 0x100) {
    state.mAlphaOps = 0x100;
    state.mColorOps = 0x100;
    GXSetTevColorOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    GXSetTevAlphaOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  }
}

void CGX::GetFog(GXFogType* fogType, float* fogStartZ, float* fogEndZ, float* fogNearZ,
                 float* fogFarZ, GXColor* fogColor) {
  if (fogType != nullptr) {
    *fogType = static_cast< GXFogType >(sGXState.mFogType);
  }
  if (fogStartZ != nullptr) {
    *fogStartZ = sGXState.mFogParams.mFogStartZ;
  }
  if (fogEndZ != nullptr) {
    *fogEndZ = sGXState.mFogParams.mFogEndZ;
  }
  if (fogNearZ != nullptr) {
    *fogNearZ = sGXState.mFogParams.mFogNearZ;
  }
  if (fogFarZ != nullptr) {
    *fogFarZ = sGXState.mFogParams.mFogFarZ;
  }
  if (fogColor != nullptr) {
    CopyGXColor(*fogColor, sGXState.mFogParams.mFogColor);
  }
}
