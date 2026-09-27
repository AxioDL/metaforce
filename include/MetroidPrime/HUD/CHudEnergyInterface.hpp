#ifndef _CHUDENERGYINTERFACE
#define _CHUDENERGYINTERFACE

#include "MetroidPrime/HUD/CHudInterface.hpp"
#include "Kyoto/Math/CVector3f.hpp"
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

class CHudEnergyInterface {
public:
  CHudEnergyInterface(CGuiFrame& hud, float energy, int totalTanks, int filledTanks, bool energyLow,
                      EHudType type);
  void Update(float dt, float energyLowPulse);
  void SetEnergyLow(bool low);
  void SetFlashMagnitude(float mag);
  void SetNumFilledEnergyTanks(int tanks);
  void SetNumTotalEnergyTanks(int tanks);
  void SetCurrEnergy(float energy, bool wrapped);
  static rstl::pair< CVector3f, CVector3f > CombatEnergyCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > BallEnergyCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > ThermalEnergyCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > XRayEnergyCoordFunc(float t);

private:
  EHudType mHudType;
  float mEnergyLowFader;
  float mFlashMag;
  float mTankEnergy;
  int mTotalEnergyTanks;
  int mNumTanksFilled;
  float mCachedBarEnergy;
  bool x1c_24_ : 1;
  bool x1c_25_ : 1;
  bool mBarDirty : 1;
  bool mEnergyLow : 1;
  CGuiTextPane* mTextpane_energydigits;
  CAuiMeter* mMeter_energytanks;
  CGuiTextPane* mTextpane_energywarning;
  CAuiEnergyBarT01* mEnergybart01_energybar;
};
CHECK_SIZEOF(CHudEnergyInterface, 0x30)

#endif // _CHUDENERGYINTERFACE
