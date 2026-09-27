#include "MetroidPrime/HUD/CHudFreeLookInterface.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "rstl/math.hpp"

static const char skFreeLookGroupWidgetName[] = "basewidget_freelook";
static const char skFreeLookLeftWidgetName[] = "model_freelookleft";
static const char skFreeLookRightWidgetName[] = "model_freelookright";
static const char skFreeLookLeftGroupWidgetName[] = "basewidget_freelookleft";
static const char skFreeLookLeftShieldWidgetName[] = "model_shieldleft";
static const char skFreeLookLeftTickUpWidgetName[] = "model_freetickaleft";
static const char skFreeLookLeftTickDownWidgetName[] = "model_freetickbleft";
static const char skFreeLookLeftDigitsName[] = "textpane_freelookleftdigits";
static const char skFreeLookLeftDecoName[] = "basewidget_freelookleftdeco0";
static const char skFreeLookRightGroupWidgetName[] = "basewidget_freelookright";
static const char skFreeLookRightShieldWidgetName[] = "model_shieldright";
static const char skFreeLookRightTickUpWidgetName[] = "model_freetickaright";
static const char skFreeLookRightTickDownWidgetName[] = "model_freetickbright";
static const char skFreeLookRightDigitsName[] = "textpane_freelookrightdigits";
static const char skFreeLookRightDecoName[] = "basewidget_freelookrightdeco0";

CHudFreeLookInterfaceXRay::~CHudFreeLookInterfaceXRay() {}

CHudFreeLookInterfaceXRay::CHudFreeLookInterfaceXRay(CGuiFrame& hud, bool freeLook,
                                                     bool controlHeld, bool lockedOn)
: mFreeLookLeftPos(CVector3f::Zero())
, mFreeLookRightPos(CVector3f::Zero())
, mFreeLookInterp(0.f)
, mInFreeLook(freeLook)
, mLookControlHeld(controlHeld)
, mVisibleDebug(true)
, mVisibleGame(true) {
  mBasewidget_freelook = hud.FindWidget(skFreeLookGroupWidgetName);
  mModel_freelookleft = static_cast< CGuiModel* >(hud.FindWidget(skFreeLookLeftWidgetName));
  mModel_freelookright = static_cast< CGuiModel* >(hud.FindWidget(skFreeLookRightWidgetName));
  mModel_shield = static_cast< CGuiModel* >(hud.FindWidget("model_shield"));
  mFreeLookLeftPos = mModel_freelookleft->GetLocalPosition();
  mFreeLookRightPos = mModel_freelookright->GetLocalPosition();
  mModel_shield->SetIsAlwaysDepthWrite(true);
}

void CHudFreeLookInterfaceXRay::SetFreeLookState(const bool freeLook, const bool controlHeld,
                                                 const bool lockedOn, float angle) {
  mInFreeLook = freeLook;
  mLookControlHeld = controlHeld;
  mModel_freelookleft->SetO2PTransform(
      CTransform4f(CMatrix3f::RotateY(CRelAngle::FromRadians(angle)), mFreeLookLeftPos));
  mModel_freelookright->SetO2PTransform(
      CTransform4f(CMatrix3f::RotateY(CRelAngle::FromRadians(-angle)), mFreeLookRightPos));
  const float alpha = mFreeLookInterp;
  mBasewidget_freelook->SetColor(CColor::White().WithAlphaOf(alpha));
  if (alpha == 0.f) {
    mBasewidget_freelook->SetVisibility(false, kTM_Children);
  } else {
    mBasewidget_freelook->SetVisibility(true, kTM_Children);
  }
}

void CHudFreeLookInterfaceXRay::SetIsVisibleDebug(bool visible) {
  mVisibleDebug = visible;
  UpdateVisibility();
}

void CHudFreeLookInterfaceXRay::SetIsVisibleGame(bool visible) {
  mVisibleGame = visible;
  UpdateVisibility();
}

void CHudFreeLookInterfaceXRay::UpdateVisibility() {
  const bool visible = mVisibleGame && mVisibleDebug;
  mModel_freelookleft->SetVisibility(visible, kTM_Children);
  mModel_freelookright->SetVisibility(visible, kTM_Children);
  if (visible) {
    Update(0.f);
  }
}

void CHudFreeLookInterfaceXRay::Update(float dt) {
  if (mInFreeLook) {
    mFreeLookInterp =
        rstl::min_val(1.f, mFreeLookInterp + dt / gpTweakGui->GetFreeLookFadeTime());
  } else {
    mFreeLookInterp =
        rstl::max_val(0.f, mFreeLookInterp - dt / gpTweakGui->GetFreeLookFadeTime());
  }
}

