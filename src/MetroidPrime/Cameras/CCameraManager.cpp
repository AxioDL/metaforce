#include "MetroidPrime/Cameras/CCameraManager.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CInterpolationCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/SFX/Misc.h"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#if VERSION >= VERSION_GM8E_02
#include "MetroidPrime/Enemies/CEnergyBall.hpp"
#endif

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

float CCameraManager::sFirstPersonFOV = 55.f;
float CCameraManager::sThirdPersonFOV = 60.f;
float CCameraManager::sNearPlane = 0.2f;
float CCameraManager::sFarPlane = 750.f;
float CCameraManager::sAspectRatio = 1.42f;
float CCameraManager::sMinShakeVolume = 100.f;
float CCameraManager::sMaxShakeVolume = 127.f;

CCameraManager::CCameraManager(TUniqueId curCamera)
: mCurCameraId(curCamera)
, mLastShakeId(0)
, mShakeOffset(CVector3f::Zero())
, mFluidCounter(0)
, mFluidId(kInvalidUniqueId)
, mFpCamera(nullptr)
, mBallCamera(nullptr)
, mRumbleId(0)
, mInterpCamera(nullptr)
, x8c_(-1)
, mRumbleCooldown(0.f)
, mFogDensityFactor(1.f)
, mFogDensitySpeed(0.f)
, mFogDensityFactorTarget(1.f)
, mPendingRumble(false)
, mRumbling(false)
, mInWater(false)
, mSpindleCamId(kInvalidUniqueId)
, mPathCamId(kInvalidUniqueId)
, mCamHintId(kInvalidUniqueId)
, mHintPriority(1000)
, x3b8_24_(false)
, x3b8_25_(false)
, mCurFov(sThirdPersonFOV) {
  CSfxManager::AddListener(CSfxManager::kSC_Game, CVector3f::Zero(), CVector3f::Zero(),
                           CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 0.f, 1.f), 50.f, 50.f, 1000.f,
                           1, CAudioSys::kMaxVolume);
  sAspectRatio =
#if VERSION >= VERSION_GM8E_02
      CGraphics::GetPixelAspectRatio() *
#endif
      (static_cast< float >(CGraphics::GetViewport().mWidth) / CGraphics::GetViewport().mHeight);
  sFirstPersonFOV = gpTweakGame->GetFirstPersonFOV();
}

float CCameraManager::GetDefaultThirdPersonVerticalFOV() { return sThirdPersonFOV; }

float CCameraManager::GetDefaultFirstPersonVerticalFOV() { return sFirstPersonFOV; }

float CCameraManager::GetDefaultFirstPersonNearClipDistance() { return sNearPlane; }

float CCameraManager::GetDefaultFirstPersonFarClipDistance() { return sFarPlane; }

float CCameraManager::GetDefaultAspectRatio() { return sAspectRatio; }

void CCameraManager::CreateCameras(CStateManager& mgr) {
  TUniqueId plId = mgr.GetPlayer()->GetUniqueId();
  CTransform4f xf = CTransform4f::Identity();
  xf.SetTranslation(mgr.GetPlayer()->GetEyePosition());
  TUniqueId fpId = mgr.AllocateUniqueId();

  mFpCamera = rs_new CFirstPersonCamera(
      fpId, xf, plId, gpTweakPlayer->GetOrbitCameraSpeed(), GetDefaultFirstPersonVerticalFOV(),
      GetDefaultFirstPersonNearClipDistance(), GetDefaultFirstPersonFarClipDistance(),
      GetDefaultAspectRatio());
  mgr.AddObject(mFpCamera);
  mgr.Player()->SetCameraState(CPlayer::kCS_FirstPerson, mgr);
  SetCurrentCameraId(fpId);
  TUniqueId ballId = mgr.AllocateUniqueId();
  mBallCamera = rs_new CBallCamera(
      ballId, plId, xf, GetDefaultThirdPersonVerticalFOV(), GetDefaultFirstPersonNearClipDistance(),
      GetDefaultFirstPersonFarClipDistance(), GetDefaultAspectRatio());
  mgr.AddObject(mBallCamera);

  TUniqueId interpId = mgr.AllocateUniqueId();
  mInterpCamera = rs_new CInterpolationCamera(interpId, xf);
  mgr.AddObject(mInterpCamera);
}

