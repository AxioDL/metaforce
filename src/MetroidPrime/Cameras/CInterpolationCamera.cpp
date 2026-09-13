#include "MetroidPrime/Cameras/CInterpolationCamera.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

CInterpolationCamera::CInterpolationCamera(TUniqueId uid, const CTransform4f& xf)
: CGameCamera(uid, false, rstl::string_l("Interpolation Camera"),
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, kInvalidEditorId), xf,
              CCameraManager::GetDefaultThirdPersonVerticalFOV(),
              CCameraManager::GetDefaultFirstPersonNearClipDistance(),
              CCameraManager::GetDefaultFirstPersonFarClipDistance(),
              CCameraManager::GetDefaultAspectRatio(), kInvalidUniqueId, false, 0)
, x188_targetId(kInvalidUniqueId)
, x18c_time(0.f)
, x190_maxTime(0.f)
, x194_startTransform(CTransform4f::Identity())
, x1c4_lookPos(CVector3f::Zero())
, x1d0_positionSpeed(0.f)
, x1d4_rotationSpeed(0.f)
, x1d8_24_sinusoidal(false)
, x1dc_closeInAngle(M_2PIF) {}

CInterpolationCamera::~CInterpolationCamera() {}

void CInterpolationCamera::Reset(const CTransform4f&, CStateManager&) {}

const bool CInterpolationCamera::InterpolateWithDistance(CTransform4f& xf,
                                                         const CVector3f& targetOrigin,
                                                         const CVector3f& lookPos,
                                                         float positionSpeed, float rotationSpeed,
                                                         float dt, float maxTime, float curTime) {
  CVector3f originDir = targetOrigin - xf.GetTranslation();
  CVector3f interpOrigin = xf.GetTranslation();
  positionSpeed *= dt;
  bool ret = false;
  bool positionDone = false;
  if (originDir.CanBeNormalized() && originDir.Magnitude() > positionSpeed) {
    const float lookDist = originDir.Magnitude();
    originDir.Normalize();
    float scale = CMath::Limit(lookDist / 0.5f, 1.f);
    scale *= positionSpeed;
    CVector3f delta = originDir;
    delta *= scale;
    interpOrigin += delta;
    if (lookDist < scale) {
      interpOrigin = targetOrigin;
      positionDone = true;
    }
  } else {
    interpOrigin = targetOrigin;
    positionDone = true;
  }

  CVector3f lookPosDelta = lookPos - x1c4_lookPos;
  if (lookPosDelta.Magnitude() > positionSpeed) {
    const float deltaMag = lookPosDelta.Magnitude();
    lookPosDelta.Normalize();
    float scale = CMath::Limit(deltaMag / 0.5f, 1.f);
    scale *= positionSpeed;
    CVector3f delta = lookPosDelta;
    delta *= scale;
    x1c4_lookPos += delta;
  } else {
    x1c4_lookPos = lookPos;
  }

  CVector3f lookDir = x1c4_lookPos - interpOrigin;
  if (lookDir.CanBeNormalized()) {
    lookDir.Normalize();
  } else {
    lookDir = GetTransform().GetForward();
  }
  const float lookProj = CMath::Limit(CVector3f::Dot(xf.GetForward(), lookDir), 1.f);
  float ang = CMath::Limit(acosf(lookProj) / (M_PIF / 6.f), 1.f);
  ang *= rotationSpeed * dt;
  CVector3f lookDirFlat = lookDir;
  lookDirFlat.SetZ(0.f);
  bool rotationDone = false;
  if (lookDirFlat.CanBeNormalized()) {
    CTransform4f lookXf = CTransform4f::LookAt(interpOrigin, interpOrigin + lookDir);
    if (lookProj < 0.999999f) {
      const CQuaternion q = CQuaternion::LookAt(xf.GetForward(), lookDir, CRelAngle(ang));
      lookXf = q.BuildTransform4f() * xf.GetRotation();
    } else {
      rotationDone = true;
    }
    lookXf.SetTranslation(interpOrigin);
    xf = lookXf;
  } else {
    xf = GetTransform();
    xf.SetTranslation(interpOrigin);
    rotationDone = true;
  }
  if (positionDone && rotationDone) {
    ret = true;
  }
  if (curTime >= maxTime && lookProj >= 0.9999f) {
    ret = true;
  }
  return ret;
}

