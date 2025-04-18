#pragma once

#include "Graphics/GX.hpp"
#include "RetroTypes.hpp"

#include <span>
#include <zeus/CColor.hpp>

namespace metaforce::CGX {
enum class EChannelId {
  Channel0, // GX_COLOR0
  Channel1, // GX_COLOR1
};

struct STevState {
  u32 x0_colorInArgs = 0;
  u32 x4_alphaInArgs = 0;
  u32 x8_colorOps = 0;
  u32 xc_alphaOps = 0;
  u32 x10_indFlags = 0;
  u32 x14_tevOrderFlags = 0;
  GXTevKColorSel x18_kColorSel = GX_TEV_KCSEL_1;
  GXTevKAlphaSel x19_kAlphaSel = GX_TEV_KASEL_1;
};
struct STexState {
  u32 x0_coordGen = 0;
};
struct SGXState {
  std::array<void*, 12> x0_arrayPtrs{};
  std::array<u16, 2> x30_prevChanCtrls{};
  std::array<u16, 2> x34_chanCtrls{0x4000, 0x4000};
  std::array<GXColor, 2> x38_chanAmbColors;
  std::array<GXColor, 2> x40_chanMatColors;
  u32 x48_descList = 0;
  union {
    u8 x4c_chanFlags = 0;
    // Ordering swapped for LE
    struct {
      u8 numDirty : 1;
      u8 chansDirty : 2;
      u8 unused : 5;
    } x4c_flags;
  };
  u8 x4d_prevNumChans = 0;
  u8 x4e_numChans = 0;
  u8 x4f_numTexGens = 0;
  u8 x50_numTevStages = 0;
  u8 x51_numIndStages = 0;
  u8 x52_zmode = 0;
  GXFogType x53_fogType = GX_FOG_NONE;
  u16 x54_lineWidthAndOffset = 0;
  u16 x56_blendMode = 0;
  std::array<GXColor, GX_MAX_KCOLOR> x58_kColors;
  std::array<STevState, GX_MAX_TEVSTAGE> x68_tevStates;
  std::array<STexState, GX_MAX_TEXCOORD> x228_texStates;
  u32 x248_alphaCompare = 0;
  float x24c_fogStartZ = 0.f;
  float x250_fogEndZ = 0.f;
  float x254_fogNearZ = 0.f;
  float x258_fogFarZ = 0.f;
  GXColor x25c_fogColor;
};
extern SGXState sGXState;
extern std::array<GXVtxDescList, 12> sVtxDescList;

static inline u32 MaskAndShiftLeft(u32 v, u32 m, u32 s) { return (v & m) << s; }
static inline u32 ShiftRightAndMask(u32 v, u32 m, u32 s) { return (v >> s) & m; }

static inline void update_fog(u32 value) noexcept {
  if (sGXState.x53_fogType == GX_FOG_NONE || (sGXState.x56_blendMode & 0xE0) == (value & 0xE0)) {
    return;
  }
  if ((value & 0xE0) == 0x20) {
    GXSetFogColor(GX_CLEAR);
    return;
  }
  GXSetFogColor(sGXState.x25c_fogColor);
}

static inline void FlushState() noexcept {
  if (sGXState.x4c_chanFlags & 1) {
    GXSetNumChans(sGXState.x4e_numChans);
    sGXState.x4d_prevNumChans = sGXState.x4e_numChans;
  }
  if (sGXState.x4c_chanFlags & 2) {
    u16 flags = sGXState.x34_chanCtrls[0];
    GXBool enable = ShiftRightAndMask(flags, 1, 0);
    GXColorSrc ambSrc = static_cast<GXColorSrc>(ShiftRightAndMask(flags, 1, 1));
    GXColorSrc matSrc = static_cast<GXColorSrc>(ShiftRightAndMask(flags, 1, 2));
    u32 lightMask = ShiftRightAndMask(flags, 0xFF, 3);
    GXDiffuseFn diffFn = static_cast<GXDiffuseFn>(ShiftRightAndMask(flags, 3, 11));
    GXAttnFn attnFn = static_cast<GXAttnFn>(ShiftRightAndMask(flags, 3, 13));
    GXSetChanCtrl(GX_COLOR0, enable, ambSrc, matSrc, lightMask, diffFn, attnFn);
    sGXState.x30_prevChanCtrls[0] = sGXState.x34_chanCtrls[0];
  }
  if (sGXState.x4c_chanFlags & 4) {
    u16 flags = sGXState.x34_chanCtrls[1];
    GXBool enable = ShiftRightAndMask(flags, 1, 0);
    GXColorSrc ambSrc = static_cast<GXColorSrc>(ShiftRightAndMask(flags, 1, 1));
    GXColorSrc matSrc = static_cast<GXColorSrc>(ShiftRightAndMask(flags, 1, 2));
    u32 lightMask = ShiftRightAndMask(flags, 0xFF, 3);
    GXDiffuseFn diffFn = static_cast<GXDiffuseFn>(ShiftRightAndMask(flags, 3, 11));
    GXAttnFn attnFn = static_cast<GXAttnFn>(ShiftRightAndMask(flags, 3, 13));
    GXSetChanCtrl(GX_COLOR1, enable, ambSrc, matSrc, lightMask, diffFn, attnFn);
    sGXState.x30_prevChanCtrls[1] = sGXState.x34_chanCtrls[1];
  }
  sGXState.x4c_chanFlags = 0;
}

static inline void Begin(GXPrimitive primitive, GXVtxFmt fmt, u16 nverts) noexcept {
  if (sGXState.x4c_chanFlags != 0) {
    FlushState();
  }
  GXBegin(primitive, fmt, nverts);
}

static inline void End() noexcept { GXEnd(); }

static inline void CallDisplayList(const void* data, u32 nbytes) noexcept {
  if (sGXState.x4c_chanFlags != 0) {
    FlushState();
  }
  GXCallDisplayList(data, nbytes);
}

static inline const GXColor& GetChanAmbColor(EChannelId id) noexcept {
  const auto idx = std::underlying_type_t<EChannelId>(id);
  return sGXState.x38_chanAmbColors[idx];
}

void ResetGXStates() noexcept;

static inline void SetAlphaCompare(GXCompare comp0, u8 ref0, GXAlphaOp op, GXCompare comp1, u8 ref1) noexcept {
  u32 flags = ref1 << 17 | (comp1 & 7) << 14 | (op & 7) << 11 | ref0 << 3 | (comp0 & 7);
  if (flags != sGXState.x248_alphaCompare) {
    sGXState.x248_alphaCompare = flags;
    GXSetAlphaCompare(comp0, ref0, op, comp1, ref1);
    GXSetZCompLoc(comp0 == GX_ALWAYS);
  }
}

static inline void SetArray(GXAttr attr, std::span<const u8> data, u8 stride) noexcept {
  const auto* ptr = static_cast<const void*>(data.data());
  if (ptr != nullptr && sGXState.x0_arrayPtrs[attr - GX_VA_POS] != ptr) {
    // sGXState.x0_arrayPtrs[attr - GX_VA_POS] = const_cast<void*>(ptr);
    GXSetArray(attr, data.data(), data.size(), stride);
  }
}

template <typename T>
static inline void SetArray(GXAttr attr, std::span<const T> data) noexcept {
  const auto* ptr = static_cast<const void*>(data.data());
  if (ptr != nullptr && sGXState.x0_arrayPtrs[attr - GX_VA_POS] != ptr) {
    // sGXState.x0_arrayPtrs[attr - GX_VA_POS] = const_cast<void*>(ptr);
    GXSetArray(attr, data.data(), data.size_bytes(), sizeof(T));
  }
}

template <typename T, size_t N>
static inline void SetArray(GXAttr attr, const std::array<T, N>& data) noexcept {
  const auto* ptr = static_cast<const void*>(data.data());
  if (ptr != nullptr && sGXState.x0_arrayPtrs[attr - GX_VA_POS] != ptr) {
    // sGXState.x0_arrayPtrs[attr - GX_VA_POS] = const_cast<void*>(ptr);
    GXSetArray(attr, ptr, data.size() * sizeof(T), sizeof(T));
  }
}

static inline void SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac, GXLogicOp op) noexcept {
  const u16 flags = (op & 0xF) << 8 | (dstFac & 7) << 5 | (srcFac & 7) << 2 | (mode & 3);
  if (flags != sGXState.x56_blendMode) {
    update_fog(flags);
    sGXState.x56_blendMode = flags;
    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

static inline void SetChanAmbColor(EChannelId id, GXColor color) noexcept {
  const auto idx = std::underlying_type_t<EChannelId>(id);
  if (color != sGXState.x38_chanAmbColors[idx]) {
    sGXState.x38_chanAmbColors[idx] = color;
    GXSetChanAmbColor(GXChannelID(idx + GX_COLOR0A0), color);
  }
}
static inline void SetChanAmbColor(EChannelId id, const zeus::CColor& color) noexcept {
  SetChanAmbColor(id, to_gx_color(color));
}

static inline void SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc, GXColorSrc matSrc,
                               GX::LightMask lights, GXDiffuseFn diffFn, GXAttnFn attnFn) noexcept {
  const auto idx = static_cast<std::underlying_type_t<EChannelId>>(channel);
  u16& state = sGXState.x34_chanCtrls[idx];
  u16 prevFlags = sGXState.x30_prevChanCtrls[idx];
  if (lights.none()) {
    enable = GX_FALSE;
  }
  u32 flags = MaskAndShiftLeft(enable, 1, 0) | MaskAndShiftLeft(ambSrc, 1, 1) | MaskAndShiftLeft(matSrc, 1, 2) |
              MaskAndShiftLeft(lights.to_ulong(), 0xFF, 3) | MaskAndShiftLeft(diffFn, 3, 11) |
              MaskAndShiftLeft(attnFn, 3, 13);
  state = flags;
  sGXState.x4c_chanFlags = ((flags != prevFlags) << (idx + 1)) | (sGXState.x4c_chanFlags & ~(1 << (idx + 1)));
}

static inline void SetChanCtrl_Compressed(EChannelId channel, GX::LightMask lights, u32 ctrl) {
  const auto idx = static_cast<std::underlying_type_t<EChannelId>>(channel);
  u16& state = sGXState.x34_chanCtrls[idx];
  u16 prevFlags = sGXState.x30_prevChanCtrls[idx];
  u32 flags = ctrl & ~1;
  if (lights.any()) {
    flags = ctrl | (lights.to_ulong() & 0xFF) << 3;
  }
  state = flags;
  sGXState.x4c_chanFlags = ((flags != prevFlags) << (idx + 1)) | (sGXState.x4c_chanFlags & ~(1 << (idx + 1)));
}

static inline void SetChanMatColor(EChannelId id, GXColor color) noexcept {
  const auto idx = std::underlying_type_t<EChannelId>(id);
  if (color != sGXState.x40_chanMatColors[idx]) {
    sGXState.x40_chanMatColors[idx] = color;
    GXSetChanMatColor(GXChannelID(idx + GX_COLOR0A0), color);
  }
}
static inline void SetChanMatColor(EChannelId id, const zeus::CColor& color) noexcept {
  SetChanMatColor(id, to_gx_color(color));
}

static inline void SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                          const GXColor& color) noexcept {
  sGXState.x25c_fogColor = color;
  sGXState.x53_fogType = type;
  sGXState.x24c_fogStartZ = startZ;
  sGXState.x250_fogEndZ = endZ;
  sGXState.x254_fogNearZ = nearZ;
  sGXState.x258_fogFarZ = farZ;
  auto fogColor = color;
  if ((sGXState.x56_blendMode & 0xE0) == 0x20) {
    fogColor = GX_CLEAR;
  }
  GXSetFog(type, startZ, endZ, nearZ, farZ, fogColor);
}

