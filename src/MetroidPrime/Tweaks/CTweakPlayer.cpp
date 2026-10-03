#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

CTweakPlayer::CTweakPlayer(CInputStream& in)
: mGrappleBeamSpeed(0.0)
, mGrappleBeamXWaveAmplitude(0.0)
, mGrappleBeamZWaveAmplitude(0.0)
, mGrappleBeamAnglePhaseDelta(0.0) {
  int i;
  for (i = 0; i < ARRAY_SIZE(mMaxTranslationalAcceleration); ++i) {
    mMaxTranslationalAcceleration[i] = in.ReadFloat();
  }
  for (i = 0; i < ARRAY_SIZE(mMaxRotationalAcceleration); ++i) {
    mMaxRotationalAcceleration[i] = in.ReadFloat();
  }
  for (i = 0; i < ARRAY_SIZE(mTranslationFriction); ++i) {
    mTranslationFriction[i] = in.ReadFloat();
  }
  for (i = 0; i < ARRAY_SIZE(mRotationFriction); ++i) {
    mRotationFriction[i] = in.ReadFloat();
  }
  for (i = 0; i < ARRAY_SIZE(mRotationMaxSpeed); ++i) {
    mRotationMaxSpeed[i] = in.ReadFloat();
  }
  for (i = 0; i < ARRAY_SIZE(mTranslationMaxSpeed); ++i) {
    mTranslationMaxSpeed[i] = in.ReadFloat();
  }

  mNormalGravAccel = in.ReadFloat();
  mFluidGravAccel = in.ReadFloat();
  mVerticalJumpAccel = in.ReadFloat();
  mHorizontalJumpAccel = in.ReadFloat();
  mVerticalDoubleJumpAccel = in.ReadFloat();
  mHorizontalDoubleJumpAccel = in.ReadFloat();
  mWaterJumpFactor = in.ReadFloat();
  mWaterBallJumpFactor = in.ReadFloat();
  mLavaJumpFactor = in.ReadFloat();
  mLavaBallJumpFactor = in.ReadFloat();
  mPhazonJumpFactor = in.ReadFloat();
  mPhazonBallJumpFactor = in.ReadFloat();
  mAllowedJumpTime = in.ReadFloat();
  mAllowedDoubleJumpTime = in.ReadFloat();
  mMinDoubleJumpWindow = in.ReadFloat();
  mMaxDoubleJumpWindow = in.ReadFloat();
  x104_ = in.ReadFloat();
  mMinJumpTime = in.ReadFloat();
  mMinDoubleJumpTime = in.ReadFloat();
  mAllowedLedgeTime = in.ReadFloat();
  mDoubleJumpImpulse = in.ReadFloat();
  mBackwardsForceMultiplier = in.ReadFloat();
  mBombJumpRadius = in.ReadFloat();
  mBombJumpHeight = in.ReadFloat();
  mEyeOffset = in.ReadFloat();
  mTurnSpeedMultiplier = in.ReadFloat();
  mFreeLookTurnSpeedMultiplier = in.ReadFloat();

  mHorizontalFreeLookAngleVel = in.ReadFloat() * (M_PIF / 180.f);
  mVerticalFreeLookAngleVel = in.ReadFloat() * (M_PIF / 180.f);
  mFreeLookSpeed = in.ReadFloat() * (M_PIF / 180.f);
  mFreeLookSnapSpeed = in.ReadFloat() * (M_PIF / 180.f);
  x140_ = in.ReadFloat() * (M_PIF / 180.f);
  mFreeLookCenteredThresholdAngle = in.ReadFloat() * (M_PIF / 180.f);

  mFreeLookCenteredTime = in.ReadFloat();
  mFreeLookDampenFactor = in.ReadFloat();
  mLeftDiv = in.ReadFloat();
  mRightDiv = in.ReadFloat();

  mFreelookTurnsPlayer = in.ReadBool();
  x228_25_ = in.ReadBool();
  x228_26_ = in.ReadBool();
  mMoveDuringFreeLook = in.ReadBool();
  mHoldButtonsForFreeLook = in.ReadBool();
  mTwoButtonsForFreeLook = in.ReadBool();
  x228_30_ = in.ReadBool();
  x228_31_ = in.ReadBool();
  x229_24_ = in.ReadBool();
  mAimWhenOrbitingPoint = in.ReadBool();
  mStayInFreeLookWhileFiring = in.ReadBool();
  x229_27_ = in.ReadBool();
  x229_28_ = in.ReadBool();
  mOrbitFixedOffset = in.ReadBool();
  mGunButtonTogglesHolster = in.ReadBool();
  mGunNotFiringHolstersGun = in.ReadBool();
  mFallingDoubleJump = in.ReadBool();
  mImpulseDoubleJump = in.ReadBool();
  mFiringCancelsCameraPitch = in.ReadBool();
  mAssistedAimingIgnoreHorizontal = in.ReadBool();
  mAssistedAimingIgnoreVertical = in.ReadBool();

  x22c_ = in.ReadFloat();
  x230_ = in.ReadFloat();
  mAimMaxDistance = in.ReadFloat();

  x238_ = in.ReadFloat() * (M_PIF / 180.f);
  x23c_ = in.ReadFloat() * (M_PIF / 180.f);
  x240_ = in.ReadFloat() * (M_PIF / 180.f);
  x244_ = in.ReadFloat() * (M_PIF / 180.f);
  x248_ = in.ReadFloat() * (M_PIF / 180.f);
  mAimThresholdDistance = in.ReadFloat();
  x250_ = in.ReadFloat() * (M_PIF / 180.f);
  x254_ = in.ReadFloat();
  mAimBoxWidth = in.ReadFloat();
  mAimBoxHeight = in.ReadFloat();
  mAimTargetTimer = in.ReadFloat();
  mAimAssistHorizontalAngle = in.ReadFloat() * (M_PIF / 180.f);
  mAimAssistVerticalAngle = in.ReadFloat() * (M_PIF / 180.f);

  for (i = 0; i < 3; ++i) {
    mOrbitMinDistance[i] = in.ReadFloat();
    mOrbitNormalDistance[i] = in.ReadFloat();
    mOrbitMaxDistance[i] = in.ReadFloat();
  }

  x17c_ = in.ReadFloat() * (M_PIF / 180.f);
  mOrbitModeTimer = in.ReadFloat();
  mOrbitCameraSpeed = in.ReadFloat() * (M_PIF / 180.f);
  mOrbitUpperAngle = in.ReadFloat() * (M_PIF / 180.f);
  mOrbitLowerAngle = in.ReadFloat() * (M_PIF / 180.f);
  mOrbitHorizAngle = in.ReadFloat() * (M_PIF / 180.f);
  x194_ = in.ReadFloat() * (M_PIF / 180.f);
  x198_ = in.ReadFloat() * (M_PIF / 180.f);
  mOrbitMaxTargetDistance = in.ReadFloat();
  mOrbitMaxLockDistance = in.ReadFloat();
  mOrbitDistanceThreshold = in.ReadFloat();

  for (i = 0; i < 2; ++i) {
    mOrbitScreenBoxHalfExtentX[i] = in.ReadLong();
    mOrbitScreenBoxHalfExtentY[i] = in.ReadLong();
    mOrbitScreenBoxCenterX[i] = in.ReadLong();
    mOrbitScreenBoxCenterY[i] = in.ReadLong();
    mOrbitZoneIdealX[i] = in.ReadLong();
    mOrbitZoneIdealY[i] = in.ReadLong();
  }

#if VERSION >= VERSION_R3IJ_00
  mOrbitScreenBoxHalfExtentX[CPlayer::kZI_Scan] = mOrbitScreenBoxHalfExtentX[CPlayer::kZI_Targeting];
  mOrbitScreenBoxHalfExtentY[CPlayer::kZI_Scan] = mOrbitScreenBoxHalfExtentY[CPlayer::kZI_Targeting];
  mOrbitScreenBoxCenterX[CPlayer::kZI_Scan] = mOrbitScreenBoxCenterX[CPlayer::kZI_Targeting];
  mOrbitScreenBoxCenterY[CPlayer::kZI_Scan] = mOrbitScreenBoxCenterY[CPlayer::kZI_Targeting];
  mOrbitZoneIdealX[CPlayer::kZI_Scan] = mOrbitZoneIdealX[CPlayer::kZI_Targeting];
  mOrbitZoneIdealY[CPlayer::kZI_Scan] = mOrbitZoneIdealY[CPlayer::kZI_Targeting];
#endif

  mOrbitNearX = in.ReadFloat();
  mOrbitNearZ = in.ReadFloat();
  x1e0_ = in.ReadFloat();
  x1e4_ = in.ReadFloat();
  mOrbitFixedOffsetZDiff = in.ReadFloat();
  mOrbitZRange = in.ReadFloat();
  x1f0_ = in.ReadFloat() * (M_PIF / 180.f);
  x1f4_ = in.ReadFloat() * (M_PIF / 180.f);
  x1f8_ = in.ReadFloat();
  mOrbitPreventionTime = in.ReadFloat();
  mDashEnabled = in.ReadBool();
  mDashOnButtonRelease = in.ReadBool();
  mDashButtonHoldCancelTime = in.ReadFloat();
  mDashStrafeInputThreshold = in.ReadFloat();
  mSidewaysDoubleJumpImpulse = in.ReadFloat();
  mSidewaysVerticalDoubleJumpAccel = in.ReadFloat();
  mSidewaysHorizontalDoubleJumpAccel = in.ReadFloat();
  mScanningRange = in.ReadFloat();
  mScanRetention = in.ReadBool();
  mScanFreezesGame = in.ReadBool();
  mOrbitWhileScanning = in.ReadBool();
  mScanMaxTargetDistance = in.ReadFloat();
  mScanMaxLockDistance = in.ReadFloat();
  mOrbitDistanceMax = in.ReadFloat();
  mGrappleSwingLength = in.ReadFloat();
  mGrappleSwingPeriod = in.ReadFloat();
  mGrapplePullSpeedMin = in.ReadFloat();
  mGrappleCameraSpeed = in.ReadFloat() * (M_PIF / 180.f);
  mMaxGrappleLockedTurnAlignDistance = in.ReadFloat();
  mGrapplePullSpeedProportion = in.ReadFloat();
  mGrapplePullSpeedMax = in.ReadFloat();
  mGrappleLookCenterSpeed = in.ReadFloat() * (M_PIF / 180.f);
  mMaxGrappleTurnSpeed = in.ReadFloat();
  mGrappleJumpForce = in.ReadFloat();
  mGrappleReleaseTime = in.ReadFloat();
  mGrappleJumpMode = in.ReadLong();
  mOrbitReleaseBreaksGrapple = in.ReadBool();
  mInvertGrappleTurn = in.ReadBool();
  mGrappleBeamSpeed = in.ReadFloat();
  mGrappleBeamXWaveAmplitude = in.ReadFloat();
  mGrappleBeamZWaveAmplitude = in.ReadFloat();
  mGrappleBeamAnglePhaseDelta = in.ReadFloat();
  mPlayerHeight = in.ReadFloat();
  mPlayerXYHalfExtent = in.ReadFloat();
  mStepUpHeight = in.ReadFloat();
  mStepDownHeight = in.ReadFloat();
  mPlayerBallHalfExtent = in.ReadFloat();
  mFirstPersonCameraSpeed = in.ReadFloat() * (M_PIF / 180.f);
  x284_ = in.ReadFloat() * (M_PIF / 180.f);
  mJumpCameraPitchDownStart = in.ReadFloat();
  mJumpCameraPitchDownFull = in.ReadFloat();
  mJumpCameraPitchDownAngle = in.ReadFloat() * (M_PIF / 180.f);
  mFallCameraPitchDownStart = in.ReadFloat();
  mFallCameraPitchDownFull = in.ReadFloat();
  mFallCameraPitchDownAngle = in.ReadFloat() * (M_PIF / 180.f);
  x2e8_ = in.ReadFloat();
  x2ec_ = in.ReadFloat();
  x2f0_ = in.ReadFloat();
  x2f4_ = in.ReadBool();
  mFrozenTimeout = in.ReadFloat();
  mIceBreakJumpCount = in.ReadLong();
  mVariaDamageReduction = in.ReadFloat();
  mGravityDamageReduction = in.ReadFloat();
  mPhazonDamageReduction = in.ReadFloat();
}

