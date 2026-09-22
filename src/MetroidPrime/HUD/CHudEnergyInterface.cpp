#include "MetroidPrime/HUD/CHudEnergyInterface.hpp"
#include "MetroidPrime/HUD/CHudBossEnergyInterface.hpp"

#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"

#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CAuiMeter.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "rstl/math.hpp"
#include <stdio.h>

static const char skEnergyGroupWidgetName[] = "basewidget_energystuff";
static const char skEnergyDigitsWidgetName[] = "textpane_energydigits";
static const char skEnergyTanksWidgetName[] = "meter_energytanks";
static const char skEnergyWarningWidgetName[] = "textpane_energywarning";
static const char skEnergyBarWidgetName[] = "energybart01_energybar";
static const char skEnergyDecoWidgetName[] = "basewidget_energydeco0";
static const char skEnemyEnergyGroupWidgetName[] = "basewidget_bossenergystuff";
static const char skEnemyEnergyBarWidgetName[] = "energybart01_bossbar";
static const CAuiEnergyBarT01::FCoordFunc skEnergyCoordFuncs[] = {
    CHudEnergyInterface::CombatEnergyCoordFunc, CHudEnergyInterface::CombatEnergyCoordFunc,
    CHudEnergyInterface::XRayEnergyCoordFunc, CHudEnergyInterface::ThermalEnergyCoordFunc,
    CHudEnergyInterface::BallEnergyCoordFunc};
static const float skEnergyTesselations[] = {0.2f, 0.2f, 0.1f, 0.2f, 1.f};

rstl::pair< CVector3f, CVector3f > CHudEnergyInterface::CombatEnergyCoordFunc(float t) {
  const float theta = -0.15882353f + 0.46764705f * t;
  const float x = 17.f * CMath::FastSinR(theta);
  const float y = 17.f * CMath::FastCosR(theta) + -17.f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, y, 0.4f), CVector3f(x, y, 0.f));
}

rstl::pair< CVector3f, CVector3f > CHudBossEnergyInterface::BossEnergyCoordFunc(float t) {
  const float x = 9.25f * t - 4.625f;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, 0.f), CVector3f(x, 0.f, 0.4f));
}

rstl::pair< CVector3f, CVector3f > CHudEnergyInterface::BallEnergyCoordFunc(float t) {
  const float x = 1.6666f * t;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, 0.f),
                                            CVector3f(x, 0.f, 0.088887997f));
}

rstl::pair< CVector3f, CVector3f > CHudEnergyInterface::ThermalEnergyCoordFunc(float t) {
  const float x = 8.1663399f * t;
  return rstl::pair< CVector3f, CVector3f >(CVector3f(x, 0.f, 0.f), CVector3f(x, 0.f, 0.4355512f));
}

rstl::pair< CVector3f, CVector3f > CHudEnergyInterface::XRayEnergyCoordFunc(float t) {
  const float theta = 1.8207964f - 0.69f * t;
  const float x = CMath::FastCosR(theta);
  const float z = CMath::FastSinR(theta);
  return rstl::pair< CVector3f, CVector3f >(CVector3f(9.4f * x, 0.f, 9.4f * z),
                                            CVector3f(9.f * x, 0.f, 9.f * z));
}

CHudEnergyInterface::CHudEnergyInterface(CGuiFrame& hud, float energy, int totalTanks,
                                         int filledTanks, bool energyLow, EHudType type)