void SetIndTexMtxSTPointFive(GXIndTexMtxID id, s8 scaleExp) noexcept;

void SetLineWidth(u8 width, GXTexOffset offset) noexcept;

static inline void SetNumChans(u8 num) noexcept {
  sGXState.x4e_numChans = num;
  sGXState.x4c_flags.numDirty = sGXState.x4e_numChans != sGXState.x4d_prevNumChans;
}

static inline void SetNumIndStages(u8 num) noexcept {
  auto& state = sGXState.x51_numIndStages;
  if (num != state) {
    state = num;
    GXSetNumIndStages(num);
  }
}

static inline void SetNumTevStages(u8 num) noexcept {
  auto& state = sGXState.x50_numTevStages;
  if (num != state) {
    state = num;
    GXSetNumTevStages(num);
  }
}

static inline void SetNumTexGens(u8 num) noexcept {
  auto& state = sGXState.x4f_numTexGens;
  if (num != state) {
    state = num;
    GXSetNumTexGens(num);
  }
}

static inline void SetStandardTevColorAlphaOp(GXTevStageID stageId) noexcept {
  auto& state = sGXState.x68_tevStates[stageId];
  if (state.x8_colorOps != 0x100 || state.xc_alphaOps != 0x100) {
    state.x8_colorOps = 0x100;
    state.xc_alphaOps = 0x100;
    GXSetTevColorOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    GXSetTevAlphaOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  }
}

