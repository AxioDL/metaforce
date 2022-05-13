#pragma once

#include "Graphics/GX.hpp"
#include "RetroTypes.hpp"

#include <aurora/model.hpp>
#include <zeus/CColor.hpp>

namespace metaforce::CGX {
enum class EChannelId {
  Channel0, // GX::COLOR0
  Channel1, // GX::COLOR1
};

struct STevState {
  u32 x0_colorInArgs = 0;
  u32 x4_alphaInArgs = 0;
  u32 x8_colorOps = 0;
  u32 xc_alphaOps = 0;
  u32 x10_indFlags = 0;
  u32 x14_tevOrderFlags = 0;
  GX::TevKColorSel x18_kColorSel = GX::TEV_KCSEL_1;
  GX::TevKAlphaSel x19_kAlphaSel = GX::TEV_KASEL_1;
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
  u8 x4c_dirtyChans = 0;
  u8 x4d_prevNumChans = 0;
  u8 x4e_numChans = 0;
  u8 x4f_numTexGens = 0;
  u8 x50_numTevStages = 0;
  u8 x51_numIndStages = 0;
  u8 x52_zmode = 0;
  GX::FogType x53_fogType = GX::FOG_NONE;
  u16 x54_lineWidthAndOffset = 0;
  u16 x56_blendMode = 0;
  std::array<GXColor, GX::MAX_KCOLOR> x58_kColors;
  std::array<STevState, GX::MAX_TEVSTAGE> x68_tevStates;
  std::array<STexState, GX::MAX_TEXCOORD> x228_texStates;
  u32 x248_alphaCompare = 0;
  float x24c_fogStartZ = 0.f;
  float x250_fogEndZ = 0.f;
  float x254_fogNearZ = 0.f;
  float x258_fogFarZ = 0.f;
  GXColor x25c_fogColor;
};
extern SGXState sGXState;
extern std::array<GX::VtxDescList, 12> sVtxDescList;

static inline void update_fog(u32 value) noexcept {
  if (sGXState.x53_fogType == GX::FOG_NONE || (sGXState.x56_blendMode & 0xE0) == (value & 0xE0)) {
    return;
  }
  if ((value & 0xE0) == 0x20) {
    GXSetFogColor(zeus::skClear);
    return;
  }
  GXSetFogColor(sGXState.x25c_fogColor);
}

static inline void FlushState() noexcept {
  if ((sGXState.x4c_dirtyChans & 1) != 0) {
    u8 numChans = sGXState.x4e_numChans;
    GXSetNumChans(numChans);
    sGXState.x4d_prevNumChans = numChans;
  }
  if ((sGXState.x4c_dirtyChans & 2) != 0) {
    // TODO actually COLOR0
    auto flags = sGXState.x34_chanCtrls[0];
    GXSetChanCtrl(GX::COLOR0A0, GXBool(flags & 1), GX::ColorSrc(flags >> 1 & 1), GX::ColorSrc(flags >> 2 & 1),
                  flags >> 3 & 0xFF, GX::DiffuseFn(flags >> 11 & 3), GX::AttnFn(flags >> 13 & 3));
    sGXState.x30_prevChanCtrls[0] = flags;
  }
  if ((sGXState.x4c_dirtyChans & 4) != 0) {
    // TODO actually COLOR1
    auto flags = sGXState.x34_chanCtrls[1];
    GXSetChanCtrl(GX::COLOR1A1, GXBool(flags & 1), GX::ColorSrc(flags >> 1 & 1), GX::ColorSrc(flags >> 2 & 1),
                  flags >> 3 & 0xFF, GX::DiffuseFn(flags >> 11 & 3), GX::AttnFn(flags >> 13 & 3));
    sGXState.x30_prevChanCtrls[1] = flags;
  }
  sGXState.x4c_dirtyChans = 0;
}

static inline void Begin(GX::Primitive primitive, GX::VtxFmt fmt, u16 nverts) noexcept {
  if (sGXState.x4c_dirtyChans != 0) {
    FlushState();
  }
  GXBegin(primitive, fmt, nverts);
}

static inline void End() noexcept { GXEnd(); }

static inline void CallDisplayList(const void* data, u32 nbytes) noexcept {
  if (sGXState.x4c_dirtyChans != 0) {
    FlushState();
  }
  GXCallDisplayList(data, nbytes);
}

static inline const GXColor& GetChanAmbColor(EChannelId id) noexcept {
  const auto idx = std::underlying_type_t<EChannelId>(id);
  return sGXState.x38_chanAmbColors[idx];
}

void ResetGXStates() noexcept;

static inline void SetAlphaCompare(GX::Compare comp0, u8 ref0, GX::AlphaOp op, GX::Compare comp1, u8 ref1) noexcept {
  u32 flags = ref1 << 17 | (comp1 & 7) << 14 | (op & 7) << 11 | ref0 << 3 | (comp0 & 7);
  if (flags != sGXState.x248_alphaCompare) {
    sGXState.x248_alphaCompare = flags;
    GXSetAlphaCompare(comp0, ref0, op, comp1, ref1);
    // GXSetZCompLoc(comp0 == GX::ALWAYS);
  }
}

template <typename T>
static inline void SetArray(GX::Attr attr, const std::vector<T>* data, bool isStatic) noexcept {
  if (data != nullptr && sGXState.x0_arrayPtrs[attr - GX::VA_POS] != data) {
    GXSetArray(attr, data, isStatic ? 1 : 0);
  }
}

static inline void SetBlendMode(GX::BlendMode mode, GX::BlendFactor srcFac, GX::BlendFactor dstFac,
                                GX::LogicOp op) noexcept {
  const u16 flags = (op & 0xF) << 8 | (dstFac & 7) << 5 | (srcFac & 7) << 2 | (mode & 3);
  if (flags != sGXState.x56_blendMode) {
    update_fog(flags);
    sGXState.x56_blendMode = flags;
    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

static inline void SetChanAmbColor(EChannelId id, const GXColor& color) noexcept {
  const auto idx = std::underlying_type_t<EChannelId>(id);
  if (color != sGXState.x38_chanAmbColors[idx]) {
    sGXState.x38_chanAmbColors[idx] = color;
    GXSetChanAmbColor(GX::ChannelID(idx + GX::COLOR0A0), color);
  }
}

static inline void SetChanCtrl(EChannelId id, GXBool enable, GX::ColorSrc ambSrc, GX::ColorSrc matSrc,
                               GX::LightMask lights, GX::DiffuseFn diffFn, GX::AttnFn attnFn) noexcept {
  const auto idx = std::underlying_type_t<EChannelId>(id);
  if (lights.none()) {
    enable = false;
  }
  const u32 flags = (attnFn & 3) << 13 | (diffFn & 3) << 11 | (lights.to_ulong() & 0xFF) << 3 | (matSrc & 1) << 2 |
                    (ambSrc & 1) << 1 | (u8(enable) & 1);
  sGXState.x34_chanCtrls[idx] = flags;
  sGXState.x4c_dirtyChans = 7; // TODO
}

// Flags with lights override
static inline void SetChanCtrl(EChannelId id, u32 flags, GX::LightMask lights) noexcept {
  const auto idx = std::underlying_type_t<EChannelId>(id);
  sGXState.x34_chanCtrls[idx] = lights.any() ? (flags | lights.to_ulong() << 3) : (flags & 0xFFFFFFFE);
  sGXState.x4c_dirtyChans = 7; // TODO
}

// Helper function for common logic
static inline void SetChanCtrl(EChannelId id, GX::LightMask lights) noexcept {
  const bool hasLights = lights.any();
  SetChanCtrl(id, hasLights, GX::SRC_REG, GX::SRC_REG, lights, hasLights ? GX::DF_CLAMP : GX::DF_NONE,
              hasLights ? GX::AF_SPOT : GX::AF_NONE);
}

static inline void SetChanMatColor(EChannelId id, const GXColor& color) noexcept {
  const auto idx = std::underlying_type_t<EChannelId>(id);
  if (color != sGXState.x40_chanMatColors[idx]) {
    sGXState.x40_chanMatColors[idx] = color;
    GXSetChanMatColor(GX::ChannelID(idx + GX::COLOR0A0), color);
  }
}

static inline void SetFog(GX::FogType type, float startZ, float endZ, float nearZ, float farZ,
                          const GXColor& color) noexcept {
  sGXState.x25c_fogColor = color;
  sGXState.x53_fogType = type;
  sGXState.x24c_fogStartZ = startZ;
  sGXState.x250_fogEndZ = endZ;
  sGXState.x254_fogNearZ = nearZ;
  sGXState.x258_fogFarZ = farZ;
  auto fogColor = color;
  if ((sGXState.x56_blendMode & 0xE0) == 0x20) {
    fogColor = zeus::skClear;
  }
  GXSetFog(type, startZ, endZ, nearZ, farZ, fogColor);
}

void SetIndTexMtxSTPointFive(GX::IndTexMtxID id, s8 scaleExp) noexcept;

void SetLineWidth(u8 width, GX::TexOffset offset) noexcept;

static inline void SetNumChans(u8 num) noexcept {
  sGXState.x4c_dirtyChans = 7; // TODO
  sGXState.x4e_numChans = num;
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

static inline void SetStandardTevColorAlphaOp(GX::TevStageID stageId) noexcept {
  auto& state = sGXState.x68_tevStates[stageId];
  if (state.x8_colorOps != 0x100 || state.xc_alphaOps != 0x100) {
    state.x8_colorOps = 0x100;
    state.xc_alphaOps = 0x100;
    GXSetTevColorOp(stageId, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
    GXSetTevAlphaOp(stageId, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
  }
}

static inline void SetTevAlphaIn(GX::TevStageID stageId, GX::TevAlphaArg a, GX::TevAlphaArg b, GX::TevAlphaArg c,
                                 GX::TevAlphaArg d) noexcept {
  u32 flags = (d & 31) << 15 | (c & 31) << 10 | (b & 31) << 5 | (a & 31);
  auto& state = sGXState.x68_tevStates[stageId].x4_alphaInArgs;
  if (flags != state) {
    state = flags;
    GXSetTevAlphaIn(stageId, a, b, c, d);
  }
}

static inline void SetTevAlphaOp(GX::TevStageID stageId, GX::TevOp op, GX::TevBias bias, GX::TevScale scale,
                                 GXBool clamp, GX::TevRegID outReg) noexcept {
  u32 flags = (outReg & 3) << 9 | (u8(clamp) & 1) << 8 | (scale & 3) << 6 | (bias & 3) << 4 | (op & 15);
  auto& state = sGXState.x68_tevStates[stageId].xc_alphaOps;
  if (flags != state) {
    state = flags;
    GXSetTevAlphaOp(stageId, op, bias, scale, clamp, outReg);
  }
}

static inline void SetTevAlphaOp_Compressed(GX::TevStageID stageId, u32 ops) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].xc_alphaOps;
  if (ops != state) {
    state = ops;
    GXSetTevAlphaOp(stageId, GX::TevOp(ops & 31), GX::TevBias(ops >> 4 & 3), GX::TevScale(ops >> 6 & 3),
                    GXBool(ops >> 8 & 1), GX::TevRegID(ops >> 9 & 3));
  }
}

static inline void SetTevColorIn(GX::TevStageID stageId, GX::TevColorArg a, GX::TevColorArg b, GX::TevColorArg c,
                                 GX::TevColorArg d) noexcept {
  u32 flags = (d & 31) << 15 | (c & 31) << 10 | (b & 31) << 5 | (a & 31);
  auto& state = sGXState.x68_tevStates[stageId].x0_colorInArgs;
  if (flags != state) {
    state = flags;
    GXSetTevColorIn(stageId, a, b, c, d);
  }
}

static inline void SetTevColorOp(GX::TevStageID stageId, GX::TevOp op, GX::TevBias bias, GX::TevScale scale,
                                 GXBool clamp, GX::TevRegID outReg) noexcept {
  u32 flags = (outReg & 3) << 9 | (u8(clamp) & 1) << 8 | (scale & 3) << 6 | (bias & 3) << 4 | (op & 15);
  auto& state = sGXState.x68_tevStates[stageId].x8_colorOps;
  if (flags != state) {
    state = flags;
    GXSetTevColorOp(stageId, op, bias, scale, clamp, outReg);
  }
}

static inline void SetTevColorOp_Compressed(GX::TevStageID stageId, u32 ops) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].x8_colorOps;
  if (ops != state) {
    state = ops;
    GXSetTevColorOp(stageId, GX::TevOp(ops & 31), GX::TevBias(ops >> 4 & 3), GX::TevScale(ops >> 6 & 3),
                    GXBool(ops >> 8 & 1), GX::TevRegID(ops >> 9 & 3));
  }
}

static inline void SetTevDirect(GX::TevStageID stageId) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].x10_indFlags;
  if (state != 0) {
    state = 0;
    GXSetTevDirect(stageId);
  }
}

