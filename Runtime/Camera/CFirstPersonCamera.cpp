#include "Runtime/Camera/CFirstPersonCamera.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptCameraPitchVolume.hpp"
#include "Runtime/World/CScriptGrapplePoint.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace DataSpec::DNAMP1 {
extern hecl::CVar* tw_fov;
}

namespace urde {

CFirstPersonCamera::CFirstPersonCamera(TUniqueId uid, const zeus::CTransform& xf, TUniqueId watchedObj,
                                       float orbitCameraSpeed, float fov, float nearz, float farz, float aspect)
: CGameCamera(uid, true, "First Person Camera", CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), xf, fov,
              nearz, farz, aspect, watchedObj, false, 0)
, x188_orbitCameraSpeed(orbitCameraSpeed)
, x190_gunFollowXf(xf) {
  DataSpec::DNAMP1::tw_fov->addListener([this](hecl::CVar* cv) { _fovListener(cv); });
}

void CFirstPersonCamera::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFirstPersonCamera::PreThink(float dt, CStateManager& mgr) {
  // Empty
}

void CFirstPersonCamera::Think(float dt, CStateManager& mgr) {
  if (TCastToPtr<CPlayer> player = mgr.ObjectById(xe8_watchedObject)) {
    if (!x1c6_24_deferBallTransitionProcessing) {
      if (player->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
        if (player->GetCameraState() != CPlayer::EPlayerCameraState::Spawned)
          return;
        SetTransform(player->CreateTransformFromMovementDirection());
        SetTranslation(player->GetEyePosition());
        return;
      }
      if (player->GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphed) {
        if (player->GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphing)
          return;
        if (std::fabs(player->GetMorphFactor() - 1.f) >= 0.00001f)
          return;
      }
    } else {
      x1c6_24_deferBallTransitionProcessing = false;
    }
    zeus::CTransform backupXf = x34_transform;
    UpdateElevation(mgr);
    UpdateTransform(mgr, dt);
    SetTransform(ValidateCameraTransform(x34_transform, backupXf));
    if (x1d4_closeInTimer > 0.f)
      x1d4_closeInTimer -= dt;
  }
}

void CFirstPersonCamera::ProcessInput(const CFinalInput&, CStateManager& mgr) {
  // Empty
}

void CFirstPersonCamera::Reset(const zeus::CTransform& xf, CStateManager& mgr) {
  SetTransform(xf);
  SetTranslation(mgr.GetPlayer().GetEyePosition());
  x190_gunFollowXf = x34_transform;
}

void CFirstPersonCamera::SkipCinematic() {
  x1c8_closeInVec = zeus::skZero3f;
  x1d4_closeInTimer = 0.f;
}

void CFirstPersonCamera::CalculateGunFollowOrientationAndTransform(zeus::CTransform& gunXf, zeus::CQuaternion& gunQ,
                                                                   float dt, zeus::CVector3f& rVec) const {
  zeus::CVector3f gunFrontVec = x190_gunFollowXf.frontVector();
  gunFrontVec.z() = 0.f;
  if (gunFrontVec.canBeNormalized())
    gunFrontVec.normalize();

  zeus::CVector3f rVecNoZ = rVec;
  rVecNoZ.z() = 0.f;
  if (rVecNoZ.canBeNormalized())
    rVecNoZ.normalize();

  gunXf = zeus::CQuaternion::lookAt(gunFrontVec, rVecNoZ, 2.f * M_PIF).toTransform() * x190_gunFollowXf.getRotation();

  zeus::CVector3f newgunFront = gunXf.frontVector();
  if (newgunFront.canBeNormalized())
    newgunFront.normalize();

  float angle = newgunFront.dot(rVec);
  angle = zeus::clamp(-1.f, angle, 1.f);
  gunQ = zeus::CQuaternion::lookAt(newgunFront, rVec, zeus::clamp(0.f, std::acos(angle) / dt, 1.f) * dt);
}

