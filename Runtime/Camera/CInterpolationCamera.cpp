#include "Runtime/Camera/CInterpolationCamera.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CBallCamera.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptSpindleCamera.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CInterpolationCamera::CInterpolationCamera(TUniqueId uid, const zeus::CTransform& xf)
: CGameCamera(uid, false, "Interpolation Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, kInvalidEditorId), xf,
              CCameraManager::ThirdPersonFOV(), CCameraManager::NearPlane(), CCameraManager::FarPlane(),
              CCameraManager::Aspect(), kInvalidUniqueId, false, 0) {}

void CInterpolationCamera::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CInterpolationCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CGameCamera::AcceptScriptMsg(msg, sender, mgr);
}

void CInterpolationCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  // Empty
}

void CInterpolationCamera::Render(CStateManager& mgr) {
  // Empty
}

void CInterpolationCamera::Reset(const zeus::CTransform&, CStateManager& mgr) {
  // Empty
}

void CInterpolationCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  x15c_currentFov = mgr.GetCameraManager()->GetBallCamera()->GetFov();
  x170_24_perspDirty = true;
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphing)
    DeactivateInterpCamera(mgr);

  x18c_time += dt;
  if (x18c_time > x190_maxTime)
    x18c_time = x190_maxTime;

  zeus::CTransform xf = GetTransform();

  if (TCastToConstPtr<CGameCamera> cam = mgr.GetObjectById(x188_targetId)) {
    zeus::CVector3f targetOrigin = cam->GetTranslation();
    zeus::CVector3f ballLookPos = mgr.GetCameraManager()->GetBallCamera()->GetLookPos();
    if (mgr.GetCameraManager()->GetBallCamera()->GetBehaviour() == CBallCamera::EBallCameraBehaviour::SpindleCamera) {
      if (TCastToConstPtr<CScriptSpindleCamera> spindle =
              mgr.GetObjectById(mgr.GetCameraManager()->GetSpindleCameraId())) {
        float mag = (mgr.GetPlayer().GetTranslation() - spindle->GetTranslation()).magnitude();
        ballLookPos = spindle->GetTranslation() + (mag * spindle->GetTransform().frontVector());
      }
    }
    bool deactivate = false;

    if (x1d8_24_sinusoidal)
      deactivate = InterpolateSinusoidal(xf, targetOrigin, ballLookPos, x190_maxTime, x18c_time);
    else
      deactivate = InterpolateWithDistance(xf, targetOrigin, ballLookPos, x1d0_positionSpeed, x1d4_rotationSpeed, dt,
                                           x190_maxTime, x18c_time);

    SetTransform(xf);
    if (deactivate)
      DeactivateInterpCamera(mgr);
  } else
    DeactivateInterpCamera(mgr);
}

void CInterpolationCamera::SetInterpolation(const zeus::CTransform& xf, const zeus::CVector3f& lookPos, float maxTime,
                                            float positionSpeed, float rotationSpeed, TUniqueId camId, bool sinusoidal,
                                            CStateManager& mgr) {
  SetActive(true);
  SetTransform(xf);
  x1c4_lookPos = lookPos;
  x188_targetId = camId;
  x1d8_24_sinusoidal = sinusoidal;
  x190_maxTime = maxTime;
  x1d0_positionSpeed = positionSpeed;
  x1d4_rotationSpeed = rotationSpeed;
  x1dc_closeInAngle = 2.f * M_PIF;
  x18c_time = 0.f;

  if (TCastToConstPtr<CGameCamera> cam = (mgr.GetObjectById(camId))) {
    x15c_currentFov = cam->GetFov();
    x170_24_perspDirty = true;
  }
}

void CInterpolationCamera::DeactivateInterpCamera(CStateManager& mgr) {
  SetActive(false);
  if (!mgr.GetCameraManager()->ShouldBypassInterpolation())
    mgr.GetCameraManager()->SetCurrentCameraId(x188_targetId, mgr);
}

