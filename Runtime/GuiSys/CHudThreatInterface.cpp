#include "Runtime/GuiSys/CHudThreatInterface.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/GuiSys/CAuiEnergyBarT01.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/Audio/CSfxManager.hpp"

namespace urde {

constexpr std::array<CAuiEnergyBarT01::FCoordFunc, 5> CoordFuncs{
    CHudThreatInterface::CombatThreatBarCoordFunc,  nullptr, CHudThreatInterface::XRayThreatBarCoordFunc,
    CHudThreatInterface::ThermalThreatBarCoordFunc, nullptr,
};

constexpr std::array IconTranslateRanges{
    6.05f, 0.f, 0.f, 8.4f, 0.f,
};

CHudThreatInterface::CHudThreatInterface(CGuiFrame& selHud, EHudType hudType, float threatDist)
: x4_hudType(hudType), x10_threatDist(threatDist) {
  x58_basewidget_threatstuff = selHud.FindWidget("basewidget_threatstuff");
  x5c_basewidget_threaticon = selHud.FindWidget("basewidget_threaticon");
  x60_model_threatarrowup = static_cast<CGuiModel*>(selHud.FindWidget("model_threatarrowup"));
  x64_model_threatarrowdown = static_cast<CGuiModel*>(selHud.FindWidget("model_threatarrowdown"));
  x68_textpane_threatwarning = static_cast<CGuiTextPane*>(selHud.FindWidget("textpane_threatwarning"));
  x6c_energybart01_threatbar = static_cast<CAuiEnergyBarT01*>(selHud.FindWidget("energybart01_threatbar"));
  x70_textpane_threatdigits = static_cast<CGuiTextPane*>(selHud.FindWidget("textpane_threatdigits"));

  if (x70_textpane_threatdigits) {
    x70_textpane_threatdigits->TextSupport().SetFontColor(g_tweakGuiColors->GetThreatDigitsFont());
    x70_textpane_threatdigits->TextSupport().SetOutlineColor(g_tweakGuiColors->GetThreatDigitsOutline());
  }

  x54_26_hasArrows = x60_model_threatarrowup && x64_model_threatarrowdown;
  x54_27_notXRay = hudType != EHudType::XRay;

  x5c_basewidget_threaticon->SetColor(g_tweakGuiColors->GetThreatIconColor());
  x18_threatIconXf = x5c_basewidget_threaticon->GetLocalTransform();

  x6c_energybart01_threatbar->SetFilledColor(g_tweakGuiColors->GetThreatBarFilled());
  x6c_energybart01_threatbar->SetShadowColor(g_tweakGuiColors->GetThreatBarShadow());
  x6c_energybart01_threatbar->SetEmptyColor(g_tweakGuiColors->GetThreatBarEmpty());
  x6c_energybart01_threatbar->SetCoordFunc(CoordFuncs[size_t(hudType)]);
  x6c_energybart01_threatbar->SetTesselation(hudType == EHudType::Combat ? 1.f : 0.1f);
  x6c_energybart01_threatbar->SetMaxEnergy(g_tweakGui->GetThreatRange());
  x6c_energybart01_threatbar->SetFilledDrainSpeed(9999.f);
  x6c_energybart01_threatbar->SetShadowDrainSpeed(9999.f);
  x6c_energybart01_threatbar->SetShadowDrainDelay(0.f);
  x6c_energybart01_threatbar->SetIsAlwaysResetTimer(false);

  if (x68_textpane_threatwarning) {
    x68_textpane_threatwarning->TextSupport().SetFontColor(g_tweakGuiColors->GetThreatWarningFont());
    x68_textpane_threatwarning->TextSupport().SetOutlineColor(g_tweakGuiColors->GetThreatWarningOutline());
  }
}

void CHudThreatInterface::SetThreatDistance(float threatDist) { x10_threatDist = threatDist; }

void CHudThreatInterface::SetIsVisibleDebug(bool v) {
  x54_24_visibleDebug = v;
  UpdateVisibility();
}

void CHudThreatInterface::SetIsVisibleGame(bool v) {
  x54_25_visibleGame = v;
  UpdateVisibility();
}

void CHudThreatInterface::UpdateVisibility() {
  bool vis = x54_24_visibleDebug && x54_25_visibleGame;
  x58_basewidget_threatstuff->SetVisibility(vis, ETraversalMode::Children);
  if (vis)
    SetThreatDistance(0.f);
}

void CHudThreatInterface::Update(float dt) {
  zeus::CColor warningColor = zeus::CColor::lerp(g_tweakGuiColors->GetThreatIconColor(),
                                                 g_tweakGuiColors->GetThreatIconWarningColor(), x50_warningColorLerp);

  float maxThreatEnergy = g_tweakGui->GetThreatRange();
  if (x70_textpane_threatdigits) {
    if (x10_threatDist < maxThreatEnergy) {
      x70_textpane_threatdigits->SetIsVisible(true);
      x70_textpane_threatdigits->TextSupport().SetText(fmt::format(FMT_STRING("{:01.1f}"), std::max(0.f, x10_threatDist)));
    } else {
      x70_textpane_threatdigits->SetIsVisible(false);
    }
  }

  if (x54_26_hasArrows) {
    if (x14_arrowTimer > 0.f) {
      x60_model_threatarrowup->SetIsVisible(true);
      x14_arrowTimer = std::max(0.f, x14_arrowTimer - dt);
      zeus::CColor color = warningColor;
      color.a() = x14_arrowTimer / g_tweakGui->GetMissileArrowVisTime();
      x60_model_threatarrowup->SetColor(color);
      x64_model_threatarrowdown->SetIsVisible(false);
    } else if (x14_arrowTimer < 0.f) {
      x64_model_threatarrowdown->SetIsVisible(true);
      x14_arrowTimer = std::min(0.f, x14_arrowTimer + dt);
      zeus::CColor color = warningColor;
      color.a() = -x14_arrowTimer / g_tweakGui->GetMissileArrowVisTime();
      x64_model_threatarrowdown->SetColor(color);
      x60_model_threatarrowup->SetIsVisible(false);
    } else {
      x60_model_threatarrowup->SetIsVisible(false);
      x64_model_threatarrowdown->SetIsVisible(false);
    }
  }

  if (x10_threatDist <= maxThreatEnergy) {
    float tmp = x10_threatDist - (maxThreatEnergy - x6c_energybart01_threatbar->GetSetEnergy());
    if (tmp < -0.01f)
      x14_arrowTimer = g_tweakGui->GetMissileArrowVisTime();
    else if (tmp > 0.01f)
      x14_arrowTimer = -g_tweakGui->GetMissileArrowVisTime();
  } else {
    x14_arrowTimer = 0.f;
  }

  if (x10_threatDist <= maxThreatEnergy) {
    x6c_energybart01_threatbar->SetCurrEnergy(x6c_energybart01_threatbar->GetMaxEnergy() - x10_threatDist,
                                              CAuiEnergyBarT01::ESetMode::Normal);
    x5c_basewidget_threaticon->SetColor(warningColor);
  } else {
    x6c_energybart01_threatbar->SetCurrEnergy(0.f, CAuiEnergyBarT01::ESetMode::Normal);
    x5c_basewidget_threaticon->SetColor(g_tweakGuiColors->GetThreatIconSafeColor());
  }

  if (x54_27_notXRay) {
    x5c_basewidget_threaticon->SetLocalTransform(
        x18_threatIconXf * zeus::CTransform::Translate(0.f, 0.f,
                                                       std::max(0.f, maxThreatEnergy - x10_threatDist) *
                                                           IconTranslateRanges[size_t(x4_hudType)] / maxThreatEnergy));
  }

  if (x68_textpane_threatwarning) {
    if (x6c_energybart01_threatbar->GetActualFraction() > g_tweakGui->GetThreatWarningFraction())
      x68_textpane_threatwarning->SetIsVisible(true);
    else
      x68_textpane_threatwarning->SetIsVisible(false);

    EThreatStatus newStatus;
    if (maxThreatEnergy == x6c_energybart01_threatbar->GetSetEnergy())
      newStatus = EThreatStatus::Damage;
    else if (x6c_energybart01_threatbar->GetActualFraction() > g_tweakGui->GetThreatWarningFraction())
      newStatus = EThreatStatus::Warning;
    else
      newStatus = EThreatStatus::Normal;

    if (x4c_threatStatus != newStatus) {
      std::u16string string;
      if (newStatus == EThreatStatus::Warning)
        string = g_MainStringTable->GetString(10);
      else if (newStatus == EThreatStatus::Damage)
        string = g_MainStringTable->GetString(11);

      x68_textpane_threatwarning->TextSupport().SetText(string);

      if (x4c_threatStatus == EThreatStatus::Normal && newStatus == EThreatStatus::Warning)
        CSfxManager::SfxStart(SFXui_threat_warning, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      else if (newStatus == EThreatStatus::Damage)
        CSfxManager::SfxStart(SFXui_threat_damage, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

      x4c_threatStatus = newStatus;
    }
  }

  float oldX8 = x8_damagePulseTimer;
  x8_damagePulseTimer = std::fmod(x8_damagePulseTimer + dt, 0.5f);
  if (x8_damagePulseTimer < 0.25f)
    xc_damagePulse = x8_damagePulseTimer / 0.25f;
  else
    xc_damagePulse = (0.5f - x8_damagePulseTimer) / 0.25f;

  if (x4c_threatStatus == EThreatStatus::Damage && x8_damagePulseTimer < oldX8)
    CSfxManager::SfxStart(SFXui_threat_damage, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

  if (x68_textpane_threatwarning) {
    if (x4c_threatStatus != EThreatStatus::Normal) {
      x48_warningLerpAlpha = std::min(x48_warningLerpAlpha + 2.f * dt, 1.f);
      zeus::CColor color = zeus::skWhite;
      color.a() = x48_warningLerpAlpha * xc_damagePulse;
      x68_textpane_threatwarning->SetColor(color);
    } else {
      x48_warningLerpAlpha = std::max(0.f, x48_warningLerpAlpha - 2.f * dt);
      zeus::CColor color = zeus::skWhite;
      color.a() = x48_warningLerpAlpha * xc_damagePulse;
      x68_textpane_threatwarning->SetColor(color);
    }
    if (x68_textpane_threatwarning->GetGeometryColor().a() > 0.f)
      x68_textpane_threatwarning->SetIsVisible(true);
    else
      x68_textpane_threatwarning->SetIsVisible(false);
  }

  if (x4c_threatStatus == EThreatStatus::Damage)
    x50_warningColorLerp = std::min(x50_warningColorLerp + 2.f * dt, 1.f);
  else
    x50_warningColorLerp = std::max(0.f, x50_warningColorLerp - 2.f * dt);
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudThreatInterface::CombatThreatBarCoordFunc(float t) {
  const float z = IconTranslateRanges[size_t(EHudType::Combat)] * t;
  return {zeus::CVector3f(-0.3f, 0.f, z), zeus::CVector3f(0.f, 0.f, z)};
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudThreatInterface::XRayThreatBarCoordFunc(float t) {
  const float theta = 0.8f * (t - 0.5f);
  const float x = -9.55f * std::cos(theta);
  const float z = 9.55f * std::sin(theta);
  return {zeus::CVector3f(0.4f + x, 0.f, z), zeus::CVector3f(x, 0.f, z)};
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudThreatInterface::ThermalThreatBarCoordFunc(float t) {
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

  return {zeus::CVector3f(0.1f, 0.f, b), zeus::CVector3f(0.5f * c + 0.1f, 0.f, b)};
}

} // namespace urde
