#ifndef _COPTIONSSCREEN
#define _COPTIONSSCREEN

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Input/CRumbleGenerator.hpp"
#include "MetroidPrime/CPauseScreenBase.hpp"
#include "rstl/single_ptr.hpp"

class CGameCubeDoll;
class CQuitGameScreen;

class COptionsScreen : public CPauseScreenBase {
public:
  ~COptionsScreen() override;
  bool InputDisabled() const override;
  void Update(float dt, CRandom16& rand, CArchitectureQueue& queue) override;
  void Touch() override;
  void ProcessInput(const CFinalInput& input) override;
  void Draw(float transInterp, float totalAlpha, float yOff) const override;
  bool VReady() const override;
  void VActivate() override;
  void RightTableSelectionChanged(int oldSel, int newSel) override;
  void ChangedMode(EMode oldMode) override;
  void UpdateRightTable() override;
  bool ShouldLeftTableAdvance() override;
  bool ShouldRightTableAdvance() override;
  uint GetRightTableCount() const override;

  COptionsScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);

private:
  void UpdateOptionView();
  void ResetOptionWidgetVisibility();
  void OnSliderChanged(CGuiSliderGroup* caller, float value);
  void OnEnumChanged(CGuiTableGroup* caller, int oldSel);

  rstl::single_ptr< CQuitGameScreen > mQuitGame;
  rstl::single_ptr< CGameCubeDoll > mGameCube;
  CSfxHandle mSliderSfx;
  CRumbleGenerator mRumble;
  float mOptionAlpha;
  bool mInOptionBody : 1;
};
CHECK_SIZEOF(COptionsScreen, 0x2a4)

#endif // _COPTIONSSCREEN