static inline void SetStandardDirectTev_Compressed(GX::TevStageID stageId, u32 colorArgs, u32 alphaArgs, u32 colorOps,
                                                   u32 alphaOps) noexcept {
  auto& state = sGXState.x68_tevStates[stageId];
  SetTevDirect(stageId);
  if (state.x0_colorInArgs != colorArgs) {
    state.x0_colorInArgs = colorArgs;
    GXSetTevColorIn(stageId, GX::TevColorArg(colorArgs & 31), GX::TevColorArg(colorArgs >> 5 & 31),
                    GX::TevColorArg(colorArgs >> 10 & 31), GX::TevColorArg(colorArgs >> 15 & 31));
  }
  if (state.x4_alphaInArgs != alphaArgs) {
    state.x4_alphaInArgs = alphaArgs;
    GXSetTevAlphaIn(stageId, GX::TevAlphaArg(alphaArgs & 31), GX::TevAlphaArg(alphaArgs >> 5 & 31),
                    GX::TevAlphaArg(alphaArgs >> 10 & 31), GX::TevAlphaArg(alphaArgs >> 15 & 31));
  }
  if (colorOps != alphaOps || (colorOps & 0x1FF) != 0x100) {
    SetTevColorOp_Compressed(stageId, colorOps);
    SetTevAlphaOp_Compressed(stageId, alphaOps);
  } else if (colorOps != state.x8_colorOps || colorOps != state.xc_alphaOps) {
    state.x8_colorOps = colorOps;
    state.xc_alphaOps = colorOps;
    const auto outReg = GX::TevRegID(colorOps >> 9 & 3);
    GXSetTevColorOp(stageId, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, outReg);
    GXSetTevAlphaOp(stageId, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, outReg);
  }
}

