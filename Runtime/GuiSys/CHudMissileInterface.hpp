#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/GuiSys/CHudInterface.hpp"
#include <zeus/CTransform.hpp>

namespace urde {
class CAuiEnergyBarT01;
class CGuiFrame;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;
class CStateManager;

class CHudMissileInterface {
  enum class EInventoryStatus { Normal, Warning, Depleted };

  EHudType x0_hudType;
  int x4_missileCapacity;
  int x8_numMissles;
  float xc_arrowTimer = 0.f;
  zeus::CTransform x10_missleIconXf;
  float x40_missileWarningAlpha = 0.f;
  EInventoryStatus x44_latestStatus = EInventoryStatus::Normal;
  float x48_missileWarningPulse = 0.f;
  float x4c_chargeBeamFactor;
  float x50_missileIconAltDeplete = 0.f;
  float x54_missileIconIncrement = 0.f;
  bool x58_24_missilesActive : 1;
  bool x58_25_visibleDebug : 1 = true;
  bool x58_26_visibleGame : 1 = true;
  bool x58_27_hasArrows : 1;
  bool x58_28_notXRay : 1;
  CGuiWidget* x5c_basewidget_missileicon;
  CGuiTextPane* x60_textpane_missiledigits;
  CAuiEnergyBarT01* x64_energybart01_missilebar;
  CGuiTextPane* x68_textpane_missilewarning;
  CGuiModel* x6c_model_missilearrowup;
  CGuiModel* x70_model_missilearrowdown;
  CGuiWidget* x74_basewidget_missileicon;
  void UpdateVisibility(const CStateManager& mgr);

public:
  CHudMissileInterface(CGuiFrame& selHud, int missileCapacity, int numMissiles, float chargeFactor, bool missilesActive,
                       EHudType hudType, const CStateManager& mgr);
  void Update(float dt, const CStateManager& mgr);
  void SetIsVisibleGame(bool v, const CStateManager& mgr);
  void SetIsVisibleDebug(bool v, const CStateManager& mgr);
  void SetIsMissilesActive(bool active);
  void SetChargeBeamFactor(float t);
  void SetNumMissiles(int numMissiles, const CStateManager& mgr);
  void SetMissileCapacity(int missileCapacity);
  EInventoryStatus GetMissileInventoryStatus() const;
  static std::pair<zeus::CVector3f, zeus::CVector3f> CombatMissileBarCoordFunc(float t);
  static std::pair<zeus::CVector3f, zeus::CVector3f> XRayMissileBarCoordFunc(float t);
  static std::pair<zeus::CVector3f, zeus::CVector3f> ThermalMissileBarCoordFunc(float t);
};

} // namespace urde
