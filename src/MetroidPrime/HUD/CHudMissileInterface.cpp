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
  if (mEnergybart01_missilebar->GetActualEnergy() == 0.f) {
    return kIS_Depleted;
  }
  const float fraction = mEnergybart01_missilebar->GetActualFraction();
  return fraction < gpTweakGui->GetMissileWarningFraction() ? kIS_Warning : kIS_Normal;
}

CHudMissileInterface::CHudMissileInterface(CGuiFrame& hud, int capacity, int missiles,
                                           float chargeFactor, bool active, EHudType type,
                                           const CStateManager& mgr)
: mHudType(type)
, mMissileCapacity(capacity)
, mNumMissiles(missiles)
, mArrowTimer(0.f)
, mMissileIconXf(CTransform4f::Identity())
, mMissileWarningAlpha(0.f)
, mLatestStatus(kIS_Normal)
, mMissileWarningPulse(0.f)
, mChargeBeamFactor(chargeFactor)
, mMissileIconAltDeplete(0.f)
, mMissileIconIncrement(0.f)
, mMissilesActive(active)
, mVisibleDebug(true)
, mVisibleGame(true) {
  mBaseWidgetMissileIconA = hud.FindWidget(rstl::string_l(skMissileGroupWidgetName));
  mTextpane_missiledigits =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skMissileDigitsWidgetName)));
  mEnergybart01_missilebar =
      static_cast< CAuiEnergyBarT01* >(hud.FindWidget(rstl::string_l(skMissileBarWidgetName)));
  mTextpane_missilewarning =
      static_cast< CGuiTextPane* >(hud.FindWidget(rstl::string_l(skMissileWarningWidgetName)));
  mModel_missilearrowup =
      static_cast< CGuiModel* >(hud.FindWidget(rstl::string_l(skMissileArrowUpWidgetName)));
  mModel_missilearrowdown =
      static_cast< CGuiModel* >(hud.FindWidget(rstl::string_l(skMissileArrowDownWidgetName)));
  mBaseWidgetMissileIconB = hud.FindWidget(rstl::string_l(skMissileIconWidgetName));
  mHasArrows = mModel_missilearrowup && mModel_missilearrowdown;
  mNotXRay = type != kHT_XRay;
  mMissileIconXf = mBaseWidgetMissileIconB->GetO2PTransform();
  mTextpane_missiledigits->TextSupport().SetFontColor(gpTweakGuiColors->GetMissileDigitsFont());
  mTextpane_missiledigits->TextSupport().SetOutlineColor(
      gpTweakGuiColors->GetMissileDigitsOutline());
  mBaseWidgetMissileIconB->SetColor(gpTweakGuiColors->GetMissileIconColorInactive());
  mEnergybart01_missilebar->SetEmptyColor(gpTweakGuiColors->GetMissileBarEmpty());
  mEnergybart01_missilebar->SetFilledColor(gpTweakGuiColors->GetMissileBarFilled());
  mEnergybart01_missilebar->SetShadowColor(gpTweakGuiColors->GetMissileBarShadow());
  mEnergybart01_missilebar->SetCoordFunc(skMissileCoordFuncs[mHudType]);
  mEnergybart01_missilebar->SetTesselation(type == kHT_Combat ? 1.f : 0.1f);
  mEnergybart01_missilebar->SetMaxEnergy(5.f);
  mEnergybart01_missilebar->SetFilledDrainSpeed(gpTweakGui->GetEnergyBarFilledSpeed());
  mEnergybart01_missilebar->SetShadowDrainSpeed(gpTweakGui->GetEnergyBarShadowSpeed());
  mEnergybart01_missilebar->SetShadowDrainDelay(gpTweakGui->GetEnergyBarDrainDelay());
  mEnergybart01_missilebar->SetIsAlwaysResetTimer(true);
  if (mTextpane_missilewarning) {
    mTextpane_missilewarning->TextSupport().SetFontColor(
        gpTweakGuiColors->GetMissileWarningFont());
    mTextpane_missilewarning->TextSupport().SetOutlineColor(
        gpTweakGuiColors->GetMissileWarningOutline());
  }
  SetNumMissiles(mNumMissiles, mgr);
  mLatestStatus = GetMissileInventoryStatus();
}

