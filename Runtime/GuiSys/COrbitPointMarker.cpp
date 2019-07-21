#include "COrbitPointMarker.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "Camera/CGameCamera.hpp"
#include "zeus/CEulerAngles.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde {

COrbitPointMarker::COrbitPointMarker() {
  x0_zOffset = g_tweakTargeting->GetOrbitPointZOffset();
  x28_orbitPointModel = g_SimplePool->GetObj("CMDL_OrbitPoint");
}

bool COrbitPointMarker::CheckLoadComplete() { return x28_orbitPointModel.IsLoaded(); }

void COrbitPointMarker::Update(float dt, const CStateManager& mgr) {
  x24_curTime += dt;
  const CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CPlayer::EPlayerOrbitState orbitState = mgr.GetPlayer().GetOrbitState();
  bool freeOrbit = orbitState >= CPlayer::EPlayerOrbitState::OrbitPoint;
  if (freeOrbit != x1c_lastFreeOrbit) {
    if (orbitState == CPlayer::EPlayerOrbitState::OrbitPoint ||
        orbitState == CPlayer::EPlayerOrbitState::OrbitCarcass) {
      ResetInterpolationTimer(g_tweakTargeting->GetOrbitPointInTime());
      zeus::CVector3f orbitTargetPosition = mgr.GetPlayer().GetHUDOrbitTargetPosition();
      if (!x4_camRelZPos)
        x10_lagTargetPos = orbitTargetPosition + zeus::CVector3f(0.f, 0.f, x0_zOffset);
      else
        x10_lagTargetPos = zeus::CVector3f(orbitTargetPosition.x(), orbitTargetPosition.y(),
                                           curCam->GetTranslation().z() + x0_zOffset);
      x8_lagAzimuth = zeus::CEulerAngles(zeus::CQuaternion(curCam->GetTransform().basis)).z() + zeus::degToRad(45.f);
    } else {
      ResetInterpolationTimer(g_tweakTargeting->GetOrbitPointOutTime());
    }
    x1c_lastFreeOrbit = !x1c_lastFreeOrbit;
  }

  if (x20_interpTimer > 0.f)
    x20_interpTimer = std::max(0.f, x20_interpTimer - dt);

  if (!x4_camRelZPos) {
    zeus::CVector3f orbitTargetPosition = mgr.GetPlayer().GetHUDOrbitTargetPosition();
    if ((orbitTargetPosition.z() + x0_zOffset) - x10_lagTargetPos.z() < 0.1f)
      x10_lagTargetPos = orbitTargetPosition + zeus::CVector3f(0.f, 0.f, x0_zOffset);
    else if ((orbitTargetPosition.z() + x0_zOffset) - x10_lagTargetPos.z() < 0.f)
      x10_lagTargetPos = zeus::CVector3f(orbitTargetPosition.x(), orbitTargetPosition.y(), x10_lagTargetPos.z() - 0.1f);
    else
      x10_lagTargetPos = zeus::CVector3f(orbitTargetPosition.x(), orbitTargetPosition.y(), x10_lagTargetPos.z() + 0.1f);
  } else {
    zeus::CVector3f orbitTargetPosition = mgr.GetPlayer().GetHUDOrbitTargetPosition();
    x10_lagTargetPos =
        zeus::CVector3f(orbitTargetPosition.x(), orbitTargetPosition.y(), x0_zOffset + orbitTargetPosition.z());
  }

  if (x1c_lastFreeOrbit) {
    float newAzimuth = zeus::CEulerAngles(zeus::CQuaternion(curCam->GetTransform().basis)).z() + zeus::degToRad(45.f);
    float aziDelta = newAzimuth - xc_azimuth;
    if (mgr.GetPlayer().IsInFreeLook())
      x8_lagAzimuth += aziDelta;
    xc_azimuth = newAzimuth;
  }
}

void COrbitPointMarker::Draw(const CStateManager& mgr) const {
  if ((x1c_lastFreeOrbit || x20_interpTimer > 0.f) && g_tweakTargeting->DrawOrbitPoint() &&
      x28_orbitPointModel.IsLoaded()) {
    SCOPED_GRAPHICS_DEBUG_GROUP("COrbitPointMarker::Draw", zeus::skCyan);
    const CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    CGraphics::SetViewPointMatrix(camXf);
    zeus::CFrustum frustum = mgr.SetupDrawFrustum(g_Viewport);
    frustum.updatePlanes(camXf, zeus::SProjPersp(zeus::degToRad(curCam->GetFov()),
                                                 g_Viewport.aspect, 1.f, 100.f));
    g_Renderer->SetClippingPlanes(frustum);
    g_Renderer->SetPerspective(curCam->GetFov(), g_Viewport.x8_width, g_Viewport.xc_height,
                               curCam->GetNearClipDistance(), curCam->GetFarClipDistance());
    float scale;
    if (x1c_lastFreeOrbit)
      scale = 1.f - x20_interpTimer / g_tweakTargeting->GetOrbitPointInTime();
    else
      scale = x20_interpTimer / g_tweakTargeting->GetOrbitPointOutTime();
    zeus::CTransform modelXf = zeus::CTransform::RotateZ(x8_lagAzimuth);
    modelXf.scaleBy(scale);
    modelXf.origin += x10_lagTargetPos;
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetOrbitPointColor();
    color.a() *= scale;
    CModelFlags flags(7, 0, 0, color);
    x28_orbitPointModel->Draw(flags);
  }
}

} // namespace urde
