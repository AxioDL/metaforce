#ifndef _CHUDVISORBEAMMENU
#define _CHUDVISORBEAMMENU

#include "rstl/reserved_vector.hpp"
#include "types.h"

class CGuiFrame;
class CGuiWidget;
class CGuiModel;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CStateManager;

class CHudVisorBeamMenu {
public:
  enum EVisorBeamMenu { kVBM_Visor, kVBM_Beam };
  CHudVisorBeamMenu(CGuiFrame& frame, EVisorBeamMenu type,
                    const rstl::reserved_vector< bool, 4 >& enables);
  void Update(float dt, bool init);
  void UpdateHudAlpha(float alpha);
  void SetIsVisibleGame(bool visible);
  void SetPlayerHas(const rstl::reserved_vector< bool, 4 >& enables);
  void SetSelection(int selection, int pending, float interp);

private:
  struct SMenuItem {
    SMenuItem() : mModel_loz(nullptr), mModel_icon(nullptr), mPositioner(0.f), mOpacity(0.f) {}
    CGuiModel* mModel_loz;
    CGuiModel* mModel_icon;
    float mPositioner;
    float mOpacity;
  };
  enum EAnimPhase { kAP_None, kAP_Steady, kAP_SelectFlash, kAP_Animate };
  EVisorBeamMenu GetSwappedType() const { return mSwapBeamControls ? static_cast< EVisorBeamMenu >(1 - mType) : mType; }
  bool GetIsVisible() const { return mVisibleDebug && mVisibleGame; }
  void UpdateMenuWidgetTransform(int idx, CGuiWidget& widget, float t);
  CGuiFrame& mBaseHud;
  EVisorBeamMenu mType;
  int mSelectedItem;
  int mPendingSelection;
  float mInterp;
  bool mVisibleDebug : 1;
  bool mVisibleGame : 1;
  bool mDirty : 1;
  CGuiWidget* mBasewidget_menu;
  CGuiWidget* mBasewidget_menutitle;
  CGuiTextPane* mTextpane_menu;
  CGuiModel* mModel_ghost;
  rstl::reserved_vector< SMenuItem, 4 > mMenuItems;
  EAnimPhase mAnimPhase;
  float x70_;
  float x74_;
  float mTextFader;
  float mAnimDur;
  bool mSwapBeamControls : 1;
};
CHECK_SIZEOF(CHudVisorBeamMenu, 0x84)

#endif // _CHUDVISORBEAMMENU