static inline void SetTevAlphaIn(GXTevStageID stageId, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c,
                                 GXTevAlphaArg d) noexcept {
  u32 flags = (d & 31) << 15 | (c & 31) << 10 | (b & 31) << 5 | (a & 31);
  auto& state = sGXState.x68_tevStates[stageId].x4_alphaInArgs;
  if (flags != state) {
    state = flags;
    GXSetTevAlphaIn(stageId, a, b, c, d);
  }
}

static inline void SetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp,
                                 GXTevRegID outReg) noexcept {
  u32 flags = (outReg & 3) << 9 | (u8(clamp) & 1) << 8 | (scale & 3) << 6 | (bias & 3) << 4 | (op & 15);
  auto& state = sGXState.x68_tevStates[stageId].xc_alphaOps;
  if (flags != state) {
    state = flags;
    GXSetTevAlphaOp(stageId, op, bias, scale, clamp, outReg);
  }
}

static inline void SetTevAlphaOp_Compressed(GXTevStageID stageId, u32 ops) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].xc_alphaOps;
  if (ops != state) {
    state = ops;
    GXSetTevAlphaOp(stageId, GXTevOp(ops & 31), GXTevBias(ops >> 4 & 3), GXTevScale(ops >> 6 & 3), GXBool(ops >> 8 & 1),
                    GXTevRegID(ops >> 9 & 3));
  }
}

