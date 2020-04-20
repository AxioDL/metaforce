#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/GuiSys/CHudInterface.hpp"
#include <zeus/CVector3f.hpp>

namespace urde {
class CAuiEnergyBarT01;
class CAuiMeter;
class CGuiFrame;
class CGuiTextPane;
class CGuiWidget;

class CHudEnergyInterface {
  EHudType x0_hudType;
  float x4_energyLowFader = 0.f;
  float x8_flashMag = 0.f;
  float xc_tankEnergy;
  int x10_totalEnergyTanks;
  int x14_numTanksFilled;
  float x18_cachedBarEnergy = 0.f;
  bool x1c_24_ : 1 = true;
  bool x1c_25_ : 1 = true;
  bool x1c_26_barDirty : 1 = true;
  bool x1c_27_energyLow : 1;
  CGuiTextPane* x20_textpane_energydigits;
  CAuiMeter* x24_meter_energytanks;
  CGuiTextPane* x28_textpane_energywarning;
  CAuiEnergyBarT01* x2c_energybart01_energybar;

public:
  CHudEnergyInterface(CGuiFrame& selHud, float tankEnergy, int totalEnergyTanks, int numTanksFilled, bool energyLow,
                      EHudType hudType);
  void Update(float dt, float energyLowPulse);
  void SetEnergyLow(bool energyLow);
  void SetFlashMagnitude(float mag);
  void SetNumFilledEnergyTanks(int numTanksFilled);
  void SetNumTotalEnergyTanks(int totalEnergyTanks);
  void SetCurrEnergy(float tankEnergy, bool wrapped);
  static std::pair<zeus::CVector3f, zeus::CVector3f> CombatEnergyCoordFunc(float t);
  static std::pair<zeus::CVector3f, zeus::CVector3f> XRayEnergyCoordFunc(float t);
  static std::pair<zeus::CVector3f, zeus::CVector3f> ThermalEnergyCoordFunc(float t);
  static std::pair<zeus::CVector3f, zeus::CVector3f> BallEnergyCoordFunc(float t);
};

} // namespace urde
