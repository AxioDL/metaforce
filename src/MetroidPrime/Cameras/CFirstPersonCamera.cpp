#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraPitchVolume.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CFirstPersonCamera::CFirstPersonCamera(const TUniqueId& uid, const CTransform4f& xf,
                                       TUniqueId watchedObj, float orbitCameraSpeed, float fov,
                                       float nearz, float farz, float aspect)
: CGameCamera(uid, true, rstl::string_l("First Person Camera"),
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), xf, fov, nearz, farz,
              aspect, watchedObj, false, 0)
, mOrbitCameraSpeed(orbitCameraSpeed)
, mLockCamera(false)
, mGunFollowXf(xf)
, mPitch(0.f)
, mPitchId(kInvalidUniqueId)
, mDeferBallTransitionProcessing(false)
, mCloseInVec(CVector3f::Zero())
, mCloseInTimer(0.f) {}

CFirstPersonCamera::~CFirstPersonCamera() {}

void CFirstPersonCamera::ProcessInput(const CFinalInput&, CStateManager&) {}

void CFirstPersonCamera::UpdateElevation(CStateManager& mgr) {
  mPitch = 0.f;
  if (const CPlayer* const player =
          TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()))) {
    if (mPitchId != kInvalidUniqueId) {
      if (const CScriptCameraPitchVolume* volume =
              TCastToConstPtr< CScriptCameraPitchVolume >(mgr.GetObjectById(mPitchId))) {
        const float upPitch = volume->GetUpPitch().AsRadians();
        const float downPitch = volume->GetDownPitch().AsRadians();
        CVector3f pitchDir = volume->GetTransform().GetForward();
        pitchDir[kDZ] = 0.f;
        if (!pitchDir.CanBeNormalized()) {
          pitchDir = CVector3f(0.f, 1.f, 0.f);
        }
        CVector3f playerDir = player->GetTransform().GetForward();
        playerDir[kDZ] = 0.f;
        playerDir.Normalize();
        float pitchDot = CVector3f::Dot(pitchDir, playerDir);
        pitchDot = CMath::Limit(pitchDot, 1.f);
        if (pitchDot < 0.f) {
          mPitch = downPitch * -pitchDot;
        } else {
          mPitch = upPitch * -pitchDot;
        }

        float pitchMul = 0.f;
        CVector3f delta = player->GetTranslation() - volume->GetTranslation();
        delta[kDZ] = 0.f;
        if (delta.CanBeNormalized()) {
          float projection = CVector3f::Dot(delta, pitchDir);
          projection = CMath::Limit(projection, 1.f);
          projection = CMath::AbsF(projection) * delta.Magnitude();
          const float scale = volume->GetScale().GetY();
          const float maxInterp = volume->GetMaxInterpolationDistance();
          if (projection <= maxInterp) {
            pitchMul = 1.f;
          } else {
            pitchMul = 1.f - CMath::Limit((projection - maxInterp) / (scale - maxInterp), 1.f);
          }
        }
        mPitch *= pitchMul;
      }
    }
  }
}