void CCameraManager::UpdateCameras(float dt, CStateManager& mgr) {
  CObjectList& objList = mgr.ObjectListById(kOL_GameCamera);
  int idx = objList.GetFirstObjectIndex();

  while (idx != -1) {
    CGameCamera* gc = static_cast< CGameCamera* >(objList[idx]);
    if (gc != nullptr) {
      gc->Think(dt, mgr);
      gc->UpdatePerspective(dt);
    }
    idx = objList.GetNextObjectIndex(idx);
  }

  if (!IsInCinematicCamera()) {
    return;
  }

  if (const CGameCamera* cam =
          TCastToConstPtr< CGameCamera >(mgr.GetObjectById(GetCurrentCinematicCameraId()))) {
    mCurFov = cam->GetFov();
  }
}

void CCameraManager::ResetCameras(CStateManager& mgr) {
  CObjectList& objList = mgr.ObjectListById(kOL_GameCamera);

  CTransform4f xf = mgr.GetPlayer()->CreateTransformFromMovementDirection();
  xf.SetTranslation(mgr.GetPlayer()->GetEyePosition());

  int idx = objList.GetFirstObjectIndex();

  while (idx != -1) {
    CGameCamera* gc = static_cast< CGameCamera* >(objList[idx]);
    if (gc != nullptr) {
      gc->Reset(xf, mgr);
    }
    idx = objList.GetNextObjectIndex(idx);
  }
}

void CCameraManager::Render(const CStateManager& mgr) {
  const CObjectList& objList = mgr.GetObjectListById(kOL_GameCamera);
  int idx = objList.GetFirstObjectIndex();

  while (idx != -1) {
    static_cast< const CGameCamera* >(objList[idx])->Render(mgr);
    idx = objList.GetNextObjectIndex(idx);
  }
}

TUniqueId CCameraManager::GetCurrentCameraId() const {
  if (mCineCameras.empty()) {
    return mCurCameraId;
  }

  return mCineCameras.back();
}

TUniqueId CCameraManager::GetCurrentCinematicCameraId() const {
  if (mCineCameras.empty()) {
    return kInvalidUniqueId;
  }

  return mCineCameras.back();
}

CGameCamera& CCameraManager::CurrentCamera(CStateManager& mgr) {
  return *static_cast< CGameCamera* >(mgr.ObjectById(GetCurrentCameraId()));
}

const CGameCamera& CCameraManager::GetCurrentCamera(const CStateManager& mgr) const {
  return *static_cast< const CGameCamera* >(mgr.GetObjectById(GetCurrentCameraId()));
}

const CGameCamera& CCameraManager::GetCurrentCinematicCamera(CStateManager& mgr) const {
  return *static_cast< const CGameCamera* >(mgr.GetObjectById(GetCurrentCinematicCameraId()));
}

void CCameraManager::SetCurrentCameraId(TUniqueId uid) { mCurCameraId = uid; }

void CCameraManager::UpdateAudioListener(CStateManager& mgr) {
  CTransform4f xf = GetCurrentCameraTransform(mgr);

  CSfxManager::UpdateListener(xf.GetTranslation(), CVector3f::Zero(), xf.GetColumn(kDY),
                              xf.GetColumn(kDZ), CAudioSys::kMaxVolume);
}

void CCameraManager::UpdateScreenShake(float dt, CStateManager& mgr) {
  mShakeOffset = CVector3f::Zero();

  rstl::list< CCameraShakeData >::iterator it = mShakers.begin();
  while (it != mShakers.end()) {
    rstl::list< CCameraShakeData >::iterator curr = it;
    ++it;
    curr->Update(dt, mgr);
    if (curr->Done()) {
      mShakers.erase(curr);
    } else {
      mShakeOffset += curr->GetPoint();
    }
  }

  if (!mShakers.empty() && !mRumbling && mPendingRumble) {
    mgr.GetRumbleManager()->Rumble(mgr, kRFX_CameraShake, 1.f, kRP_Two);
    mRumbling = true;
  }
  if (mRumbleCooldown > 0.f) {
    mRumbleCooldown -= dt;
  } else if (mRumbling) {
    mRumbling = mPendingRumble = false;
  }

  if (mgr.GetPlayer()->GetCameraState() != CPlayer::kCS_FirstPerson && !IsInCinematicCamera()) {
    mShakeOffset = CVector3f::Zero();
  }
}

