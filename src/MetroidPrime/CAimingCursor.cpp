#include "MetroidPrime/CAimingCursor.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControl.hpp"
#include "MetroidPrime/Tweaks/CTweaks.hpp"

CAimingCursor::CAimingCursor(bool reservedFlag, uint reservedValue)
: mCursor2D(CVector2f::Zero())
, mCursorOnPlane(CVector3f::Zero())
, mRaycastResult()
, mLastValidPointerPlane(CVector3f::Zero())
, mCursorOrbitPosition(CVector3f::Zero())
, mCursorVelocity(CVector3f::Zero())
, mCursorVelocityMagnitude(0.f)
, mCursorVelocity2D(CVector2f::Zero())
, mCursorVelocity2DMagnitude(0.f)
, mCursorInWorld(CVector3f::Zero())
, mCursorObjectId(kInvalidUniqueId)
, mCursorObjectCount(0)
, mCursorLockTimer(0.f)
, mCursorAlpha(1.f)
, mCursorValid(false)
, mReservedFlag(reservedFlag)
, mNunchukPitch(CRelAngle::FromRadians(0.f))
, mCursorFade(0.f)
, mHideForCSICount(0)
, mReservedValue(reservedValue) {}

CVector3f CAimingCursor::GetCursorOrbitPosition(const CStateManager& mgr) const {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mCursorObjectId))) {
    return actor->GetAimPosition(mgr, 0.f);
  }
  return mCursorOrbitPosition;
}

void CAimingCursor::UpdateAlpha(const CFinalInput& input, float dt, const CStateManager& mgr) {
  float alpha = 1.f;
  switch (input.GetControllerData().GetPointerState()) {
  case CControllerData::kPS_Tracking:
    alpha = 1.f;
    break;
  case CControllerData::kPS_RecentlyLost:
    if (ShowOffScreen(mgr)) {
      float time = static_cast< int >(input.GetControllerData().GetPointerInvalidFrameCount() & 31);
      if (time < 16.f) {
        float t = time / 15.f;
        time = t * ((3.f - 2.f * t) * t);
      } else {
        float t = (31.f - time) / 15.f;
        time = t * ((3.f - 2.f * t) * t);
      }
      alpha = CMath::FastMin(CMath::FastMax(0.f, time), 1.f);
    } else {
      alpha = 1.f;
    }
    break;
  case CControllerData::kPS_Lost:
    alpha = 0.f;
    break;
  case CControllerData::kPS_Reacquiring:
    alpha = CMath::FastMin(
        CMath::FastMax(
            0.f, static_cast< int >(input.GetControllerData().GetPointerValidFrameCount()) / 30.f),
        1.f);
    break;
  }

  if (mgr.GetDeferredStateTransition() != kSMT_InGame) {
    mCursorFade = 1.f;
    alpha = 0.f;
  }
  alpha *= 1.f - CMath::FastMin(CMath::FastMax(0.f, mCursorFade), 1.f);
  mCursorAlpha = alpha;
}

void CAimingCursor::UpdateValidity(const CFinalInput& input, float dt, const CStateManager& mgr) {
  mCursorValid = input.GetInputType() <= 2
                           ? input.GetControllerData().GetPointerValidFrameCount() > 10
                           : false;
}

bool CAimingCursor::CheckZeroCursorPosition(const CStateManager& mgr) const {
  bool zero = false;
  if (mgr.GetPlayer()->GetOrbitState() == CPlayer::kOS_OrbitObject) {
    if (CPlayer::GetOrbitLockGun() ||
        gpGameState->GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) {
      zero = true;
    }
  }
  return zero;
}

