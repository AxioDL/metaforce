#pragma once

#include "Graphics/GX.hpp"
#include "RetroTypes.hpp"

#include <compare>

namespace metaforce {
enum class ERglTevStage : std::underlying_type_t<GX::TevStageID> {
  Stage0 = GX::TEVSTAGE0,
  Stage1 = GX::TEVSTAGE1,
  Stage2 = GX::TEVSTAGE2,
  Stage3 = GX::TEVSTAGE3,
  Stage4 = GX::TEVSTAGE4,
  Stage5 = GX::TEVSTAGE5,
  Stage6 = GX::TEVSTAGE6,
  Stage7 = GX::TEVSTAGE7,
  Stage8 = GX::TEVSTAGE8,
  Stage9 = GX::TEVSTAGE9,
  Stage10 = GX::TEVSTAGE10,
  Stage11 = GX::TEVSTAGE11,
  Stage12 = GX::TEVSTAGE12,
  Stage13 = GX::TEVSTAGE13,
  Stage14 = GX::TEVSTAGE14,
  Stage15 = GX::TEVSTAGE15,
  Max = GX::MAX_TEVSTAGE,
  None = GX::NULL_STAGE,
};

namespace CTevCombiners {
struct CTevOp {
  bool x0_clamp = true;
  GX::TevOp x4_op = GX::TevOp::TEV_ADD;
  GX::TevBias x8_bias = GX::TevBias::TB_ZERO;
  GX::TevScale xc_scale = GX::TevScale::CS_SCALE_1;
  GX::TevRegID x10_regId = GX::TevRegID::TEVPREV;

  constexpr CTevOp() = default;
  constexpr CTevOp(bool clamp, GX::TevOp op, GX::TevBias bias, GX::TevScale scale, GX::TevRegID regId)
  : x0_clamp(clamp), x4_op(op), x8_bias(bias), xc_scale(scale), x10_regId(regId) {}
  constexpr CTevOp(u32 compressedDesc)
  : x0_clamp((compressedDesc >> 8 & 1) != 0)
  , x4_op(static_cast<GX::TevOp>(compressedDesc & 0xF))
  , x8_bias(static_cast<GX::TevBias>(compressedDesc >> 4 & 3))
  , xc_scale(static_cast<GX::TevScale>(compressedDesc >> 6 & 3))
  , x10_regId(static_cast<GX::TevRegID>(compressedDesc >> 9 & 3)) {}

  auto operator<=>(const CTevOp&) const = default;
};
struct ColorPass {
  GX::TevColorArg x0_a;
  GX::TevColorArg x4_b;
  GX::TevColorArg x8_c;
  GX::TevColorArg xc_d;

  constexpr ColorPass(GX::TevColorArg a, GX::TevColorArg b, GX::TevColorArg c, GX::TevColorArg d)
  : x0_a(a), x4_b(b), x8_c(c), xc_d(d) {}
  constexpr ColorPass(u32 compressedDesc)
  : x0_a(static_cast<GX::TevColorArg>(compressedDesc & 0x1F))
  , x4_b(static_cast<GX::TevColorArg>(compressedDesc >> 5 & 0x1F))
  , x8_c(static_cast<GX::TevColorArg>(compressedDesc >> 10 & 0x1F))
  , xc_d(static_cast<GX::TevColorArg>(compressedDesc >> 15 & 0x1F)) {}

  auto operator<=>(const ColorPass&) const = default;
};
struct AlphaPass {
  GX::TevAlphaArg x0_a;
  GX::TevAlphaArg x4_b;
  GX::TevAlphaArg x8_c;
  GX::TevAlphaArg xc_d;

  constexpr AlphaPass(GX::TevAlphaArg a, GX::TevAlphaArg b, GX::TevAlphaArg c, GX::TevAlphaArg d)
  : x0_a(a), x4_b(b), x8_c(c), xc_d(d) {}
  constexpr AlphaPass(u32 compressedDesc)
  : x0_a(static_cast<GX::TevAlphaArg>(compressedDesc & 0x1F))
  , x4_b(static_cast<GX::TevAlphaArg>(compressedDesc >> 5 & 0x1F))
  , x8_c(static_cast<GX::TevAlphaArg>(compressedDesc >> 10 & 0x1F))
  , xc_d(static_cast<GX::TevAlphaArg>(compressedDesc >> 15 & 0x1F)) {}

  auto operator<=>(const AlphaPass&) const = default;
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

  auto operator<=>(const CTevPass&) const = default;
};

extern const CTevPass skPassThru;
extern const CTevPass sTevPass804bfcc0;
extern const CTevPass sTevPass804bfe68;
extern const CTevPass sTevPass805a5698;
extern const CTevPass sTevPass805a5e70;
extern const CTevPass sTevPass805a5ebc;
extern const CTevPass sTevPass805a5f08;
extern const CTevPass sTevPass805a5f54;
extern const CTevPass sTevPass805a5fa0;
extern const CTevPass sTevPass805a5fec;
extern const CTevPass sTevPass805a6038;
extern const CTevPass sTevPass805a6084;

void Init();
void SetupPass(ERglTevStage stage, const CTevPass& pass);
void DeletePass(ERglTevStage stage);
bool SetPassCombiners(ERglTevStage stage, const CTevPass& pass);
void RecomputePasses();
void ResetStates();
} // namespace CTevCombiners
} // namespace metaforce
