#include "Graphics/CTevCombiners.hpp"

#include "Graphics/CGX.hpp"

namespace metaforce::CTevCombiners {
u32 CTevPass::sNextUniquePass = 0;

void CTevPass::Execute(ERglTevStage stage) const {
  const auto stageId = GX::TevStageID(stage);
  CGX::SetTevColorIn(stageId, x4_colorPass.x0_a, x4_colorPass.x4_b, x4_colorPass.x8_c, x4_colorPass.xc_d);
  CGX::SetTevAlphaIn(stageId, x14_alphaPass.x0_a, x14_alphaPass.x4_b, x14_alphaPass.x8_c, x14_alphaPass.xc_d);
  CGX::SetTevColorOp(stageId, x24_colorOp.x4_op, x24_colorOp.x8_bias, x24_colorOp.xc_scale, x24_colorOp.x0_clamp,
                     x24_colorOp.x10_regId);
  CGX::SetTevAlphaOp(stageId, x38_alphaOp.x4_op, x38_alphaOp.x8_bias, x38_alphaOp.xc_scale, x38_alphaOp.x0_clamp,
                     x38_alphaOp.x10_regId);
  CGX::SetTevKColorSel(stageId, GX::TevKColorSel::TEV_KCSEL_8_8);
  CGX::SetTevKAlphaSel(stageId, GX::TevKAlphaSel::TEV_KASEL_8_8);
}

constexpr u32 maxTevPasses = 2;
static u32 sNumEnabledPasses;
static std::array<bool, maxTevPasses> sValidPasses;

const CTevPass skPassThru{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_RASC},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_RASA},
};
const CTevPass sTevPass804bfcc0{
    {GX::TevColorArg::CC_C0, GX::TevColorArg::CC_TEXC, GX::TevColorArg::CC_RASC, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_RASA},
};
const CTevPass sTevPass804bfe68{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_CPREV, GX::TevColorArg::CC_RASC, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_APREV, GX::TevAlphaArg::CA_RASA, GX::TevAlphaArg::CA_ZERO},
};
const CTevPass sTevPass805a5698{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_RASC, GX::TevColorArg::CC_C0, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_RASA, GX::TevAlphaArg::CA_A0, GX::TevAlphaArg::CA_ZERO},
};
const CTevPass sTevPass805a5e70{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_C0},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_A0},
};
const CTevPass sTevPass805a5ebc{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_RASC, GX::TevColorArg::CC_TEXC, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_RASA, GX::TevAlphaArg::CA_TEXA, GX::TevAlphaArg::CA_ZERO},
};
const CTevPass sTevPass805a5f08{
    {GX::TevColorArg::CC_RASC, GX::TevColorArg::CC_TEXC, GX::TevColorArg::CC_TEXA, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_RASA},
};
const CTevPass sTevPass805a5f54{
    {GX::TevColorArg::CC_RASC, GX::TevColorArg::CC_ONE, GX::TevColorArg::CC_TEXC, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_TEXA, GX::TevAlphaArg::CA_RASA, GX::TevAlphaArg::CA_ZERO},
};
const CTevPass sTevPass805a5fa0{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_TEXC},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_TEXA},
};
const CTevPass sTevPass805a5fec{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_RASC},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_TEXA, GX::TevAlphaArg::CA_RASA, GX::TevAlphaArg::CA_ZERO},
};
const CTevPass sTevPass805a6038{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_TEXC, GX::TevColorArg::CC_RASC, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_KONST, GX::TevAlphaArg::CA_RASA, GX::TevAlphaArg::CA_ZERO},
};
const CTevPass sTevPass805a6084{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_CPREV, GX::TevColorArg::CC_APREV, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_APREV},
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
  if (pass == skPassThru) {
    DeletePass(stage);
    return;
  }
  if (SetPassCombiners(stage, pass)) {
    sValidPasses[static_cast<size_t>(stage)] = true;
    RecomputePasses();
  }
}

void DeletePass(ERglTevStage stage) {
  SetPassCombiners(stage, skPassThru);
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
  skPassThru.Execute(ERglTevStage::Stage0);
  sNumEnabledPasses = 1;
  CGX::SetNumTevStages(1);
}
} // namespace metaforce::CTevCombiners