void CFirstPersonCamera::UpdateTransform(CStateManager& mgr, float dt) {
  TCastToPtr<CPlayer> player(mgr.ObjectById(GetWatchedObject()));
  if (!player) {
    SetTransform(zeus::CTransform());
    return;
  }

  zeus::CTransform playerXf = player->GetTransform();
  zeus::CVector3f rVec = playerXf.rotate(
      {0.f, zeus::clamp(-1.f, std::cos(x1c0_pitch), 1.0f), zeus::clamp(-1.f, std::sin(x1c0_pitch), 1.0f)});
  if (player->x3dc_inFreeLook) {
    float angle = player->x3ec_freeLookPitchAngle;
    float angleClamp = g_tweakPlayer->GetVerticalFreeLookAngleVel() - std::fabs(x1c0_pitch);
    angle = zeus::clamp(-angleClamp, angle, angleClamp);
    zeus::CVector3f vec;
    vec.z() = std::sin(angle);
    vec.y() = std::cos(-player->x3e4_freeLookYawAngle) * std::cos(angle);
    vec.x() = std::sin(-player->x3e4_freeLookYawAngle) * std::cos(angle);
    if (g_tweakPlayer->GetFreeLookTurnsPlayer()) {
      vec.x() = 0.f;
      if (!zeus::close_enough(vec, zeus::skZero3f))
        vec.normalize();
    }

    rVec = zeus::CQuaternion::lookAt({0.f, 1.f, 0.f}, rVec, 2.f * M_PIF).transform(vec);
  }

  zeus::CVector3f eyePos = player->GetEyePosition();
  if (x1d4_closeInTimer > 0.f) {
    eyePos += zeus::clamp(0.f, 0.5f * x1d4_closeInTimer, 1.f) * x1c8_closeInVec;
    player->GetCameraBob()->ResetCameraBobTime();
    player->GetCameraBob()->SetCameraBobTransform(zeus::CTransform());
  }

  switch (player->GetOrbitState()) {
  case CPlayer::EPlayerOrbitState::ForcedOrbitObject:
  case CPlayer::EPlayerOrbitState::OrbitObject: {
    const CActor* act = TCastToConstPtr<CActor>(mgr.GetObjectById(player->x310_orbitTargetId));
    if (act && act->GetMaterialList().HasMaterial(EMaterialTypes::Orbit)) {
      zeus::CVector3f v = player->x314_orbitPoint - eyePos;
      if (v.canBeNormalized())
        v.normalize();

      rVec = v;
    } else {
      rVec = player->x314_orbitPoint - eyePos;
    }
    break;
  }
  case CPlayer::EPlayerOrbitState::OrbitPoint:
  case CPlayer::EPlayerOrbitState::OrbitCarcass: {
    if (!player->x3dd_lookButtonHeld) {
      rVec = player->x314_orbitPoint - eyePos;
    }
    break;
  }
  case CPlayer::EPlayerOrbitState::NoOrbit: {
    if (player->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
        !player->x3dc_inFreeLook && x1c4_pitchId == kInvalidUniqueId) {
      if (player->x294_jumpCameraTimer > 0.f) {
        float angle = zeus::clamp(0.f,
                                  (player->x294_jumpCameraTimer - g_tweakPlayer->GetJumpCameraPitchDownStart()) /
                                      g_tweakPlayer->GetJumpCameraPitchDownFull(),
                                  1.f) *
                      g_tweakPlayer->GetJumpCameraPitchDownAngle();
        angle += x1c0_pitch;
        rVec.x() = 0.f;
        rVec.y() = std::cos(angle);
        rVec.z() = -std::sin(angle);

        rVec = playerXf.rotate(rVec);
      } else if (player->x29c_fallCameraTimer > 0.f) {
        float angle = zeus::clamp(0.f,
                                  (player->x29c_fallCameraTimer - g_tweakPlayer->GetFallCameraPitchDownStart()) /
                                      g_tweakPlayer->GetFallCameraPitchDownFull(),
                                  1.f) *
                      g_tweakPlayer->GetFallCameraPitchDownAngle();
        rVec.x() = 0.f;
        rVec.y() = std::cos(angle);
        rVec.z() = -std::sin(angle);

        rVec = playerXf.rotate(rVec);
      }
    }
    break;
  }
  default:
    break;
  }

  if (rVec.canBeNormalized())
    rVec.normalize();

  zeus::CTransform gunXf = x190_gunFollowXf;
  zeus::CQuaternion qGun;

  if (!player->x3dc_inFreeLook) {
    switch (player->GetOrbitState()) {
    default: {
      CalculateGunFollowOrientationAndTransform(gunXf, qGun, dt * g_tweakPlayer->GetFirstPersonCameraSpeed(), rVec);
      break;
    }
    case CPlayer::EPlayerOrbitState::Grapple: {
      CalculateGunFollowOrientationAndTransform(gunXf, qGun, dt * g_tweakPlayer->GetGrappleCameraSpeed(), rVec);
      break;
    }
    case CPlayer::EPlayerOrbitState::OrbitPoint:
    case CPlayer::EPlayerOrbitState::OrbitCarcass: {
      CalculateGunFollowOrientationAndTransform(gunXf, qGun, dt * g_tweakPlayer->GetOrbitCameraSpeed() * 0.25f, rVec);
      break;
    }
    case CPlayer::EPlayerOrbitState::ForcedOrbitObject:
    case CPlayer::EPlayerOrbitState::OrbitObject: {
      zeus::CVector3f gunFrontVec = x190_gunFollowXf.frontVector();

      if (gunFrontVec.canBeNormalized())
        gunFrontVec.normalize();

      float scaledDt = (dt * g_tweakPlayer->GetOrbitCameraSpeed());
      float angle = gunFrontVec.dot(rVec);
      angle = zeus::clamp(-1.f, angle, 1.f);
      float clampedAngle = zeus::clamp(0.f, std::acos(angle) / scaledDt, 1.f);
      if (angle > 0.999f || x18c_lockCamera || player->x374_orbitLockEstablished)
        qGun = zeus::CQuaternion::lookAt(gunFrontVec, rVec, 2.f * M_PIF);
      else
        qGun = zeus::CQuaternion::lookAt(gunFrontVec, rVec, scaledDt * clampedAngle);

      const CScriptGrapplePoint* gPoint =
          TCastToConstPtr<CScriptGrapplePoint>(mgr.GetObjectById(player->x310_orbitTargetId));
      if (gPoint && player->x29c_fallCameraTimer > 0.f) {
        gunFrontVec = x190_gunFollowXf.frontVector();
        if (gunFrontVec.canBeNormalized())
          gunFrontVec.normalize();

        zeus::CVector3f rVecCpy = rVec;
        rVecCpy.z() = 0.f;
        if (rVecCpy.canBeNormalized())
          rVecCpy.normalize();

        gunXf =
            zeus::CQuaternion::lookAt(gunFrontVec, rVecCpy, 2.f * M_PIF).toTransform() * x190_gunFollowXf.getRotation();

        gunFrontVec = gunXf.frontVector();
        if (gunFrontVec.canBeNormalized())
          gunFrontVec.normalize();

        // float angle = gunFrontVec.dot(rVec);
        // float sdt = dt * g_tweakPlayer->GetGrappleCameraSpeed();

        // angle = zeus::clamp(-1.f, angle, 1.f);
        // angle = zeus::clamp(0.f, std::acos(angle) / sdt, 1.f);
        qGun = zeus::CQuaternion::lookAt(gunFrontVec, rVecCpy, 2.f * M_PIF);
      }
      break;
    }
    }
  } else {
    zeus::CVector3f gunFront = x190_gunFollowXf.frontVector();
    gunFront.z() = 0.f;
    if (gunFront.canBeNormalized())
      gunFront.normalize();

    zeus::CVector3f rVecCpy = rVec;
    rVecCpy.z() = 0.f;
    if (rVecCpy.canBeNormalized())
      rVecCpy.normalize();

    gunXf = zeus::CQuaternion::lookAt(gunFront, rVecCpy, 2.f * M_PIF).toTransform() * x190_gunFollowXf.getRotation();
    gunFront = gunXf.frontVector();
    if (gunFront.canBeNormalized())
      gunFront.normalize();

    float angle = gunFront.dot(rVec);
    angle = zeus::clamp(-1.f, angle, 1.f);
    float sdt = dt * g_tweakPlayer->GetFreeLookSpeed();
    qGun = zeus::CQuaternion::lookAt(
        gunFront, rVec,
        sdt * zeus::clamp(0.f, g_tweakPlayer->GetFreeLookDampenFactor() * (std::acos(angle) / sdt), 1.f));
  }
  zeus::CTransform bobXf = player->GetCameraBob()->GetCameraBobTransformation();

  if (player->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ||
      player->GetOrbitState() == CPlayer::EPlayerOrbitState::Grapple ||
      player->GetGrappleState() != CPlayer::EGrappleState::None ||
      mgr.GetGameState() == CStateManager::EGameState::SoftPaused || mgr.GetCameraManager()->IsInCinematicCamera() ||
      x1d4_closeInTimer > 0.f) {
    bobXf = zeus::CTransform();
    player->GetCameraBob()->SetCameraBobTransform(bobXf);
  }

  x190_gunFollowXf = qGun.toTransform() * gunXf;
  SetTransform(x190_gunFollowXf * bobXf.getRotation());
  x190_gunFollowXf.origin = eyePos;
  CActor::SetTranslation(eyePos + player->GetTransform().rotate(bobXf.origin));
  x190_gunFollowXf.orthonormalize();
}