bool CInterpolationCamera::InterpolateSinusoidal(CTransform4f& xf, const CVector3f& targetOrigin,
                                                 CVector3f lookPos, float maxTime, float curTime) {
  if (curTime > maxTime) {
    curTime = maxTime;
  }
  float t = CMath::Limit(curTime / maxTime, 1.f);
  const float sinT = CMath::Limit(sinf(t * (M_PIF / 2.f)), 1.f);
  const float blend = 2.f * t - sinT;
  const CVector3f interpOrigin = targetOrigin + (1.f - blend) * (GetTranslation() - targetOrigin);
  CVector3f lookDir = lookPos - interpOrigin;
  if (lookDir.CanBeNormalized()) {
    lookDir.Normalize();
  } else {
    lookDir = GetTransform().GetForward();
  }
  CVector3f lookDirFlat = lookDir;
  lookDirFlat.SetZ(0.f);
  if (lookDirFlat.CanBeNormalized()) {
    const float lookProj = CMath::Limit(CVector3f::Dot(GetTransform().GetForward(), lookDir), 1.f);
    float ang = acosf(lookProj) * (1.f - CMath::Limit(2.f * t, 1.f));
    if (ang > x1dc_closeInAngle) {
      ang = x1dc_closeInAngle;
    } else {
      x1dc_closeInAngle = ang;
    }
    CTransform4f lookXf = CTransform4f::LookAt(interpOrigin, interpOrigin + lookDir);
    if (CMath::AbsF(lookProj) < 0.999999f) {
      const CQuaternion q =
          CQuaternion::LookAt(lookDir, GetTransform().GetForward(), CRelAngle(ang));
      const CVector3f xfLookDir = q.Transform(lookDir);
      lookXf = CTransform4f::LookAt(interpOrigin, interpOrigin + xfLookDir);
    }
    xf = lookXf;
  } else {
    xf = GetTransform();
    xf.SetTranslation(interpOrigin);
  }
  return curTime >= maxTime;
}

void CInterpolationCamera::SetInterpolation(const CTransform4f& xf, CVector3f lookPos,
                                            float maxTime, float positionSpeed, float rotationSpeed,
                                            TUniqueId targetId, bool sinusoidal,
                                            CStateManager& mgr) {
  SetActive(true);
  SetTransform(xf);
  x194_startTransform = xf;
  x1c4_lookPos = lookPos;
  x188_targetId = targetId;
  x1d8_24_sinusoidal = sinusoidal;
  x190_maxTime = maxTime;
  x1d0_positionSpeed = positionSpeed;
  x1d4_rotationSpeed = rotationSpeed;
  x1dc_closeInAngle = M_2PIF;
  x18c_time = 0.f;

  if (const CGameCamera* cam = TCastToConstPtr< CGameCamera >(mgr.GetObjectById(targetId))) {
    SetFov(cam->GetFov());
  }
}

void CInterpolationCamera::EndInterpolation(CStateManager& mgr) {
  SetActive(false);
  if (!mgr.GetCameraManager()->ShouldBypassInterpolationCamera()) {
    mgr.CameraManager()->SetCurrentCameraId(x188_targetId);
  }
}

void CInterpolationCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  SetFov(mgr.GetCameraManager()->GetBallCamera()->GetFov());
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
    EndInterpolation(mgr);
  }
  x18c_time += dt;
  if (x18c_time > x190_maxTime) {
    x18c_time = x190_maxTime;
  }
  CTransform4f xf = GetTransform();
  const CGameCamera* cam = TCastToConstPtr< CGameCamera >(mgr.GetObjectById(x188_targetId));
  if (!cam) {
    EndInterpolation(mgr);
    return;
  }
  const CVector3f targetOrigin = cam->GetTranslation();
  CVector3f ballLookPos = mgr.GetCameraManager()->GetBallCamera()->GetLookAtPosition();
  switch (mgr.GetCameraManager()->GetBallCamera()->GetBehaviourType()) {
  case CBallCamera::kBCB_SpindleCamera:
    if (const CScriptSpindleCamera* spindle = TCastToConstPtr< CScriptSpindleCamera >(
            mgr.GetObjectById(mgr.GetCameraManager()->GetSpindleCameraId()))) {
      const CVector3f& ballToSpindle =
          mgr.GetPlayer()->GetBallPosition() - spindle->GetTranslation();
      const CVector3f delta = ballToSpindle;
      const float mag = delta.Magnitude();
      ballLookPos = spindle->GetTranslation() + mag * spindle->GetTransform().GetForward();
    }
    break;
  default:
    break;
  }
  bool deactivate = false;
  if (x1d8_24_sinusoidal) {
    deactivate = InterpolateSinusoidal(xf, targetOrigin, ballLookPos, x190_maxTime, x18c_time);
  } else {
    deactivate = InterpolateWithDistance(xf, targetOrigin, ballLookPos, x1d0_positionSpeed,
                                         x1d4_rotationSpeed, dt, x190_maxTime, x18c_time);
  }
  SetTransform(xf);
  if (deactivate) {
    EndInterpolation(mgr);
  }
}

void CInterpolationCamera::ProcessInput(const CFinalInput&, CStateManager&) {}

ENTITY_ACCEPT_IMPL(CInterpolationCamera)

void CInterpolationCamera::Render(const CStateManager&) const {}

void CInterpolationCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                           CStateManager& mgr) {
  CGameCamera::AcceptScriptMsg(msg, sender, mgr);
}
