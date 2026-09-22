#include "MetroidPrime/HUD/CHudMissileInterface.hpp"

#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

#include <float.h>
#include <stdio.h>

static const char skMissileGroupWidgetName[] = "basewidget_missileicon";
static const char skMissileDigitsWidgetName[] = "textpane_missiledigits";
static const char skMissileBarWidgetName[] = "energybart01_missilebar";
static const char skMissileWarningWidgetName[] = "textpane_missilewarning";
static const char skMissileArrowUpWidgetName[] = "model_missilearrowup";
static const char skMissileArrowDownWidgetName[] = "model_missilearrowdown";
static const char skMissileIconWidgetName[] = "basewidget_missileicon";
static const char skMissileGroupWidgetNameXRay[] = "basewidget_missilestuff";
static const char skMissileMeterTopNameXRay[] = "meter_mistop";
static const char skMissileMeterBottomNameXRay[] = "meter_misbot";
static const float skIconTranslateRanges[] = {6.05f, 0.f, 0.f, 8.4f, 0.f};
static const CAuiEnergyBarT01::FCoordFunc skMissileCoordFuncs[] = {
    CHudMissileInterface::CombatMissileBarCoordFunc, nullptr,
    CHudMissileInterface::XRayMissileBarCoordFunc, CHudMissileInterface::ThermalMissileBarCoordFunc,
    nullptr};

rstl::pair< CVector3f, CVector3f > CHudMissileInterface::CombatMissileBarCoordFunc(float t) {
  const float z = t * skIconTranslateRanges[kHT_Combat];
  return rstl::pair< CVector3f, CVector3f >(CVector3f(0.f, 0.f, z), CVector3f(0.3f, 0.f, z));
}

rstl::pair< CVector3f, CVector3f > CHudMissileInterface::ThermalMissileBarCoordFunc(float t) {
  const float range = skIconTranslateRanges[kHT_Thermal];
  const float bevel = 0.08f * range;
  const float z = t * range;
  float width;
  if (z < bevel) {
    width = z / bevel;
  } else if (z < range - bevel) {
    width = 1.f;
  } else {
    width = 1.f - (z - (range - bevel)) / bevel;
  }
  return rstl::pair< CVector3f, CVector3f >(CVector3f(-0.5f * width - 0.1f, 0.f, z),
                                            CVector3f(-0.1f, 0.f, z));
}

rstl::pair< CVector3f, CVector3f > CHudMissileInterface::XRayMissileBarCoordFunc(float t) {
  const float theta = 0.8f * (t - 0.5f);
  const float x = 9.55f * CMath::FastCosR(theta);
  const float z = 9.55f * CMath::FastSinR(theta);
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x - 0.4f, 0.f, z), CVector3f(x, 0.f, z));
}

CHudMissileInterface::EInventoryStatus CHudMissileInterface::GetMissileInventoryStatus() {
  if (x64_energybart01_missilebar->GetActualEnergy() == 0.f) {
    return kIS_Depleted;
  }
  const float fraction = x64_energybart01_missilebar->GetActualFraction();
  return fraction < gpTweakGui->GetMissileWarningFraction() ? kIS_Warning : kIS_Normal;
}

CHudMissileInterface::CHudMissileInterface(CGuiFrame& hud, int capacity, int missiles,
                                           float chargeFactor, bool active, EHudType type,
                                           const CStateManager& mgr)
