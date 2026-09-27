#ifndef _CHUDBOSSENERGYINTERFACE
#define _CHUDBOSSENERGYINTERFACE

#include "rstl/string.hpp"
#include "rstl/pair.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "types.h"

class CGuiFrame;
class CGuiWidget;
class CGuiModel;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CStateManager;

class CHudBossEnergyInterface {
public:
  explicit CHudBossEnergyInterface(CGuiFrame& frame);
  void Update(float dt);
  void SetAlpha(float alpha);
  void SetBossParams(bool visible, const rstl::wstring& name, float energy, float maxEnergy);
  static rstl::pair< CVector3f, CVector3f > BossEnergyCoordFunc(float t);

private:
  float mAlpha;
  float mFader;
  float mCurEnergy;
  float mMaxEnergy;
  bool mVisible : 1;
  CGuiWidget* mBasewidget_bossenergystuff;
  CAuiEnergyBarT01* mEnergybart01_bossbar;
  CGuiTextPane* mTextpane_boss;
};
CHECK_SIZEOF(CHudBossEnergyInterface, 0x20)

#endif // _CHUDBOSSENERGYINTERFACE
