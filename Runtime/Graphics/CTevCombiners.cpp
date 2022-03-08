#include "Graphics/CTevCombiners.hpp"

namespace metaforce::CTevCombiners {
u32 CTevPass::sNextUniquePass = 0;

void CTevPass::Execute(ERglTevStage stage) const {
  aurora::gfx::update_tev_stage(stage, x4_colorPass, x14_alphaPass, x24_colorOp, x38_alphaOp);
  aurora::gfx::set_tev_k_color_sel(static_cast<GX::TevStageID>(stage), GX::TevKColorSel::TEV_KCSEL_8_8);
  aurora::gfx::set_tev_k_alpha_sel(static_cast<GX::TevStageID>(stage), GX::TevKAlphaSel::TEV_KASEL_8_8);
}

constexpr u32 maxTevPasses = 2;
static u32 sNumEnabledPasses;
static std::array<bool, maxTevPasses> sValidPasses;

const CTevPass skPassThru{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_RASC},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_RASA},
};
const CTevPass skPassZero{
    {GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO},
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
const CTevPass sTevPass804bfcc0{
    {GX::TevColorArg::CC_C0, GX::TevColorArg::CC_TEXC, GX::TevColorArg::CC_RASC, GX::TevColorArg::CC_ZERO},
    {GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_ZERO, GX::TevAlphaArg::CA_RASA},
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
  sNumEnabledPasses = 1 - static_cast<int>(sValidPasses[1]);
  for (u32 i = sNumEnabledPasses; i < u32(ERglTevStage::MAX); ++i) {
    aurora::gfx::disable_tev_stage(ERglTevStage(i));
  }
  // CGX::SetNumTevStages(sNumEnabledPasses);
}

void ResetStates() {
  sValidPasses.fill(false);
  skPassThru.Execute(ERglTevStage::Stage0);
  sNumEnabledPasses = 1;
  // CGX::SetNumTevStages(1);
}
} // namespace metaforce::CTevCombiners
