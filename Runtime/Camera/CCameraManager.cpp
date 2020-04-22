#include "Runtime/Camera/CCameraManager.hpp"

#include <algorithm>

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CBallCamera.hpp"
#include "Runtime/Camera/CCameraShakeData.hpp"
#include "Runtime/Camera/CCinematicCamera.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Camera/CInterpolationCamera.hpp"
#include "Runtime/Camera/CPathCamera.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptCameraHint.hpp"
#include "Runtime/World/CScriptSpindleCamera.hpp"
#include "Runtime/World/CScriptWater.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
float CCameraManager::sFirstPersonFOV = 55.f;

CCameraManager::CCameraManager(TUniqueId curCameraId) : x0_curCameraId(curCameraId) {
  CSfxManager::AddListener(CSfxManager::ESfxChannels::Game, zeus::skZero3f, zeus::skZero3f, {1.f, 0.f, 0.f},
                           {0.f, 0.f, 1.f}, 50.f, 50.f, 1000.f, 1, 1.f);
  sFirstPersonFOV = g_tweakGame->GetFirstPersonFOV();
}

bool CCameraManager::IsInFirstPersonCamera() const { return x7c_fpCamera->GetUniqueId() == x0_curCameraId; }

zeus::CVector3f CCameraManager::GetGlobalCameraTranslation(const CStateManager& stateMgr) const {
  const CGameCamera* camera = GetCurrentCamera(stateMgr);
  return camera->GetTransform().rotate(x30_shakeOffset);
}

zeus::CTransform CCameraManager::GetCurrentCameraTransform(const CStateManager& stateMgr) const {
  const CGameCamera* camera = GetCurrentCamera(stateMgr);
  return camera->GetTransform() * zeus::CTransform::Translate(x30_shakeOffset);
}

void CCameraManager::RemoveCameraShaker(u32 id) {
  const auto iter = std::find_if(x14_shakers.cbegin(), x14_shakers.cend(),
                                 [id](const auto& shaker) { return shaker.xbc_shakerId == id; });
  if (iter == x14_shakers.cend()) {
    return;
  }
  x14_shakers.erase(iter);
}