#if VERSION >= VERSION_R3IJ_00
int CTweakPlayer::GetOrbitZoneWidth(int zone) const {
  return static_cast< int >(0.65f * CCast::LtoF(mOrbitScreenBoxHalfExtentX[zone]) /
                            CGraphics::GetPixelAspectRatio());
}

int CTweakPlayer::GetOrbitZoneHeight(int zone) const {
  return static_cast< int >(0.65f * CCast::LtoF(mOrbitScreenBoxHalfExtentY[zone]));
}

int CTweakPlayer::GetOrbitZoneCentreX(int zone, const CStateManager& mgr) const {
  float center = CCast::LtoF(mOrbitScreenBoxCenterX[zone]);
  const CVector3f cursor = mgr.GetCameraManager()->GetCurrentCamera(mgr).ConvertToScreenSpace(
      mgr.GetPlayer()->GetAimingCursor().GetCursorInWorld());
  center = cursor.GetX() * (0.5f * CCast::LtoF(CGraphics::GetViewportWidth()));
  const float offset = 0.5f * CCast::LtoF(CGraphics::GetViewportWidth());
  center += offset;
  return static_cast< int >(center);
}

int CTweakPlayer::GetOrbitZoneCentreY(int zone, const CStateManager& mgr) const {
  float center = CCast::LtoF(mOrbitScreenBoxCenterY[zone]);
  const CVector3f cursor = mgr.GetCameraManager()->GetCurrentCamera(mgr).ConvertToScreenSpace(
      mgr.GetPlayer()->GetAimingCursor().GetCursorInWorld());
  center = cursor.GetY() * (0.5f * CCast::LtoF(CGraphics::GetViewportHeight()));
  const float offset = 0.5f * CCast::LtoF(CGraphics::GetViewportHeight());
  center += offset;
  return static_cast< int >(center);
}
#endif