: x0_hudType(type)
, x4_missileCapacity(capacity)
, x8_numMissiles(missiles)
, xc_arrowTimer(0.f)
, x10_missileIconXf(CTransform4f::Identity())
, x40_missileWarningAlpha(0.f)
, x44_latestStatus(kIS_Normal)
, x48_missileWarningPulse(0.f)
, x4c_chargeBeamFactor(chargeFactor)
, x50_missileIconAltDeplete(0.f)
, x54_missileIconIncrement(0.f)
, x58_24_missilesActive(active)
, x58_25_visibleDebug(true)
, x58_26_visibleGame(true) {
  x5c_basewidget_missileicon = hud.FindWidget(rstl::string_l(skMissileGroupWidgetName));
  x60_textpane_missiledigits =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skMissileDigitsWidgetName)));
  x64_energybart01_missilebar =
      static_cast< CAuiEnergyBarT01* >(hud.FindWidget(rstl::string_l(skMissileBarWidgetName)));
  x68_textpane_missilewarning =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skMissileWarningWidgetName)));
  x6c_model_missilearrowup =
      static_cast< CGuiModel* >(hud.FindWidget(rstl::string_l(skMissileArrowUpWidgetName)));
  x70_model_missilearrowdown =
      static_cast< CGuiModel* >(hud.FindWidget(rstl::string_l(skMissileArrowDownWidgetName)));
  x74_basewidget_missileicon = hud.FindWidget(rstl::string_l(skMissileIconWidgetName));
  x58_27_hasArrows = x6c_model_missilearrowup && x70_model_missilearrowdown;
  x58_28_notXRay = type != kHT_XRay;
  x10_missileIconXf = x74_basewidget_missileicon->GetO2PTransform();
  x60_textpane_missiledigits->TextSupport().SetFontColor(gpTweakGuiColors->GetMissileDigitsFont());
  x60_textpane_missiledigits->TextSupport().SetOutlineColor(
      gpTweakGuiColors->GetMissileDigitsOutline());
  x74_basewidget_missileicon->SetColor(gpTweakGuiColors->GetMissileIconColorInactive());
  x64_energybart01_missilebar->SetEmptyColor(gpTweakGuiColors->GetMissileBarEmpty());
  x64_energybart01_missilebar->SetFilledColor(gpTweakGuiColors->GetMissileBarFilled());
  x64_energybart01_missilebar->SetShadowColor(gpTweakGuiColors->GetMissileBarShadow());
  x64_energybart01_missilebar->SetCoordFunc(skMissileCoordFuncs[x0_hudType]);
  x64_energybart01_missilebar->SetTesselation(type == kHT_Combat ? 1.f : 0.1f);
  x64_energybart01_missilebar->SetMaxEnergy(5.f);
  x64_energybart01_missilebar->SetFilledDrainSpeed(gpTweakGui->GetEnergyBarFilledSpeed());
  x64_energybart01_missilebar->SetShadowDrainSpeed(gpTweakGui->GetEnergyBarShadowSpeed());
  x64_energybart01_missilebar->SetShadowDrainDelay(gpTweakGui->GetEnergyBarDrainDelay());
  x64_energybart01_missilebar->SetIsAlwaysResetTimer(true);
  if (x68_textpane_missilewarning) {
    x68_textpane_missilewarning->TextSupport().SetFontColor(
        gpTweakGuiColors->GetMissileWarningFont());
    x68_textpane_missilewarning->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetMissileWarningOutline());
  }
  SetNumMissiles(x8_numMissiles, mgr);
  x44_latestStatus = GetMissileInventoryStatus();
}

void CHudMissileInterface::SetMissileCapacity(int capacity) { x4_missileCapacity = capacity; }

void CHudMissileInterface::SetNumMissiles(int missiles, const CStateManager& mgr) {
  char digits[4];
#if NONMATCHING
  snprintf(digits, sizeof(digits), "%3d", CMath::Clamp(0, missiles, 999));
#else
  sprintf(digits, "%3d", CMath::Clamp(0, missiles, 999));
#endif
  x60_textpane_missiledigits->TextSupport().SetText(rstl::string(digits));
  if (x8_numMissiles < missiles) {
    xc_arrowTimer = gpTweakGui->GetMissileArrowVisTime();
    x54_missileIconIncrement = -FLT_EPSILON;
  } else if (x8_numMissiles > missiles) {
    xc_arrowTimer = -1.f * gpTweakGui->GetMissileArrowVisTime();
  }
  if (mgr.GetPlayerState()->GetMissileCostForAltAttack() + missiles <= x8_numMissiles) {
    x50_missileIconAltDeplete = 1.f;
  }
  x8_numMissiles = missiles;
}

