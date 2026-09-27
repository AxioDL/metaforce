#include "MetroidPrime/HUD/CHudThreatInterface.hpp"

#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

#include <stdio.h>

static const char skThreatGroupWidgetName[] = "basewidget_threatstuff";
static const char skThreatIconWidgetName[] = "basewidget_threaticon";
static const char skThreatArrowUpWidgetName[] = "model_threatarrowup";
static const char skThreatArrowDownWidgetName[] = "model_threatarrowdown";
static const char skThreatWarningWidgetName[] = "textpane_threatwarning";
static const char skThreatBarWidgetName[] = "energybart01_threatbar";
static const char skThreatDigitsNameXRay[] = "textpane_threatdigits";
static const float skIconTranslateRanges[] = {6.05f, 0.f, 0.f, 8.4f, 0.f};
static const CAuiEnergyBarT01::FCoordFunc skThreatCoordFuncs[] = {
    CHudThreatInterface::CombatThreatBarCoordFunc, nullptr,
    CHudThreatInterface::XRayThreatBarCoordFunc, CHudThreatInterface::ThermalThreatBarCoordFunc,
    nullptr};

rstl::pair< CVector3f, CVector3f > CHudThreatInterface::CombatThreatBarCoordFunc(float t) {
  const float z = t * skIconTranslateRanges[kHT_Combat];
  return rstl::pair< CVector3f, CVector3f >(CVector3f(-0.3f, 0.f, z), CVector3f(0.f, 0.f, z));
}

rstl::pair< CVector3f, CVector3f > CHudThreatInterface::ThermalThreatBarCoordFunc(float t) {
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
  return rstl::pair< CVector3f, CVector3f >(CVector3f(0.1f, 0.f, z),
                                            CVector3f(0.5f * width + 0.1f, 0.f, z));
}

rstl::pair< CVector3f, CVector3f > CHudThreatInterface::XRayThreatBarCoordFunc(float t) {
  const float theta = 0.8f * (t - 0.5f);
  const float x = -1.f * (9.55f * CMath::FastCosR(theta));
  const float z = 9.55f * CMath::FastSinR(theta);
  return rstl::pair< CVector3f, CVector3f >(CVector3f(0.4f + x, 0.f, z), CVector3f(x, 0.f, z));
}

CHudThreatInterface::CHudThreatInterface(
  CGuiFrame& hud,
#if VERSION >= VERSION_GM8E_02
  StringTableHolder* stringTable,
#endif
  EHudType type,
  float distance
)
#if VERSION >= VERSION_GM8E_02
: mStringTable(stringTable),
#else
:
#endif
  mHudType(type)
, mDamagePulseTimer(0.f)
, mDamagePulse(0.f)
, mThreatDist(distance)
, mArrowTimer(0.f)
, mThreatIconXf(CTransform4f::Identity())
, mWarningLerpAlpha(0.f)
, mThreatStatus(kTS_Normal)
, mWarningColorLerp(0.f)
, mVisibleDebug(true)
, mVisibleGame(true) {
  mBasewidget_threatstuff = hud.FindWidget(skThreatGroupWidgetName);
  mBasewidget_threaticon = hud.FindWidget(skThreatIconWidgetName);
  mModel_threatarrowup = static_cast< CGuiModel* >(hud.FindWidget(skThreatArrowUpWidgetName));
  mModel_threatarrowdown =
      static_cast< CGuiModel* >(hud.FindWidget(skThreatArrowDownWidgetName));
  mTextpane_threatwarning =
      static_cast< CGuiTextPane* >(hud.FindWidget(skThreatWarningWidgetName));
  mEnergybart01_threatbar =
      static_cast< CAuiEnergyBarT01* >(hud.FindWidget(skThreatBarWidgetName));
  mTextpane_threatdigits =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skThreatDigitsNameXRay)));
  if (mTextpane_threatdigits) {
    mTextpane_threatdigits->TextSupport().SetFontColor(gpTweakGuiColors->GetThreatDigitsFont());
    mTextpane_threatdigits->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetThreatDigitsOutline());
  }
  mHasArrows = mModel_threatarrowup && mModel_threatarrowdown;
  mNotXRay = type != kHT_XRay;
  mBasewidget_threaticon->SetColor(gpTweakGuiColors->GetThreatIconColor());
  mThreatIconXf = mBasewidget_threaticon->GetO2PTransform();
  mEnergybart01_threatbar->SetFilledColor(gpTweakGuiColors->GetThreatBarFilled());
  mEnergybart01_threatbar->SetEmptyColor(gpTweakGuiColors->GetThreatBarEmpty());
  mEnergybart01_threatbar->SetShadowColor(gpTweakGuiColors->GetThreatBarShadow());
  mEnergybart01_threatbar->SetCoordFunc(skThreatCoordFuncs[type]);
  mEnergybart01_threatbar->SetTesselation(type == kHT_Combat ? 1.f : 0.1f);
  mEnergybart01_threatbar->SetMaxEnergy(gpTweakGui->GetThreatRange());
  mEnergybart01_threatbar->SetFilledDrainSpeed(9999.f);
  mEnergybart01_threatbar->SetShadowDrainSpeed(9999.f);
  mEnergybart01_threatbar->SetShadowDrainDelay(0.f);
  mEnergybart01_threatbar->SetIsAlwaysResetTimer(false);
  if (mTextpane_threatwarning) {
    mTextpane_threatwarning->TextSupport().SetFontColor(
        gpTweakGuiColors->GetThreatWarningFont());
    mTextpane_threatwarning->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetThreatWarningOutline());
  }
}