void CCameraManager::UpdateFilters(float dt, CStateManager& mgr) {
  if (mFogDensitySpeed != 0.f) {
    mFogDensityFactor += dt * mFogDensitySpeed;
    if ((mFogDensitySpeed > 0.f) ? mFogDensityFactor > mFogDensityFactorTarget
                                    : mFogDensityFactor < mFogDensityFactorTarget) {
      mFogDensityFactor = mFogDensityFactorTarget;
      mFogDensitySpeed = 0.f;
    }
  }

  CCameraFilterPass& pass = mgr.CameraFilterPass(CStateManager::kCFS_Four);
  if (mFluidCounter) {
    const CScriptWater* const water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(GetFluidId()));
    const CGameCamera& camera = GetCurrentCamera(mgr);
    if (water) {
      const float near = camera.GetNearClipDistance();
      const float far = GetWaterFarDistance(mgr, water);
      const CColor& color = water->GetUnderwaterFogColor();
      mFog.SetFogExplicit(kRFM_PerspExp, color, CVector2f(near, far));
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
        pass.DisableFilter(0.f);
      } else {
        pass.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                       color, kInvalidAssetId);
      }
    }
    mInWater = true;
  } else if (IsInsideFluid()) {
    mgr.CameraManager()->mFog.DisableFog();
    pass.DisableFilter(0.f);
    mInWater = false;
  }

  mFog.Update(dt);
}

float CCameraManager::GetWaterFarDistance(CStateManager& mgr, const CScriptWater* water) {
  float density = 1.f - water->GetFluidPlane().GetAlpha();
  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
    density = gpTweakGame->mGravityWaterFogDistanceRange * density +
              gpTweakGame->mGravityWaterFogDistanceBase;
  } else {
    density =
        gpTweakGame->mWaterFogDistanceRange * density + gpTweakGame->mWaterFogDistanceBase;
  }
  return density * mFogDensityFactor;
}

void CCameraManager::SetWaterFogScale(float fogDensityTarget, float fogDensitySpeed) {
  mFogDensityFactorTarget = fogDensityTarget;
  if (mFogDensityFactorTarget < mFogDensityFactor) {
    mFogDensitySpeed = -fogDensitySpeed;
  } else {
    mFogDensitySpeed = fogDensitySpeed;
  }
}

void CCameraManager::Update(float dt, CStateManager& mgr) {
  UpdateCameraHints(dt, mgr);
  UpdateCameras(dt, mgr);
  UpdateAudioListener(mgr);
  UpdateScreenShake(dt, mgr);
  UpdateFilters(dt, mgr);
}

void CCameraManager::SetInsideFluid(bool isInside, TUniqueId fluidId) {
  if (isInside) {
    ++mFluidCounter;
    mFluidId = fluidId;
  } else {
    --mFluidCounter;
  }
}

void CCameraManager::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  CObjectList& objList = mgr.ObjectListById(kOL_GameCamera);
  int idx = objList.GetFirstObjectIndex();
  while (idx != -1) {
    CGameCamera* camera = reinterpret_cast< CGameCamera* >(objList[idx]);
    if (camera != nullptr && camera->GetControllerNumber() == input.ControllerNumber()) {
      camera->ProcessInput(input, mgr);
    }
    idx = objList.GetNextObjectIndex(idx);
  }
}

void CCameraManager::AddCinemaCamera(TUniqueId uid, CStateManager& mgr) {
  if (mCineCameras.empty()) {
    EnterCinematic(mgr);
  }

  RemoveCinemaCamera(uid, mgr);
  mCineCameras.reserve(mCineCameras.size() + 1);
  mCineCameras.push_back(uid);

  if (CCinematicCamera* cam = TCastToPtr< CCinematicCamera >(mgr.ObjectById(uid))) {
    // Into player eye
    if ((cam->GetFlags() & 0x4) != 0) {
      float duration = cam->GetDuration();
      float time = 4.f;
      float delayTime = duration - 4.f;
      if (delayTime < 0.f) {
        delayTime = 0.f;
        time = duration;
      }
      cam->InterpolateFOV(cam->GetFov(), sFirstPersonFOV, time, delayTime);
    }
  }
}

void CCameraManager::RemoveCinemaCamera(TUniqueId uid, CStateManager& mgr) {
  rstl::vector< TUniqueId >::iterator it =
      rstl::find(mCineCameras.begin(), mCineCameras.end(), uid);
  if (it != mCineCameras.end()) {
    mCineCameras.erase(it);
  }
}

