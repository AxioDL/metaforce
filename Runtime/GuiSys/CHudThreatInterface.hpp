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

class CHudThreatInterface {
  enum class EThreatStatus { Normal, Warning, Damage };

  EHudType x4_hudType;
  float x8_damagePulseTimer = 0.f;
  float xc_damagePulse = 0.f;
  float x10_threatDist;
  float x14_arrowTimer = 0.f;
  zeus::CTransform x18_threatIconXf;
  float x48_warningLerpAlpha = 0.f;
  EThreatStatus x4c_threatStatus = EThreatStatus::Normal;
  float x50_warningColorLerp = 0.f;
  bool x54_24_visibleDebug : 1 = true;
  bool x54_25_visibleGame : 1 = true;
  bool x54_26_hasArrows : 1;
  bool x54_27_notXRay : 1;
  CGuiWidget* x58_basewidget_threatstuff;
  CGuiWidget* x5c_basewidget_threaticon;
  CGuiModel* x60_model_threatarrowup;
  CGuiModel* x64_model_threatarrowdown;
  CGuiTextPane* x68_textpane_threatwarning;
  CAuiEnergyBarT01* x6c_energybart01_threatbar;
  CGuiTextPane* x70_textpane_threatdigits;
  void UpdateVisibility();

public:
  CHudThreatInterface(CGuiFrame& selHud, EHudType hudType, float threatDist);
  void SetThreatDistance(float threatDist);
  void SetIsVisibleDebug(bool v);
  void SetIsVisibleGame(bool v);
  void Update(float dt);
  static std::pair<zeus::CVector3f, zeus::CVector3f> CombatThreatBarCoordFunc(float t);
  static std::pair<zeus::CVector3f, zeus::CVector3f> XRayThreatBarCoordFunc(float t);
  static std::pair<zeus::CVector3f, zeus::CVector3f> ThermalThreatBarCoordFunc(float t);
};

} // namespace urde
