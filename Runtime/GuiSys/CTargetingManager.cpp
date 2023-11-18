#include "Runtime/GuiSys/CTargetingManager.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"

namespace metaforce {

CTargetingManager::CTargetingManager(const CStateManager& mgr) : x0_targetReticule(mgr) {}

bool CTargetingManager::CheckLoadComplete() const {
  return x0_targetReticule.CheckLoadComplete() && x21c_orbitPointMarker.CheckLoadComplete();
}

void CTargetingManager::Update(float dt, const CStateManager& stateMgr) {
  x0_targetReticule.Update(dt, stateMgr);
  x21c_orbitPointMarker.Update(dt, stateMgr);
}

void CTargetingManager::Draw(const CStateManager& mgr, bool hideLockon) {
  CGraphics::SetAmbientColor(zeus::skWhite);
  CGraphics::DisableAllLights();
  x21c_orbitPointMarker.Draw(mgr);
  const CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CGraphics::SetViewPointMatrix(camXf);
  zeus::CFrustum frustum;
  frustum.updatePlanes(camXf,
                       zeus::SProjPersp(zeus::degToRad(curCam->GetFov()), CGraphics::GetViewportAspect(), 1.f, 100.f));
  g_Renderer->SetClippingPlanes(frustum);
  g_Renderer->SetPerspective(curCam->GetFov(), CGraphics::GetViewportWidth(), CGraphics::GetViewportHeight(),
                             curCam->GetNearClipDistance(), curCam->GetFarClipDistance());
  x0_targetReticule.Draw(mgr, hideLockon);
}

void CTargetingManager::Touch() { x0_targetReticule.Touch(); }

} // namespace metaforce
