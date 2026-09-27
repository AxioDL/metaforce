#ifndef _SOPTIONSFRONTENDFRAME
#define _SOPTIONSFRONTENDFRAME

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Input/CRumbleGenerator.hpp"
#include "Kyoto/TToken.hpp"

class CFinalInput;
class CGuiFrame;
class CGuiTableGroup;
class CGuiSliderGroup;
class CStringTable;
class CSaveGameScreen;

class SOptionsFrontEndFrame {
public:
  SOptionsFrontEndFrame();
  ~SOptionsFrontEndFrame();
  void Update(float dt, const CSaveGameScreen* saveUI);
  bool ProcessUserInput(const CFinalInput& input, CSaveGameScreen* saveUI);
  void Draw() const;

private:
  bool PumpLoad();
  void FinishedLoading();
  void SetRightUIText();
  void HandleRightSelectionChange();
  void DeactivateRightMenu();
  void DoLeftMenuAdvance(CGuiTableGroup* caller);
  void DoMenuSelectionChange(CGuiTableGroup* caller, int oldSel);
  void DoMenuCancel(CGuiTableGroup* caller);
  void SetTableColors(CGuiTableGroup* table) const;
  void DoSliderChange(CGuiSliderGroup* caller, float value);

  float mUiAlpha;
  TCachedToken< CGuiFrame > mFrme;
  TCachedToken< CStringTable > mPauseScreen;
  CGuiFrame* mLoadedFrame;
  CStringTable* mLoadedPauseStrg;
  CGuiTableGroup* mTablegroup_leftmenu;
  CGuiTableGroup* mTablegroup_rightmenu;
  CGuiTableGroup* mTablegroup_double;
  CGuiTableGroup* mTablegroup_triple;
  CGuiSliderGroup* mSlidergroup_slider;
  float mRowPitch;
  CSfxHandle mSliderSfx;
  CRumbleGenerator mRumbleGen;
  bool mVisible : 1;
  bool mExitOptions : 1;
};
CHECK_SIZEOF(SOptionsFrontEndFrame, 0x138)

#endif // _SOPTIONSFRONTENDFRAME