void CCameraManager::EnterCinematic(CStateManager& mgr) {
  mgr.Player()->PlayerGun()->CancelFiring(mgr);
  mgr.Player()->BreakFrozenState(mgr);

  CObjectList& objList = mgr.ObjectListById(kOL_All);
  long idx = objList.GetFirstObjectIndex();
  while (idx != -1) {
    if (CExplosion* explosion = TCastToPtr< CExplosion >(objList[idx])) {
      mgr.DeleteObjectRequest(explosion->GetUniqueId());
    } else {
      CWeapon* const weapon = TCastToPtr< CWeapon >(objList[idx]);
      if (weapon && weapon->GetActive() &&
          (weapon->GetAttribField() & CWeapon::kPA_KeepInCinematic) != CWeapon::kPA_KeepInCinematic) {
        CPatterned* patterned = TCastToPtr< CPatterned >(mgr.ObjectById(weapon->GetOwnerId()));
        CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(weapon->GetOwnerId()));
        if (patterned || player) {
          mgr.DeleteObjectRequest(weapon->GetUniqueId());
        }
      }
#if VERSION >= VERSION_GM8E_02
      else if (CEnergyBall* ball = TCastToPtr< CEnergyBall >(objList[idx])) {
        if (ball->GetActive()) {
          mgr.DeleteObjectRequest(ball->GetUniqueId());
        }
      }
#endif
    }
    idx = objList.GetNextObjectIndex(idx);
  }
}

void CCameraManager::StopCinematics(CStateManager& mgr) {
  CCinematicCamera* ent = static_cast< CCinematicCamera* >(mgr.ObjectById(GetCurrentCameraId()));
  while (ent) {
    ent->SetActive(false);
    ent->WasDeactivated(mgr);
    ent = TCastToPtr< CCinematicCamera >(CurrentCamera(mgr));
  }
  mgr.Player()->UpdateCinematicState(mgr);
  mFpCamera->CancelCinematicOffset();
}

int CCameraManager::AddCameraShaker(const CCameraShakeData& data, const bool sfx) {
  CCameraShakeData shakeData = data;
  shakeData.SetId(++mLastShakeId);
  mShakers.push_back(shakeData);
  if (mPendingRumble != true) {
    mPendingRumble = true;
    mRumbleCooldown = 0.5f;
  }
  float duration = data.GetDuration();
  if (sfx && duration > 0.f) {
    float component1 = data.GetMaxAmplitude();
    float component2 = data.GetMaxSeverity();
    if (component2 > component1) {
      component1 = component2;
    }
    uchar vol = static_cast< uchar >(
        CMath::Clamp(sMinShakeVolume, component1 * 9.f + 100.f, sMaxShakeVolume));
    CSfxHandle handle;
    if (data.GetFlags() & 1) {
      CVector3f pos = data.GetSfxPos();
      handle = CSfxManager::AddEmitter(SFXamb_x_rumble_lp_00, pos, CVector3f::Zero(), vol);
    } else {
      handle = CSfxManager::SfxStart(SFXamb_x_rumble_lp_00, vol, 64);
    }
    CSfxManager::SetDuration(handle, duration);
  }
  return mLastShakeId;
}

void CCameraManager::RemoveCameraShaker(int id) {
  rstl::list< CCameraShakeData >::iterator it = mShakers.begin();
  while (it != mShakers.end()) {
    if (it->GetId() == id) {
      mShakers.erase(it);
      return;
    }
    ++it;
  }
}

CTransform4f CCameraManager::GetCurrentCameraTransform(const CStateManager& mgr) const {
  return GetCurrentCamera(mgr).GetTransform() * CTransform4f::Translate(mShakeOffset);
}

CVector3f CCameraManager::GetGlobalCameraTranslation(const CStateManager& mgr) const {
  return GetCurrentCamera(mgr).GetTransform().Rotate(mShakeOffset);
}

bool CCameraManager::IsInCinematicCamera() const { return !mCineCameras.empty(); }

bool CCameraManager::IsInFPCamera() const { return mCurCameraId == mFpCamera->GetUniqueId(); }

bool CCameraManager::IsInterpolationCameraActive() const { return mInterpCamera->GetActive(); }

bool CCameraManager::ShouldBypassInterpolationCamera() const { return false; }

void CCameraManager::SetPlayerCamera(CStateManager& mgr, TUniqueId uid) {
  if (!mInterpCamera->GetActive()) {
    return;
  }
  mInterpCamera->SetActive(false);
  mBallCamera->SkipFovInterpolation();
  if (!ShouldBypassInterpolationCamera()) {
    SetCurrentCameraId(uid);
  }
}

void CCameraManager::SetupInterpolation(const CTransform4f& xf, TUniqueId camId, CVector3f lookPos,
                                        float maxTime, float positionSpeed, float rotationSpeed,
                                        bool sinusoidal, CStateManager& mgr) {
  if (IsInFPCamera()) {
    return;
  }
  mInterpCamera->SetInterpolation(xf, lookPos, maxTime, positionSpeed, rotationSpeed, camId,
                                     sinusoidal, mgr);
  if (!ShouldBypassInterpolationCamera()) {
    SetCurrentCameraId(mInterpCamera->GetUniqueId());
  }
}