void CHudMissileInterface::SetMissileCapacity(int capacity) { mMissileCapacity = capacity; }

void CHudMissileInterface::SetNumMissiles(int missiles, const CStateManager& mgr) {
  char digits[4];
#if NONMATCHING
  snprintf(digits, sizeof(digits), "%3d", CMath::Clamp(0, missiles, 999));
#else
  sprintf(digits, "%3d", CMath::Clamp(0, missiles, 999));
#endif
  mTextpane_missiledigits->TextSupport().SetText(rstl::string(digits));
  if (mNumMissiles < missiles) {
    mArrowTimer = gpTweakGui->GetMissileArrowVisTime();
    mMissileIconIncrement = -FLT_EPSILON;
  } else if (mNumMissiles > missiles) {
    mArrowTimer = -1.f * gpTweakGui->GetMissileArrowVisTime();
  }
  if (mgr.GetPlayerState()->GetMissileCostForAltAttack() + missiles <= mNumMissiles) {
    mMissileIconAltDeplete = 1.f;
  }
  mNumMissiles = missiles;
}

void CHudMissileInterface::SetChargeBeamFactor(float factor) { mChargeBeamFactor = factor; }

void CHudMissileInterface::SetIsMissilesActive(bool active) { mMissilesActive = active; }

void CHudMissileInterface::SetIsVisibleGame(bool visible, const CStateManager& mgr) {
  mVisibleGame = visible;
  UpdateVisibility(mgr);
}

void CHudMissileInterface::UpdateVisibility(const CStateManager& mgr) {
  const bool visible = mVisibleGame && mVisibleDebug;
  mBaseWidgetMissileIconA->SetVisibility(visible, kTM_Children);
  mEnergybart01_missilebar->SetVisibility(visible, kTM_Children);
  if (visible) {
    Update(0.f, mgr);
  }
}