int CCameraManager::AddCameraShaker(const CCameraShakeData& data, bool sfx) {
  x14_shakers.emplace_back(data).xbc_shakerId = ++x2c_lastShakeId;
  if (!xa0_24_pendingRumble) {
    xa0_24_pendingRumble = true;
    x90_rumbleCooldown = 0.5f;
  }
  if (sfx && data.x0_duration > 0.f) {
    float vol = zeus::clamp(100.f, std::max(data.GetMaxAMComponent(), data.GetMaxFMComponent()) * 9.f + 100.f, 127.f);
    CSfxHandle sfxHandle;
    if (data.xc0_flags & 0x1)
      sfxHandle = CSfxManager::AddEmitter(SFXamb_x_rumble_lp_00, data.xc4_sfxPos, zeus::skZero3f, vol / 127.f,
                                          false, false, 0x7f, kInvalidAreaId);
    else
      sfxHandle = CSfxManager::SfxStart(SFXamb_x_rumble_lp_00, vol / 127.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    sfxHandle->SetTimeRemaining(data.x0_duration);
  }
  return x2c_lastShakeId;
}

void CCameraManager::EnterCinematic(CStateManager& mgr) {
  mgr.GetPlayer().GetPlayerGun()->CancelFiring(mgr);
  mgr.GetPlayer().UnFreeze(mgr);

  for (const CEntity* ent : mgr.GetAllObjectList()) {
    if (const TCastToConstPtr<CExplosion> explo = ent) {
      mgr.FreeScriptObject(explo->GetUniqueId());
    } else if (const TCastToConstPtr<CWeapon> weap = ent) {
      if (weap->GetActive()) {
        if (False(weap->GetAttribField() & EProjectileAttrib::KeepInCinematic)) {
          if (TCastToConstPtr<CAi>(mgr.GetObjectById(weap->GetOwnerId())) ||
              TCastToConstPtr<CPlayer>(mgr.GetObjectById(weap->GetOwnerId())))
            mgr.FreeScriptObject(weap->GetUniqueId());
        }
      }
    }
  }
}

void CCameraManager::AddCinemaCamera(TUniqueId id, CStateManager& stateMgr) {
  if (x4_cineCameras.empty()) {
    EnterCinematic(stateMgr);
  }

  RemoveCinemaCamera(id, stateMgr);
  x4_cineCameras.push_back(id);

  if (const TCastToPtr<CCinematicCamera> cam = stateMgr.ObjectById(id)) {
    // Into player eye
    if ((cam->GetFlags() & 0x4) != 0) {
      float time = 4.f;
      float delayTime = cam->GetDuration() - 4.f;
      if (delayTime < 0.f) {
        delayTime = 0.f;
        time = cam->GetDuration();
      }
      cam->SetFovInterpolation(cam->GetFov(), 55.f, time, delayTime);
    }
  }
}

void CCameraManager::SetInsideFluid(bool isInside, TUniqueId fluidId) {
  if (isInside) {
    ++x74_fluidCounter;
    x78_fluidId = fluidId;
  } else {
    --x74_fluidCounter;
  }
}

void CCameraManager::Update(float dt, CStateManager& stateMgr) {
  UpdateCameraHints(dt, stateMgr);
  ThinkCameras(dt, stateMgr);
  UpdateListener(stateMgr);
  UpdateRumble(dt, stateMgr);
  UpdateFog(dt, stateMgr);
}

CGameCamera* CCameraManager::GetCurrentCamera(CStateManager& stateMgr) const {
  CObjectList* camList = stateMgr.ObjectListById(EGameObjectList::GameCamera);
  return static_cast<CGameCamera*>(camList->GetObjectById(GetCurrentCameraId()));
}

const CGameCamera* CCameraManager::GetCurrentCamera(const CStateManager& stateMgr) const {
  const CObjectList* camList = stateMgr.GetObjectListById(EGameObjectList::GameCamera);
  return static_cast<const CGameCamera*>(camList->GetObjectById(GetCurrentCameraId()));
}

void CCameraManager::CreateStandardCameras(CStateManager& stateMgr) {
  TUniqueId fpId = stateMgr.AllocateUniqueId();
  x7c_fpCamera =
      new CFirstPersonCamera(fpId, zeus::CTransform(), stateMgr.Player()->GetUniqueId(),
                             g_tweakPlayer->GetOrbitCameraSpeed(), sFirstPersonFOV, NearPlane(), FarPlane(), Aspect());
  stateMgr.AddObject(x7c_fpCamera);
  stateMgr.Player()->SetCameraState(CPlayer::EPlayerCameraState::FirstPerson, stateMgr);
  SetCurrentCameraId(fpId, stateMgr);

  x80_ballCamera = new CBallCamera(stateMgr.AllocateUniqueId(), stateMgr.Player()->GetUniqueId(), zeus::CTransform(),
                                   ThirdPersonFOV(), NearPlane(), FarPlane(), Aspect());
  stateMgr.AddObject(x80_ballCamera);

  x88_interpCamera = new CInterpolationCamera(stateMgr.AllocateUniqueId(), zeus::CTransform());
  stateMgr.AddObject(x88_interpCamera);
}

void CCameraManager::SkipCinematic(CStateManager& stateMgr) {
  const TUniqueId camId = GetCurrentCameraId();
  auto* ent = static_cast<CCinematicCamera*>(stateMgr.ObjectById(camId));
  while (ent) {
    ent->SetActive(false);
    ent->WasDeactivated(stateMgr);
    ent = TCastToPtr<CCinematicCamera>(GetCurrentCamera(stateMgr)).GetPtr();
  }
  stateMgr.GetPlayer().UpdateCinematicState(stateMgr);
  x7c_fpCamera->SkipCinematic();
}

void CCameraManager::SetPathCamera(TUniqueId id, CStateManager& mgr) {
  xa4_pathCamId = id;
  if (const TCastToPtr<CPathCamera> cam = mgr.ObjectById(id)) {
    cam->Reset(GetCurrentCameraTransform(mgr), mgr);
    x80_ballCamera->TeleportCamera(cam->GetTransform(), mgr);
  }
}

void CCameraManager::SetSpindleCamera(TUniqueId id, CStateManager& mgr) {
  xa2_spindleCamId = id;
  if (const TCastToPtr<CScriptSpindleCamera> cam = mgr.ObjectById(id)) {
    cam->Reset(GetCurrentCameraTransform(mgr), mgr);
    x80_ballCamera->TeleportCamera(cam->GetTransform(), mgr);
  }
}

void CCameraManager::InterpolateToBallCamera(const zeus::CTransform& xf, TUniqueId camId,
                                             const zeus::CVector3f& lookPos, float maxTime, float positionSpeed,
                                             float rotationSpeed, bool sinusoidal, CStateManager& mgr) {
  if (!IsInFirstPersonCamera()) {
    x88_interpCamera->SetInterpolation(xf, lookPos, maxTime, positionSpeed, rotationSpeed, camId, sinusoidal, mgr);
    if (!ShouldBypassInterpolation())
      SetCurrentCameraId(x88_interpCamera->GetUniqueId(), mgr);
  }
}

void CCameraManager::RestoreHintlessCamera(CStateManager& mgr) {
  const TCastToConstPtr<CScriptCameraHint> hint = mgr.ObjectById(xa6_camHintId);
  const zeus::CTransform ballCamXf = x80_ballCamera->GetTransform();

  xa6_camHintId = kInvalidUniqueId;
  xa8_hintPriority = 1000;

  if (!hint) {
    return;
  }

  zeus::CVector3f camToPlayerFlat = mgr.GetPlayer().GetTranslation() - ballCamXf.origin;
  camToPlayerFlat.z() = 0.f;
  if (camToPlayerFlat.canBeNormalized()) {
    camToPlayerFlat.normalize();
  } else {
    camToPlayerFlat = mgr.GetPlayer().GetMoveDir();
  }

  x80_ballCamera->ResetToTweaks(mgr);
  x80_ballCamera->UpdateLookAtPosition(0.f, mgr);
  if (!mgr.GetPlayer().IsMorphBallTransitioning() &&
      hint->GetHint().GetBehaviourType() != CBallCamera::EBallCameraBehaviour::Default) {
    if ((hint->GetHint().GetOverrideFlags() & 0x1000) != 0) {
      x80_ballCamera->SetClampVelRange(hint->GetHint().GetClampVelRange());
      x80_ballCamera->SetClampVelTimer(hint->GetHint().GetClampVelTime());
    } else {
      x80_ballCamera->TeleportCamera(x80_ballCamera->UpdateLookDirection(camToPlayerFlat, mgr), mgr);
      InterpolateToBallCamera(ballCamXf, x80_ballCamera->GetUniqueId(), x80_ballCamera->GetLookPos(),
                              hint->GetHint().GetClampVelTime(), hint->GetHint().GetClampVelRange(),
                              hint->GetHint().GetClampRotRange(), (hint->GetHint().GetOverrideFlags() & 0x800) != 0,
                              mgr);
    }
  }
}

void CCameraManager::SkipBallCameraCinematic(CStateManager& mgr) {
  if (IsInCinematicCamera()) {
    x80_ballCamera->TeleportCamera(GetLastCineCamera(mgr)->GetTransform(), mgr);
    x80_ballCamera->SetFovInterpolation(GetLastCineCamera(mgr)->GetFov(), x80_ballCamera->GetFov(), 1.f, 0.f);
    SkipCinematic(mgr);
    SetCurrentCameraId(x80_ballCamera->GetUniqueId(), mgr);
  }
}

void CCameraManager::ApplyCameraHint(const CScriptCameraHint& hint, CStateManager& mgr) {
  if (x80_ballCamera->GetState() == CBallCamera::EBallCameraState::ToBall) {
    x80_ballCamera->SetState(CBallCamera::EBallCameraState::Default, mgr);
    mgr.GetPlayer().SetCameraState(CPlayer::EPlayerCameraState::Ball, mgr);
  }

  const TCastToConstPtr<CScriptCameraHint> oldHint = mgr.ObjectById(xa6_camHintId);
  xa6_camHintId = hint.GetUniqueId();
  xa8_hintPriority = hint.GetPriority();

  const zeus::CTransform camXf = GetCurrentCameraTransform(mgr);
  x80_ballCamera->ApplyCameraHint(mgr);

  if ((hint.GetHint().GetOverrideFlags() & 0x20) != 0) {
    x80_ballCamera->ResetPosition(mgr);
  }

  switch (hint.GetHint().GetBehaviourType()) {
  case CBallCamera::EBallCameraBehaviour::PathCameraDesiredPos:
  case CBallCamera::EBallCameraBehaviour::PathCamera:
    SetPathCamera(hint.GetDelegatedCamera(), mgr);
    break;
  case CBallCamera::EBallCameraBehaviour::SpindleCamera:
    SetSpindleCamera(hint.GetDelegatedCamera(), mgr);
    break;
  default:
    SetPathCamera(kInvalidUniqueId, mgr);
    SetSpindleCamera(kInvalidUniqueId, mgr);
    break;
  }

  if ((hint.GetHint().GetOverrideFlags() & 0x2000) != 0) {
    SkipBallCameraCinematic(mgr);
  }

  x80_ballCamera->UpdateLookAtPosition(0.f, mgr);

  if ((hint.GetHint().GetOverrideFlags() & 0x20) == 0 &&
      (hint.GetHint().GetBehaviourType() != CBallCamera::EBallCameraBehaviour::Default ||
       (oldHint && oldHint->GetHint().GetBehaviourType() != CBallCamera::EBallCameraBehaviour::Default))) {
    InterpolateToBallCamera(camXf, x80_ballCamera->GetUniqueId(), x80_ballCamera->GetLookPos(),
                            hint.GetHint().GetInterpolateTime(), hint.GetHint().GetClampVelRange(),
                            hint.GetHint().GetClampRotRange(), (hint.GetHint().GetOverrideFlags() & 0x400) != 0, mgr);
  }
}

void CCameraManager::UpdateCameraHints(float, CStateManager& mgr) {
  bool invalidHintRemoved = false;
  for (auto it = xac_cameraHints.begin(); it != xac_cameraHints.end();) {
    if (!TCastToConstPtr<CScriptCameraHint>(mgr.ObjectById(it->second))) {
      invalidHintRemoved = true;
      it = xac_cameraHints.erase(it);
      continue;
    }
    ++it;
  }

  bool inactiveHintRemoved = false;
  for (const TUniqueId id : x2b0_inactiveCameraHints) {
    if (const TCastToConstPtr<CScriptCameraHint> hint = mgr.GetObjectById(id)) {
      if (hint->GetHelperCount() == 0 || hint->GetInactive()) {
        for (auto it = xac_cameraHints.begin(); it != xac_cameraHints.end(); ++it) {
          if (it->second == id) {
            xac_cameraHints.erase(it);
            if (xa6_camHintId == id) {
              inactiveHintRemoved = true;
              SetPathCamera(kInvalidUniqueId, mgr);
              SetSpindleCamera(kInvalidUniqueId, mgr);
            }
            break;
          }
        }
      }
    }
  }
  x2b0_inactiveCameraHints.clear();

  bool activeHintAdded = false;
  for (const TUniqueId id : x334_activeCameraHints) {
    if (const TCastToConstPtr<CScriptCameraHint> hint = mgr.GetObjectById(id)) {
      bool activeHintPresent = false;
      for (auto it = xac_cameraHints.begin(); it != xac_cameraHints.end(); ++it) {
        if (it->second == id) {
          activeHintPresent = true;
          break;
        }
      }

      if (!activeHintPresent) {
        activeHintAdded = true;
        xac_cameraHints.emplace_back(hint->GetPriority(), id);
      }
    }
  }
  x334_activeCameraHints.clear();

  if (inactiveHintRemoved || activeHintAdded || invalidHintRemoved) {
    std::sort(xac_cameraHints.begin(), xac_cameraHints.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    zeus::CTransform ballCamXf = x80_ballCamera->GetTransform();
    if ((inactiveHintRemoved || invalidHintRemoved) && xac_cameraHints.empty()) {
      RestoreHintlessCamera(mgr);
      return;
    }
    bool foundHint = false;
    const CScriptCameraHint* bestHint = nullptr;
    for (auto& h : xac_cameraHints) {
      if (const TCastToConstPtr<CScriptCameraHint> hint = mgr.ObjectById(h.second)) {
        bestHint = hint.GetPtr();
        foundHint = true;
        break;
      }
    }
    if (!foundHint) {
      RestoreHintlessCamera(mgr);
    }

    bool changeHint = false;
    if (bestHint && foundHint) {
      if ((bestHint->GetHint().GetOverrideFlags() & 0x80) != 0 && xac_cameraHints.size() > 1) {
        zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
        if ((bestHint->GetHint().GetOverrideFlags() & 0x100) != 0) {
          zeus::CVector3f camToBall = ballPos - ballCamXf.origin;
          if (camToBall.canBeNormalized()) {
            camToBall.normalize();
          } else {
            camToBall = ballCamXf.basis[1];
          }

          for (auto it = xac_cameraHints.begin() + 1; it != xac_cameraHints.end(); ++it) {
            if (const TCastToConstPtr<CScriptCameraHint> hint = mgr.ObjectById(it->second)) {
              if ((hint->GetHint().GetOverrideFlags() & 0x80) != 0 && hint->GetPriority() == bestHint->GetPriority() &&
                  hint->GetAreaIdAlways() == bestHint->GetAreaIdAlways()) {
                zeus::CVector3f hintToBall = ballPos - bestHint->GetTranslation();
                if (hintToBall.canBeNormalized()) {
                  hintToBall.normalize();
                } else {
                  hintToBall = bestHint->GetTransform().basis[1];
                }

                const float camHintDot = zeus::clamp(-1.f, camToBall.dot(hintToBall), 1.f);

                zeus::CVector3f thisHintToBall = ballPos - hint->GetTranslation();
                if (thisHintToBall.canBeNormalized()) {
                  thisHintToBall.normalize();
                } else {
                  thisHintToBall = hint->GetTransform().basis[1];
                }

                const float camThisHintDot = zeus::clamp(-1.f, camToBall.dot(thisHintToBall), 1.f);

                if (camThisHintDot > camHintDot) {
                  bestHint = hint.GetPtr();
                }
              } else {
                break;
              }
            } else {
              break;
            }
          }
        } else {
          if (const TCastToConstPtr<CActor> act = mgr.GetObjectById(bestHint->GetFirstHelper())) {
            const zeus::CVector3f f26 = act->GetTranslation() - mgr.GetPlayer().GetBallPosition();
            zeus::CVector3f ballToHelper = f26;
            if (ballToHelper.canBeNormalized()) {
              ballToHelper.normalize();
            } else {
              ballToHelper = bestHint->GetTransform().basis[1];
            }

            for (auto it = xac_cameraHints.begin() + 1; it != xac_cameraHints.end(); ++it) {
              if (const TCastToConstPtr<CScriptCameraHint> hint = mgr.ObjectById(it->second)) {
                if ((hint->GetHint().GetOverrideFlags() & 0x80) != 0 &&
                    hint->GetPriority() == bestHint->GetPriority() &&
                    hint->GetAreaIdAlways() == bestHint->GetAreaIdAlways()) {
                  zeus::CVector3f hintToHelper = act->GetTranslation() - bestHint->GetTranslation();
                  if (hintToHelper.canBeNormalized()) {
                    hintToHelper.normalize();
                  } else {
                    hintToHelper = bestHint->GetTransform().basis[1];
                  }

                  const float ballHintDot = zeus::clamp(-1.f, ballToHelper.dot(hintToHelper), 1.f);

                  zeus::CVector3f thisBallToHelper = f26;
                  if (thisBallToHelper.canBeNormalized()) {
                    thisBallToHelper.normalize();
                  } else {
                    thisBallToHelper = hint->GetTransform().basis[1];
                  }

                  zeus::CVector3f thisHintToHelper = act->GetTranslation() - hint->GetTranslation();
                  if (thisHintToHelper.canBeNormalized()) {
                    thisHintToHelper.normalize();
                  } else {
                    thisHintToHelper = hint->GetTransform().basis[1];
                  }

                  const float thisBallHintDot = zeus::clamp(-1.f, thisBallToHelper.dot(thisHintToHelper), 1.f);

                  if (thisBallHintDot > ballHintDot) {
                    bestHint = hint.GetPtr();
                  }
                } else {
                  break;
                }
              } else {
                break;
              }
            }
          }
        }

        if (bestHint->GetUniqueId() != xa6_camHintId) {
          changeHint = true;
        }
      } else if (xa6_camHintId != bestHint->GetUniqueId()) {
        if (bestHint->GetHint().GetBehaviourType() == CBallCamera::EBallCameraBehaviour::HintInitializePosition) {
          if ((bestHint->GetHint().GetOverrideFlags() & 0x20) != 0) {
            x80_ballCamera->TeleportCamera(zeus::lookAt(bestHint->GetTranslation(), x80_ballCamera->GetLookPos()), mgr);
          }
          DeleteCameraHint(bestHint->GetUniqueId(), mgr);
          if ((bestHint->GetHint().GetOverrideFlags() & 0x2000) != 0) {
            SkipBallCameraCinematic(mgr);
          }
          changeHint = false;
        } else {
          changeHint = true;
        }
      }

      if (changeHint) {
        ApplyCameraHint(*bestHint, mgr);
      }
    }
  }
}

void CCameraManager::ThinkCameras(float dt, CStateManager& mgr) {
  CGameCameraList gcList = mgr.GetCameraObjectList();

  for (CEntity* ent : gcList) {
    if (const TCastToPtr<CGameCamera> gc = ent) {
      gc->Think(dt, mgr);
      gc->UpdatePerspective(dt);
    }
  }

  if (IsInCinematicCamera()) {
    return;
  }

  const TUniqueId camId = GetLastCameraId();
  if (const CGameCamera* cam = TCastToConstPtr<CGameCamera>(mgr.GetObjectById(camId))) {
    x3bc_curFov = cam->GetFov();
  }
}

void CCameraManager::UpdateFog(float dt, CStateManager& mgr) {
  if (x98_fogDensitySpeed != 0.f) {
    x94_fogDensityFactor += dt * x98_fogDensitySpeed;
    if ((x98_fogDensitySpeed > 0.f) ? x94_fogDensityFactor > x9c_fogDensityFactorTarget
                                    : x94_fogDensityFactor < x9c_fogDensityFactorTarget) {
      x94_fogDensityFactor = x9c_fogDensityFactorTarget;
      x98_fogDensitySpeed = 0.f;
    }
  }

  if (x74_fluidCounter) {
    if (const TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(x78_fluidId)) {
      const zeus::CVector2f zRange(GetCurrentCamera(mgr)->GetNearClipDistance(),
                                   CalculateFogDensity(mgr, water.GetPtr()));
      x3c_fog.SetFogExplicit(ERglFogMode::PerspExp, water->GetInsideFogColor(), zRange);
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal) {
        mgr.GetCameraFilterPass(4).DisableFilter(0.f);
      } else {
        mgr.GetCameraFilterPass(4).SetFilter(EFilterType::Multiply, EFilterShape::Fullscreen, 0.f,
                                             water->GetInsideFogColor(), {});
      }
    }
    xa0_26_inWater = true;
  } else if (xa0_26_inWater) {
    mgr.GetCameraManager()->x3c_fog.DisableFog();
    mgr.GetCameraFilterPass(4).DisableFilter(0.f);
    xa0_26_inWater = false;
  }

  x3c_fog.Update(dt);
}

void CCameraManager::UpdateRumble(float dt, CStateManager& mgr) {
  x30_shakeOffset = zeus::skZero3f;
  for (auto it = x14_shakers.begin(); it != x14_shakers.end();) {
    CCameraShakeData& shaker = *it;
    shaker.Update(dt, mgr);
    if (shaker.x4_curTime >= shaker.x0_duration) {
      it = x14_shakers.erase(it);
      continue;
    }
    x30_shakeOffset += shaker.GetPoint();
    ++it;
  }

  if (!x14_shakers.empty() && !xa0_25_rumbling && xa0_24_pendingRumble) {
    mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::CameraShake, 1.f, ERumblePriority::Two);
    xa0_25_rumbling = true;
  }

  if (x90_rumbleCooldown > 0.f) {
    x90_rumbleCooldown -= dt;
  } else if (xa0_25_rumbling) {
    xa0_24_pendingRumble = false;
    xa0_25_rumbling = false;
  }

  if (mgr.GetPlayer().GetCameraState() != CPlayer::EPlayerCameraState::FirstPerson && !IsInCinematicCamera()) {
    x30_shakeOffset = zeus::skZero3f;
  }
}

