#ifndef _CGX_IMPL
#define _CGX_IMPL

#include "CGX.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CTexture.hpp"

#include "dolphin/gx.h"

#ifndef CGX_INLINE
#define CGX_INLINE inline
#endif

CGX_INLINE void CGX::SetNumChans(uchar num) {
  sGXState.x4e_numChans = num;
  sGXState.x4c_flags.numDirty = num != sGXState.x4d_prevNumChans;
}

CGX_INLINE void CGX::SetNumTexGens(uchar num) {
  if (num != sGXState.x4f_numTexGens) {
    sGXState.x4f_numTexGens = num;
    GXSetNumTexGens(num);
  }
}

CGX_INLINE void CGX::SetChanAmbColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, sGXState.x38_chanAmbColors[channel])) {
    CopyGXColor(sGXState.x38_chanAmbColors[channel], color);
    GXSetChanAmbColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE GXColor CGX::GetChanAmbColor(EChannelId channel) {
  return sGXState.x38_chanAmbColors[channel];
}

CGX_INLINE void CGX::SetChanMatColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, sGXState.x40_chanMatColors[channel])) {
    CopyGXColor(sGXState.x40_chanMatColors[channel], color);
    GXSetChanMatColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE void CGX::SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc,
                                 GXColorSrc matSrc, GXLightID lights, GXDiffuseFn diffFn,
                                 GXAttnFn attnFn) {
  ushort& state = sGXState.x34_chanCtrls[channel];
  ushort prevFlags = sGXState.x30_prevChanCtrls[channel];
  if (lights == GX_LIGHT_NULL) {
    enable = GX_FALSE;
  }
  uint flags = MaskAndShiftLeft(enable, 1, 0) | MaskAndShiftLeft(ambSrc, 1, 1) |
               MaskAndShiftLeft(matSrc, 1, 2) | MaskAndShiftLeft(lights, 0xFF, 3) |
               MaskAndShiftLeft(diffFn, 3, 11) | MaskAndShiftLeft(attnFn, 3, 13);
  state = flags;
  sGXState.x4c_chanFlags =
      ((flags != prevFlags) << (channel + 1)) | (sGXState.x4c_chanFlags & ~(1 << (channel + 1)));
}

CGX_INLINE void CGX::SetChanCtrl_Compressed(EChannelId channel, GXLightID lights, uint ctrl) {
  ushort& state = sGXState.x34_chanCtrls[channel];
  ushort prevFlags = sGXState.x30_prevChanCtrls[channel];
  uint flags = ctrl & ~1;
  if (lights != 0) {
    flags = ctrl | MaskAndShiftLeft(lights, 0xFF, 3);
  }
  state = flags;
  sGXState.x4c_chanFlags =
      ((flags != prevFlags) << (channel + 1)) | (sGXState.x4c_chanFlags & ~(1 << (channel + 1)));
}

CGX_INLINE void CGX::SetNumTevStages(uchar num) {
  if (sGXState.x50_numTevStages != num) {
    sGXState.x50_numTevStages = num;
    GXSetNumTevStages(num);
  }
}

CGX_INLINE void CGX::SetTevKColor(GXTevKColorID id, const GXColor& color) {
  if (!CompareGXColors(sGXState.x58_kColors[id], color)) {
    CopyGXColor(sGXState.x58_kColors[id], color);
    GXSetTevKColor(id, color);
  }
}

CGX_INLINE void CGX::SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b,
                                   GXTevColorArg c, GXTevColorArg d) {
  const uint flags = (a & 0x1F) | ((b & 0x1F) << 5) | ((c & 0x1F) << 10) | ((d & 0x1F) << 15);
  STevState& state = sGXState.x68_tevStates[stageId];
  if (flags != state.x0_colorInArgs) {
    state.x0_colorInArgs = flags;
    GXSetTevColorIn(stageId, a, b, c, d);
  }
}

CGX_INLINE void CGX::SetTevColorIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.x68_tevStates[stageId];
  if (flags != state.x0_colorInArgs) {
    state.x0_colorInArgs = flags;
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
  STevState& state = sGXState.x68_tevStates[stageId];
  if (flags != state.x4_alphaInArgs) {
    state.x4_alphaInArgs = flags;
    GXSetTevAlphaIn(stageId, a, b, c, d);
  }
}

CGX_INLINE void CGX::SetTevAlphaIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.x68_tevStates[stageId];
  if (flags != state.x4_alphaInArgs) {
    state.x4_alphaInArgs = flags;
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
  STevState& state = sGXState.x68_tevStates[stageId];
  if (flags != state.x8_colorOps) {
    state.x8_colorOps = flags;
    GXSetTevColorOp(stageId, op, bias, scale, clamp, outReg);
  }
}

