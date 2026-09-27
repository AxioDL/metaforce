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
: mHudType(type)
, mEnergyLowFader(0.f)
, mFlashMag(0.f)
, mTankEnergy(energy)
, mTotalEnergyTanks(totalTanks)
, mNumTanksFilled(filledTanks)
, mCachedBarEnergy(0.f)
, x1c_24_(true)
, x1c_25_(true)
, mBarDirty(true)
, mEnergyLow(energyLow) {
  mTextpane_energydigits =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skEnergyDigitsWidgetName)));
  mMeter_energytanks =
      static_cast< CAuiMeter* >(hud.FindWidget(rstl::string_l(skEnergyTanksWidgetName)));
  mTextpane_energywarning =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skEnergyWarningWidgetName)));
  mEnergybart01_energybar =
      static_cast< CAuiEnergyBarT01* >(hud.FindWidget(rstl::string_l(skEnergyBarWidgetName)));

  const CTweakGuiColors::SPerVisorColors& colors = gpTweakGuiColors->GetVisorColors(mHudType);
  mEnergybart01_energybar->SetCoordFunc(skEnergyCoordFuncs[mHudType]);
  mEnergybart01_energybar->SetTesselation(skEnergyTesselations[mHudType]);
  mTextpane_energydigits->TextSupport().SetFontColor(colors.mEnergyDigitsFont);
  mTextpane_energydigits->TextSupport().SetOutlineColor(colors.mEnergyDigitsOutline);
  mEnergybart01_energybar->SetMaxEnergy(CPlayerState::GetBaseHealthCapacity());
  mEnergybart01_energybar->SetFilledColor(colors.mEnergyBarFilled);
  mEnergybart01_energybar->SetShadowColor(colors.mEnergyBarShadow);
  mEnergybart01_energybar->SetEmptyColor(colors.mEnergyBarEmpty);
  mEnergybart01_energybar->SetFilledDrainSpeed(gpTweakGui->GetEnergyBarFilledSpeed());
  mEnergybart01_energybar->SetShadowDrainSpeed(gpTweakGui->GetEnergyBarShadowSpeed());
  mEnergybart01_energybar->SetShadowDrainDelay(gpTweakGui->GetEnergyBarDrainDelay());
  mEnergybart01_energybar->SetIsAlwaysResetTimer(gpTweakGui->GetEnergyBarAlwaysResetDelay());
  mMeter_energytanks->SetMaxCapacity(14);
  if (mTextpane_energywarning) {
    mTextpane_energywarning->TextSupport().SetFontColor(
        gpTweakGuiColors->GetEnergyWarningFont());
    mTextpane_energywarning->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetEnergyWarningOutline());
    const rstl::wstring text =
        mEnergyLow ? rstl::wstring_l(gpStringTable->GetString(9)) : rstl::wstring_l(L"");
    mTextpane_energywarning->TextSupport().SetText(text);
  }
  const CColor& tankFilled = colors.mEnergyTankFilled;
  const CColor& tankEmpty = colors.mEnergyTankEmpty;
  for (int i = 0; i < 14; ++i) {
    CGuiWidget* workerGroup = mMeter_energytanks->GetWorkerWidget(i);
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
  mTankEnergy = energy;
  mEnergybart01_energybar->SetCurrEnergy(
      energy, energy == 0.f
                  ? CAuiEnergyBarT01::kSM_Instant
                  : (wrapped ? CAuiEnergyBarT01::kSM_Wrapped : CAuiEnergyBarT01::kSM_Normal));
}

void CHudEnergyInterface::SetNumTotalEnergyTanks(int tanks) {
  mTotalEnergyTanks = tanks;
  mMeter_energytanks->SetCapacity(tanks);
}

void CHudEnergyInterface::SetNumFilledEnergyTanks(int tanks) {
  mNumTanksFilled = tanks;
  mMeter_energytanks->SetCurrValue(tanks);
}

void CHudEnergyInterface::SetFlashMagnitude(float mag) {
  mFlashMag = CMath::Clamp(0.f, mag, 1.f);
}

void CHudEnergyInterface::SetEnergyLow(bool low) {
  if (low != mEnergyLow) {
    const rstl::wstring text =
        low ? rstl::wstring_l(gpStringTable->GetString(9)) : rstl::wstring_l(L"");
    if (mTextpane_energywarning) {
      mTextpane_energywarning->TextSupport().SetText(text);
    }
    if (low) {
      CSfxManager::SfxStart(0x57d);
    }
    mEnergyLow = low;
  }
}