void CAimingCursor::Update(const CFinalInput& input, float dt, CStateManager& mgr) {
  const CVector3f previousPosition = mCursorOrbitPosition;
  const CVector2f previousCursor = mCursor2D;
  const CVector2f& pointer = input.GetControllerData().GetPointerPosition();
  CVector3f cursor;
  cursor.SetX(pointer.GetX());
  cursor.SetZ(-pointer.GetY());
  cursor.SetY(1.25f);
  if (input.GetControllerData().GetPointerState() == CControllerData::kPS_Tracking) {
    mLastValidPointerPlane = cursor;
  }

  const CPlayer& player = *mgr.GetPlayer();
  if (player.GetFrozenState()) {
    return;
  }

  CVector3f normalCursor = cursor;
  CVector3f ballCursor = cursor;
  normalCursor.SetX(
      normalCursor.GetX() > 0.f
          ? gpTweakPlayerControlCurrent->GetCursorRightResponse().EvaluateAt(normalCursor.GetX())
          : -gpTweakPlayerControlCurrent->GetCursorLeftResponse().EvaluateAt(-normalCursor.GetX()));
  if (player.GetOrbitState() != CPlayer::kOS_OrbitObject) {
    if (player.GetPointerAimHeld()) {
      if (normalCursor.GetZ() > 0.f) {
        normalCursor.SetZ(
            gpTweakPlayerControlCurrent->GetHeldCursorUpResponse().EvaluateAt(normalCursor.GetZ()));
      } else {
        normalCursor.SetZ(-gpTweakPlayerControlCurrent->GetHeldCursorDownResponse().EvaluateAt(
            -normalCursor.GetZ()));
      }
    } else {
      if (normalCursor.GetZ() > 0.f) {
        normalCursor.SetZ(
            gpTweakPlayerControlCurrent->GetCursorUpResponse().EvaluateAt(normalCursor.GetZ()));
      } else {
        normalCursor.SetZ(
            -gpTweakPlayerControlCurrent->GetCursorDownResponse().EvaluateAt(-normalCursor.GetZ()));
      }
    }
  } else {
    if (normalCursor.GetZ() > 0.f) {
      normalCursor.SetZ(
          gpTweakPlayerControlCurrent->GetCursorUpResponse().EvaluateAt(normalCursor.GetZ()));
    } else {
      normalCursor.SetZ(
          -gpTweakPlayerControlCurrent->GetCursorDownResponse().EvaluateAt(-normalCursor.GetZ()));
    }
  }
  ballCursor.SetX(ballCursor.GetX() > 0.f
                      ? gpTweakPlayerControlCurrent->GetBallCursorHorizontalResponse().EvaluateAt(
                            ballCursor.GetX())
                      : -gpTweakPlayerControlCurrent->GetBallCursorHorizontalResponse().EvaluateAt(
                            -ballCursor.GetX()));
  if (ballCursor.GetZ() > 0.f) {
    ballCursor.SetZ(
        gpTweakPlayerControlCurrent->GetBallCursorVerticalResponse().EvaluateAt(ballCursor.GetZ()));
  } else {
    ballCursor.SetZ(-gpTweakPlayerControlCurrent->GetBallCursorVerticalResponse().EvaluateAt(
        -ballCursor.GetZ()));
  }

  switch (player.GetMorphballTransitionState()) {
  case CPlayer::kMS_Unmorphed:
    cursor = normalCursor;
    break;
  case CPlayer::kMS_Morphed:
    cursor = ballCursor;
    break;
  case CPlayer::kMS_Unmorphing:
    cursor = CVector3f::Lerp(ballCursor, normalCursor, player.GetMorphBallTransitionFactor());
    break;
  case CPlayer::kMS_Morphing:
    cursor = CVector3f::Lerp(normalCursor, ballCursor, player.GetMorphBallTransitionFactor());
    break;
  }
  cursor.SetX(cursor.GetX() * CGraphics::GetPixelAspectRatio());
  if (CheckZeroCursorPosition(mgr)) {
    cursor.SetX(0.f);
    cursor.SetZ(0.f);
  }

  if (mCursorLockTimer > 0.f) {
    mCursorLockTimer += dt;
    if (mCursorLockTimer < 0.5f) {
      const float scale =
          CMath::FastMin(CMath::FastMax(0.f, (0.2f - mCursorLockTimer) / 0.2f), 1.f);
      cursor.SetX(cursor.GetX() * scale);
      cursor.SetZ(cursor.GetZ() * scale);
    } else if (mCursorLockTimer < 0.6f) {
      const float scale =
          CMath::FastMin(CMath::FastMax(0.f, (mCursorLockTimer - 0.5f) / (0.6f - 0.5f)), 1.f);
      cursor.SetX(cursor.GetX() * scale);
      cursor.SetZ(cursor.GetZ() * scale);
    } else {
      mCursorLockTimer = 0.f;
    }
  }
  if (mCursorObjectCount != 0 && mCursorObjectCount < 60) {
    cursor *= 1.f - CMath::FastMin(CMath::FastMax(0.f, mCursorObjectCount / 60.f), 1.f);
  }
  mCursor2D[0] = cursor.GetX();
  mCursor2D[1] = cursor.GetZ();

  const CTransform4f cameraTransform = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform();
  const CQuaternion cameraRotation =
      CQuaternion::FromMatrix(mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform());
  mCursorOnPlane = cameraTransform.GetTranslation() + cameraRotation.Transform(cursor);
  const CVector3f rayPoint = mCursorOnPlane;

  static const CMaterialList include(kMT_Solid, kMT_Character, kMT_NonSolidDamageable);
  static const CMaterialList exclude(kMT_ProjectilePassthrough, kMT_Player);
  static const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(include, exclude);
  CVector3f direction = rayPoint - cameraTransform.GetTranslation();
  direction.Normalize();
  TUniqueId objectId = kInvalidUniqueId;
  TEntityList nearList;
  mgr.BuildNearList(nearList, cameraTransform.GetTranslation(), direction, 100.f, filter, nullptr);
  const CRayCastResult result = CGameCollision::RayWorldIntersection(
      mgr, objectId, cameraTransform.GetTranslation(), direction, 100.f, filter, nearList);
  if (result.IsValid()) {
    mCursorInWorld = result.GetPoint();
    mRaycastResult = result;
    mCursorObjectId = objectId;
    mCursorOrbitPosition = result.GetPoint();
    if (result.GetTime() < 0.25f) {
      mCursorInWorld = cameraTransform.GetTranslation() + 0.25f * direction;
    }
  } else {
    mCursorObjectId = objectId;
    mCursorInWorld = cameraTransform.GetTranslation() + 100.f * direction;
    mCursorOrbitPosition = mCursorInWorld;
    mRaycastResult = result;
  }

  if (!close_enough(dt, 0.f)) {
    const CVector3f delta = mCursorInWorld - previousPosition;
    if (delta.IsMagnitudeSafe()) {
      mCursorVelocity = delta / dt;
      mCursorVelocityMagnitude = mCursorVelocity.Magnitude();
    } else {
      mCursorVelocity = CVector3f::Zero();
      mCursorVelocityMagnitude = 0.f;
    }
    const CVector2f delta2D = mCursor2D - previousCursor;
    if (delta2D.IsMagnitudeSafe()) {
      mCursorVelocity2D = delta2D / dt;
      mCursorVelocity2DMagnitude = mCursorVelocity2D.Magnitude();
    } else {
      mCursorVelocity2D = CVector2f::Zero();
      mCursorVelocity2DMagnitude = 0.f;
    }
  }
  mNunchukPitch = CRelAngle::FromRadians(
      input.GetControllerData().GetContinuousAngleAxis(2).GetAbsoluteValue());
  if (mHideForCSICount > 0) {
    mCursorFade += dt;
  } else if (mgr.GetDeferredStateTransition() == kSMT_InGame) {
    mCursorFade -= dt;
  }
  mCursorFade = CMath::FastMin(CMath::FastMax(0.f, mCursorFade), 1.f);
  UpdateAlpha(input, dt, mgr);
  UpdateValidity(input, dt, mgr);
}