void CCameraManager::UpdateListener(CStateManager& mgr) {
  const zeus::CTransform xf = GetCurrentCameraTransform(mgr);
  CSfxManager::UpdateListener(xf.origin, zeus::skZero3f, xf.frontVector(), xf.upVector(), 1.f);
}

float CCameraManager::CalculateFogDensity(CStateManager& mgr, const CScriptWater* water) const {
  const float distanceFactor = 1.f - water->GetFluidPlane().GetAlpha();
  float distance = 0;
  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit)) {
    distance =
        g_tweakGame->GetGravityWaterFogDistanceRange() * distanceFactor + g_tweakGame->GetGravityWaterFogDistanceBase();
  } else {
    distance = g_tweakGame->GetWaterFogDistanceRange() * distanceFactor + g_tweakGame->GetWaterFogDistanceBase();
  }

  return distance * x94_fogDensityFactor;
}

void CCameraManager::ResetCameras(CStateManager& mgr) {
  zeus::CTransform xf = mgr.GetPlayer().CreateTransformFromMovementDirection();
  xf.origin = mgr.GetPlayer().GetEyePosition();

  for (CEntity* ent : mgr.GetCameraObjectList()) {
    const TCastToPtr<CGameCamera> camObj(ent);
    camObj->Reset(xf, mgr);
  }
}