static inline void SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c,
                                 GXTevColorArg d) noexcept {
  u32 flags = (d & 31) << 15 | (c & 31) << 10 | (b & 31) << 5 | (a & 31);
  auto& state = sGXState.x68_tevStates[stageId].x0_colorInArgs;
  if (flags != state) {
    state = flags;
    GXSetTevColorIn(stageId, a, b, c, d);
  }
}

static inline void SetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp,
                                 GXTevRegID outReg) noexcept {
  u32 flags = (outReg & 3) << 9 | (u8(clamp) & 1) << 8 | (scale & 3) << 6 | (bias & 3) << 4 | (op & 15);
  auto& state = sGXState.x68_tevStates[stageId].x8_colorOps;
  if (flags != state) {
    state = flags;
    GXSetTevColorOp(stageId, op, bias, scale, clamp, outReg);
  }
}

static inline void SetTevColorOp_Compressed(GXTevStageID stageId, u32 ops) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].x8_colorOps;
  if (ops != state) {
    state = ops;
    GXSetTevColorOp(stageId, GXTevOp(ops & 31), GXTevBias(ops >> 4 & 3), GXTevScale(ops >> 6 & 3), GXBool(ops >> 8 & 1),
                    GXTevRegID(ops >> 9 & 3));
  }
}

