#include "Runtime/GuiSys/CHudFreeLookInterface.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"

namespace urde {

CHudFreeLookInterface::CHudFreeLookInterface(CGuiFrame& selHud, EHudType hudType, bool inFreeLook, bool lookControlHeld,
                                             bool lockedOnObj)
: x4_hudType(hudType)
, x70_24_inFreeLook(inFreeLook)
, x70_25_lookControlHeld(lookControlHeld)
, x70_26_lockedOnObj(lockedOnObj) {
  x6c_lockOnInterp = (lockedOnObj && hudType == EHudType::Scan) ? 0.f : 1.f;

  x74_basewidget_freelookleft = selHud.FindWidget("basewidget_freelookleft");
  x78_model_shieldleft = static_cast<CGuiModel*>(selHud.FindWidget("model_shieldleft"));
  x7c_model_freelookleft = static_cast<CGuiModel*>(selHud.FindWidget("model_freelookleft"));
  x80_basewidget_freelookright = selHud.FindWidget("basewidget_freelookright");
  x84_model_shieldright = static_cast<CGuiModel*>(selHud.FindWidget("model_shieldright"));
  x88_model_freelookright = static_cast<CGuiModel*>(selHud.FindWidget("model_freelookright"));
  x8c_basewidget_outlinesb = selHud.FindWidget("basewidget_outlinesb");

  x8_freeLookLeftXf = x7c_model_freelookleft->GetTransform();
  x38_freeLookRightXf = x88_model_freelookright->GetTransform();

  x78_model_shieldleft->SetDepthWrite(true);
  x84_model_shieldright->SetDepthWrite(true);
}

void CHudFreeLookInterface::Update(float dt) {
  if (x70_24_inFreeLook)
    x68_freeLookInterp = std::min(x68_freeLookInterp + dt / g_tweakGui->GetFreeLookFadeTime(), 1.f);
  else
    x68_freeLookInterp = std::max(0.f, x68_freeLookInterp - dt / g_tweakGui->GetFreeLookFadeTime());

  if (x70_26_lockedOnObj && x4_hudType == EHudType::Scan)
    x6c_lockOnInterp = std::min(x6c_lockOnInterp + 2.f * dt, 1.f);
  else
    x6c_lockOnInterp = std::max(0.f, x6c_lockOnInterp - 2.f * dt);
}

void CHudFreeLookInterface::SetIsVisibleDebug(bool v) {
  x70_27_visibleDebug = v;
  UpdateVisibility();
}

void CHudFreeLookInterface::SetIsVisibleGame(bool v) {
  x70_28_visibleGame = v;
  UpdateVisibility();
}

void CHudFreeLookInterface::UpdateVisibility() {
  bool vis = x70_27_visibleDebug && x70_28_visibleGame;
  x74_basewidget_freelookleft->SetVisibility(vis, ETraversalMode::Children);
  x80_basewidget_freelookright->SetVisibility(vis, ETraversalMode::Children);
  if (vis)
    Update(0.f);
}

void CHudFreeLookInterface::SetFreeLookState(bool inFreeLook, bool lookControlHeld, bool lockedOnObj,
                                             float vertLookAngle) {
  x70_24_inFreeLook = inFreeLook;
  vertLookAngle *= 8.f;
  x70_25_lookControlHeld = lookControlHeld;
  x70_26_lockedOnObj = lockedOnObj;

  x7c_model_freelookleft->SetLocalTransform(x8_freeLookLeftXf * zeus::CTransform::Translate(0.f, 0.f, vertLookAngle));

  x88_model_freelookright->SetLocalTransform(x38_freeLookRightXf *
                                             zeus::CTransform::Translate(0.f, 0.f, vertLookAngle));

  zeus::CColor color = zeus::skWhite;
  float totalInterp = x68_freeLookInterp * (1.f - x6c_lockOnInterp);
  color.a() = totalInterp;
  x74_basewidget_freelookleft->SetColor(color);
  x80_basewidget_freelookright->SetColor(color);

  if (x8c_basewidget_outlinesb) {
    color.a() = 0.7f * totalInterp + 0.3f;
    x8c_basewidget_outlinesb->SetColor(color);
  }

  const bool visible = totalInterp != 0.0f;
  x74_basewidget_freelookleft->SetVisibility(visible, ETraversalMode::Children);
  x80_basewidget_freelookright->SetVisibility(visible, ETraversalMode::Children);
}

CHudFreeLookInterfaceXRay::CHudFreeLookInterfaceXRay(CGuiFrame& selHud, bool inFreeLook, bool lookControlHeld,
                                                     bool lockedOnObj) {
  x20_inFreeLook = inFreeLook;
  x21_lookControlHeld = lookControlHeld;
  x24_basewidget_freelook = selHud.FindWidget("basewidget_freelook");
  x28_model_shield = static_cast<CGuiModel*>(selHud.FindWidget("model_shield"));
  x2c_model_freelookleft = static_cast<CGuiModel*>(selHud.FindWidget("model_freelookleft"));
  x30_model_freelookright = static_cast<CGuiModel*>(selHud.FindWidget("model_freelookright"));

  x4_freeLookLeftPos = x2c_model_freelookleft->GetLocalPosition();
  x10_freeLookRightPos = x30_model_freelookright->GetLocalPosition();

  x28_model_shield->SetDepthWrite(true);
}

void CHudFreeLookInterfaceXRay::Update(float dt) {
  if (x20_inFreeLook)
    x1c_freeLookInterp = std::min(x1c_freeLookInterp + dt / g_tweakGui->GetFreeLookFadeTime(), 1.f);
  else
    x1c_freeLookInterp = std::max(0.f, x1c_freeLookInterp - dt / g_tweakGui->GetFreeLookFadeTime());
}

void CHudFreeLookInterfaceXRay::UpdateVisibility() {
  bool vis = x22_24_visibleDebug && x22_25_visibleGame;
  x2c_model_freelookleft->SetVisibility(vis, ETraversalMode::Children);
  x30_model_freelookright->SetVisibility(vis, ETraversalMode::Children);
  if (vis)
    Update(0.f);
}

void CHudFreeLookInterfaceXRay::SetIsVisibleDebug(bool v) {
  x22_24_visibleDebug = v;
  UpdateVisibility();
}

void CHudFreeLookInterfaceXRay::SetIsVisibleGame(bool v) {
  x22_25_visibleGame = v;
  UpdateVisibility();
}

void CHudFreeLookInterfaceXRay::SetFreeLookState(bool inFreeLook, bool lookControlHeld, bool lockedOnObj,
                                                 float vertLookAngle) {
  x20_inFreeLook = inFreeLook;
  x21_lookControlHeld = lookControlHeld;

  x2c_model_freelookleft->SetLocalTransform(
      zeus::CTransform(zeus::CMatrix3f::RotateY(vertLookAngle), x4_freeLookLeftPos));
  x30_model_freelookright->SetLocalTransform(
      zeus::CTransform(zeus::CMatrix3f::RotateY(-vertLookAngle), x10_freeLookRightPos));

  zeus::CColor color = zeus::skWhite;
  color.a() = x1c_freeLookInterp;
  x24_basewidget_freelook->SetColor(color);

  const bool visible = x1c_freeLookInterp != 0.0f;
  x24_basewidget_freelook->SetVisibility(visible, ETraversalMode::Children);
}

} // namespace urde