void CCameraManager::SetSpecialCameras(CFirstPersonCamera& fp, CBallCamera& ball) {
  x7c_fpCamera = &fp;
  x80_ballCamera = &ball;
}

void CCameraManager::ProcessInput(const CFinalInput& input, CStateManager& stateMgr) {
  for (CEntity* ent : stateMgr.GetCameraObjectList()) {
    if (ent == nullptr) {
      continue;
    }
    auto& cam = static_cast<CGameCamera&>(*ent);
    if (input.ControllerIdx() != cam.x16c_controllerIdx) {
      continue;
    }
    cam.ProcessInput(input, stateMgr);
  }
}

void CCameraManager::RenderCameras(CStateManager& mgr) {
  for (CEntity* cam : mgr.GetCameraObjectList()) {
    static_cast<CGameCamera*>(cam)->Render(mgr);
  }
}

void CCameraManager::SetupBallCamera(CStateManager& mgr) {
  if (const TCastToConstPtr<CScriptCameraHint> hint = mgr.ObjectById(xa6_camHintId)) {
    if (hint->GetHint().GetBehaviourType() == CBallCamera::EBallCameraBehaviour::HintInitializePosition) {
      if ((hint->GetHint().GetOverrideFlags() & 0x20) != 0) {
        x80_ballCamera->TeleportCamera(hint->GetTransform(), mgr);
      }
      AddInactiveCameraHint(xa6_camHintId, mgr);
    } else {
      ApplyCameraHint(*hint, mgr);
    }
  }
}

