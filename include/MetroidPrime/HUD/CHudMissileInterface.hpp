#ifndef _CHUDMISSILEINTERFACE
#define _CHUDMISSILEINTERFACE

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/HUD/CHudInterface.hpp"
#include "rstl/pair.hpp"
#include "types.h"

class CGuiFrame;
class CGuiWidget;
class CGuiModel;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CStateManager;

class CHudMissileInterface {
public:
  CHudMissileInterface(CGuiFrame& hud, int capacity, int missiles, float chargeFactor, bool active,
                       EHudType type, const CStateManager& mgr);
  void Update(float dt, const CStateManager& mgr);
  void SetIsVisibleGame(bool visible, const CStateManager& mgr);
  void SetIsMissilesActive(bool active);
  void SetChargeBeamFactor(float factor);
  void SetNumMissiles(int missiles, const CStateManager& mgr);
  void SetMissileCapacity(int capacity);
  static rstl::pair< CVector3f, CVector3f > CombatMissileBarCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > ThermalMissileBarCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > XRayMissileBarCoordFunc(float t);

private:
  enum EInventoryStatus { kIS_Normal, kIS_Warning, kIS_Depleted };
  EInventoryStatus GetMissileInventoryStatus();
  void UpdateVisibility(const CStateManager& mgr);

  EHudType mHudType;
  int mMissileCapacity;
  int mNumMissiles;
  float mArrowTimer;
  CTransform4f mMissileIconXf;
  float mMissileWarningAlpha;
  EInventoryStatus mLatestStatus;
  float mMissileWarningPulse;
  float mChargeBeamFactor;
  float mMissileIconAltDeplete;
  float mMissileIconIncrement;
  bool mMissilesActive : 1;
  bool mVisibleDebug : 1;
  bool mVisibleGame : 1;
  bool mHasArrows : 1;
  bool mNotXRay : 1;
  CGuiWidget* mBaseWidgetMissileIconA;
  CGuiTextPane* mTextpane_missiledigits;
  CAuiEnergyBarT01* mEnergybart01_missilebar;
  CGuiTextPane* mTextpane_missilewarning;
  CGuiModel* mModel_missilearrowup;
  CGuiModel* mModel_missilearrowdown;
  CGuiWidget* mBaseWidgetMissileIconB;
};
CHECK_SIZEOF(CHudMissileInterface, 0x78)

#endif // _CHUDMISSILEINTERFACE