CGX_INLINE void CGX::SetTevColorOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.x68_tevStates[stageId];
  if (flags != state.x8_colorOps) {
    state.x8_colorOps = flags;
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
  STevState& state = sGXState.x68_tevStates[stageId];
  if (flags != state.xc_alphaOps) {
    state.xc_alphaOps = flags;
    GXSetTevAlphaOp(stageId, op, bias, scale, clamp, outReg);
  }
}

CGX_INLINE void CGX::SetTevAlphaOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.x68_tevStates[stageId];
  if (flags != state.xc_alphaOps) {
    state.xc_alphaOps = flags;
    GXSetTevAlphaOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}

CGX_INLINE void CGX::SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel) {
  STevState& state = sGXState.x68_tevStates[stageId];
  if (sel != state.x18_kColorSel) {
    state.x18_kColorSel = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel) {
  STevState& state = sGXState.x68_tevStates[stageId];
  if (sel != state.x19_kAlphaSel) {
    state.x19_kAlphaSel = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                                 GXChannelID color) {
  STevState& state = sGXState.x68_tevStates[stageId];
  uint flags = (texCoord & 0xFF) | ((texMap & 0xFF) << 8) | ((color & 0xFF) << 16);
  if (state.x14_tevOrderFlags != flags) {
    state.x14_tevOrderFlags = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

CGX_INLINE void CGX::SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac,
                                  GXLogicOp op) {
  uint flags = MaskAndShiftLeft(mode, 3, 0) | MaskAndShiftLeft(srcFac, 7, 2) |
               MaskAndShiftLeft(dstFac, 7, 5) | MaskAndShiftLeft(op, 0xF, 8);
  if (flags != sGXState.x56_blendMode) {
    update_fog(flags);
    sGXState.x56_blendMode = flags;
    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

CGX_INLINE void CGX::SetZMode(const GXBool compareEnable, GXCompare func,
                              const GXBool updateEnable) {
  uchar flags = MaskAndShiftLeft(compareEnable, 0xFF, 0) | MaskAndShiftLeft(updateEnable, 0xFF, 1) |
                MaskAndShiftLeft(func, 0xFF, 2);
  if (flags != sGXState.x52_zmode) {
    sGXState.x52_zmode = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}

CGX_INLINE void CGX::SetAlphaCompare(GXCompare comp0, uchar ref0, GXAlphaOp op, GXCompare comp1,
                                     uchar ref1) {
  uint flags = MaskAndShiftLeft(comp0, 7, 0) | MaskAndShiftLeft(ref0, 0xFF, 3) |
               MaskAndShiftLeft(op, 7, 11) | MaskAndShiftLeft(comp1, 7, 14) |
               MaskAndShiftLeft(ref1, 0xFF, 17);
  if (sGXState.x248_alphaCompare != flags) {
    sGXState.x248_alphaCompare = flags;
    GXSetAlphaCompare(comp0, ref0, op, comp1, ref1);
    GXSetZCompLoc(comp0 == GX_ALWAYS);
  }
}

CGX_INLINE void CGX::SetTevIndirect(const GXTevStageID stageId, const GXIndTexStageID indStage,
                                    const GXIndTexFormat fmt, const GXIndTexBiasSel biasSel,
                                    const GXIndTexMtxID mtxSel, const GXIndTexWrap wrapS,
                                    const GXIndTexWrap wrapT, GXBool addPrev, const GXBool indLod,
                                    const GXIndTexAlphaSel alphaSel) {
  STevState& state = sGXState.x68_tevStates[stageId];
  uint flags = MaskAndShiftLeft(indStage, 3, 0) | MaskAndShiftLeft(fmt, 3, 2) |
               MaskAndShiftLeft(biasSel, 7, 4) | MaskAndShiftLeft(mtxSel, 15, 7) |
               MaskAndShiftLeft(wrapS, 7, 11) | MaskAndShiftLeft(wrapT, 7, 14) |
               MaskAndShiftLeft(addPrev, 1, 17) | MaskAndShiftLeft(indLod, 1, 18) |
               MaskAndShiftLeft(alphaSel, 3, 19);
  if (state.x10_indFlags != flags) {
    state.x10_indFlags = flags;
    GXSetTevIndirect(stageId, indStage, fmt, biasSel, mtxSel, wrapS, wrapT, addPrev, indLod,
                     alphaSel);
  }
}

CGX_INLINE void CGX::SetTevDirect(GXTevStageID stageId) {
  STevState& state = sGXState.x68_tevStates[stageId];
  if (state.x10_indFlags != 0) {
    state.x10_indFlags = 0;
    GXSetTevDirect(stageId);
  }
}

CGX_INLINE void CGX::SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src,
                                    GXTexMtx mtx, GXBool normalize, GXPTTexMtx postMtx) {
  STexState& state = sGXState.x228_texStates[dstCoord];
  uint vm = (mtx - GX_TEXMTX0) / 3;
  uint vp = postMtx - GX_PTTEXMTX0;
#if NONMATCHING
  // Similarly to GXTexMtx, this should also be divided by 3
  vp /= 3;
#endif
  uint flags = (fn & 0xF) | ((src & 0x1F) << 4) | ((vm & 0x1F) << 9) | ((normalize & 1) << 14) |
               ((vp & 0x3F) << 15);
  if (state.x0_coordGen != flags) {
    state.x0_coordGen = flags;
    GXSetTexCoordGen2(dstCoord, fn, src, mtx, normalize, postMtx);
  }
}

CGX_INLINE void CGX::SetTexCoordGen_Compressed(GXTexCoordID dstCoord, uint flags) {
  STexState& state = sGXState.x228_texStates[dstCoord];
  if (state.x0_coordGen != flags) {
    GXTexMtx vm = static_cast< GXTexMtx >((flags >> 9 & 0x1F) + GX_TEXMTX0);
    GXPTTexMtx vp = static_cast< GXPTTexMtx >((flags >> 15 & 0x3F) + GX_PTTEXMTX0);
    state.x0_coordGen = flags;
    GXSetTexCoordGen2(dstCoord, static_cast< GXTexGenType >(flags & 0xF),
                      static_cast< GXTexGenSrc >(flags >> 4 & 0x1F), vm,
                      static_cast< GXBool >(flags >> 14 & 1), vp);
  }
}

CGX_INLINE void CGX::SetNumIndStages(uchar num) {
  if (sGXState.x51_numIndStages != num) {
    sGXState.x51_numIndStages = num;
    GXSetNumIndStages(num);
  }
}

CGX_INLINE void CGX::SetArray(GXAttr attr, const void* data, uchar stride) {
  uint idx = attr - GX_VA_POS;
  if (data == nullptr || sGXState.x0_arrayPtrs[idx] == data) {
    return;
  }
  sGXState.x0_arrayPtrs[idx] = data;
  GXSetArray(attr, data, stride);
}

CGX_INLINE void CGX::CallDisplayList(const void* ptr, size_t size) {
  if (sGXState.x4c_chanFlags != 0) {
    FlushState();
  }
  GXCallDisplayList(ptr, size);
}

CGX_INLINE void CGX::Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx) {
  if (sGXState.x4c_chanFlags != 0) {
    FlushState();
  }
  GXBegin(prim, fmt, numVtx);
}

CGX_INLINE void CGX::End() { GXEnd(); }

CGX_INLINE void CGX::apply_fog() {
  static const GXColor black = {0, 0, 0, 0};
  GXSetFog(static_cast< GXFogType >(sGXState.x53_fogType), sGXState.x24c_fogParams.x0_fogStartZ,
           sGXState.x24c_fogParams.x4_fogEndZ, sGXState.x24c_fogParams.x8_fogNearZ,
           sGXState.x24c_fogParams.xc_fogFarZ,
           (sGXState.x56_blendMode & (7 << 5)) == (GX_BL_ONE << 5)
               ? black
               : sGXState.x24c_fogParams.x10_fogColor);
}

CGX_INLINE void CGX::SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                            const GXColor& color) {
  sGXState.x53_fogType = type;
  sGXState.x24c_fogParams.x0_fogStartZ = startZ;
  sGXState.x24c_fogParams.x4_fogEndZ = endZ;
  sGXState.x24c_fogParams.x8_fogNearZ = nearZ;
  sGXState.x24c_fogParams.xc_fogFarZ = farZ;
  CopyGXColor(sGXState.x24c_fogParams.x10_fogColor, color);
  apply_fog();
}

CGX_INLINE void CGX::SetLineWidth(uchar width, GXTexOffset offset) {
  ushort flags = width | offset << 8;
  if (flags != sGXState.x54_lineWidthAndOffset) {
    sGXState.x54_lineWidthAndOffset = flags;
    GXSetLineWidth(width, offset);
  }
}

#endif // _CGX_IMPL