void CCameraManager::SetPlayerCamera(CStateManager& mgr, TUniqueId newCamId) {
  if (x88_interpCamera->GetActive()) {
    x88_interpCamera->SetActive(false);
    x80_ballCamera->SkipFovInterpolation();
    if (!ShouldBypassInterpolation())
      SetCurrentCameraId(newCamId, mgr);
  }
}

float CCameraManager::GetCameraBobMagnitude() const {
  return 1.f - zeus::clamp(
                   -1.f,
                   std::fabs(zeus::clamp(-1.f, x7c_fpCamera->GetTransform().basis[1].dot(zeus::skUp), 1.f)) /
                       std::cos(2.f * M_PIF / 12.f),
                   1.f);
}

bool CCameraManager::HasBallCameraInitialPositionHint(CStateManager& mgr) const {
  if (HasCameraHint(mgr)) {
    switch (mgr.GetCameraManager()->GetCameraHint(mgr)->GetHint().GetBehaviourType()) {
    case CBallCamera::EBallCameraBehaviour::HintBallToCam:
    case CBallCamera::EBallCameraBehaviour::HintFixedPosition:
    case CBallCamera::EBallCameraBehaviour::HintFixedTransform:
    case CBallCamera::EBallCameraBehaviour::PathCamera:
    case CBallCamera::EBallCameraBehaviour::SpindleCamera:
      return true;
    default:
      return false;
    }
  }
  return false;
}