#if VERSION >= VERSION_GM8E_02
CHudThreatInterface::~CHudThreatInterface() {}
#endif

void CHudThreatInterface::SetThreatDistance(float distance) { mThreatDist = distance; }

void CHudThreatInterface::SetIsVisibleDebug(bool visible) {
  mVisibleDebug = visible;
  UpdateVisibility();
}

void CHudThreatInterface::SetIsVisibleGame(bool visible) {
  mVisibleGame = visible;
  UpdateVisibility();
}

void CHudThreatInterface::UpdateVisibility() {
  const bool visible = mVisibleGame && mVisibleDebug;
  mBasewidget_threatstuff->SetVisibility(visible, kTM_Children);
  if (visible) {
    Update(0.f);
  }
}

void CHudThreatInterface::Update(float dt) {
  const CTweakGuiColors& colors = *gpTweakGuiColors;
  const CColor warningColor = CColor::Lerp(
      colors.GetThreatIconColor(), colors.GetThreatIconWarningColor(), mWarningColorLerp);
  const float maxThreatEnergy = gpTweakGui->GetThreatRange();
  const float oldThreatDistance = maxThreatEnergy - mEnergybart01_threatbar->GetActualEnergy();
  if (mTextpane_threatdigits) {
    if (mThreatDist < maxThreatEnergy) {
      mTextpane_threatdigits->SetIsVisible(true);
      char digits[8];
      sprintf(digits, "%01.1f", rstl::max_val(0.f, mThreatDist));
      mTextpane_threatdigits->TextSupport().SetText(rstl::string(digits));
    } else {
      mTextpane_threatdigits->SetIsVisible(false);
    }
  }
  if (mHasArrows) {
    if (mArrowTimer > 0.f) {
      mArrowTimer = rstl::max_val(0.f, mArrowTimer - dt);
      const float alpha = mArrowTimer / gpTweakGui->GetMissileArrowVisTime();
      mModel_threatarrowup->SetIsVisible(true);
      mModel_threatarrowup->SetColor(warningColor.WithAlphaModulatedBy(alpha));
      mModel_threatarrowdown->SetIsVisible(false);
    } else if (mArrowTimer < 0.f) {
      mArrowTimer = rstl::min_val(0.f, mArrowTimer + dt);
      const float alpha = -mArrowTimer / gpTweakGui->GetMissileArrowVisTime();
      mModel_threatarrowdown->SetIsVisible(true);
      mModel_threatarrowdown->SetColor(warningColor.WithAlphaModulatedBy(alpha));
      mModel_threatarrowup->SetIsVisible(false);
    } else {
      mModel_threatarrowup->SetIsVisible(false);
      mModel_threatarrowdown->SetIsVisible(false);
    }
  }
  if (mThreatDist <= maxThreatEnergy) {
    const float delta = mThreatDist - oldThreatDistance;
    if (delta < -0.01f) {
      mArrowTimer = gpTweakGui->GetMissileArrowVisTime();
    } else if (delta > 0.01f) {
      mArrowTimer = -1.f * gpTweakGui->GetMissileArrowVisTime();
    }
  } else {
    mArrowTimer = 0.f;
  }
  if (mThreatDist <= maxThreatEnergy) {
    mEnergybart01_threatbar->SetCurrEnergy(
        mEnergybart01_threatbar->GetMaxEnergy() - mThreatDist, CAuiEnergyBarT01::kSM_Normal);
    mBasewidget_threaticon->SetColor(warningColor);
  } else {
    mEnergybart01_threatbar->SetCurrEnergy(0.f, CAuiEnergyBarT01::kSM_Normal);
    mBasewidget_threaticon->SetColor(gpTweakGuiColors->GetThreatIconSafeColor());
  }
  mEnergybart01_threatbar->SetFilledColor(warningColor);
  if (mNotXRay) {
    mBasewidget_threaticon->SetO2PTransform(
        mThreatIconXf *
        CTransform4f::Translate(CVector3f(0.f, 0.f,
                                          skIconTranslateRanges[mHudType] *
                                              rstl::max_val(0.f, maxThreatEnergy - mThreatDist) /
                                              maxThreatEnergy)));
  }
  if (mTextpane_threatwarning) {
    const float fraction = mEnergybart01_threatbar->GetActualFraction();
    if (fraction > gpTweakGui->GetThreatWarningFraction()) {
      mTextpane_threatwarning->SetIsVisible(true);
    } else {
      mTextpane_threatwarning->SetIsVisible(false);
    }
    EThreatStatus status;
    if (maxThreatEnergy == mEnergybart01_threatbar->GetActualEnergy()) {
      status = kTS_Damage;
    } else {
      const float fraction = mEnergybart01_threatbar->GetActualFraction();
      status = fraction > gpTweakGui->GetThreatWarningFraction() ? kTS_Warning : kTS_Normal;
    }
    if (status != mThreatStatus) {
#if VERSION < VERSION_GM8E_02
      const rstl::wstring text =
          status == kTS_Warning  ? rstl::wstring_l(gpStringTable->GetString(10))
          : status == kTS_Damage ? rstl::wstring_l(gpStringTable->GetString(11))
                                 : rstl::wstring_l(L"");
#else
      const rstl::wstring text = BuildWarningString(status);
#endif
      mTextpane_threatwarning->TextSupport().SetText(text);
      if (mThreatStatus == kTS_Normal && status == kTS_Warning) {
        CSfxManager::SfxStart(0x574);
      } else if (status == kTS_Damage) {
        CSfxManager::SfxStart(0x577);
      }
      mThreatStatus = status;
    }
  }
  const float oldPulseTimer = mDamagePulseTimer;
  mDamagePulseTimer = CMath::ModF(mDamagePulseTimer + dt, 0.5f);
  mDamagePulse = mDamagePulseTimer < 0.25f ? mDamagePulseTimer / 0.25f
                                               : (0.5f - mDamagePulseTimer) / 0.25f;
  if (mThreatStatus == kTS_Damage && mDamagePulseTimer < oldPulseTimer) {
    CSfxManager::SfxStart(0x577);
  }
  if (mTextpane_threatwarning) {
    if (mThreatStatus != kTS_Normal) {
      mWarningLerpAlpha = rstl::min_val(1.f, mWarningLerpAlpha + 2.f * dt);
      mTextpane_threatwarning->SetColor(
          CColor::White().WithAlphaOf(mWarningLerpAlpha * mDamagePulse));
    } else {
      mWarningLerpAlpha = rstl::max_val(0.f, mWarningLerpAlpha - 2.f * dt);
      mTextpane_threatwarning->SetColor(
          CColor::White().WithAlphaOf(mWarningLerpAlpha * mDamagePulse));
    }
    if (mTextpane_threatwarning->GetModifiedColor().GetAlphau8()) {
      mTextpane_threatwarning->SetIsVisible(true);
    } else {
      mTextpane_threatwarning->SetIsVisible(false);
    }
  }
  if (mThreatStatus == kTS_Damage) {
    mWarningColorLerp = rstl::min_val(1.f, mWarningColorLerp + 2.f * dt);
  } else {
    mWarningColorLerp = rstl::max_val(0.f, mWarningColorLerp - 2.f * dt);
  }
}


#if VERSION >= VERSION_GM8E_02

void CHudThreatInterface::ReinitializeStrings() {
  mTextpane_threatwarning->TextSupport().SetText(BuildWarningString(mThreatStatus));
}

const rstl::wstring CHudThreatInterface::BuildWarningString(CHudThreatInterface::EThreatStatus status) {
  return status == kTS_Warning  ? rstl::wstring_l(mStringTable->mTable->GetString(9))
          : status == kTS_Damage ? rstl::wstring_l(mStringTable->mTable->GetString(10))
                                 : rstl::wstring_l(L"");

}

#endif