: x0_hudType(type)
, x4_energyLowFader(0.f)
, x8_flashMag(0.f)
, xc_tankEnergy(energy)
, x10_totalEnergyTanks(totalTanks)
, x14_numTanksFilled(filledTanks)
, x18_cachedBarEnergy(0.f)
, x1c_24_(true)
, x1c_25_(true)
, x1c_26_barDirty(true)
, x1c_27_energyLow(energyLow) {
  x20_textpane_energydigits =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skEnergyDigitsWidgetName)));
  x24_meter_energytanks =
      static_cast< CAuiMeter* >(hud.FindWidget(rstl::string_l(skEnergyTanksWidgetName)));
  x28_textpane_energywarning =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skEnergyWarningWidgetName)));
  x2c_energybart01_energybar =
      static_cast< CAuiEnergyBarT01* >(hud.FindWidget(rstl::string_l(skEnergyBarWidgetName)));

  const CTweakGuiColors::SPerVisorColors& colors = gpTweakGuiColors->GetVisorColors(x0_hudType);
  x2c_energybart01_energybar->SetCoordFunc(skEnergyCoordFuncs[x0_hudType]);
  x2c_energybart01_energybar->SetTesselation(skEnergyTesselations[x0_hudType]);
  x20_textpane_energydigits->TextSupport().SetFontColor(colors.x14_energyDigitsFont);
  x20_textpane_energydigits->TextSupport().SetOutlineColor(colors.x18_energyDigitsOutline);
  x2c_energybart01_energybar->SetMaxEnergy(CPlayerState::GetBaseHealthCapacity());
  x2c_energybart01_energybar->SetFilledColor(colors.x0_energyBarFilled);
  x2c_energybart01_energybar->SetShadowColor(colors.x8_energyBarShadow);
  x2c_energybart01_energybar->SetEmptyColor(colors.x4_energyBarEmpty);
  x2c_energybart01_energybar->SetFilledDrainSpeed(gpTweakGui->GetEnergyBarFilledSpeed());
  x2c_energybart01_energybar->SetShadowDrainSpeed(gpTweakGui->GetEnergyBarShadowSpeed());
  x2c_energybart01_energybar->SetShadowDrainDelay(gpTweakGui->GetEnergyBarDrainDelay());
  x2c_energybart01_energybar->SetIsAlwaysResetTimer(gpTweakGui->GetEnergyBarAlwaysResetDelay());
  x24_meter_energytanks->SetMaxCapacity(14);
  if (x28_textpane_energywarning) {
    x28_textpane_energywarning->TextSupport().SetFontColor(
        gpTweakGuiColors->GetEnergyWarningFont());
    x28_textpane_energywarning->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetEnergyWarningOutline());
    const rstl::wstring text =
        x1c_27_energyLow ? rstl::wstring_l(gpStringTable->GetString(9)) : rstl::wstring_l(L"");
    x28_textpane_energywarning->TextSupport().SetText(text);
  }
  const CColor& tankFilled = colors.xc_energyTankFilled;
  const CColor& tankEmpty = colors.x10_energyTankEmpty;
  for (int i = 0; i < 14; ++i) {
    CGuiWidget* workerGroup = x24_meter_energytanks->GetWorkerWidget(i);
    CGuiGroup* group = static_cast< CGuiGroup* >(workerGroup);
    CGuiWidget* workers[2];
    workers[0] = group->GetWorkerWidget(0);
    workers[1] = group->GetWorkerWidget(1);
    if (workers[0]) {
      workers[0]->SetColor(tankFilled);
    }
    if (workers[1]) {
      workers[1]->SetColor(tankEmpty);
    }
  }
}

void CHudEnergyInterface::SetCurrEnergy(float energy, bool wrapped) {
  xc_tankEnergy = energy;
  x2c_energybart01_energybar->SetCurrEnergy(
      energy, energy == 0.f
                  ? CAuiEnergyBarT01::kSM_Instant
                  : (wrapped ? CAuiEnergyBarT01::kSM_Wrapped : CAuiEnergyBarT01::kSM_Normal));
}

void CHudEnergyInterface::SetNumTotalEnergyTanks(int tanks) {
  x10_totalEnergyTanks = tanks;
  x24_meter_energytanks->SetCapacity(tanks);
}

void CHudEnergyInterface::SetNumFilledEnergyTanks(int tanks) {
  x14_numTanksFilled = tanks;
  x24_meter_energytanks->SetCurrValue(tanks);
}

void CHudEnergyInterface::SetFlashMagnitude(float mag) {
  x8_flashMag = CMath::Clamp(0.f, mag, 1.f);
}

void CHudEnergyInterface::SetEnergyLow(bool low) {
  if (low != x1c_27_energyLow) {
    const rstl::wstring text =
        low ? rstl::wstring_l(gpStringTable->GetString(9)) : rstl::wstring_l(L"");
    if (x28_textpane_energywarning) {
      x28_textpane_energywarning->TextSupport().SetText(text);
    }
    if (low) {
      CSfxManager::SfxStart(0x57d);
    }
    x1c_27_energyLow = low;
  }
}