void CCameraManager::RemoveCinemaCamera(TUniqueId uid, CStateManager& mgr) {
  const auto search = std::find(x4_cineCameras.cbegin(), x4_cineCameras.cend(), uid);

  if (search == x4_cineCameras.cend()) {
    return;
  }

  x4_cineCameras.erase(search);
}

void CCameraManager::DeleteCameraHint(TUniqueId id, CStateManager& mgr) {
  const TCastToPtr<CScriptCameraHint> hint = mgr.ObjectById(id);

  if (!hint) {
    return;
  }

  const auto search = std::find_if(x2b0_inactiveCameraHints.cbegin(), x2b0_inactiveCameraHints.cend(),
                                   [id](TUniqueId tid) { return tid == id; });

  if (search != x2b0_inactiveCameraHints.cend()) {
    return;
  }

  hint->ClearIdList();
  hint->SetInactive(true);
  if (x2b0_inactiveCameraHints.size() != 64) {
    x2b0_inactiveCameraHints.push_back(id);
  }
}

void CCameraManager::AddInactiveCameraHint(TUniqueId id, CStateManager& mgr) {
  if (const TCastToConstPtr<CScriptCameraHint> hint = mgr.ObjectById(id)) {
    const auto search = std::find_if(x2b0_inactiveCameraHints.cbegin(), x2b0_inactiveCameraHints.cend(),
                                     [id](TUniqueId tid) { return tid == id; });
    if (search == x2b0_inactiveCameraHints.cend() && x2b0_inactiveCameraHints.size() != 64) {
      x2b0_inactiveCameraHints.push_back(id);
    }
  }
}