bool CInterpolationCamera::InterpolateSinusoidal(zeus::CTransform& xf, const zeus::CVector3f& targetOrigin,
                                                 const zeus::CVector3f& lookPos, float maxTime, float curTime) {
  if (curTime > maxTime)
    curTime = maxTime;

  float t = zeus::clamp(-1.f, curTime / maxTime, 1.f);
  float sinT = std::sin(t * (M_PIF / 2.f));
  t *= 2.f;
  zeus::CVector3f interpOrigin = (1.f - (t - sinT)) * (GetTranslation() - targetOrigin) + targetOrigin;
  zeus::CVector3f lookDir = lookPos - interpOrigin;
  if (lookDir.canBeNormalized())
    lookDir.normalize();
  else
    lookDir = x34_transform.basis[1];
  zeus::CVector3f lookDirFlat = lookDir;
  lookDirFlat.z() = 0.f;
  if (lookDirFlat.canBeNormalized()) {
    t = zeus::clamp(-1.f, t, 1.f);
    float lookProj = zeus::clamp(-1.f, x34_transform.basis[1].dot(lookDir), 1.f);
    float ang = (1.f - t) * std::acos(lookProj);
    if (ang > x1dc_closeInAngle)
      ang = x1dc_closeInAngle;
    else
      x1dc_closeInAngle = ang;
    zeus::CTransform lookXf = zeus::lookAt(interpOrigin, interpOrigin + lookDir);
    if (std::fabs(lookProj) < 0.999999f) {
      zeus::CVector3f xfLookDir = zeus::CQuaternion::lookAt(lookDir, x34_transform.basis[1], ang).transform(lookDir);
      lookXf = zeus::lookAt(interpOrigin, interpOrigin + xfLookDir);
    }
    xf = lookXf;
  } else {
    xf = x34_transform;
    xf.origin = interpOrigin;
  }

  return curTime >= maxTime;
}

bool CInterpolationCamera::InterpolateWithDistance(zeus::CTransform& xf, const zeus::CVector3f& targetOrigin,
                                                   const zeus::CVector3f& lookPos, float positionSpeed,
                                                   float rotationSpeed, float dt, float maxTime, float curTime) {
  zeus::CVector3f interpOrigin = xf.origin;
  zeus::CVector3f originDir = targetOrigin - interpOrigin;
  float sdt = positionSpeed * dt;
  bool ret = false;
  bool positionFail = false;
  if (originDir.canBeNormalized() && originDir.magnitude() > sdt) {
    float lookDist = originDir.magnitude();
    originDir.normalize();
    float scale = zeus::clamp(-1.f, lookDist / 0.5f, 1.f) * sdt;
    interpOrigin += originDir * scale;
    if (lookDist < scale) {
      interpOrigin = targetOrigin;
      positionFail = true;
    }
  } else {
    interpOrigin = targetOrigin;
    positionFail = true;
  }

  zeus::CVector3f lookPosDelta = lookPos - x1c4_lookPos;
  if (lookPosDelta.magnitude() > sdt) {
    float deltaMag = lookPosDelta.magnitude();
    lookPosDelta.normalize();
    float scale = zeus::clamp(-1.f, deltaMag / 0.5f, 1.f) * sdt;
    x1c4_lookPos += lookPosDelta * scale;
  } else {
    x1c4_lookPos = lookPos;
  }

  zeus::CVector3f lookDir = x1c4_lookPos - interpOrigin;
  if (lookDir.canBeNormalized())
    lookDir.normalize();
  else
    lookDir = x34_transform.basis[1];

  float lookProj = zeus::clamp(-1.f, xf.basis[1].dot(lookDir), 1.f);
  float ang = zeus::clamp(-1.f, std::acos(lookProj) / (M_PIF / 6.f), 1.f) * rotationSpeed * dt;

  zeus::CVector3f lookDirFlat = lookDir;
  lookDirFlat.z() = 0.f;
  bool rotationFail = false;
  if (lookDirFlat.canBeNormalized()) {
    zeus::CTransform lookXf = zeus::lookAt(interpOrigin, interpOrigin + lookDir);
    if (lookProj < 0.999999f)
      lookXf = zeus::CQuaternion::lookAt(xf.basis[1], lookDir, ang).toTransform() * xf.getRotation();
    else
      rotationFail = true;
    lookXf.origin = interpOrigin;
    xf = lookXf;
  } else {
    xf = x34_transform;
    xf.origin = interpOrigin;
    rotationFail = true;
  }

  if (positionFail && rotationFail)
    ret = true;

  if (curTime >= maxTime && lookProj >= 0.9999f)
    ret = true;

  return ret;
}

} // namespace urde