void CCameraManager::CinematicCut(CStateManager& mgr) {
  if (IsInCinematicCamera()) {
    mBallCamera->TeleportCamera(GetCurrentCinematicCamera(mgr).GetTransform(), mgr);
    mBallCamera->InterpolateFOV(GetCurrentCinematicCamera(mgr).GetFov(),
                                   mBallCamera->GetFov(), 1.f, 0.f);
    StopCinematics(mgr);
    SetCurrentCameraId(mBallCamera->GetUniqueId());
  }
}

void CCameraManager::UseCameraHint(const CScriptCameraHint& hint, CStateManager& mgr) {
  if (mBallCamera->GetState() == CBallCamera::kBCS_ToBall) {
    mBallCamera->SetState(CBallCamera::kBCS_Default, mgr);
    mgr.Player()->SetCameraState(CPlayer::kCS_Ball, mgr);
  }

  const CScriptCameraHint* const oldHint =
      TCastToConstPtr< CScriptCameraHint >(mgr.ObjectById(mCamHintId));
  mCamHintId = hint.GetUniqueId();
  mHintPriority = hint.GetPriority();

  CTransform4f camXf = GetCurrentCameraTransform(mgr);
  mBallCamera->OverrideCameraInfo(mgr);

  if (hint.GetOverrideFlags() & 0x20) {
    mBallCamera->TeleportLookAtStuff(mgr);
  }

  switch (hint.GetBehaviourType()) {
  case CBallCamera::kBCB_PathCameraDesiredPos:
  case CBallCamera::kBCB_PathCamera:
    SetPathCamera(hint.GetDelegatedCameraId(), mgr);
    break;
  case CBallCamera::kBCB_SpindleCamera:
    SetSpindleCamera(hint.GetDelegatedCameraId(), mgr);
    break;
  default:
    SetPathCamera(kInvalidUniqueId, mgr);
    SetSpindleCamera(kInvalidUniqueId, mgr);
    break;
  }

  if (hint.GetOverrideFlags() & 0x2000) {
    CinematicCut(mgr);
  }

  mBallCamera->UpdateLookAtPosition(0.f, mgr);

  if ((hint.GetOverrideFlags() & 0x20) == 0 &&
      (hint.GetBehaviourType() != CBallCamera::kBCB_Default ||
       (oldHint && (!oldHint || oldHint->GetBehaviourType() != CBallCamera::kBCB_Default)))) {
    const CVector3f& lookAtPos = mBallCamera->GetLookAtPosition();
    SetupInterpolation(camXf, mBallCamera->GetUniqueId(), lookAtPos,
                       hint.GetInfo().GetInterpolateTime(), hint.GetInfo().GetClampVelRange(),
                       hint.GetInfo().GetClampRotRange(), (hint.GetOverrideFlags() & 0x400) != 0,
                       mgr);
  }
}

void CCameraManager::NoCameraHintsLeft(CStateManager& mgr) {
  CScriptCameraHint* hint = TCastToPtr< CScriptCameraHint >(mgr.ObjectById(mCamHintId));
  CTransform4f ballCamXf = mBallCamera->GetTransform();

  mCamHintId = kInvalidUniqueId;
  mHintPriority = 1000;

  if (!hint) {
    return;
  }

  CVector3f camToPlayerFlat = mgr.GetPlayer()->GetTranslation() - ballCamXf.GetTranslation();
  camToPlayerFlat.SetZ(0.f);
  if (camToPlayerFlat.CanBeNormalized()) {
    camToPlayerFlat.Normalize();
  } else {
    camToPlayerFlat = mgr.GetPlayer()->GetMovementDirection();
  }

  mBallCamera->ResetToTweaks(mgr);
  mBallCamera->UpdateLookAtPosition(0.f, mgr);
  if (!mgr.GetPlayer()->IsMorphBallTransitioning() &&
      hint->GetInfo().GetBehaviourType() != CBallCamera::kBCB_Default) {
    if ((hint->GetInfo().GetOverrideFlags() & 0x1000) != 0) {
      mBallCamera->SetClampVelRange(hint->GetInfo().GetClampVelRange());
      mBallCamera->SetClampVelTimer(hint->GetInfo().GetClampVelTime());
    } else {
      mBallCamera->TeleportCamera(mBallCamera->FindDesiredTransform(camToPlayerFlat, mgr),
                                     mgr);
      SetupInterpolation(ballCamXf, mBallCamera->GetUniqueId(),
                         mBallCamera->GetLookAtPosition(), hint->GetInfo().GetClampVelTime(),
                         hint->GetInfo().GetClampVelRange(), hint->GetInfo().GetClampRotRange(),
                         hint->GetInfo().Flagx800(), mgr);
    }
  }
}

