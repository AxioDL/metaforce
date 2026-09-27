#include "MetroidPrime/HUD/CHudHelmetInterface.hpp"

#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/string.hpp"

static const char skPivotWidgetName[] = "BaseWidget_Pivot";
static const char skHelmetWidgetName[] = "BaseWidget_Helmet";
static const char skGlowWidgetName[] = "BaseWidget_Glow";
static const char skHelmetLightWidgetName[] = "BaseWidget_HelmetLight";

CHudHelmetInterface::CHudHelmetInterface(CGuiFrame& frame)
: mHudLagRotation(CMatrix3f::Identity())
, mPivotPosition(CVector3f::Zero())
, mHudLagPosition(CVector3f::Zero())
, mHelmetVisibleDebug(true)
, mHelmetVisibleGame(true)
, mGlowVisibleDebug(true)
, mGlowVisibleGame(true)
, mHudLagDirty(false) {
  mCamera = frame.GetFrameCamera();
  mBaseWidget_Pivot = frame.FindWidget(rstl::string_l(skPivotWidgetName));
  mBaseWidget_Helmet = frame.FindWidget(rstl::string_l(skHelmetWidgetName));
  mBaseWidget_Glow = frame.FindWidget(rstl::string_l(skGlowWidgetName));
  mBaseWidget_HelmetLight = frame.FindWidget(rstl::string_l(skHelmetLightWidgetName));
  mPivotPosition = mBaseWidget_Pivot->GetIdleXform().GetTranslation();
  mBaseWidget_HelmetLight->SetColor(gpTweakGuiColors->GetHelmetLightColor());
}

void CHudHelmetInterface::SetIsVisibleDebug(const bool helmet, const bool glow) {
  mHelmetVisibleDebug = helmet;
  mGlowVisibleDebug = glow;
  UpdateVisibility();
}

void CHudHelmetInterface::UpdateHelmetAlpha() {
  mBaseWidget_Pivot->SetColor(
      CColor::White().WithAlphaOf(gpGameState->GameOptions().GetHelmetAlpha()));
}

void CHudHelmetInterface::UpdateVisibility() {
  const bool helmet = mHelmetVisibleDebug && mHelmetVisibleGame;
  const bool glow = mGlowVisibleDebug && mGlowVisibleGame;
  mBaseWidget_Helmet->SetVisibility(helmet, kTM_Children);
  mBaseWidget_Glow->SetVisibility(glow, kTM_Children);
}

void CHudHelmetInterface::UpdateCameraDebugSettings(float fov, float y, float z) {
  CGuiCamera::UCameraParms parms = mCamera->GetParms();
  parms.perspective.fov = fov;
  mCamera->SetParms(parms);
  mCamera->SetIdleXform(
      CTransform4f(mCamera->GetIdleXform().BuildMatrix3f(), CVector3f(0.f, y, z)));
}

void CHudHelmetInterface::AddHelmetLightValue(float value) {
  const CColor light(value, value, value, value);
  mBaseWidget_HelmetLight->SetColor(CColor::Add(gpTweakGuiColors->GetHelmetLightColor(), light));
}

void CHudHelmetInterface::SetHudLagRotation(const CMatrix3f& rotation) {
  mHudLagRotation = rotation;
  mHudLagDirty = true;
}

void CHudHelmetInterface::SetHudLagOffset(const CVector3f& offset) {
  mHudLagPosition = offset;
  mHudLagDirty = true;
}

void CHudHelmetInterface::Update(float dt) {
  if (mHudLagDirty) {
    mHudLagDirty = false;
    mBaseWidget_Pivot->SetIdleXform(
        CTransform4f(mHudLagRotation, mPivotPosition + mHudLagPosition));
  }
}