void CFirstPersonCamera::UpdateTransform(CStateManager& mgr, float dt) {
  CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(GetWatchedObject()));
  if (player == nullptr) {
    SetTransform(CTransform4f::Identity());
    return;
  }

  const CTransform4f playerXf = player->GetTransform();
  float sinPitch = sinf(mPitch);
  sinPitch = CMath::Limit(sinPitch, 1.f);
  float cosPitch = cosf(mPitch);
  cosPitch = CMath::Limit(cosPitch, 1.f);
  CVector3f lookDir = playerXf.Rotate(CVector3f(0.f, cosPitch, sinPitch));
  if (player->IsInFreeLook()) {
    CRelAngle angle(player->GetFreeLookAngleX());
    const CRelAngle maxAngle(gpTweakPlayer->GetVerticalFreeLookAngleVel() -
                             CMath::AbsF(mPitch));
    if (fabs(angle.AsRadians()) > maxAngle.AsRadians()) {
      angle = maxAngle * CMath::Sign(angle.AsRadians());
    }
    CVector3f freeLookDir(sinf(-player->GetFreeLookAngleZ()) * cosf(angle.AsRadians()),
                          cosf(-player->GetFreeLookAngleZ()) * cosf(angle.AsRadians()),
                          sinf(angle.AsRadians()));
    if (gpTweakPlayer->GetFreeLookTurnsPlayer()) {
      freeLookDir[kDX] = 0.f;
      if (!close_enough(freeLookDir, CVector3f::Zero())) {
        freeLookDir.Normalize();
      }
    }
    const CQuaternion pitchRotation =
        CQuaternion::LookAt(CUnitVector3f(0.f, 1.f, 0.f, CUnitVector3f::kN_Yes), lookDir,
                            CRelAngle::FromRadians(M_2PIF));
    lookDir = pitchRotation.Transform(freeLookDir);
  }

  CVector3f eyePos = player->GetEyePosition();
  if (mCloseInTimer > 0.f) {
    eyePos += CMath::Clamp(0.f, mCloseInTimer / 2.f, 1.f) * mCloseInVec;
    CPlayerCameraBob* bob = player->CameraBobObject();
    bob->ResetCameraBobTime();
    bob->SetCameraBobTransform(CTransform4f::Identity());
  }

  switch (player->GetOrbitState()) {
  case CPlayer::kOS_OrbitObject:
  case CPlayer::kOS_ForcedOrbitObject: {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(player->GetOrbitTargetId()));
    if (actor != nullptr && actor->GetMaterialList().HasMaterial(kMT_Orbit)) {
      CVector3f orbitDir = player->GetOrbitPoint() - eyePos;
      if (orbitDir.CanBeNormalized()) {
        orbitDir.Normalize();
      }
      lookDir = orbitDir;
    } else {
      lookDir = player->GetOrbitPoint() - eyePos;
    }
    break;
  }
  case CPlayer::kOS_OrbitPoint:
  case CPlayer::kOS_OrbitCarcass:
    if (!player->IsLookButtonHeld()) {
      lookDir = player->GetOrbitPoint() - eyePos;
    }
    break;
  case CPlayer::kOS_NoOrbit:
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
        !player->IsInFreeLook() && mPitchId == kInvalidUniqueId) {
      if (player->GetJumpCameraTimer() > 0.f) {
        float t = (player->GetJumpCameraTimer() - gpTweakPlayer->GetJumpCameraPitchDownStart()) /
                  gpTweakPlayer->GetJumpCameraPitchDownFull();
        t = CMath::Clamp(0.f, t, 1.f);
        float angle = t * gpTweakPlayer->GetJumpCameraPitchDownAngle();
        angle += mPitch;
        lookDir = CVector3f(0.f, cosf(angle), -sinf(angle));
        lookDir = playerXf.Rotate(lookDir);
      } else if (player->GetFallCameraTimer() > 0.f) {
        float t = (player->GetFallCameraTimer() - gpTweakPlayer->GetFallCameraPitchDownStart()) /
                  gpTweakPlayer->GetFallCameraPitchDownFull();
        t = CMath::Clamp(0.f, t, 1.f);
        const float angle = t * gpTweakPlayer->GetFallCameraPitchDownAngle();
        lookDir = CVector3f(0.f, cosf(angle), -sinf(angle));
        lookDir = playerXf.Rotate(lookDir);
      }
    }
    break;
  case CPlayer::kOS_Grapple:
  default:
    break;
  }

  if (lookDir.CanBeNormalized()) {
    lookDir.Normalize();
  }
  float angularStep = dt;
  CQuaternion gunRotation = CQuaternion::NoRotation();
  CTransform4f gunXf = mGunFollowXf;
  if (!player->IsInFreeLook()) {
    switch (player->GetOrbitState()) {
    default: {
      CVector3f gunFront = mGunFollowXf.GetForward();
      gunFront[kDZ] = 0.f;
      if (gunFront.CanBeNormalized()) {
        gunFront.Normalize();
      }
      CVector3f flatLookDir = lookDir;
      flatLookDir[kDZ] = 0.f;
      if (flatLookDir.CanBeNormalized()) {
        flatLookDir.Normalize();
      }
      const CQuaternion yawRotation =
          CQuaternion::LookAt(gunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
      gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
      CVector3f newFront = gunXf.GetForward();
      if (newFront.CanBeNormalized()) {
        newFront.Normalize();
      }
      angularStep *= gpTweakPlayer->GetFirstPersonCameraSpeed();
      float angle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
      float t = acosf(angle) / angularStep;
      t = CMath::Clamp(0.f, t, 1.f);
      gunRotation = CQuaternion::LookAt(newFront, lookDir, CRelAngle::FromRadians(angularStep) * t);
      break;
    }
    case CPlayer::kOS_Grapple: {
      CVector3f gunFront = mGunFollowXf.GetForward();
      gunFront[kDZ] = 0.f;
      if (gunFront.CanBeNormalized()) {
        gunFront.Normalize();
      }
      CVector3f flatLookDir = lookDir;
      flatLookDir[kDZ] = 0.f;
      if (flatLookDir.CanBeNormalized()) {
        flatLookDir.Normalize();
      }
      const CQuaternion yawRotation =
          CQuaternion::LookAt(gunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
      gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
      CVector3f newFront = gunXf.GetForward();
      if (newFront.CanBeNormalized()) {
        newFront.Normalize();
      }
      angularStep *= gpTweakPlayer->GetGrappleCameraSpeed();
      float angle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
      float t = acosf(angle) / angularStep;
      t = CMath::Clamp(0.f, t, 1.f);
      gunRotation = CQuaternion::LookAt(newFront, lookDir, CRelAngle::FromRadians(angularStep) * t);
      break;
    }
    case CPlayer::kOS_OrbitPoint:
    case CPlayer::kOS_OrbitCarcass: {
      CVector3f gunFront = mGunFollowXf.GetForward();
      gunFront[kDZ] = 0.f;
      if (gunFront.CanBeNormalized()) {
        gunFront.Normalize();
      }
      CVector3f flatLookDir = lookDir;
      flatLookDir[kDZ] = 0.f;
      if (flatLookDir.CanBeNormalized()) {
        flatLookDir.Normalize();
      }
      const CQuaternion yawRotation =
          CQuaternion::LookAt(gunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
      gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
      CVector3f newFront = gunXf.GetForward();
      if (newFront.CanBeNormalized()) {
        newFront.Normalize();
      }
      const CRelAngle scaledAngle =
          CRelAngle::FromRadians(angularStep * gpTweakPlayer->GetOrbitCameraSpeed()) * 0.25f;
      float angle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
      float t = acosf(angle) / scaledAngle.AsRadians();
      t = CMath::Clamp(0.f, t, 1.f);
      gunRotation = CQuaternion::LookAt(newFront, lookDir, scaledAngle * t);
      break;
    }
    case CPlayer::kOS_ForcedOrbitObject:
    case CPlayer::kOS_OrbitObject: {
      CVector3f gunFront = mGunFollowXf.GetForward();
      if (gunFront.CanBeNormalized()) {
        gunFront.Normalize();
      }
      angularStep *= gpTweakPlayer->GetOrbitCameraSpeed();
      float angle = CMath::Limit(CVector3f::Dot(gunFront, lookDir), 1.f);
      float t = acosf(angle) / angularStep;
      t = CMath::Clamp(0.f, t, 1.f);
      if (angle > 0.9999f || mLockCamera || player->GetOrbitLockAcquired()) {
        gunRotation = CQuaternion::LookAt(gunFront, lookDir, CRelAngle::FromRadians(M_2PIF));
      } else {
        gunRotation =
            CQuaternion::LookAt(gunFront, lookDir, CRelAngle::FromRadians(angularStep) * t);
      }

      const CScriptGrapplePoint* grapple =
          TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(player->GetOrbitTargetId()));
      if (grapple != nullptr && player->GetFallCameraTimer() > 0.f) {
        CVector3f flatGunFront = mGunFollowXf.GetForward();
        flatGunFront[kDZ] = 0.f;
        if (flatGunFront.CanBeNormalized()) {
          flatGunFront.Normalize();
        }
        CVector3f flatLookDir = lookDir;
        flatLookDir[kDZ] = 0.f;
        if (flatLookDir.CanBeNormalized()) {
          flatLookDir.Normalize();
        }
        const CQuaternion yawRotation =
            CQuaternion::LookAt(flatGunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
        gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
        CVector3f newFront = gunXf.GetForward();
        if (newFront.CanBeNormalized()) {
          newFront.Normalize();
        }
        // Retail evaluates this interpolation even though it uses a full rotation below.
        float grappleDt = dt * gpTweakPlayer->GetGrappleCameraSpeed();
        float grappleAngle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
        float grappleT = acosf(grappleAngle) / grappleDt;
        t = CMath::Clamp(0.f, grappleT, 1.f);
        gunRotation = CQuaternion::LookAt(newFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
      }
      break;
    }
    }
  } else {
    CVector3f gunFront = mGunFollowXf.GetForward();
    gunFront[kDZ] = 0.f;
    if (gunFront.CanBeNormalized()) {
      gunFront.Normalize();
    }
    CVector3f flatLookDir = lookDir;
    flatLookDir[kDZ] = 0.f;
    if (flatLookDir.CanBeNormalized()) {
      flatLookDir.Normalize();
    }
    const CQuaternion yawRotation =
        CQuaternion::LookAt(gunFront, flatLookDir, CRelAngle::FromRadians(M_2PIF));
    gunXf = yawRotation.BuildTransform4f() * mGunFollowXf.GetRotation();
    CVector3f newFront = gunXf.GetForward();
    if (newFront.CanBeNormalized()) {
      newFront.Normalize();
    }
    angularStep *= gpTweakPlayer->GetFreeLookSpeed();
    float angle = CMath::Limit(CVector3f::Dot(newFront, lookDir), 1.f);
    float t = gpTweakPlayer->GetFreeLookDampenFactor() * (acosf(angle) / angularStep);
    t = CMath::Clamp(0.f, t, 1.f);
    gunRotation = CQuaternion::LookAt(newFront, lookDir, CRelAngle::FromRadians(angularStep) * t);
  }

  CPlayerCameraBob* bob = player->CameraBobObject();
  CTransform4f bobXf = bob->GetCameraBobTransformation();
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed ||
      player->GetOrbitState() == CPlayer::kOS_Grapple ||
      player->GetGrappleState() != CPlayer::kGS_None ||
      mgr.GetGameState() == CStateManager::kGS_SoftPaused ||
      mgr.GetCameraManager()->IsInCinematicCamera() || mCloseInTimer > 0.f) {
    bobXf = CTransform4f::Identity();
    bob->SetCameraBobTransform(bobXf);
  }
  mGunFollowXf = gunRotation.BuildTransform4f() * gunXf;
  SetTransform(mGunFollowXf * bobXf.GetRotation());
  mGunFollowXf.SetTranslation(eyePos);
  SetTranslation(eyePos + player->GetTransform().Rotate(bobXf.GetTranslation()));
  mGunFollowXf.Orthonormalize();
}

void CFirstPersonCamera::PreThink(float, CStateManager&) {}

void CFirstPersonCamera::Render(const CStateManager&) const {}

void CFirstPersonCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  SetTransform(xf);
  SetTranslation(mgr.GetPlayer()->GetEyePosition());
  mGunFollowXf = GetTransform();
}

void CFirstPersonCamera::CancelCinematicOffset() {
  mCloseInVec = CVector3f::Zero();
  mCloseInTimer = 0.f;
}

void CFirstPersonCamera::Think(float dt, CStateManager& mgr) {
  if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(GetWatchedObject()))) {
    if (!mDeferBallTransitionProcessing) {
      if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
        if (player->GetCameraState() != CPlayer::kCS_Spawned) {
          return;
        }
        SetTransform(player->CreateTransformFromMovementDirection());
        SetTranslation(player->GetEyePosition());
        return;
      }
      if (player->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
        if (player->GetMorphballTransitionState() != CPlayer::kMS_Unmorphing) {
          return;
        }
        if (!close_enough(player->GetMorphBallTransitionFactor(), 1.f)) {
          return;
        }
      }
    } else {
      mDeferBallTransitionProcessing = false;
    }
    const CTransform4f backupXf = GetTransform();
    UpdateElevation(mgr);
    UpdateTransform(mgr, dt);
    SetTransform(ValidateCameraTransform(GetTransform(), backupXf));
    if (mCloseInTimer > 0.f) {
      mCloseInTimer -= dt;
    }
  }
}

ENTITY_ACCEPT_IMPL(CFirstPersonCamera)

const CTransform4f& CFirstPersonCamera::GetGunFollowTransform() const { return mGunFollowXf; }
