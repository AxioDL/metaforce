#include "CTargetingManager.hpp"
#include "Camera/CGameCamera.hpp"
#include "CStateManager.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde {

CTargetingManager::CTargetingManager(const CStateManager& mgr) : x0_targetReticule(mgr) {}

bool CTargetingManager::CheckLoadComplete() {
  return (x0_targetReticule.CheckLoadComplete() && x21c_orbitPointMarker.CheckLoadComplete());
}

void CTargetingManager::Update(float dt, const CStateManager& stateMgr) {
  x0_targetReticule.Update(dt, stateMgr);
  x21c_orbitPointMarker.Update(dt, stateMgr);
}

void CTargetingManager::Draw(const CStateManager& mgr, bool hideLockon) const {
  CGraphics::SetAmbientColor(zeus::CColor::skWhite);
  CGraphics::DisableAllLights();
  x21c_orbitPointMarker.Draw(mgr);
  const CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CGraphics::SetViewPointMatrix(camXf);
  zeus::CFrustum frustum;
  frustum.updatePlanes(camXf, zeus::SProjPersp(zeus::degToRad(curCam->GetFov()),
                                               g_Viewport.x8_width / float(g_Viewport.xc_height), 1.f, 100.f));
  g_Renderer->SetClippingPlanes(frustum);
  g_Renderer->SetPerspective(curCam->GetFov(), g_Viewport.x8_width, g_Viewport.xc_height, curCam->GetNearClipDistance(),
                             curCam->GetFarClipDistance());
  x0_targetReticule.Draw(mgr, hideLockon);
}

void CTargetingManager::Touch() { x0_targetReticule.Touch(); }

} // namespace urde