CHudFreeLookInterface::CHudFreeLookInterface(CGuiFrame& hud, EHudType type, bool freeLook,
                                             bool controlHeld, bool lockedOn)
: mHudType(type)
, mFreeLookLeftXf(CTransform4f::Identity())
, mFreeLookRightXf(CTransform4f::Identity())
, mFreeLookInterp(0.f)
, mLockOnInterp(lockedOn && type == kHT_Scan ? 0.f : 1.f)
, mInFreeLook(freeLook)
, mLookControlHeld(controlHeld)
, mLockedOnObj(lockedOn)
, mVisibleDebug(true)
, mVisibleGame(true) {
  mBasewidget_freelookleft = hud.FindWidget(skFreeLookLeftGroupWidgetName);
  mModel_shieldleft = static_cast< CGuiModel* >(hud.FindWidget(skFreeLookLeftShieldWidgetName));
  mModel_freelookleft = static_cast< CGuiModel* >(hud.FindWidget(skFreeLookLeftWidgetName));
  mBasewidget_freelookright = hud.FindWidget(skFreeLookRightGroupWidgetName);
  mModel_shieldright =
      static_cast< CGuiModel* >(hud.FindWidget(skFreeLookRightShieldWidgetName));
  mModel_freelookright = static_cast< CGuiModel* >(hud.FindWidget(skFreeLookRightWidgetName));
  mBasewidget_outlinesb = hud.FindWidget("basewidget_outlinesb");
  mFreeLookLeftXf = mModel_freelookleft->GetTransform();
  mFreeLookRightXf = mModel_freelookright->GetTransform();
  mModel_shieldleft->SetIsAlwaysDepthWrite(true);
  mModel_shieldright->SetIsAlwaysDepthWrite(true);
}

void CHudFreeLookInterface::SetFreeLookState(const bool freeLook, const bool controlHeld,
                                             const bool lockedOn, float angle) {
  mInFreeLook = freeLook;
  const float offset = 8.f * angle;
  mLookControlHeld = controlHeld;
  mLockedOnObj = lockedOn;
  mModel_freelookleft->SetO2PTransform(mFreeLookLeftXf *
                                          CTransform4f::Translate(0.f, 0.f, offset));
  mModel_freelookright->SetO2PTransform(mFreeLookRightXf *
                                           CTransform4f::Translate(0.f, 0.f, offset));
  const float alpha = mFreeLookInterp * (1.f - mLockOnInterp);
  mBasewidget_freelookleft->SetColor(CColor::White().WithAlphaOf(alpha));
  mBasewidget_freelookright->SetColor(CColor::White().WithAlphaOf(alpha));
  if (mBasewidget_outlinesb) {
    mBasewidget_outlinesb->SetColor(CColor::White().WithAlphaOf(0.7f * alpha + 0.3f));
  }
  if (alpha == 0.f) {
    mBasewidget_freelookleft->SetVisibility(false, kTM_Children);
    mBasewidget_freelookright->SetVisibility(false, kTM_Children);
  } else {
    mBasewidget_freelookleft->SetVisibility(true, kTM_Children);
    mBasewidget_freelookright->SetVisibility(true, kTM_Children);
  }
}

void CHudFreeLookInterface::SetIsVisibleDebug(bool visible) {
  mVisibleDebug = visible;
  UpdateVisibility();
}

void CHudFreeLookInterface::SetIsVisibleGame(bool visible) {
  mVisibleGame = visible;
  UpdateVisibility();
}

void CHudFreeLookInterface::UpdateVisibility() {
  const bool visible = mVisibleGame && mVisibleDebug;
  mBasewidget_freelookleft->SetVisibility(visible, kTM_Children);
  mBasewidget_freelookright->SetVisibility(visible, kTM_Children);
  if (visible) {
    Update(0.f);
  }
}

void CHudFreeLookInterface::Update(float dt) {
  if (mInFreeLook) {
    mFreeLookInterp =
        rstl::min_val(1.f, mFreeLookInterp + dt / gpTweakGui->GetFreeLookFadeTime());
  } else {
    mFreeLookInterp =
        rstl::max_val(0.f, mFreeLookInterp - dt / gpTweakGui->GetFreeLookFadeTime());
  }
  if (mLockedOnObj && mHudType == kHT_Scan) {
    mLockOnInterp = rstl::min_val(1.f, mLockOnInterp + 2.f * dt);
  } else {
    mLockOnInterp = rstl::max_val(0.f, mLockOnInterp - 2.f * dt);
  }
}

CHudFreeLookInterface::~CHudFreeLookInterface() {}
