#pragma once

#include "Graphics/GX.hpp"
#include "RetroTypes.hpp"

namespace metaforce {
enum class ERglTevStage : std::underlying_type_t<GXTevStageID> {
  Stage0 = GX_TEVSTAGE0,
  Stage1 = GX_TEVSTAGE1,
  Stage2 = GX_TEVSTAGE2,
  Stage3 = GX_TEVSTAGE3,
  Stage4 = GX_TEVSTAGE4,
  Stage5 = GX_TEVSTAGE5,
  Stage6 = GX_TEVSTAGE6,
  Stage7 = GX_TEVSTAGE7,
  Stage8 = GX_TEVSTAGE8,
  Stage9 = GX_TEVSTAGE9,
  Stage10 = GX_TEVSTAGE10,
  Stage11 = GX_TEVSTAGE11,
  Stage12 = GX_TEVSTAGE12,
  Stage13 = GX_TEVSTAGE13,
  Stage14 = GX_TEVSTAGE14,
  Stage15 = GX_TEVSTAGE15,
  Max = GX_MAX_TEVSTAGE,
};

namespace CTevCombiners {
struct CTevOp {
  bool x0_clamp = true;
  GXTevOp x4_op = GX_TEV_ADD;
  GXTevBias x8_bias = GX_TB_ZERO;
  GXTevScale xc_scale = GX_CS_SCALE_1;
  GXTevRegID x10_regId = GX_TEVPREV;

  constexpr CTevOp() = default;
  constexpr CTevOp(bool clamp, GXTevOp op, GXTevBias bias, GXTevScale scale, GXTevRegID regId)
  : x0_clamp(clamp), x4_op(op), x8_bias(bias), xc_scale(scale), x10_regId(regId) {}
  constexpr CTevOp(u32 compressedDesc)
  : x0_clamp((compressedDesc >> 8 & 1) != 0)
  , x4_op(static_cast<GXTevOp>(compressedDesc & 0xF))
  , x8_bias(static_cast<GXTevBias>(compressedDesc >> 4 & 3))
  , xc_scale(static_cast<GXTevScale>(compressedDesc >> 6 & 3))
  , x10_regId(static_cast<GXTevRegID>(compressedDesc >> 9 & 3)) {}

  bool operator==(const CTevOp& rhs) const {
    return x0_clamp == rhs.x0_clamp && x4_op == rhs.x4_op && x8_bias == rhs.x8_bias && xc_scale == rhs.xc_scale;
  }
};
struct ColorPass {
  GXTevColorArg x0_a;
  GXTevColorArg x4_b;
  GXTevColorArg x8_c;
  GXTevColorArg xc_d;

  constexpr ColorPass(GXTevColorArg a, GXTevColorArg b, GXTevColorArg c, GXTevColorArg d)
  : x0_a(a), x4_b(b), x8_c(c), xc_d(d) {}
  constexpr ColorPass(u32 compressedDesc)
  : x0_a(static_cast<GXTevColorArg>(compressedDesc & 0x1F))
  , x4_b(static_cast<GXTevColorArg>(compressedDesc >> 5 & 0x1F))
  , x8_c(static_cast<GXTevColorArg>(compressedDesc >> 10 & 0x1F))
  , xc_d(static_cast<GXTevColorArg>(compressedDesc >> 15 & 0x1F)) {}

  bool operator==(const ColorPass& rhs) const { return memcmp(this, &rhs, sizeof(*this)) == 0; }
};
struct AlphaPass {
  GXTevAlphaArg x0_a;
  GXTevAlphaArg x4_b;
  GXTevAlphaArg x8_c;
  GXTevAlphaArg xc_d;

  constexpr AlphaPass(GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c, GXTevAlphaArg d)
  : x0_a(a), x4_b(b), x8_c(c), xc_d(d) {}
  constexpr AlphaPass(u32 compressedDesc)
  : x0_a(static_cast<GXTevAlphaArg>(compressedDesc & 0x1F))
  , x4_b(static_cast<GXTevAlphaArg>(compressedDesc >> 5 & 0x1F))
  , x8_c(static_cast<GXTevAlphaArg>(compressedDesc >> 10 & 0x1F))
  , xc_d(static_cast<GXTevAlphaArg>(compressedDesc >> 15 & 0x1F)) {}

  bool operator==(const AlphaPass& rhs) const { return memcmp(this, &rhs, sizeof(*this)) == 0; }
};
class CTevPass {
  u32 x0_id;
  ColorPass x4_colorPass;
  AlphaPass x14_alphaPass;
  CTevOp x24_colorOp;
  CTevOp x38_alphaOp;

  static u32 sNextUniquePass;

public:
  CTevPass(const ColorPass& colPass, const AlphaPass& alphaPass, const CTevOp& colorOp = {}, const CTevOp& alphaOp = {})
  : x0_id(++sNextUniquePass)
  , x4_colorPass(colPass)
  , x14_alphaPass(alphaPass)
  , x24_colorOp(colorOp)
  , x38_alphaOp(alphaOp) {}

  void Execute(ERglTevStage stage) const;

  bool operator==(const CTevPass& rhs) const {
    return x0_id == rhs.x0_id && x4_colorPass == rhs.x4_colorPass && x14_alphaPass == rhs.x14_alphaPass &&
           x24_colorOp == rhs.x24_colorOp && x38_alphaOp == rhs.x38_alphaOp;
  }
};

extern const CTevPass kEnvPassthru;
// TODO move below to CGraphics
extern const CTevPass kEnvBlendCTandCConCF;
extern const CTevPass kEnvModulateConstColor;
extern const CTevPass kEnvConstColor;
extern const CTevPass kEnvModulate;
extern const CTevPass kEnvDecal;
extern const CTevPass kEnvBlend;
extern const CTevPass kEnvReplace;
extern const CTevPass kEnvModulateAlpha;
extern const CTevPass kEnvModulateColor;
extern const CTevPass kEnvModulateColorByAlpha;

void Init();
void SetupPass(ERglTevStage stage, const CTevPass& pass);
void DeletePass(ERglTevStage stage);
bool SetPassCombiners(ERglTevStage stage, const CTevPass& pass);
void RecomputePasses();
void ResetStates();
} // namespace CTevCombiners
} // namespace metaforce
