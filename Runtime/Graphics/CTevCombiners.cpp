#include "Graphics/CTevCombiners.hpp"

#include "Graphics/CGX.hpp"

namespace metaforce::CTevCombiners {
u32 CTevPass::sNextUniquePass = 0;

void CTevPass::Execute(ERglTevStage stage) const {
  const auto stageId = GXTevStageID(stage);
  CGX::SetTevColorIn(stageId, x4_colorPass.x0_a, x4_colorPass.x4_b, x4_colorPass.x8_c, x4_colorPass.xc_d);
  CGX::SetTevAlphaIn(stageId, x14_alphaPass.x0_a, x14_alphaPass.x4_b, x14_alphaPass.x8_c, x14_alphaPass.xc_d);
  CGX::SetTevColorOp(stageId, x24_colorOp.x4_op, x24_colorOp.x8_bias, x24_colorOp.xc_scale, x24_colorOp.x0_clamp,
                     x24_colorOp.x10_regId);
  CGX::SetTevAlphaOp(stageId, x38_alphaOp.x4_op, x38_alphaOp.x8_bias, x38_alphaOp.xc_scale, x38_alphaOp.x0_clamp,
                     x38_alphaOp.x10_regId);
  CGX::SetTevKColorSel(stageId, GX_TEV_KCSEL_8_8);
  CGX::SetTevKAlphaSel(stageId, GX_TEV_KASEL_8_8);
}

constexpr u32 maxTevPasses = 2;
static u32 sNumEnabledPasses;
static std::array<bool, maxTevPasses> sValidPasses;

const CTevPass kEnvPassthru{
    {GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC},
    {GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA},
};
const CTevPass kEnvBlendCTandCConCF{
    {GX_CC_C0, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO},
    {GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA},
};
const CTevPass kEnvModulateConstColor{
    {GX_CC_ZERO, GX_CC_RASC, GX_CC_C0, GX_CC_ZERO},
    {GX_CA_ZERO, GX_CA_RASA, GX_CA_A0, GX_CA_ZERO},
};
const CTevPass kEnvConstColor{
    {GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0},
    {GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0},
};
const CTevPass kEnvModulate{
    {GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO},
    {GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO},
};
const CTevPass kEnvDecal{
    {GX_CC_RASC, GX_CC_TEXC, GX_CC_TEXA, GX_CC_ZERO},
    {GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA},
};
const CTevPass kEnvBlend{
    {GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, GX_CC_ZERO},
    {GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO},
};
const CTevPass kEnvReplace{
    {GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC},
    {GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA},
};
const CTevPass kEnvModulateAlpha{
    {GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC},
    {GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO},
};
const CTevPass kEnvModulateColor{
    {GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO},
    {GX_CA_ZERO, GX_CA_KONST, GX_CA_RASA, GX_CA_ZERO},
};
const CTevPass kEnvModulateColorByAlpha{
    {GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO},
    {GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV},
};

void Init() {
  sNumEnabledPasses = maxTevPasses;
  sValidPasses.fill(true);
  for (int i = 0; i < maxTevPasses; ++i) {
    DeletePass(static_cast<ERglTevStage>(i));
  }
  sValidPasses.fill(false);
  RecomputePasses();
}

void SetupPass(ERglTevStage stage, const CTevPass& pass) {
  if (pass == kEnvPassthru) {
    DeletePass(stage);
    return;
  }
  if (SetPassCombiners(stage, pass)) {
    sValidPasses[static_cast<size_t>(stage)] = true;
    RecomputePasses();
  }
}

void DeletePass(ERglTevStage stage) {
  SetPassCombiners(stage, kEnvPassthru);
  sValidPasses[static_cast<size_t>(stage)] = false;
  RecomputePasses();
}

bool SetPassCombiners(ERglTevStage stage, const CTevPass& pass) {
  pass.Execute(stage);
  return true;
}

void RecomputePasses() {
  sNumEnabledPasses = 1 - static_cast<int>(sValidPasses[maxTevPasses - 1]);
  CGX::SetNumTevStages(sNumEnabledPasses);
}

void ResetStates() {
  sValidPasses.fill(false);
  kEnvPassthru.Execute(ERglTevStage::Stage0);
  sNumEnabledPasses = 1;
  CGX::SetNumTevStages(1);
}
} // namespace metaforce::CTevCombiners