void CFirstPersonCamera::UpdateElevation(CStateManager& mgr) {
  x1c0_pitch = 0.f;
  if (TCastToConstPtr<CPlayer> player = mgr.GetObjectById(xe8_watchedObject)) {
    if (x1c4_pitchId != kInvalidUniqueId) {
      if (TCastToConstPtr<CScriptCameraPitchVolume> pvol = mgr.GetObjectById(x1c4_pitchId)) {
        zeus::CVector3f pitchDirFlat = pvol->GetTransform().basis[1];
        pitchDirFlat.z() = 0.f;
        if (!pitchDirFlat.canBeNormalized())
          pitchDirFlat = zeus::skForward;

        zeus::CVector3f playerDirFlat = player->GetTransform().basis[1];
        playerDirFlat.z() = 0.f;
        playerDirFlat.normalize();

        float pitchDot = zeus::clamp(-1.f, pitchDirFlat.dot(playerDirFlat), 1.f);
        if (pitchDot < 0.f)
          x1c0_pitch = pvol->GetDownPitch() * -pitchDot;
        else
          x1c0_pitch = pvol->GetUpPitch() * -pitchDot;

        zeus::CVector3f pvolToPlayerFlat = player->GetTranslation() - pvol->GetTranslation();
        pvolToPlayerFlat.z() = 0.f;
        float pitchMul = 0.f;
        if (pvolToPlayerFlat.canBeNormalized()) {
          float pvolPlayerProj =
              std::fabs(zeus::clamp(-1.f, pvolToPlayerFlat.dot(pitchDirFlat), 1.f)) * pvolToPlayerFlat.magnitude();
          if (pvolPlayerProj <= pvol->GetMaxInterpolationDistance())
            pitchMul = 1.f;
          else
            pitchMul = 1.f - zeus::clamp(-1.f,
                                         (pvolPlayerProj - pvol->GetMaxInterpolationDistance()) /
                                             (pvol->GetScale().y() - pvol->GetMaxInterpolationDistance()),
                                         1.f);
        }
        x1c0_pitch *= pitchMul;
      }
    }
  }
}

void CFirstPersonCamera::_fovListener(hecl::CVar* cv) {
  x15c_currentFov = x180_perspInterpStartFov = x184_perspInterpEndFov = cv->toReal();
  x170_24_perspDirty = true;
}

} // namespace urde
