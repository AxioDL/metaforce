#include "Runtime/GuiSys/CHudHelmetInterface.hpp"

#include "Runtime/CGameState.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"

namespace urde {

CHudHelmetInterface::CHudHelmetInterface(CGuiFrame& helmetFrame) {
  x40_camera = helmetFrame.GetFrameCamera();
  x44_BaseWidget_Pivot = helmetFrame.FindWidget("BaseWidget_Pivot");
  x48_BaseWidget_Helmet = helmetFrame.FindWidget("BaseWidget_Helmet");
  x4c_BaseWidget_Glow = helmetFrame.FindWidget("BaseWidget_Glow");
  x50_BaseWidget_HelmetLight = helmetFrame.FindWidget("BaseWidget_HelmetLight");
  x24_pivotPosition = x44_BaseWidget_Pivot->GetIdlePosition();
  x50_BaseWidget_HelmetLight->SetColor(g_tweakGuiColors->GetHelmetLightColor());
}

void CHudHelmetInterface::UpdateVisibility() {
  x48_BaseWidget_Helmet->SetVisibility(x3c_24_helmetVisibleDebug && x3c_25_helmetVisibleGame, ETraversalMode::Children);
  x4c_BaseWidget_Glow->SetVisibility(x3c_26_glowVisibleDebug && x3c_27_glowVisibleGame, ETraversalMode::Children);
}

void CHudHelmetInterface::Update(float dt) {
  if (x3c_28_hudLagDirty) {
    x3c_28_hudLagDirty = false;
    x44_BaseWidget_Pivot->SetTransform(zeus::CTransform(x0_hudLagRotation, x24_pivotPosition + x30_hudLagPosition));
  }
}

void CHudHelmetInterface::SetHudLagOffset(const zeus::CVector3f& off) {
  x30_hudLagPosition = off;
  x3c_28_hudLagDirty = true;
}

void CHudHelmetInterface::SetHudLagRotation(const zeus::CMatrix3f& rot) {
  x0_hudLagRotation = rot;
  x3c_28_hudLagDirty = true;
}

void CHudHelmetInterface::AddHelmetLightValue(float val) {
  x50_BaseWidget_HelmetLight->SetColor(g_tweakGuiColors->GetHelmetLightColor() + zeus::CColor(val, val));
}

void CHudHelmetInterface::UpdateCameraDebugSettings(float fov, float y, float z) {
  x40_camera->SetFov(fov);
  x40_camera->SetTransform(zeus::CTransform(x40_camera->GetTransform().buildMatrix3f(), zeus::CVector3f(0.f, y, z)));
}

void CHudHelmetInterface::UpdateHelmetAlpha() {
  zeus::CColor color = zeus::skWhite;
  color.a() = g_GameState->GameOptions().GetHelmetAlpha() / 255.f;
  x44_BaseWidget_Pivot->SetColor(color);
}

void CHudHelmetInterface::SetIsVisibleDebug(bool helmet, bool glow) {
  x3c_24_helmetVisibleDebug = helmet;
  x3c_26_glowVisibleDebug = glow;
  UpdateVisibility();
}

} // namespace urde