void CHudMissileInterface::SetChargeBeamFactor(float factor) { x4c_chargeBeamFactor = factor; }

void CHudMissileInterface::SetIsMissilesActive(bool active) { x58_24_missilesActive = active; }

void CHudMissileInterface::SetIsVisibleGame(bool visible, const CStateManager& mgr) {
  x58_26_visibleGame = visible;
  UpdateVisibility(mgr);
}

void CHudMissileInterface::UpdateVisibility(const CStateManager& mgr) {
  const bool visible = x58_26_visibleGame && x58_25_visibleDebug;
  x5c_basewidget_missileicon->SetVisibility(visible, kTM_Children);
  x64_energybart01_missilebar->SetVisibility(visible, kTM_Children);
  if (visible) {
    Update(0.f, mgr);
  }
}

void CHudMissileInterface::Update(float dt, const CStateManager& mgr) {
  const CPlayerState& playerState = *mgr.GetPlayerState();
  if (x4_missileCapacity < 1) {
    x5c_basewidget_missileicon->SetIsVisible(false);
  } else {
    x5c_basewidget_missileicon->SetIsVisible(true);
  }
  if (x54_missileIconIncrement < 0.f) {
    x54_missileIconIncrement -= 3.f * dt;
    if (x54_missileIconIncrement <= -1.f) {
      x54_missileIconIncrement = 1.f;
    }
  } else if (x54_missileIconIncrement > 0.f) {
    x54_missileIconIncrement = rstl::max_val(0.f, x54_missileIconIncrement - dt);
  }
  const CColor& inactiveColor = gpTweakGuiColors->GetMissileIconColorInactive();
  const uchar flashValue = CCast::ToUint8(255.f * CMath::AbsF(x54_missileIconIncrement));
  const CColor flash(flashValue, flashValue, flashValue, flashValue);
  const CColor addColor = CColor::Modulate(flash, gpTweakGuiColors->GetMissileIconColorActive());
  if (x50_missileIconAltDeplete > 0.f) {
    CColor color = CColor::Lerp(inactiveColor, gpTweakGuiColors->GetMissileIconColorDepleteAlt(),
                                x50_missileIconAltDeplete);
    color = CColor::Add(color, addColor);
    x74_basewidget_missileicon->SetColor(color);
  } else if (x4c_chargeBeamFactor > 0.f) {
    const float factor =
        rstl::min_val(1.f, x4c_chargeBeamFactor / CPlayerState::GetMissileComboChargeFactor());
    if (x8_numMissiles >= playerState.GetMissileCostForAltAttack()) {
      CColor color =
          CColor::Lerp(inactiveColor, gpTweakGuiColors->GetMissileIconColorChargedCanAlt(), factor);
      color = CColor::Add(color, addColor);
      x74_basewidget_missileicon->SetColor(color);
    } else {
      CColor color =
          CColor::Lerp(inactiveColor, gpTweakGuiColors->GetMissileIconColorChargedNoAlt(), factor);
      color = CColor::Add(color, addColor);
      x74_basewidget_missileicon->SetColor(color);
    }
  } else if (x58_24_missilesActive) {
    const CColor color = CColor::Add(gpTweakGuiColors->GetMissileIconColorActive(), addColor);
    x74_basewidget_missileicon->SetColor(color);
  } else {
    const CColor color = CColor::Add(gpTweakGuiColors->GetMissileIconColorInactive(), addColor);
    x74_basewidget_missileicon->SetColor(color);
  }
  x50_missileIconAltDeplete = rstl::max_val(0.f, x50_missileIconAltDeplete - dt);
  x64_energybart01_missilebar->SetMaxEnergy(x4_missileCapacity);
  x64_energybart01_missilebar->SetCurrEnergy(x8_numMissiles, CAuiEnergyBarT01::kSM_Normal);
  if (x58_28_notXRay) {
    x74_basewidget_missileicon->SetO2PTransform(
        x10_missileIconXf *
        CTransform4f::Translate(CVector3f(0.f, 0.f,
                                          x8_numMissiles * skIconTranslateRanges[x0_hudType] /
                                              float(x4_missileCapacity))));
  }
  const CColor& activeColor = gpTweakGuiColors->GetMissileIconColorActive();
  if (x58_27_hasArrows) {
    if (xc_arrowTimer > 0.f) {
      xc_arrowTimer = rstl::max_val(0.f, xc_arrowTimer - dt);
      const float alpha = xc_arrowTimer / gpTweakGui->GetMissileArrowVisTime();
      x6c_model_missilearrowup->SetIsVisible(true);
      x6c_model_missilearrowup->SetColor(activeColor.WithAlphaModulatedBy(alpha));
      x70_model_missilearrowdown->SetIsVisible(false);
    } else if (xc_arrowTimer < 0.f) {
      xc_arrowTimer = rstl::min_val(0.f, xc_arrowTimer + dt);
      const float alpha = -xc_arrowTimer / gpTweakGui->GetMissileArrowVisTime();
      x70_model_missilearrowdown->SetIsVisible(true);
      x70_model_missilearrowdown->SetColor(activeColor.WithAlphaModulatedBy(alpha));
      x6c_model_missilearrowup->SetIsVisible(false);
    } else {
      x6c_model_missilearrowup->SetIsVisible(false);
      x70_model_missilearrowdown->SetIsVisible(false);
    }
  }
  if (x68_textpane_missilewarning) {
    const EInventoryStatus status = GetMissileInventoryStatus();
    if (status != x44_latestStatus) {
      const rstl::wstring text =
          status == kIS_Warning    ? rstl::wstring_l(gpStringTable->GetString(12))
          : status == kIS_Depleted ? rstl::wstring_l(gpStringTable->GetString(13))
                                   : rstl::wstring_l(L"");
      x68_textpane_missilewarning->TextSupport().SetText(text);
      if (x44_latestStatus == kIS_Normal && status == kIS_Warning) {
        CSfxManager::SfxStart(0x575);
        x48_missileWarningPulse = gpTweakGui->GetMissileWarningPulseTime();
      } else if (status == kIS_Depleted) {
        CSfxManager::SfxStart(0x575);
        x48_missileWarningPulse = gpTweakGui->GetMissileWarningPulseTime();
      }
      x44_latestStatus = status;
    }
    x48_missileWarningPulse = rstl::max_val(0.f, x48_missileWarningPulse - dt);
    const float warningPulse = rstl::min_val(1.f, x48_missileWarningPulse);
    if (x44_latestStatus != kIS_Normal) {
      x40_missileWarningAlpha = rstl::min_val(1.f, x40_missileWarningAlpha + 2.f * dt);
    } else {
      x40_missileWarningAlpha = rstl::max_val(0.f, x40_missileWarningAlpha - 2.f * dt);
    }
    float pulse = CMath::AbsF(CMath::ModF(CGraphics::GetSecondsMod900(), 0.5f));
    const float alpha =
        x40_missileWarningAlpha * (pulse < 0.25f ? pulse / 0.25f : (0.5f - pulse) / 0.25f);
    const float warningAlpha = warningPulse * alpha;
    x68_textpane_missilewarning->SetColor(CColor::White().WithAlphaOf(warningAlpha));
    if (x68_textpane_missilewarning->GetModifiedColor().GetAlphau8()) {
      x68_textpane_missilewarning->SetIsVisible(true);
    } else {
      x68_textpane_missilewarning->SetIsVisible(false);
    }
  }
}