void CHudEnergyInterface::Update(float dt, float energyLowPulse) {
  if (x28_textpane_energywarning) {
    if (x1c_27_energyLow) {
      x4_energyLowFader = rstl::min_val(1.f, x4_energyLowFader + 2.f * dt);
      x28_textpane_energywarning->SetColor(
          CColor::White().WithAlphaOf(x4_energyLowFader * energyLowPulse));
    } else {
      x4_energyLowFader = rstl::max_val(0.f, x4_energyLowFader - 2.f * dt);
      x28_textpane_energywarning->SetColor(
          CColor::White().WithAlphaOf(x4_energyLowFader * energyLowPulse));
    }
    if (x28_textpane_energywarning->GetModifiedColor().GetAlphau8()) {
      x28_textpane_energywarning->SetIsVisible(true);
    } else {
      x28_textpane_energywarning->SetIsVisible(false);
    }
  }

  const float barEnergy = x2c_energybart01_energybar->GetLaggedEnergy();
  if (barEnergy != x18_cachedBarEnergy || x1c_26_barDirty) {
    x1c_26_barDirty = false;
    x18_cachedBarEnergy = barEnergy;
    char digits[4];
#if NONMATCHING
    snprintf(digits, sizeof(digits), "%02d",
            static_cast< int >(CMath::ModF(barEnergy, CPlayerState::GetEnergyTankCapacity())));
#else
    sprintf(digits, "%02d",
            static_cast< int >(CMath::ModF(barEnergy, CPlayerState::GetEnergyTankCapacity())));
#endif
    x20_textpane_energydigits->TextSupport().SetText(rstl::string(digits));
  }

  const CTweakGuiColors::SPerVisorColors& colors = gpTweakGuiColors->GetVisorColors(x0_hudType);
  const CColor barEmpty = colors.x4_energyBarEmpty;
  const CColor barFilled = colors.x0_energyBarFilled;
  const CColor barShadow = colors.x8_energyBarShadow;
  const CColor lowEmpty = gpTweakGuiColors->GetEnergyBarEmptyLowEnergy();
  const CColor lowFilled = gpTweakGuiColors->GetEnergyBarFilledLowEnergy();
  const CColor lowShadow = gpTweakGuiColors->GetEnergyBarShadowLowEnergy();
  const CColor emptyColor = x1c_27_energyLow ? lowEmpty : barEmpty;
  const CColor filledColor = x1c_27_energyLow ? lowFilled : barFilled;
  const CColor shadowColor = x1c_27_energyLow ? lowShadow : barShadow;
  CColor useFillColor =
      CColor::Lerp(filledColor, gpTweakGuiColors->GetEnergyBarFlashColor(), x8_flashMag);
  if (x1c_27_energyLow) {
    const CColor pulseColor(1.f, 0.8f, 0.4f, 1.f);
    useFillColor = CColor::Lerp(useFillColor, pulseColor, energyLowPulse);
  }
  x2c_energybart01_energybar->SetFilledColor(useFillColor);
  x2c_energybart01_energybar->SetShadowColor(shadowColor);
  x2c_energybart01_energybar->SetEmptyColor(emptyColor);
}

CHudBossEnergyInterface::CHudBossEnergyInterface(CGuiFrame& frame)
: x0_alpha(1.f), x4_fader(0.f), x8_curEnergy(0.f), xc_maxEnergy(0.f), x10_24_visible(false) {
  x14_basewidget_bossenergystuff = frame.FindWidget(skEnemyEnergyGroupWidgetName);
  x18_energybart01_bossbar =
      static_cast< CAuiEnergyBarT01* >(frame.FindWidget(skEnemyEnergyBarWidgetName));
  x1c_textpane_boss = static_cast< CGuiTextPane* >(frame.FindWidget("textpane_boss"));
  x18_energybart01_bossbar->SetCoordFunc(BossEnergyCoordFunc);
  x18_energybart01_bossbar->SetTesselation(0.2f);
  const CTweakGuiColors::SPerVisorColors& colors = gpTweakGuiColors->GetVisorColors(0);
  x18_energybart01_bossbar->SetFilledColor(colors.x0_energyBarFilled);
  x18_energybart01_bossbar->SetShadowColor(colors.x8_energyBarShadow);
  x18_energybart01_bossbar->SetEmptyColor(colors.x4_energyBarEmpty);
}

void CHudBossEnergyInterface::SetBossParams(const bool visible, const rstl::wstring& name,
                                            float energy, float maxEnergy) {
  x10_24_visible = visible;
  if (visible) {
    x18_energybart01_bossbar->SetFilledDrainSpeed(1000.f * (0.001f * maxEnergy));
    x18_energybart01_bossbar->SetCurrEnergy(energy, CAuiEnergyBarT01::kSM_Normal);
    x18_energybart01_bossbar->SetMaxEnergy(maxEnergy);
    x1c_textpane_boss->TextSupport().SetText(name);
  }
  x8_curEnergy = energy;
  xc_maxEnergy = maxEnergy;
}

void CHudBossEnergyInterface::SetAlpha(float alpha) { x0_alpha = alpha; }

void CHudBossEnergyInterface::Update(float dt) {
  if (x10_24_visible) {
    x4_fader = rstl::min_val(1.f, x4_fader + dt);
  } else {
    x4_fader = rstl::max_val(0.f, x4_fader - dt);
  }
  if (x4_fader > 0.f) {
    x14_basewidget_bossenergystuff->SetColor(CColor::White().WithAlphaOf(x0_alpha * x4_fader));
    x14_basewidget_bossenergystuff->SetVisibility(true, kTM_Children);
  } else {
    x14_basewidget_bossenergystuff->SetVisibility(false, kTM_Children);
  }
}
