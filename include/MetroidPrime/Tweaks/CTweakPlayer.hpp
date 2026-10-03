#ifndef _CTWEAKPLAYER
#define _CTWEAKPLAYER

#include "types.h"

#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/TOneStatic.hpp"

#include "rstl/string.hpp"

class CInputStream;
class CTweakPlayer;

class CTweakPlayer : public ITweakObject, public TOneStatic< CTweakPlayer > {
public:
  CTweakPlayer(CInputStream&);

  float GetMaxTranslationalAcceleration(CPlayer::ESurfaceRestraints s) const {
    if (s < CPlayer::kSR_Normal || s > CPlayer::kSR_Shrubbery) {
      s = CPlayer::kSR_Normal;
    }
    return mMaxTranslationalAcceleration[s];
  }
  float GetMaxRotationalAcceleration(CPlayer::ESurfaceRestraints s) const {
    if (s < CPlayer::kSR_Normal || s > CPlayer::kSR_Shrubbery) {
      s = CPlayer::kSR_Normal;
    }
    return mMaxRotationalAcceleration[s];
  }
  float GetPlayerTranslationFriction(CPlayer::ESurfaceRestraints s) const {
    if (s < CPlayer::kSR_Normal || s > CPlayer::kSR_Shrubbery) {
      s = CPlayer::kSR_Normal;
    }
    return mTranslationFriction[s];
  }
  float GetPlayerRotationFriction(CPlayer::ESurfaceRestraints s) const {
    if (s < CPlayer::kSR_Normal || s > CPlayer::kSR_Shrubbery) {
      s = CPlayer::kSR_Normal;
    }
    return mRotationFriction[s];
  }
  float GetPlayerRotationMaxSpeed(CPlayer::ESurfaceRestraints s) const {
    if (s < CPlayer::kSR_Normal || s > CPlayer::kSR_Shrubbery) {
      s = CPlayer::kSR_Normal;
    }
    return mRotationMaxSpeed[s];
  }
  float GetPlayerTranslationMaxSpeed(CPlayer::ESurfaceRestraints s) const {
    if (s < CPlayer::kSR_Normal || s > CPlayer::kSR_Shrubbery) {
      s = CPlayer::kSR_Normal;
    }
    return mTranslationMaxSpeed[s];
  }
  float GetNormalGravAccel() const { return mNormalGravAccel; }
  float GetFluidGravAccel() const { return mFluidGravAccel; }
  float GetVerticalJumpAccel() const { return mVerticalJumpAccel; }
  float GetHorizontalJumpAccel() const { return mHorizontalJumpAccel; }
  float GetVerticalDoubleJumpAccel() const { return mVerticalDoubleJumpAccel; }
  float GetHorizontalDoubleJumpAccel() const { return mHorizontalDoubleJumpAccel; }
  float GetWaterJumpFactor() const { return mWaterJumpFactor; }
  float GetWaterBallJumpFactor() const { return mWaterBallJumpFactor; }
  float GetLavaJumpFactor() const { return mLavaJumpFactor; }
  float GetLavaBallJumpFactor() const { return mLavaBallJumpFactor; }
  float GetPhazonJumpFactor() const { return mPhazonJumpFactor; }
  float GetPhazonBallJumpFactor() const { return mPhazonBallJumpFactor; }
  float GetAllowedJumpTime() const { return mAllowedJumpTime; }
  float GetAllowedDoubleJumpTime() const { return mAllowedDoubleJumpTime; }
  float GetMinDoubleJumpWindow() const { return mMinDoubleJumpWindow; }
  float GetMaxDoubleJumpWindow() const { return mMaxDoubleJumpWindow; }
  float GetMinJumpTime() const { return mMinJumpTime; }
  float GetMinDoubleJumpTime() const { return mMinDoubleJumpTime; }
  float GetAllowedLedgeTime() const { return mAllowedLedgeTime; }
  float GetDoubleJumpImpulse() const { return mDoubleJumpImpulse; }
  float GetBackwardsForceMultiplier() const { return mBackwardsForceMultiplier; }
  float GetBombJumpRadius() const { return mBombJumpRadius; }
  float GetBombJumpHeight() const { return mBombJumpHeight; }
  float GetEyeOffset() const { return mEyeOffset; }
  float GetTurnSpeedMultiplier() const { return mTurnSpeedMultiplier; }
  float GetFreeLookTurnSpeedMultiplier() const { return mFreeLookTurnSpeedMultiplier; }
  float GetFreeLookSpeed() const { return mFreeLookSpeed; }
  float GetFreeLookSnapSpeed() const { return mFreeLookSnapSpeed; }
  float GetFreeLookCenteredThresholdAngle() const { return mFreeLookCenteredThresholdAngle; }
  float GetFreeLookCenteredTime() const { return mFreeLookCenteredTime; }
  float GetOrbitModeTimer() const { return mOrbitModeTimer; }
  float GetOrbitUpperAngle() const { return mOrbitUpperAngle; }
  float GetOrbitLowerAngle() const { return mOrbitLowerAngle; }
  float GetOrbitHorizAngle() const { return mOrbitHorizAngle; }
  float GetOrbitMaxTargetDistance() const { return mOrbitMaxTargetDistance; }
  float GetOrbitMaxLockDistance() const { return mOrbitMaxLockDistance; }
  float GetOrbitDistanceThreshold() const { return mOrbitDistanceThreshold; }
#if VERSION >= VERSION_R3IJ_00
  int GetOrbitZoneWidth(int zone) const;
  int GetOrbitZoneHeight(int zone) const;
  int GetOrbitZoneCentreX(int zone, const CStateManager& mgr) const;
  int GetOrbitZoneCentreY(int zone, const CStateManager& mgr) const;
#else
  uint GetOrbitZoneWidth(int zone) const { return mOrbitScreenBoxHalfExtentX[zone]; }
  uint GetOrbitZoneHeight(int zone) const { return mOrbitScreenBoxHalfExtentY[zone]; }
  uint GetOrbitZoneCentreX(int zone) const { return mOrbitScreenBoxCenterX[zone]; }
  uint GetOrbitZoneCentreY(int zone) const { return mOrbitScreenBoxCenterY[zone]; }
#endif
  uint GetOrbitZoneIdealX(int zone) const { return mOrbitZoneIdealX[zone]; }
  uint GetOrbitZoneIdealY(int zone) const { return mOrbitZoneIdealY[zone]; }
  float GetOrbitNearX() const { return mOrbitNearX; }
  float GetOrbitNearZ() const { return mOrbitNearZ; }
  float GetOrbitFixedOffsetZDiff() const { return mOrbitFixedOffsetZDiff; }
  float GetOrbitZRange() const { return mOrbitZRange; }
  bool GetDashEnabled() const { return mDashEnabled; }
  bool GetDashOnButtonRelease() const { return mDashOnButtonRelease; }
  float GetDashButtonHoldCancelTime() const { return mDashButtonHoldCancelTime; }
  float GetDashStrafeInputThreshold() const { return mDashStrafeInputThreshold; }
  float GetSidewaysDoubleJumpImpulse() const { return mSidewaysDoubleJumpImpulse; }
  float GetSidewaysVerticalDoubleJumpAccel() const { return mSidewaysVerticalDoubleJumpAccel; }
  float GetSidewaysHorizontalDoubleJumpAccel() const { return mSidewaysHorizontalDoubleJumpAccel; }
  float GetScanningRange() const { return mScanningRange; }
  bool GetScanRetention() const { return mScanRetention; }
  bool GetScanFreezesGame() const { return mScanFreezesGame; }
  bool GetOrbitWhileScanning() const { return mOrbitWhileScanning; }
  float GetScanMaxTargetDistance() const { return mScanMaxTargetDistance; }
  float GetScanMaxLockDistance() const { return mScanMaxLockDistance; }
  bool GetMoveDuringFreeLook() const { return mMoveDuringFreeLook; }
  bool GetHoldButtonsForFreeLook() const { return mHoldButtonsForFreeLook; }
  bool GetTwoButtonsForFreeLook() const { return mTwoButtonsForFreeLook; }
  bool GetAimWhenOrbitingPoint() const { return mAimWhenOrbitingPoint; }
  bool GetStayInFreeLookWhileFiring() const { return mStayInFreeLookWhileFiring; }
  bool GetOrbitFixedOffset() const { return mOrbitFixedOffset; }
  bool GetGunButtonTogglesHolster() const { return mGunButtonTogglesHolster; }
  bool GetGunNotFiringHolstersGun() const { return mGunNotFiringHolstersGun; }
  bool GetFallingDoubleJump() const { return mFallingDoubleJump; }
  bool GetImpulseDoubleJump() const { return mImpulseDoubleJump; }
  bool GetFiringCancelsCameraPitch() const { return mFiringCancelsCameraPitch; }
  bool GetAssistedAimingIgnoreHorizontal() const { return mAssistedAimingIgnoreHorizontal; }
  bool GetAssistedAimingIgnoreVertical() const { return mAssistedAimingIgnoreVertical; }
  float GetAimMaxDistance() const { return mAimMaxDistance; }
  float GetAimThresholdDistance() const { return mAimThresholdDistance; }
  float GetAimBoxWidth() const { return mAimBoxWidth; }
  float GetAimBoxHeight() const { return mAimBoxHeight; }
  float GetAimTargetTimer() const { return mAimTargetTimer; }
  float GetAimAssistHorizontalAngle() const { return mAimAssistHorizontalAngle; }
  float GetAimAssistVerticalAngle() const { return mAimAssistVerticalAngle; }
  float GetPlayerHeight() const { return mPlayerHeight; }
  float GetPlayerXYHalfExtent() const { return mPlayerXYHalfExtent; }
  float GetStepUpHeight() const { return mStepUpHeight; }
  float GetStepDownHeight() const { return mStepDownHeight; }
  float GetPlayerBallHalfExtent() const { return mPlayerBallHalfExtent; }
  float GetOrbitDistanceMax() const { return mOrbitDistanceMax; }
  float GetGrappleSwingLength() const { return mGrappleSwingLength; }
  float GetGrappleSwingPeriod() const { return mGrappleSwingPeriod; }
  float GetGrapplePullSpeedMin() const { return mGrapplePullSpeedMin; }
  float GetMaxGrappleLockedTurnAlignDistance() const { return mMaxGrappleLockedTurnAlignDistance; }
  float GetGrapplePullSpeedProportion() const { return mGrapplePullSpeedProportion; }
  float GetGrapplePullSpeedMax() const { return mGrapplePullSpeedMax; }
  float GetGrappleLookCenterSpeed() const { return mGrappleLookCenterSpeed; }
  float GetMaxGrappleTurnSpeed() const { return mMaxGrappleTurnSpeed; }
  float GetGrappleJumpForce() const { return mGrappleJumpForce; }
  float GetGrappleReleaseTime() const { return mGrappleReleaseTime; }
  uint GetGrappleJumpMode() const { return mGrappleJumpMode; }
  bool GetOrbitReleaseBreaksGrapple() const { return mOrbitReleaseBreaksGrapple; }
  bool GetInvertGrappleTurn() const { return mInvertGrappleTurn; }
  float GetGrappleBeamSpeed() const { return mGrappleBeamSpeed; }
  float GetGrappleBeamXWaveAmplitude() const { return mGrappleBeamXWaveAmplitude; }
  float GetGrappleBeamZWaveAmplitude() const { return mGrappleBeamZWaveAmplitude; }
  float GetGrappleBeamAnglePhaseDelta() const { return mGrappleBeamAnglePhaseDelta; }
  float GetHorizontalFreeLookAngleVel() const { return mHorizontalFreeLookAngleVel; }
  float GetVerticalFreeLookAngleVel() const { return mVerticalFreeLookAngleVel; }
  float GetOrbitCameraSpeed() const { return mOrbitCameraSpeed; }
  float GetOrbitPreventionTime() const { return mOrbitPreventionTime; }
  bool GetFreeLookTurnsPlayer() const { return mFreelookTurnsPlayer; }
  float GetJumpCameraPitchDownStart() const { return mJumpCameraPitchDownStart; }
  float GetJumpCameraPitchDownFull() const { return mJumpCameraPitchDownFull; }
  float GetJumpCameraPitchDownAngle() const { return mJumpCameraPitchDownAngle; }
  float GetFallCameraPitchDownStart() const { return mFallCameraPitchDownStart; }
  float GetFallCameraPitchDownFull() const { return mFallCameraPitchDownFull; }
  float GetFallCameraPitchDownAngle() const { return mFallCameraPitchDownAngle; }
  float GetFirstPersonCameraSpeed() const { return mFirstPersonCameraSpeed; }
  float GetGrappleCameraSpeed() const { return mGrappleCameraSpeed; }
  float GetFreeLookDampenFactor() const { return mFreeLookDampenFactor; }
  float GetLeftAnalogMax() const { return mLeftDiv; }
  float GetRightAnalogMax() const { return mRightDiv; }
  float GetOrbitMinDistance(int type) const { return mOrbitMinDistance[type]; }
  float GetOrbitNormalDistance(int type) const { return mOrbitNormalDistance[type]; }
  float GetOrbitMaxDistance(int type) const { return mOrbitMaxDistance[type]; }
  float GetFrozenTimeout() const { return mFrozenTimeout; }
  uint GetIceBreakJumpCount() const { return mIceBreakJumpCount; }
  float GetVariaDamageReduction() const { return mVariaDamageReduction; }
  float GetGravityDamageReduction() const { return mGravityDamageReduction; }
  float GetPhazonDamageReduction() const { return mPhazonDamageReduction; }