void CCameraManager::AddActiveCameraHint(TUniqueId id, CStateManager& mgr) {
  if (const TCastToConstPtr<CScriptCameraHint> hint = mgr.ObjectById(id)) {
    const auto search = std::find_if(x334_activeCameraHints.cbegin(), x334_activeCameraHints.cend(),
                                     [id](TUniqueId tid) { return tid == id; });
    if (search == x334_activeCameraHints.cend() && xac_cameraHints.size() != 64 && x334_activeCameraHints.size() != 64) {
      x334_activeCameraHints.push_back(id);
    }
  }
}

TUniqueId CCameraManager::GetLastCineCameraId() const {
  if (x4_cineCameras.empty()) {
    return kInvalidUniqueId;
  }
  return x4_cineCameras.back();
}

const CCinematicCamera* CCameraManager::GetLastCineCamera(CStateManager& mgr) const {
  return static_cast<const CCinematicCamera*>(mgr.GetObjectById(GetLastCineCameraId()));
}

const CScriptCameraHint* CCameraManager::GetCameraHint(CStateManager& mgr) const {
  return TCastToConstPtr<CScriptCameraHint>(mgr.GetObjectById(xa6_camHintId)).GetPtr();
}

bool CCameraManager::HasCameraHint(CStateManager& mgr) const {
  if (xac_cameraHints.empty() || xa6_camHintId == kInvalidUniqueId)
    return false;
  return mgr.GetObjectById(xa6_camHintId) != nullptr;
}

bool CCameraManager::IsInterpolationCameraActive() const { return x88_interpCamera->GetActive(); }

void CCameraManager::SetFogDensity(float fogDensityTarget, float fogDensitySpeed) {
  x9c_fogDensityFactorTarget = fogDensityTarget;
  x98_fogDensitySpeed = (x9c_fogDensityFactorTarget >= x94_fogDensityFactor ? fogDensitySpeed : -fogDensitySpeed);
}
} // namespace urde