void CHudMissileInterface::Update(float dt, const CStateManager& mgr) {
  const CPlayerState& playerState = *mgr.GetPlayerState();
  if (mMissileCapacity < 1) {
    mBaseWidgetMissileIconA->SetIsVisible(false);
  } else {
    mBaseWidgetMissileIconA->SetIsVisible(true);
  }
  if (mMissileIconIncrement < 0.f) {
    mMissileIconIncrement -= 3.f * dt;
    if (mMissileIconIncrement <= -1.f) {
      mMissileIconIncrement = 1.f;
    }
  } else if (mMissileIconIncrement > 0.f) {
    mMissileIconIncrement = rstl::max_val(0.f, mMissileIconIncrement - dt);
  }
  const CColor& inactiveColor = gpTweakGuiColors->GetMissileIconColorInactive();
  const uchar flashValue = CCast::ToUint8(255.f * CMath::AbsF(mMissileIconIncrement));
  const CColor flash(flashValue, flashValue, flashValue, flashValue);
  const CColor addColor = CColor::Modulate(flash, gpTweakGuiColors->GetMissileIconColorActive());
  if (mMissileIconAltDeplete > 0.f) {
    CColor color = CColor::Lerp(inactiveColor, gpTweakGuiColors->GetMissileIconColorDepleteAlt(),
                                mMissileIconAltDeplete);
    color = CColor::Add(color, addColor);
    mBaseWidgetMissileIconB->SetColor(color);
  } else if (mChargeBeamFactor > 0.f) {
    const float factor =
        rstl::min_val(1.f, mChargeBeamFactor / CPlayerState::GetMissileComboChargeFactor());
    if (mNumMissiles >= playerState.GetMissileCostForAltAttack()) {
      CColor color =
          CColor::Lerp(inactiveColor, gpTweakGuiColors->GetMissileIconColorChargedCanAlt(), factor);
      color = CColor::Add(color, addColor);
      mBaseWidgetMissileIconB->SetColor(color);
    } else {
      CColor color =
          CColor::Lerp(inactiveColor, gpTweakGuiColors->GetMissileIconColorChargedNoAlt(), factor);
      color = CColor::Add(color, addColor);
      mBaseWidgetMissileIconB->SetColor(color);
    }
  } else if (mMissilesActive) {
    const CColor color = CColor::Add(gpTweakGuiColors->GetMissileIconColorActive(), addColor);
    mBaseWidgetMissileIconB->SetColor(color);
  } else {
    const CColor color = CColor::Add(gpTweakGuiColors->GetMissileIconColorInactive(), addColor);
    mBaseWidgetMissileIconB->SetColor(color);
  }
  mMissileIconAltDeplete = rstl::max_val(0.f, mMissileIconAltDeplete - dt);
  mEnergybart01_missilebar->SetMaxEnergy(mMissileCapacity);
  mEnergybart01_missilebar->SetCurrEnergy(mNumMissiles, CAuiEnergyBarT01::kSM_Normal);
  if (mNotXRay) {
    mBaseWidgetMissileIconB->SetO2PTransform(
        mMissileIconXf *
        CTransform4f::Translate(CVector3f(0.f, 0.f,
                                          mNumMissiles * skIconTranslateRanges[mHudType] /
                                              float(mMissileCapacity))));
  }
  const CColor& activeColor = gpTweakGuiColors->GetMissileIconColorActive();
  if (mHasArrows) {
    if (mArrowTimer > 0.f) {
      mArrowTimer = rstl::max_val(0.f, mArrowTimer - dt);
      const float alpha = mArrowTimer / gpTweakGui->GetMissileArrowVisTime();
      mModel_missilearrowup->SetIsVisible(true);
      mModel_missilearrowup->SetColor(activeColor.WithAlphaModulatedBy(alpha));
      mModel_missilearrowdown->SetIsVisible(false);
    } else if (mArrowTimer < 0.f) {
      mArrowTimer = rstl::min_val(0.f, mArrowTimer + dt);
      const float alpha = -mArrowTimer / gpTweakGui->GetMissileArrowVisTime();
      mModel_missilearrowdown->SetIsVisible(true);
      mModel_missilearrowdown->SetColor(activeColor.WithAlphaModulatedBy(alpha));
      mModel_missilearrowup->SetIsVisible(false);
    } else {
      mModel_missilearrowup->SetIsVisible(false);
      mModel_missilearrowdown->SetIsVisible(false);
    }
  }
  if (mTextpane_missilewarning) {
    const EInventoryStatus status = GetMissileInventoryStatus();
    if (status != mLatestStatus) {
      const rstl::wstring text =
          status == kIS_Warning    ? rstl::wstring_l(gpStringTable->GetString(12))
          : status == kIS_Depleted ? rstl::wstring_l(gpStringTable->GetString(13))
                                   : rstl::wstring_l(L"");
      mTextpane_missilewarning->TextSupport().SetText(text);
      if (mLatestStatus == kIS_Normal && status == kIS_Warning) {
        CSfxManager::SfxStart(0x575);
        mMissileWarningPulse = gpTweakGui->GetMissileWarningPulseTime();
      } else if (status == kIS_Depleted) {
        CSfxManager::SfxStart(0x575);
        mMissileWarningPulse = gpTweakGui->GetMissileWarningPulseTime();
      }
      mLatestStatus = status;
    }
    mMissileWarningPulse = rstl::max_val(0.f, mMissileWarningPulse - dt);
    const float warningPulse = rstl::min_val(1.f, mMissileWarningPulse);
    if (mLatestStatus != kIS_Normal) {
      mMissileWarningAlpha = rstl::min_val(1.f, mMissileWarningAlpha + 2.f * dt);
    } else {
      mMissileWarningAlpha = rstl::max_val(0.f, mMissileWarningAlpha - 2.f * dt);
    }
    float pulse = CMath::AbsF(CMath::ModF(CGraphics::GetSecondsMod900(), 0.5f));
    const float alpha =
        mMissileWarningAlpha * (pulse < 0.25f ? pulse / 0.25f : (0.5f - pulse) / 0.25f);
    const float warningAlpha = warningPulse * alpha;
    mTextpane_missilewarning->SetColor(CColor::White().WithAlphaOf(warningAlpha));
    if (mTextpane_missilewarning->GetModifiedColor().GetAlphau8()) {
      mTextpane_missilewarning->SetIsVisible(true);
    } else {
      mTextpane_missilewarning->SetIsVisible(false);
    }
  }
}