  float mMaxTranslationalAcceleration[8];
  float mMaxRotationalAcceleration[8];
  float mTranslationFriction[8];
  float mRotationFriction[8];
  float mRotationMaxSpeed[8];
  float mTranslationMaxSpeed[8];
  float mNormalGravAccel;
  float mFluidGravAccel;
  float mVerticalJumpAccel;
  float mHorizontalJumpAccel;
  float mVerticalDoubleJumpAccel;
  float mHorizontalDoubleJumpAccel;
  float mWaterJumpFactor;
  float mWaterBallJumpFactor;
  float mLavaJumpFactor;
  float mLavaBallJumpFactor;
  float mPhazonJumpFactor;
  float mPhazonBallJumpFactor;
  float mAllowedJumpTime;
  float mAllowedDoubleJumpTime;
  float mMinDoubleJumpWindow;
  float mMaxDoubleJumpWindow;
  float x104_;
  float mMinJumpTime;
  float mMinDoubleJumpTime;
  float mAllowedLedgeTime;
  float mDoubleJumpImpulse;
  float mBackwardsForceMultiplier;
  float mBombJumpRadius;
  float mBombJumpHeight;
  float mEyeOffset;
  float mTurnSpeedMultiplier;
  float mFreeLookTurnSpeedMultiplier;
  float mHorizontalFreeLookAngleVel;
  float mVerticalFreeLookAngleVel;
  float mFreeLookSpeed;
  float mFreeLookSnapSpeed;
  float x140_;
  float mFreeLookCenteredThresholdAngle;
  float mFreeLookCenteredTime;
  float mFreeLookDampenFactor;
  float mLeftDiv;
  float mRightDiv;
  float mOrbitMinDistance[3];
  float mOrbitNormalDistance[3];
  float mOrbitMaxDistance[3];
  float x17c_;
  float mOrbitModeTimer;
  float mOrbitCameraSpeed;
  float mOrbitUpperAngle;
  float mOrbitLowerAngle;
  float mOrbitHorizAngle;
  float x194_;
  float x198_;
  float mOrbitMaxTargetDistance;
  float mOrbitMaxLockDistance;
  float mOrbitDistanceThreshold;
  uint mOrbitScreenBoxHalfExtentX[2];
  uint mOrbitScreenBoxHalfExtentY[2];
  uint mOrbitScreenBoxCenterX[2];
  uint mOrbitScreenBoxCenterY[2];
  uint mOrbitZoneIdealX[2];
  uint mOrbitZoneIdealY[2];
  float mOrbitNearX;
  float mOrbitNearZ;
  float x1e0_;
  float x1e4_;
  float mOrbitFixedOffsetZDiff;
  float mOrbitZRange;
  float x1f0_;
  float x1f4_;
  float x1f8_;
  float mOrbitPreventionTime;
  bool mDashEnabled : 1;
  bool mDashOnButtonRelease : 1;
  float mDashButtonHoldCancelTime;
  float mDashStrafeInputThreshold;
  float mSidewaysDoubleJumpImpulse;
  float mSidewaysVerticalDoubleJumpAccel;
  float mSidewaysHorizontalDoubleJumpAccel;
  float mScanningRange;
  bool mScanRetention : 1;
  bool mScanFreezesGame : 1;
  bool mOrbitWhileScanning : 1;
  float mScanMaxTargetDistance;
  float mScanMaxLockDistance;
  bool mFreelookTurnsPlayer : 1;
  bool x228_25_ : 1;
  bool x228_26_ : 1;
  bool mMoveDuringFreeLook : 1;
  bool mHoldButtonsForFreeLook : 1;
  bool mTwoButtonsForFreeLook : 1;
  bool x228_30_ : 1;
  bool x228_31_ : 1;
  bool x229_24_ : 1;
  bool mAimWhenOrbitingPoint : 1;
  bool mStayInFreeLookWhileFiring : 1;
  bool x229_27_ : 1;
  bool x229_28_ : 1;
  bool mOrbitFixedOffset : 1;
  bool mGunButtonTogglesHolster : 1;
  bool mGunNotFiringHolstersGun : 1;
  bool mFallingDoubleJump : 1;
  bool mImpulseDoubleJump : 1;
  bool mFiringCancelsCameraPitch : 1;
  bool mAssistedAimingIgnoreHorizontal : 1;
  bool mAssistedAimingIgnoreVertical : 1;
  float x22c_;
  float x230_;
  float mAimMaxDistance;
  float x238_;
  float x23c_;
  float x240_;
  float x244_;
  float x248_;
  float mAimThresholdDistance;
  float x250_;
  float x254_;
  float mAimBoxWidth;
  float mAimBoxHeight;
  float mAimTargetTimer;
  float mAimAssistHorizontalAngle;
  float mAimAssistVerticalAngle;
  float mPlayerHeight;
  float mPlayerXYHalfExtent;
  float mStepUpHeight;
  float mStepDownHeight;
  float mPlayerBallHalfExtent;
  float mFirstPersonCameraSpeed;
  float x284_;
  float mJumpCameraPitchDownStart;
  float mJumpCameraPitchDownFull;
  float mJumpCameraPitchDownAngle;
  float mFallCameraPitchDownStart;
  float mFallCameraPitchDownFull;
  float mFallCameraPitchDownAngle;
  float mOrbitDistanceMax;
  float mGrappleSwingLength;
  float mGrappleSwingPeriod;
  float mGrapplePullSpeedMin;
  float mGrappleCameraSpeed;
  float mMaxGrappleLockedTurnAlignDistance;
  float mGrapplePullSpeedProportion;
  float mGrapplePullSpeedMax;
  float mGrappleLookCenterSpeed;
  float mMaxGrappleTurnSpeed;
  float mGrappleJumpForce;
  float mGrappleReleaseTime;
  uint mGrappleJumpMode;
  bool mOrbitReleaseBreaksGrapple;
  bool mInvertGrappleTurn;
  float mGrappleBeamSpeed;
  float mGrappleBeamXWaveAmplitude;
  float mGrappleBeamZWaveAmplitude;
  float mGrappleBeamAnglePhaseDelta;
  float x2e8_;
  float x2ec_;
  float x2f0_;
  bool x2f4_;
  float mFrozenTimeout;
  int mIceBreakJumpCount;
  float mVariaDamageReduction;
  float mGravityDamageReduction;
  float mPhazonDamageReduction;
};

CHECK_SIZEOF(CTweakPlayer, 0x30c)

extern CTweakPlayer* gpTweakPlayer;

#endif // _CTWEAKPLAYER