static inline void SetTevDirect(GXTevStageID stageId) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].x10_indFlags;
  if (state != 0) {
    state = 0;
    GXSetTevDirect(stageId);
  }
}

static inline void SetStandardDirectTev_Compressed(GXTevStageID stageId, u32 colorArgs, u32 alphaArgs, u32 colorOps,
                                                   u32 alphaOps) noexcept {
  auto& state = sGXState.x68_tevStates[stageId];
  SetTevDirect(stageId);
  if (state.x0_colorInArgs != colorArgs) {
    state.x0_colorInArgs = colorArgs;
    GXSetTevColorIn(stageId, GXTevColorArg(colorArgs & 31), GXTevColorArg(colorArgs >> 5 & 31),
                    GXTevColorArg(colorArgs >> 10 & 31), GXTevColorArg(colorArgs >> 15 & 31));
  }
  if (state.x4_alphaInArgs != alphaArgs) {
    state.x4_alphaInArgs = alphaArgs;
    GXSetTevAlphaIn(stageId, GXTevAlphaArg(alphaArgs & 31), GXTevAlphaArg(alphaArgs >> 5 & 31),
                    GXTevAlphaArg(alphaArgs >> 10 & 31), GXTevAlphaArg(alphaArgs >> 15 & 31));
  }
  if (colorOps != alphaOps || (colorOps & 0x1FF) != 0x100) {
    SetTevColorOp_Compressed(stageId, colorOps);
    SetTevAlphaOp_Compressed(stageId, alphaOps);
  } else if (colorOps != state.x8_colorOps || colorOps != state.xc_alphaOps) {
    state.x8_colorOps = colorOps;
    state.xc_alphaOps = colorOps;
    const auto outReg = GXTevRegID(colorOps >> 9 & 3);
    GXSetTevColorOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, outReg);
    GXSetTevAlphaOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, outReg);
  }
}

static inline void SetTevIndirect(GXTevStageID stageId, GXIndTexStageID indStage, GXIndTexFormat fmt,
                                  GXIndTexBiasSel biasSel, GXIndTexMtxID mtxSel, GXIndTexWrap wrapS, GXIndTexWrap wrapT,
                                  GXBool addPrev, GXBool indLod, GXIndTexAlphaSel alphaSel) noexcept {
  // TODO
  GXSetTevIndirect(stageId, indStage, fmt, biasSel, mtxSel, wrapS, wrapT, addPrev, indLod, alphaSel);
}

static inline void SetTevIndWarp(GXTevStageID stageId, GXIndTexStageID indStage, GXBool signedOffset,
                                 GXBool replaceMode, GXIndTexMtxID mtxSel) noexcept {
  // TODO
  GXSetTevIndWarp(stageId, indStage, signedOffset, replaceMode, mtxSel);
}

