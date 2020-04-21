#pragma once

#include <memory>

#include "Runtime/MP1/CGameCubeDoll.hpp"
#include "Runtime/MP1/CInGameGuiManager.hpp"
#include "Runtime/MP1/CPauseScreenBase.hpp"
#include "Runtime/MP1/CQuitGameScreen.hpp"

namespace urde::MP1 {

class COptionsScreen : public CPauseScreenBase {
  std::unique_ptr<CQuitGameScreen> x19c_quitGame;
  std::unique_ptr<CGameCubeDoll> x1a0_gameCube;
  CSfxHandle x1a4_sliderSfx;
  CRumbleGenerator x1a8_rumble;
  float x29c_optionAlpha = 0.f;
  bool x2a0_24_inOptionBody : 1 = false;

  void UpdateOptionView();
  void ResetOptionWidgetVisibility();
  void OnSliderChanged(CGuiSliderGroup* caller, float val);
  void OnEnumChanged(CGuiTableGroup* caller, int oldSel);

public:
  COptionsScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);
  ~COptionsScreen() override;

  bool InputDisabled() const override;
  void Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue) override;
  void Touch() override;
  void ProcessControllerInput(const CFinalInput& input) override;
  void Draw(float transInterp, float totalAlpha, float yOff) override;
  bool VReady() const override;
  void VActivate() override;
  void RightTableSelectionChanged(int oldSel, int newSel) override;
  void ChangedMode(EMode oldMode) override;
  void UpdateRightTable() override;
  bool ShouldLeftTableAdvance() const override;
  bool ShouldRightTableAdvance() const override;
  u32 GetRightTableCount() const override;
};

} // namespace urde::MP1