void CCameraManager::UpdateCameraHints(float dt, CStateManager& mgr) {
  bool invalidHintRemoved = false;
  for (rstl::reserved_vector< rstl::pair< int, TUniqueId >, 64 >::iterator it =
           mCameraHints.begin();
       it != mCameraHints.end();) {
    if (!TCastToConstPtr< CScriptCameraHint >(mgr.ObjectById(it->second))) {
      it = mCameraHints.erase(it);
      invalidHintRemoved = true;
    } else {
      ++it;
    }
  }

  bool inactiveHintRemoved = false;
  if (!mInactiveCameraHints.empty()) {
    for (rstl::reserved_vector< TUniqueId, 64 >::iterator id = mInactiveCameraHints.begin();
         id != mInactiveCameraHints.end(); ++id) {
      TUniqueId uid = *id;
      const CScriptCameraHint* const hint =
          TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(uid));
      if (hint) {
        if (hint->GetSenderCount() == 0 || hint->GetInactive()) {
          for (rstl::reserved_vector< rstl::pair< int, TUniqueId >, 64 >::iterator it =
                   mCameraHints.begin();
               it != mCameraHints.end(); ++it) {
            if (it->second == uid) {
              mCameraHints.erase(it);
              if (uid == mCamHintId) {
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
    mInactiveCameraHints.clear();
  }

  bool activeHintAdded = false;
  if (!mActiveCameraHints.empty()) {
    for (rstl::reserved_vector< TUniqueId, 64 >::iterator id = mActiveCameraHints.begin();
         id != mActiveCameraHints.end(); ++id) {
      TUniqueId uid = *id;
      const CScriptCameraHint* const hint =
          TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(uid));
      if (hint) {
        bool activeHintPresent = false;
        for (rstl::reserved_vector< rstl::pair< int, TUniqueId >, 64 >::const_iterator it =
                 mCameraHints.begin();
             it != mCameraHints.end(); ++it) {
          if (it->second == uid) {
            activeHintPresent = true;
            break;
          }
        }

        if (!activeHintPresent) {
          mCameraHints.push_back(rstl::pair< int, TUniqueId >(hint->GetPriority(), uid));
          activeHintAdded = true;
        }
      }
    }
    mActiveCameraHints.clear();
  }

  if (inactiveHintRemoved || activeHintAdded || invalidHintRemoved) {
    rstl::less< int > less;
    rstl::pair_sorter_finder< rstl::pair< int, TUniqueId >, rstl::less< int > > sorter(less);
    rstl::sort(mCameraHints.begin(), mCameraHints.end(), sorter);
    CTransform4f ballCamXf = mBallCamera->GetTransform();
    if ((inactiveHintRemoved || invalidHintRemoved) && mCameraHints.empty()) {
      NoCameraHintsLeft(mgr);
      return;
    }
    CScriptCameraHint* bestHint = nullptr;
    bool foundHint = false;
    for (rstl::reserved_vector< rstl::pair< int, TUniqueId >, 64 >::iterator it =
             mCameraHints.begin();
         it != mCameraHints.end(); ++it) {
      bestHint = TCastToPtr< CScriptCameraHint >(mgr.ObjectById(it->second));
      if (bestHint) {
        foundHint = true;
        break;
      }
    }
    if (!foundHint) {
      NoCameraHintsLeft(mgr);
    }

    bool changeHint = false;
    if (bestHint && foundHint) {
      if ((bestHint->GetInfo().GetOverrideFlags() & 0x80) != 0 && mCameraHints.size() > 1) {
        CVector3f ballPos = mgr.GetPlayer()->GetBallPosition();
        if ((bestHint->GetInfo().GetOverrideFlags() & 0x100) != 0) {
          CVector3f camToBall = ballPos - ballCamXf.GetTranslation();
          if (camToBall.CanBeNormalized()) {
            camToBall.Normalize();
          } else {
            camToBall = ballCamXf.GetColumn(kDY);
          }

          for (rstl::reserved_vector< rstl::pair< int, TUniqueId >, 64 >::iterator it =
                   mCameraHints.begin() + 1;
               it != mCameraHints.end(); ++it) {
            if (CScriptCameraHint* hint =
                    TCastToPtr< CScriptCameraHint >(mgr.ObjectById(it->second))) {
              if ((hint->GetInfo().GetOverrideFlags() & 0x80) != 0 &&
                  hint->GetPriority() == bestHint->GetPriority() &&
                  hint->GetCurrentAreaId() == bestHint->GetCurrentAreaId()) {
                CVector3f hintToBall = ballPos - bestHint->GetTranslation();
                if (hintToBall.CanBeNormalized()) {
                  hintToBall.Normalize();
                } else {
                  hintToBall = bestHint->GetTransform().GetColumn(kDY);
                }

                float camHintDot = CMath::Limit(CVector3f::Dot(camToBall, hintToBall), 1.f);

                CVector3f thisHintToBall = ballPos - hint->GetTranslation();
                if (thisHintToBall.CanBeNormalized()) {
                  thisHintToBall.Normalize();
                } else {
                  thisHintToBall = hint->GetTransform().GetColumn(kDY);
                }

                float camThisHintDot = CMath::Limit(CVector3f::Dot(camToBall, thisHintToBall), 1.f);

                if (camThisHintDot > camHintDot) {
                  bestHint = hint;
                }
              } else {
                break;
              }
            } else {
              break;
            }
          }
        } else if (const CActor* act =
                       TCastToConstPtr< CActor >(mgr.GetObjectById(bestHint->GetSenderId()))) {
          CVector3f actorPos = act->GetTranslation();
          CVector3f ballPos = mgr.GetPlayer()->GetBallPosition();
          CVector3f ballToHelper = actorPos - ballPos;
          if (ballToHelper.CanBeNormalized()) {
            ballToHelper.Normalize();
          } else {
            ballToHelper = bestHint->GetTransform().GetColumn(kDY);
          }

          for (rstl::reserved_vector< rstl::pair< int, TUniqueId >, 64 >::iterator it =
                   mCameraHints.begin() + 1;
               it != mCameraHints.end(); ++it) {
            if (CScriptCameraHint* hint =
                    TCastToPtr< CScriptCameraHint >(mgr.ObjectById(it->second))) {
              if ((hint->GetInfo().GetOverrideFlags() & 0x80) != 0 &&
                  hint->GetPriority() == bestHint->GetPriority() &&
                  hint->GetCurrentAreaId() == bestHint->GetCurrentAreaId()) {
                CVector3f hintToHelper = actorPos - bestHint->GetTranslation();
                if (hintToHelper.CanBeNormalized()) {
                  hintToHelper.Normalize();
                } else {
                  hintToHelper = bestHint->GetTransform().GetColumn(kDY);
                }

                float ballHintDot = CMath::Limit(CVector3f::Dot(hintToHelper, ballToHelper), 1.f);

                CVector3f thisBallToHelper = actorPos - ballPos;
                if (thisBallToHelper.CanBeNormalized()) {
                  thisBallToHelper.Normalize();
                } else {
                  thisBallToHelper = hint->GetTransform().GetColumn(kDY);
                }

                CVector3f thisHintToHelper = actorPos - hint->GetTranslation();
                if (thisHintToHelper.CanBeNormalized()) {
                  thisHintToHelper.Normalize();
                } else {
                  thisHintToHelper = hint->GetTransform().GetColumn(kDY);
                }

                float thisBallHintDot =
                    CMath::Limit(CVector3f::Dot(thisHintToHelper, thisBallToHelper), 1.f);

                if (thisBallHintDot > ballHintDot) {
                  bestHint = hint;
                }
              } else {
                break;
              }
            } else {
              break;
            }
          }
        }

        if (bestHint->GetUniqueId() != mCamHintId) {
          changeHint = true;
        }
      } else if (mCamHintId != bestHint->GetUniqueId()) {
        if (bestHint->GetInfo().GetBehaviourType() == CBallCamera::kBCB_HintInitializePosition) {
          if ((bestHint->GetInfo().GetOverrideFlags() & 0x20) != 0) {
            mBallCamera->TeleportCamera(
                CTransform4f::LookAt(bestHint->GetTranslation(),
                                     mBallCamera->GetLookAtPosition()),
                mgr);
          }
          ReallyRemoveCameraHint(bestHint->GetUniqueId(), mgr);
          if ((bestHint->GetInfo().GetOverrideFlags() & 0x2000) != 0) {
            CinematicCut(mgr);
          }
          changeHint = false;
        } else {
          changeHint = true;
        }
      }

      if (changeHint) {
        UseCameraHint(*bestHint, mgr);
      }
    }
  }
}

bool CCameraManager::HasBallCameraInitialPositionHint(CStateManager& mgr) const {
  if (HasCameraHint(mgr)) {
    switch (mgr.CameraManager()->GetCameraHint(mgr)->GetBehaviourType()) {
    case CBallCamera::kBCB_HintBallToCam:
    case CBallCamera::kBCB_HintFixedPosition:
    case CBallCamera::kBCB_HintFixedTransform:
    case CBallCamera::kBCB_PathCamera:
    case CBallCamera::kBCB_SpindleCamera:
      return true;
    default:
      break;
    }
  }
  return false;
}

void CCameraManager::AddCameraHint(TUniqueId uid, CStateManager& mgr) {
  if (CScriptCameraHint* hint = TCastToPtr< CScriptCameraHint >(mgr.ObjectById(uid))) {
    const rstl::reserved_vector< TUniqueId, 64 >::iterator search =
        rstl::find(mActiveCameraHints.begin(), mActiveCameraHints.end(), uid);
    if (search != mActiveCameraHints.end() || mCameraHints.size() == 64 ||
        mActiveCameraHints.size() == 64) {
      return;
    }
    mActiveCameraHints.push_back(uid);
  }
}

void CCameraManager::ReallyRemoveCameraHint(TUniqueId uid, CStateManager& mgr) {
  if (CScriptCameraHint* const hint = TCastToPtr< CScriptCameraHint >(mgr.ObjectById(uid))) {
    if (rstl::find(mInactiveCameraHints.begin(), mInactiveCameraHints.end(), uid) ==
        mInactiveCameraHints.end()) {
      hint->ClearSenders();
      hint->SetInactive(true);
      if (mInactiveCameraHints.size() != 64) {
        mInactiveCameraHints.push_back(uid);
      }
    }
  }
}

void CCameraManager::DeleteCameraHint(TUniqueId uid, CStateManager& mgr) {
  const CScriptCameraHint* const hint = TCastToConstPtr< CScriptCameraHint >(mgr.ObjectById(uid));
  const rstl::reserved_vector< TUniqueId, 64 >::iterator search =
      rstl::find(mInactiveCameraHints.begin(), mInactiveCameraHints.end(), uid);
  if (search == mInactiveCameraHints.end() && hint && mInactiveCameraHints.size() != 64) {
    mInactiveCameraHints.push_back(uid);
  }
}

void CCameraManager::ResetCameraHint(CStateManager& mgr) {
  if (const CScriptCameraHint* hint =
          TCastToPtr< CScriptCameraHint >(mgr.ObjectById(mCamHintId))) {
    if (hint->GetBehaviourType() == CBallCamera::kBCB_HintInitializePosition) {
      if (hint->GetOverrideFlags() & 0x20) {
        mBallCamera->TeleportCamera(hint->GetTransform(), mgr);
      }
      DeleteCameraHint(mCamHintId, mgr);
    } else {
      UseCameraHint(*hint, mgr);
    }
  }
}

const CScriptCameraHint* CCameraManager::GetCameraHint(CStateManager& mgr) const {
  return TCastToConstPtr< CScriptCameraHint >(mgr.GetObjectById(mCamHintId));
}

bool CCameraManager::HasCameraHint(CStateManager& mgr) const {
  if (!mCameraHints.empty() && mCamHintId != kInvalidUniqueId &&
      mgr.GetObjectById(mCamHintId)) {
    return true;
  }
  return false;
}

void CCameraManager::SetPathCamera(TUniqueId uid, CStateManager& mgr) {
  mPathCamId = uid;
  if (CPathCamera* camera = TCastToPtr< CPathCamera >(mgr.ObjectById(uid))) {
    camera->Reset(GetCurrentCameraTransform(mgr), mgr);
    mBallCamera->TeleportCamera(camera->GetTransform(), mgr);
  }
}

TUniqueId CCameraManager::GetPathCameraId() const { return mPathCamId; }

void CCameraManager::SetSpindleCamera(TUniqueId uid, CStateManager& mgr) {
  mSpindleCamId = uid;
  if (CScriptSpindleCamera* camera = TCastToPtr< CScriptSpindleCamera >(mgr.ObjectById(uid))) {
    camera->Reset(GetCurrentCameraTransform(mgr), mgr);
    mBallCamera->TeleportCamera(camera->GetTransform(), mgr);
  }
}

TUniqueId CCameraManager::GetSpindleCameraId() const { return mSpindleCamId; }

float CCameraManager::GetCameraBobMagnitude() const {
  float dot = CMath::AbsF(CMath::Limit(
      CVector3f::Dot(mFpCamera->GetTransform().GetForward(), CVector3f::Up()), 1.f));
  float pitch = CMath::Limit(dot / cosf(M_PIF / 6.f), 1.f);
  return 1.f - pitch;
}
