#ifndef _CGX_IMPL
#define _CGX_IMPL

#include "CGX.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CTexture.hpp"

#include "dolphin/gx.h"

// TODO: figure out why MSVC doesn't like this
#if !defined(TARGET_PC) || defined(CGX_CPP)

#ifndef CGX_INLINE
#define CGX_INLINE inline
#endif

CGX_INLINE void CGX::SetNumChans(uchar num) {
  sGXState.mNumChans = num;
  sGXState.mFlags.numDirty = num != sGXState.mPrevNumChans;
}

CGX_INLINE void CGX::SetNumTexGens(uchar num) {
  if (num != sGXState.mNumTexGens) {
    sGXState.mNumTexGens = num;
    GXSetNumTexGens(num);
  }
}

CGX_INLINE void CGX::SetChanAmbColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, sGXState.mChanAmbColors[channel])) {
    CopyGXColor(sGXState.mChanAmbColors[channel], color);
    GXSetChanAmbColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE GXColor CGX::GetChanAmbColor(EChannelId channel) {
  return sGXState.mChanAmbColors[channel];
}

CGX_INLINE void CGX::SetChanMatColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, sGXState.mChanMatColors[channel])) {
    CopyGXColor(sGXState.mChanMatColors[channel], color);
    GXSetChanMatColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE void CGX::SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc,
                                 GXColorSrc matSrc, GXLightID lights, GXDiffuseFn diffFn,
                                 GXAttnFn attnFn) {
  ushort& state = sGXState.mChanCtrls[channel];
  ushort prevFlags = sGXState.mPrevChanCtrls[channel];
  if (lights == GX_LIGHT_NULL) {
    enable = GX_FALSE;
  }
  uint flags = MaskAndShiftLeft(enable, 1, 0) | MaskAndShiftLeft(ambSrc, 1, 1) |
               MaskAndShiftLeft(matSrc, 1, 2) | MaskAndShiftLeft(lights, 0xFF, 3) |
               MaskAndShiftLeft(diffFn, 3, 11) | MaskAndShiftLeft(attnFn, 3, 13);
  state = flags;
  sGXState.mChanFlags =
      ((flags != prevFlags) << (channel + 1)) | (sGXState.mChanFlags & ~(1 << (channel + 1)));
}

CGX_INLINE void CGX::SetChanCtrl_Compressed(EChannelId channel, GXLightID lights, uint ctrl) {
  ushort& state = sGXState.mChanCtrls[channel];
  ushort prevFlags = sGXState.mPrevChanCtrls[channel];
  uint flags = ctrl & ~1;
  if (lights != 0) {
    flags = ctrl | MaskAndShiftLeft(lights, 0xFF, 3);
  }
  state = flags;
  sGXState.mChanFlags =
      ((flags != prevFlags) << (channel + 1)) | (sGXState.mChanFlags & ~(1 << (channel + 1)));
}

CGX_INLINE void CGX::SetNumTevStages(uchar num) {
  if (sGXState.mNumTevStages != num) {
    sGXState.mNumTevStages = num;
    GXSetNumTevStages(num);
  }
}

CGX_INLINE void CGX::SetTevKColor(GXTevKColorID id, const GXColor& color) {
  if (!CompareGXColors(sGXState.mKColors[id], color)) {
    CopyGXColor(sGXState.mKColors[id], color);
    GXSetTevKColor(id, color);
  }
}

CGX_INLINE void CGX::SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b,
                                   GXTevColorArg c, GXTevColorArg d) {
  const uint flags = (a & 0x1F) | ((b & 0x1F) << 5) | ((c & 0x1F) << 10) | ((d & 0x1F) << 15);
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mColorInArgs) {
    state.mColorInArgs = flags;
    GXSetTevColorIn(stageId, a, b, c, d);
  }
}

CGX_INLINE void CGX::SetTevColorIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mColorInArgs) {
    state.mColorInArgs = flags;
    GXSetTevColorIn(stageId, static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 0)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 5)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 10)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 15)));
  }
}

CGX_INLINE void CGX::SetTevAlphaIn(GXTevStageID stageId, GXTevAlphaArg a, GXTevAlphaArg b,
                                   GXTevAlphaArg c, GXTevAlphaArg d) {
  uint flags = MaskAndShiftLeft(a, 0x1F, 0) | MaskAndShiftLeft(b, 0x1F, 5) |
               MaskAndShiftLeft(c, 0x1F, 10) | MaskAndShiftLeft(d, 0x1F, 15);
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, a, b, c, d);
  }
}