CVector2f CAimingCursor::GetCursor2D() const { return mCursor2D; }

CVector3f CAimingCursor::GetCursorOnPlane() const { return mCursorOnPlane; }

CVector3f CAimingCursor::GetCursorInWorld() const { return mCursorInWorld; }

TUniqueId CAimingCursor::GetCursorObjectId() const { return mCursorObjectId; }

uint CAimingCursor::GetCursorObjectCount() const { return mCursorObjectCount; }

bool CAimingCursor::GetCursorValid() const { return mCursorValid; }

float CAimingCursor::GetCursorAlpha() const { return mCursorAlpha; }

CRayCastResult CAimingCursor::GetRaycastResult() const { return mRaycastResult; }

bool CAimingCursor::IsHiddenForCSI() const { return mHideForCSICount > 0; }

float CAimingCursor::GetCursorPlaneDistance() { return 1.25f; }

bool CAimingCursor::ShowOffScreen(const CStateManager& mgr) const {
  const CPlayer& player = *mgr.GetPlayer();
  bool show = false;
  if (player.GetLastInput().GetControllerData().GetPointerInvalidFrameCount() != 0 &&
      !mgr.GetCameraManager()->IsInCinematicCamera() && !player.IsMorphBallTransitioning() &&
      !IsHiddenForCSI() &&
      (player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed ||
       player.GetMorphBall()->GetBoostChargeTimer() > 0.f)) {
    show = true;
  }
  return show;
}