void CHudEnergyInterface::Update(float dt, float energyLowPulse) {
  if (mTextpane_energywarning) {
    if (mEnergyLow) {
      mEnergyLowFader = rstl::min_val(1.f, mEnergyLowFader + 2.f * dt);
      mTextpane_energywarning->SetColor(
          CColor::White().WithAlphaOf(mEnergyLowFader * energyLowPulse));
    } else {
      mEnergyLowFader = rstl::max_val(0.f, mEnergyLowFader - 2.f * dt);
      mTextpane_energywarning->SetColor(
          CColor::White().WithAlphaOf(mEnergyLowFader * energyLowPulse));
    }
    if (mTextpane_energywarning->GetModifiedColor().GetAlphau8()) {
      mTextpane_energywarning->SetIsVisible(true);
    } else {
      mTextpane_energywarning->SetIsVisible(false);
    }
  }

  const float barEnergy = mEnergybart01_energybar->GetLaggedEnergy();
  if (barEnergy != mCachedBarEnergy || mBarDirty) {
    mBarDirty = false;
    mCachedBarEnergy = barEnergy;
    char digits[4];
#if NONMATCHING
    snprintf(digits, sizeof(digits), "%02d",
            static_cast< int >(CMath::ModF(barEnergy, CPlayerState::GetEnergyTankCapacity())));
#else
    sprintf(digits, "%02d",
            static_cast< int >(CMath::ModF(barEnergy, CPlayerState::GetEnergyTankCapacity())));
#endif
    mTextpane_energydigits->TextSupport().SetText(rstl::string(digits));
  }

  const CTweakGuiColors::SPerVisorColors& colors = gpTweakGuiColors->GetVisorColors(mHudType);
  const CColor barEmpty = colors.mEnergyBarEmpty;
  const CColor barFilled = colors.mEnergyBarFilled;
  const CColor barShadow = colors.mEnergyBarShadow;
  const CColor lowEmpty = gpTweakGuiColors->GetEnergyBarEmptyLowEnergy();
  const CColor lowFilled = gpTweakGuiColors->GetEnergyBarFilledLowEnergy();
  const CColor lowShadow = gpTweakGuiColors->GetEnergyBarShadowLowEnergy();
  const CColor emptyColor = mEnergyLow ? lowEmpty : barEmpty;
  const CColor filledColor = mEnergyLow ? lowFilled : barFilled;
  const CColor shadowColor = mEnergyLow ? lowShadow : barShadow;
  CColor useFillColor =
      CColor::Lerp(filledColor, gpTweakGuiColors->GetEnergyBarFlashColor(), mFlashMag);
  if (mEnergyLow) {
    const CColor pulseColor(1.f, 0.8f, 0.4f, 1.f);
    useFillColor = CColor::Lerp(useFillColor, pulseColor, energyLowPulse);
  }
  mEnergybart01_energybar->SetFilledColor(useFillColor);
  mEnergybart01_energybar->SetShadowColor(shadowColor);
  mEnergybart01_energybar->SetEmptyColor(emptyColor);
}

CHudBossEnergyInterface::CHudBossEnergyInterface(CGuiFrame& frame)
: mAlpha(1.f), mFader(0.f), mCurEnergy(0.f), mMaxEnergy(0.f), mVisible(false) {
  mBasewidget_bossenergystuff = frame.FindWidget(skEnemyEnergyGroupWidgetName);
  mEnergybart01_bossbar =
      static_cast< CAuiEnergyBarT01* >(frame.FindWidget(skEnemyEnergyBarWidgetName));
  mTextpane_boss = static_cast< CGuiTextPane* >(frame.FindWidget("textpane_boss"));
  mEnergybart01_bossbar->SetCoordFunc(BossEnergyCoordFunc);
  mEnergybart01_bossbar->SetTesselation(0.2f);
  const CTweakGuiColors::SPerVisorColors& colors = gpTweakGuiColors->GetVisorColors(0);
  mEnergybart01_bossbar->SetFilledColor(colors.mEnergyBarFilled);
  mEnergybart01_bossbar->SetShadowColor(colors.mEnergyBarShadow);
  mEnergybart01_bossbar->SetEmptyColor(colors.mEnergyBarEmpty);
}

void CHudBossEnergyInterface::SetBossParams(const bool visible, const rstl::wstring& name,
                                            float energy, float maxEnergy) {
  mVisible = visible;
  if (visible) {
    mEnergybart01_bossbar->SetFilledDrainSpeed(1000.f * (0.001f * maxEnergy));
    mEnergybart01_bossbar->SetCurrEnergy(energy, CAuiEnergyBarT01::kSM_Normal);
    mEnergybart01_bossbar->SetMaxEnergy(maxEnergy);
    mTextpane_boss->TextSupport().SetText(name);
  }
  mCurEnergy = energy;
  mMaxEnergy = maxEnergy;
}

void CHudBossEnergyInterface::SetAlpha(float alpha) { mAlpha = alpha; }

void CHudBossEnergyInterface::Update(float dt) {
  if (mVisible) {
    mFader = rstl::min_val(1.f, mFader + dt);
  } else {
    mFader = rstl::max_val(0.f, mFader - dt);
  }
  if (mFader > 0.f) {
    mBasewidget_bossenergystuff->SetColor(CColor::White().WithAlphaOf(mAlpha * mFader));
    mBasewidget_bossenergystuff->SetVisibility(true, kTM_Children);
  } else {
    mBasewidget_bossenergystuff->SetVisibility(false, kTM_Children);
  }
}