CGX_INLINE void CGX::SetTevAlphaIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 0)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 5)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 10)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 15)));
  }
}

CGX_INLINE void CGX::SetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias,
                                   GXTevScale scale, GXBool clamp, GXTevRegID outReg) {
  const uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
                     MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
                     MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, op, bias, scale, clamp, outReg);
  }
}

CGX_INLINE void CGX::SetTevColorOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}

CGX_INLINE void CGX::SetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias,
                                   GXTevScale scale, GXBool clamp, GXTevRegID outReg) {
  uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
               MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
               MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, op, bias, scale, clamp, outReg);
  }
}

CGX_INLINE void CGX::SetTevAlphaOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}

CGX_INLINE void CGX::SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel) {
  STevState& state = sGXState.mTevStates[stageId];
  if (sel != state.mKColorSel) {
    state.mKColorSel = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel) {
  STevState& state = sGXState.mTevStates[stageId];
  if (sel != state.mKAlphaSel) {
    state.mKAlphaSel = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                                 GXChannelID color) {
  STevState& state = sGXState.mTevStates[stageId];
  uint flags = (texCoord & 0xFF) | ((texMap & 0xFF) << 8) | ((color & 0xFF) << 16);
  if (state.mTevOrderFlags != flags) {
    state.mTevOrderFlags = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

CGX_INLINE void CGX::SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac,
                                  GXLogicOp op) {
  uint flags = MaskAndShiftLeft(mode, 3, 0) | MaskAndShiftLeft(srcFac, 7, 2) |
               MaskAndShiftLeft(dstFac, 7, 5) | MaskAndShiftLeft(op, 0xF, 8);
  if (flags != sGXState.mBlendMode) {
    update_fog(flags);
    sGXState.mBlendMode = flags;
    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

CGX_INLINE void CGX::SetZMode(const GXBool compareEnable, GXCompare func,
                              const GXBool updateEnable) {
  uchar flags = MaskAndShiftLeft(compareEnable, 0xFF, 0) | MaskAndShiftLeft(updateEnable, 0xFF, 1) |
                MaskAndShiftLeft(func, 0xFF, 2);
  if (flags != sGXState.mZmode) {
    sGXState.mZmode = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}

CGX_INLINE void CGX::SetAlphaCompare(GXCompare comp0, uchar ref0, GXAlphaOp op, GXCompare comp1,
                                     uchar ref1) {
  uint flags = MaskAndShiftLeft(comp0, 7, 0) | MaskAndShiftLeft(ref0, 0xFF, 3) |
               MaskAndShiftLeft(op, 7, 11) | MaskAndShiftLeft(comp1, 7, 14) |
               MaskAndShiftLeft(ref1, 0xFF, 17);
  if (sGXState.mAlphaCompare != flags) {
    sGXState.mAlphaCompare = flags;
    GXSetAlphaCompare(comp0, ref0, op, comp1, ref1);
    GXSetZCompLoc(comp0 == GX_ALWAYS);
  }
}

CGX_INLINE void CGX::SetTevIndirect(const GXTevStageID stageId, const GXIndTexStageID indStage,
                                    const GXIndTexFormat fmt, const GXIndTexBiasSel biasSel,
                                    const GXIndTexMtxID mtxSel, const GXIndTexWrap wrapS,
                                    const GXIndTexWrap wrapT, GXBool addPrev, const GXBool indLod,
                                    const GXIndTexAlphaSel alphaSel) {
  STevState& state = sGXState.mTevStates[stageId];
  uint flags = MaskAndShiftLeft(indStage, 3, 0) | MaskAndShiftLeft(fmt, 3, 2) |
               MaskAndShiftLeft(biasSel, 7, 4) | MaskAndShiftLeft(mtxSel, 15, 7) |
               MaskAndShiftLeft(wrapS, 7, 11) | MaskAndShiftLeft(wrapT, 7, 14) |
               MaskAndShiftLeft(addPrev, 1, 17) | MaskAndShiftLeft(indLod, 1, 18) |
               MaskAndShiftLeft(alphaSel, 3, 19);
  if (state.mIndFlags != flags) {
    state.mIndFlags = flags;
    GXSetTevIndirect(stageId, indStage, fmt, biasSel, mtxSel, wrapS, wrapT, addPrev, indLod,
                     alphaSel);
  }
}

CGX_INLINE void CGX::SetTevDirect(GXTevStageID stageId) {
  STevState& state = sGXState.mTevStates[stageId];
  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
}

CGX_INLINE void CGX::SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src,
                                    GXTexMtx mtx, GXBool normalize, GXPTTexMtx postMtx) {
  STexState& state = sGXState.mTexStates[dstCoord];
  uint vm = (mtx - GX_TEXMTX0) / 3;
  uint vp = postMtx - GX_PTTEXMTX0;
#if NONMATCHING
  // Similarly to GXTexMtx, this should also be divided by 3
  vp /= 3;
#endif
  uint flags = (fn & 0xF) | ((src & 0x1F) << 4) | ((vm & 0x1F) << 9) | ((normalize & 1) << 14) |
               ((vp & 0x3F) << 15);
  if (state.mCoordGen != flags) {
    state.mCoordGen = flags;
    GXSetTexCoordGen2(dstCoord, fn, src, mtx, normalize, postMtx);
  }
}

CGX_INLINE void CGX::SetTexCoordGen_Compressed(GXTexCoordID dstCoord, uint flags) {
  STexState& state = sGXState.mTexStates[dstCoord];
  if (state.mCoordGen != flags) {
    GXTexMtx vm = static_cast< GXTexMtx >((flags >> 9 & 0x1F) + GX_TEXMTX0);
    GXPTTexMtx vp = static_cast< GXPTTexMtx >((flags >> 15 & 0x3F) + GX_PTTEXMTX0);
    state.mCoordGen = flags;
    GXSetTexCoordGen2(dstCoord, static_cast< GXTexGenType >(flags & 0xF),
                      static_cast< GXTexGenSrc >(flags >> 4 & 0x1F), vm,
                      static_cast< GXBool >(flags >> 14 & 1), vp);
  }
}

CGX_INLINE void CGX::SetNumIndStages(uchar num) {
  if (sGXState.mNumIndStages != num) {
    sGXState.mNumIndStages = num;
    GXSetNumIndStages(num);
  }
}

#if defined(TARGET_PC)
CGX_INLINE void CGX::SetArray(GXAttr attr, const void* data, uchar stride, size_t size, bool le) {
  if (data == nullptr) {
    return;
  }
  sGXState.mArrayPtrs[attr - GX_VA_POS] = data;
  GXSetArray(attr, data, size, stride, le);
}
#else
CGX_INLINE void CGX::SetArray(GXAttr attr, const void* data, uchar stride) {
  uint idx = attr - GX_VA_POS;
  if (data == nullptr || sGXState.mArrayPtrs[idx] == data) {
    return;
  }
  sGXState.mArrayPtrs[idx] = data;
  GXSetArray(attr, data, stride);
}
#endif

CGX_INLINE void CGX::CallDisplayList(const void* ptr, size_t size) {
  if (sGXState.mChanFlags != 0) {
    FlushState();
  }
  GXCallDisplayList(ptr, size);
}

CGX_INLINE void CGX::Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx) {
  if (sGXState.mChanFlags != 0) {
    FlushState();
  }
  GXBegin(prim, fmt, numVtx);
}

CGX_INLINE void CGX::End() { GXEnd(); }

CGX_INLINE void CGX::apply_fog() {
  static const GXColor black = {0, 0, 0, 0};
  GXSetFog(static_cast< GXFogType >(sGXState.mFogType), sGXState.mFogParams.mFogStartZ,
           sGXState.mFogParams.mFogEndZ, sGXState.mFogParams.mFogNearZ,
           sGXState.mFogParams.mFogFarZ,
           (sGXState.mBlendMode & (7 << 5)) == (GX_BL_ONE << 5)
               ? black
               : sGXState.mFogParams.mFogColor);
}

CGX_INLINE void CGX::SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                            const GXColor& color) {
  sGXState.mFogType = type;
  sGXState.mFogParams.mFogStartZ = startZ;
  sGXState.mFogParams.mFogEndZ = endZ;
  sGXState.mFogParams.mFogNearZ = nearZ;
  sGXState.mFogParams.mFogFarZ = farZ;
  CopyGXColor(sGXState.mFogParams.mFogColor, color);
  apply_fog();
}

CGX_INLINE void CGX::SetLineWidth(uchar width, GXTexOffset offset) {
  ushort flags = width | offset << 8;
  if (flags != sGXState.mLineWidthAndOffset) {
    sGXState.mLineWidthAndOffset = flags;
    GXSetLineWidth(width, offset);
  }
}

#endif // !defined(TARGET_PC) || defined(CGX_CPP)

#endif // _CGX_IMPL