static inline void SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].x19_kAlphaSel;
  if (sel != state) {
    state = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

static inline void SetTevKColor(GXTevKColorID id, const GXColor& color) noexcept {
  auto& state = sGXState.x58_kColors[id];
  if (color != state) {
    state = color;
    GXSetTevKColor(id, color);
  }
}
static inline void SetTevKColor(GXTevKColorID id, const zeus::CColor& color) noexcept {
  SetTevKColor(id, to_gx_color(color));
}

static inline void SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].x18_kColorSel;
  if (sel != state) {
    state = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

static inline void SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                               GXChannelID color) noexcept {
  u32 flags = (color & 0xFF) << 16 | (texMap & 0xFF) << 8 | (texCoord & 0xFF);
  auto& state = sGXState.x68_tevStates[stageId].x14_tevOrderFlags;
  if (flags != state) {
    state = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

static inline void SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src, GXTexMtx mtx,
                                  GXBool normalize, GXPTTexMtx postMtx) noexcept {
  u32 flags = ((postMtx - GX_PTTEXMTX0) & 63) << 15 | (u8(normalize) & 1) << 14 | ((mtx - GX_TEXMTX0) & 31) << 9 |
              (src & 31) << 4 | (fn & 15);
  auto& state = sGXState.x228_texStates[dstCoord].x0_coordGen;
  if (flags != state) {
    state = flags;
    GXSetTexCoordGen2(dstCoord, fn, src, mtx, normalize, postMtx);
  }
}

static inline void SetTexCoordGen(GXTexCoordID dstCoord, u32 flags) noexcept {
  auto& state = sGXState.x228_texStates[dstCoord].x0_coordGen;
  if (flags != state) {
    state = flags;
    GXSetTexCoordGen2(dstCoord, GXTexGenType(flags & 15), GXTexGenSrc(flags >> 4 & 31),
                      GXTexMtx((flags >> 9 & 31) + GX_TEXMTX0), GXBool(flags >> 14 & 1),
                      GXPTTexMtx((flags >> 15 & 63) + GX_PTTEXMTX0));
  }
}

static inline void SetVtxDescv_Compressed(u32 flags) noexcept {
  if (flags == sGXState.x48_descList) {
    return;
  }
  GXVtxDescList* list = sVtxDescList.data();
  for (u32 idx = 0; idx < sVtxDescList.size() - 1; ++idx) {
    u32 shift = idx * 2;
    if ((flags & 3 << shift) == (sGXState.x48_descList & 3 << shift)) {
      continue;
    }
    list->attr = static_cast<GXAttr>(GX_VA_POS + idx);
    list->type = static_cast<GXAttrType>(flags >> shift & 3);
    ++list;
  }
  list->attr = GX_VA_NULL;
  list->type = GX_NONE;
  GXSetVtxDescv(sVtxDescList.data());
  sGXState.x48_descList = flags;
}

static inline void SetVtxDescv(const GXVtxDescList* descList) noexcept {
  u32 flags = 0;
  for (; descList->attr != GX_VA_NULL; ++descList) {
    flags |= (descList->type & 3) << (descList->attr - GX_VA_POS) * 2;
  }
  SetVtxDescv_Compressed(flags);
}

static inline void SetZMode(GXBool compareEnable, GXCompare func, GXBool updateEnable) noexcept {
  u32 flags = (func & 0xFF) << 2 | (u8(updateEnable) << 1) | (u8(compareEnable) & 1);
  auto& state = sGXState.x52_zmode;
  if (flags != state) {
    state = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}

static inline void GetFog(GXFogType* fogType, float* fogStartZ, float* fogEndZ, float* fogNearZ, float* fogFarZ,
                          GXColor* fogColor) {
  if (fogType != nullptr) {
    *fogType = sGXState.x53_fogType;
  }
  if (fogStartZ != nullptr) {
    *fogStartZ = sGXState.x24c_fogStartZ;
  }
  if (fogEndZ != nullptr) {
    *fogEndZ = sGXState.x250_fogEndZ;
  }
  if (fogNearZ != nullptr) {
    *fogNearZ = sGXState.x254_fogNearZ;
  }
  if (fogFarZ != nullptr) {
    *fogFarZ = sGXState.x258_fogFarZ;
  }
  if (fogColor != nullptr) {
    *fogColor = sGXState.x25c_fogColor;
  }
}
} // namespace metaforce::CGX
