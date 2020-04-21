#include "Runtime/GuiSys/CHudMissileInterface.hpp"

#include <array>

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/GuiSys/CAuiEnergyBarT01.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"

namespace urde {

constexpr std::array<CAuiEnergyBarT01::FCoordFunc, 5> CoordFuncs{
    CHudMissileInterface::CombatMissileBarCoordFunc,  nullptr, CHudMissileInterface::XRayMissileBarCoordFunc,
    CHudMissileInterface::ThermalMissileBarCoordFunc, nullptr,
};

constexpr std::array IconTranslateRanges{
    6.05f, 0.f, 0.f, 8.4f, 0.f,
};

CHudMissileInterface::CHudMissileInterface(CGuiFrame& selHud, int missileCapacity, int numMissiles, float chargeFactor,
                                           bool missilesActive, EHudType hudType, const CStateManager& mgr)
: x0_hudType(hudType)
, x4_missileCapacity(missileCapacity)
, x8_numMissles(numMissiles)
, x4c_chargeBeamFactor(chargeFactor)
, x58_24_missilesActive(missilesActive) {
  x5c_basewidget_missileicon = selHud.FindWidget("basewidget_missileicon");
  x60_textpane_missiledigits = static_cast<CGuiTextPane*>(selHud.FindWidget("textpane_missiledigits"));
  x64_energybart01_missilebar = static_cast<CAuiEnergyBarT01*>(selHud.FindWidget("energybart01_missilebar"));
  x68_textpane_missilewarning = static_cast<CGuiTextPane*>(selHud.FindWidget("textpane_missilewarning"));
  x6c_model_missilearrowup = static_cast<CGuiModel*>(selHud.FindWidget("model_missilearrowup"));
  x70_model_missilearrowdown = static_cast<CGuiModel*>(selHud.FindWidget("model_missilearrowdown"));
  x74_basewidget_missileicon = selHud.FindWidget("basewidget_missileicon");

  x58_27_hasArrows = x6c_model_missilearrowup && x70_model_missilearrowdown;
  x58_28_notXRay = hudType != EHudType::XRay;

  x10_missleIconXf = x74_basewidget_missileicon->GetLocalTransform();

  x60_textpane_missiledigits->TextSupport().SetFontColor(g_tweakGuiColors->GetMissileDigitsFont());
  x60_textpane_missiledigits->TextSupport().SetOutlineColor(g_tweakGuiColors->GetMissileDigitsOutline());
  x74_basewidget_missileicon->SetColor(g_tweakGuiColors->GetMissileIconColorInactive());
  x64_energybart01_missilebar->SetEmptyColor(g_tweakGuiColors->GetMissileBarEmpty());
  x64_energybart01_missilebar->SetFilledColor(g_tweakGuiColors->GetMissileBarFilled());
  x64_energybart01_missilebar->SetShadowColor(g_tweakGuiColors->GetMissileBarShadow());
  x64_energybart01_missilebar->SetCoordFunc(CoordFuncs[size_t(hudType)]);
  x64_energybart01_missilebar->SetTesselation(hudType == EHudType::Combat ? 1.f : 0.1f);
  x64_energybart01_missilebar->SetMaxEnergy(5.f);
  x64_energybart01_missilebar->SetFilledDrainSpeed(g_tweakGui->GetEnergyBarFilledSpeed());
  x64_energybart01_missilebar->SetShadowDrainSpeed(g_tweakGui->GetEnergyBarShadowSpeed());
  x64_energybart01_missilebar->SetShadowDrainDelay(g_tweakGui->GetEnergyBarDrainDelay());
  x64_energybart01_missilebar->SetIsAlwaysResetTimer(true);

  if (x68_textpane_missilewarning) {
    x68_textpane_missilewarning->TextSupport().SetFontColor(g_tweakGuiColors->GetMissileWarningFont());
    x68_textpane_missilewarning->TextSupport().SetOutlineColor(g_tweakGuiColors->GetMissileWarningOutline());
  }

  SetNumMissiles(x8_numMissles, mgr);
  x44_latestStatus = GetMissileInventoryStatus();
}

void CHudMissileInterface::UpdateVisibility(const CStateManager& mgr) {
  bool vis = x58_25_visibleDebug && x58_26_visibleGame;
  x5c_basewidget_missileicon->SetVisibility(vis, ETraversalMode::Children);
  x64_energybart01_missilebar->SetVisibility(vis, ETraversalMode::Children);
  if (vis)
    Update(0.f, mgr);
}

void CHudMissileInterface::Update(float dt, const CStateManager& mgr) {
  if (x4_missileCapacity < 1)
    x5c_basewidget_missileicon->SetIsVisible(false);
  else
    x5c_basewidget_missileicon->SetIsVisible(true);

  if (x54_missileIconIncrement < 0.f) {
    x54_missileIconIncrement -= 3.f * dt;
    if (x54_missileIconIncrement <= -1.f)
      x54_missileIconIncrement = 1.f;
  } else if (x54_missileIconIncrement > 0.f) {
    x54_missileIconIncrement = std::max(0.f, x54_missileIconIncrement - dt);
  }

  zeus::CColor addColor = g_tweakGuiColors->GetMissileIconColorActive() * x54_missileIconIncrement;

  if (x50_missileIconAltDeplete > 0.f) {
    x74_basewidget_missileicon->SetColor(zeus::CColor::lerp(g_tweakGuiColors->GetMissileIconColorInactive(),
                                                            g_tweakGuiColors->GetMissileIconColorDepleteAlt(),
                                                            x50_missileIconAltDeplete) +
                                         addColor);
  } else {
    if (x4c_chargeBeamFactor > 0.f) {
      float factor = std::min(x4c_chargeBeamFactor / CPlayerState::GetMissileComboChargeFactor(), 1.f);
      if (x8_numMissles > mgr.GetPlayerState()->GetMissileCostForAltAttack()) {
        x74_basewidget_missileicon->SetColor(zeus::CColor::lerp(g_tweakGuiColors->GetMissileIconColorInactive(),
                                                                g_tweakGuiColors->GetMissileIconColorChargedCanAlt(),
                                                                factor) +
                                             addColor);
      } else {
        x74_basewidget_missileicon->SetColor(zeus::CColor::lerp(g_tweakGuiColors->GetMissileIconColorInactive(),
                                                                g_tweakGuiColors->GetMissileIconColorChargedNoAlt(),
                                                                factor) +
                                             addColor);
      }
    } else {
      if (x58_24_missilesActive)
        x74_basewidget_missileicon->SetColor(g_tweakGuiColors->GetMissileIconColorActive() + addColor);
      else
        x74_basewidget_missileicon->SetColor(g_tweakGuiColors->GetMissileIconColorInactive() + addColor);
    }
  }

  x50_missileIconAltDeplete = std::max(0.f, x50_missileIconAltDeplete - dt);

  x64_energybart01_missilebar->SetMaxEnergy(x4_missileCapacity);
  x64_energybart01_missilebar->SetCurrEnergy(x8_numMissles, CAuiEnergyBarT01::ESetMode::Normal);

  if (x58_28_notXRay) {
    x74_basewidget_missileicon->SetLocalTransform(
        x10_missleIconXf *
        zeus::CTransform::Translate(
            0.f, 0.f, x8_numMissles * IconTranslateRanges[size_t(x0_hudType)] / float(x4_missileCapacity)));
  }

  if (x58_27_hasArrows) {
    if (xc_arrowTimer > 0.f) {
      xc_arrowTimer = std::max(0.f, xc_arrowTimer - dt);
      zeus::CColor color = g_tweakGuiColors->GetMissileIconColorActive();
      color.a() *= xc_arrowTimer / g_tweakGui->GetMissileArrowVisTime();
      x6c_model_missilearrowup->SetColor(color);
      x70_model_missilearrowdown->SetIsVisible(false);
    } else if (xc_arrowTimer < 0.f) {
      xc_arrowTimer = std::min(0.f, xc_arrowTimer + dt);
      zeus::CColor color = g_tweakGuiColors->GetMissileIconColorActive();
      color.a() *= -xc_arrowTimer / g_tweakGui->GetMissileArrowVisTime();
      x70_model_missilearrowdown->SetColor(color);
      x6c_model_missilearrowup->SetIsVisible(false);
    } else {
      x6c_model_missilearrowup->SetIsVisible(false);
      x70_model_missilearrowdown->SetIsVisible(false);
    }
  }

  if (x68_textpane_missilewarning) {
    EInventoryStatus curStatus = GetMissileInventoryStatus();
    if (curStatus != x44_latestStatus) {
      std::u16string string;
      switch (curStatus) {
      case EInventoryStatus::Warning:
        string = g_MainStringTable->GetString(12); // Missiles Low
        break;
      case EInventoryStatus::Depleted:
        string = g_MainStringTable->GetString(13); // Depleted
        break;
      default:
        break;
      }
      x68_textpane_missilewarning->TextSupport().SetText(string);

      if (x44_latestStatus == EInventoryStatus::Normal && curStatus == EInventoryStatus::Warning) {
        CSfxManager::SfxStart(SFXui_missile_warning, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x48_missileWarningPulse = g_tweakGui->GetMissileWarningPulseTime();
      } else if (curStatus == EInventoryStatus::Depleted) {
        CSfxManager::SfxStart(SFXui_missile_warning, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x48_missileWarningPulse = g_tweakGui->GetMissileWarningPulseTime();
      }

      x44_latestStatus = curStatus;
    }

    x48_missileWarningPulse = std::max(0.f, x48_missileWarningPulse - dt);
    float warnPulse = std::min(x48_missileWarningPulse, 1.f);

    if (x44_latestStatus != EInventoryStatus::Normal)
      x40_missileWarningAlpha = std::min(x40_missileWarningAlpha + 2.f * dt, 1.f);
    else
      x40_missileWarningAlpha = std::max(0.f, x40_missileWarningAlpha - 2.f * dt);

    float tmp = std::fabs(std::fmod(CGraphics::GetSecondsMod900(), 0.5f));
    if (tmp < 0.25f)
      tmp = tmp / 0.25f;
    else
      tmp = (0.5f - tmp) / 0.25f;

    zeus::CColor color = zeus::skWhite;
    color.a() = x40_missileWarningAlpha * tmp * warnPulse;
    x68_textpane_missilewarning->SetColor(color);
    if (x68_textpane_missilewarning->GetGeometryColor().a())
      x68_textpane_missilewarning->SetIsVisible(true);
    else
      x68_textpane_missilewarning->SetIsVisible(false);
  }
}

void CHudMissileInterface::SetIsVisibleGame(bool v, const CStateManager& mgr) {
  x58_26_visibleGame = v;
  UpdateVisibility(mgr);
}

void CHudMissileInterface::SetIsVisibleDebug(bool v, const CStateManager& mgr) {
  x58_25_visibleDebug = v;
  UpdateVisibility(mgr);
}

void CHudMissileInterface::SetIsMissilesActive(bool active) { x58_24_missilesActive = active; }

void CHudMissileInterface::SetChargeBeamFactor(float t) { x4c_chargeBeamFactor = t; }

void CHudMissileInterface::SetNumMissiles(int numMissiles, const CStateManager& mgr) {
  numMissiles = zeus::clamp(0, numMissiles, 999);

  x60_textpane_missiledigits->TextSupport().SetText(fmt::format(FMT_STRING("{:3d}"), numMissiles));

  if (x8_numMissles < numMissiles) {
    xc_arrowTimer = g_tweakGui->GetMissileArrowVisTime();
    x54_missileIconIncrement = -FLT_EPSILON;
  } else if (x8_numMissles > numMissiles) {
    xc_arrowTimer = -g_tweakGui->GetMissileArrowVisTime();
  }

  if (mgr.GetPlayerState()->GetMissileCostForAltAttack() + numMissiles <= x8_numMissles)
    x50_missileIconAltDeplete = 1.f;

  x8_numMissles = numMissiles;
}

void CHudMissileInterface::SetMissileCapacity(int missileCapacity) { x4_missileCapacity = missileCapacity; }

CHudMissileInterface::EInventoryStatus CHudMissileInterface::GetMissileInventoryStatus() const {
  if (x64_energybart01_missilebar->GetSetEnergy() == 0.f)
    return EInventoryStatus::Depleted;
  return EInventoryStatus(x64_energybart01_missilebar->GetActualFraction() < g_tweakGui->GetMissileWarningFraction());
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudMissileInterface::CombatMissileBarCoordFunc(float t) {
  const float z = t * IconTranslateRanges[size_t(EHudType::Combat)];
  return {zeus::CVector3f(0.f, 0.f, z), zeus::CVector3f(0.3f, 0.f, z)};
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudMissileInterface::XRayMissileBarCoordFunc(float t) {
  const float theta = 0.8f * (t - 0.5f);
  const float x = 9.55f * std::cos(theta);
  const float z = 9.55f * std::sin(theta);
  return {zeus::CVector3f(x - 0.4f, 0.f, z), zeus::CVector3f(x, 0.f, z)};
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudMissileInterface::ThermalMissileBarCoordFunc(float t) {
  const float transRange = IconTranslateRanges[size_t(EHudType::Thermal)];
  const float a = 0.08f * transRange;
  const float b = t * transRange;

  float c;
  if (b < a) {
    c = b / a;
  } else if (b < transRange - a) {
    c = 1.f;
  } else {
    c = 1.f - (b - (transRange - a)) / a;
  }

  return {zeus::CVector3f(-0.5f * c - 0.1f, 0.f, b), zeus::CVector3f(-0.1f, 0.f, b)};
}

} // namespace urde