static inline void SetTevIndirect(GX::TevStageID stageId, GX::IndTexStageID indStage, GX::IndTexFormat fmt,
                                  GX::IndTexBiasSel biasSel, GX::IndTexMtxID mtxSel, GX::IndTexWrap wrapS,
                                  GX::IndTexWrap wrapT, GXBool addPrev, GXBool indLod,
                                  GX::IndTexAlphaSel alphaSel) noexcept {
  // TODO
  GXSetTevIndirect(stageId, indStage, fmt, biasSel, mtxSel, wrapS, wrapT, addPrev, indLod, alphaSel);
}

static inline void SetTevIndWarp(GX::TevStageID stageId, GX::IndTexStageID indStage, GXBool signedOffset, GXBool replaceMode,
                   GX::IndTexMtxID mtxSel) noexcept {
  // TODO
  GXSetTevIndWarp(stageId, indStage, signedOffset, replaceMode, mtxSel);
}

static inline void SetTevKAlphaSel(GX::TevStageID stageId, GX::TevKAlphaSel sel) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].x19_kAlphaSel;
  if (sel != state) {
    state = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

static inline void SetTevKColor(GX::TevKColorID id, const GXColor& color) noexcept {
  auto& state = sGXState.x58_kColors[id];
  if (color != state) {
    state = color;
    GXSetTevKColor(id, color);
  }
}

static inline void SetTevKColorSel(GX::TevStageID stageId, GX::TevKColorSel sel) noexcept {
  auto& state = sGXState.x68_tevStates[stageId].x18_kColorSel;
  if (sel != state) {
    state = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

static inline void SetTevOrder(GX::TevStageID stageId, GX::TexCoordID texCoord, GX::TexMapID texMap,
                               GX::ChannelID color) noexcept {
  u32 flags = (color & 0xFF) << 16 | (texMap & 0xFF) << 8 | (texCoord & 0xFF);
  auto& state = sGXState.x68_tevStates[stageId].x14_tevOrderFlags;
  if (flags != state) {
    state = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

static inline void SetTexCoordGen(GX::TexCoordID dstCoord, GX::TexGenType fn, GX::TexGenSrc src, GX::TexMtx mtx,
                                  GXBool normalize, GX::PTTexMtx postMtx) noexcept {
  u32 flags = ((postMtx - GX::PTTEXMTX0) & 63) << 15 | (u8(normalize) & 1) << 14 | ((mtx - GX::TEXMTX0) & 31) << 9 |
              (src & 31) << 4 | (fn & 15);
  auto& state = sGXState.x228_texStates[dstCoord].x0_coordGen;
  if (flags != state) {
    state = flags;
    GXSetTexCoordGen2(dstCoord, fn, src, mtx, normalize, postMtx);
  }
}

static inline void SetTexCoordGen(GX::TexCoordID dstCoord, u32 flags) noexcept {
  auto& state = sGXState.x228_texStates[dstCoord].x0_coordGen;
  if (flags != state) {
    state = flags;
    GXSetTexCoordGen2(dstCoord, GX::TexGenType(flags & 15), GX::TexGenSrc(flags >> 4 & 31),
                      GX::TexMtx((flags >> 9 & 31) + GX::TEXMTX0), GXBool(flags >> 14 & 1),
                      GX::PTTexMtx((flags >> 15 & 63) + GX::PTTEXMTX0));
  }
}

static inline void SetVtxDescv_Compressed(u32 descList) noexcept {
  u32 currentDescList = sGXState.x48_descList;
  if (descList != currentDescList) {
    size_t remain = sVtxDescList.size() - 1;
    u32 shift = 0;
    u32 attrIdx = 0;
    do {
      sVtxDescList[attrIdx] = {
          GX::Attr(GX::VA_POS + attrIdx),
          GX::AttrType(descList >> shift & 3),
      };
      shift += 2;
      ++attrIdx;
      --remain;
    } while (remain != 0);
    sVtxDescList[attrIdx] = {};
    GXSetVtxDescv(sVtxDescList.data());
    sGXState.x48_descList = descList;
  }
}

static inline void SetVtxDescv(const GX::VtxDescList* descList) noexcept {
  u32 flags = 0;
  for (; descList->attr != GX::VA_NULL; ++descList) {
    flags |= (descList->type & 3) << (descList->attr - GX::VA_POS) * 2;
  }
  SetVtxDescv_Compressed(flags);
}

static inline void SetZMode(GXBool compareEnable, GX::Compare func, GXBool updateEnable) noexcept {
  u32 flags = (func & 0xFF) << 2 | (u8(updateEnable) << 1) | (u8(compareEnable) & 1);
  auto& state = sGXState.x52_zmode;
  if (flags != state) {
    state = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}
} // namespace metaforce::CGX
