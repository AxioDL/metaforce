#ifndef __DNACOMMON_ITWEAKPLAYER_HPP__
#define __DNACOMMON_ITWEAKPLAYER_HPP__

#include "ITweak.hpp"
#include "zeus/CAABox.hpp"

namespace DataSpec
{

struct ITweakPlayer : ITweak
{
    virtual float GetMaxTranslationalAcceleration(int s) const=0;
    virtual float GetMaxRotationalAcceleration(int s) const=0;
    virtual float GetPlayerTranslationFriction(int s) const=0;
    virtual float GetPlayerRotationFriction(int s) const=0;
    virtual float GetPlayerRotationMaxSpeed(int s) const=0;
    virtual float GetPlayerTranslationMaxSpeed(int s) const=0;
    virtual float GetNormalGravAccel() const=0;
    virtual float GetFluidGravAccel() const=0;
    virtual float GetVerticalJumpAccel() const=0;
    virtual float GetHorizontalJumpAccel() const=0;
    virtual float GetVerticalDoubleJumpAccel() const=0;
    virtual float GetHorizontalDoubleJumpAccel() const=0;
    virtual float GetWaterJumpFactor() const=0;
    virtual float GetWaterBallJumpFactor() const=0;
    virtual float GetLavaJumpFactor() const=0;
    virtual float GetLavaBallJumpFactor() const=0;
    virtual float GetPhazonJumpFactor() const=0;
    virtual float GetPhazonBallJumpFactor() const=0;
    virtual float GetAllowedJumpTime() const=0;
    virtual float GetAllowedDoubleJumpTime() const=0;
    virtual float GetMinDoubleJumpWindow() const=0;
    virtual float GetMaxDoubleJumpWindow() const=0;
    virtual float GetMinJumpTime() const=0;
    virtual float GetMinDoubleJumpTime() const=0;
    virtual float GetAllowedLedgeTime() const=0;
    virtual float GetDoubleJumpImpulse() const=0;
    virtual float GetBackwardsForceMultiplier() const=0;
    virtual float GetBombJumpRadius() const=0;
    virtual float GetBombJumpHeight() const=0;
    virtual float GetEyeOffset() const=0;
    virtual float GetTurnSpeedMultiplier() const=0;
    virtual float GetFreeLookTurnSpeedMultiplier() const=0;
    virtual float GetFreeLookSpeed() const=0;
    virtual float GetFreeLookSnapSpeed() const=0;
    virtual float GetFreeLookCenteredThresholdAngle() const=0;
    virtual float GetFreeLookCenteredTime() const=0;
    virtual float GetOrbitModeTimer() const=0;
    virtual float GetOrbitUpperAngle() const=0;
    virtual float GetOrbitLowerAngle() const=0;
    virtual float GetOrbitHorizAngle() const=0;
    virtual float GetOrbitMaxTargetDistance() const=0;
    virtual float GetOrbitMaxLockDistance() const=0;
    virtual float GetOrbitDistanceThreshold() const=0;
    virtual uint32_t GetOrbitScreenBoxHalfExtentX(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxHalfExtentY(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxCenterX(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxCenterY(int zone) const=0;
    virtual uint32_t GetOrbitZoneIdealX(int zone) const=0;
    virtual uint32_t GetOrbitZoneIdealY(int zone) const=0;
    virtual float GetOrbitNearX() const=0;
    virtual float GetOrbitNearZ() const=0;
    virtual float GetOrbitFixedOffsetZDiff() const=0;
    virtual float GetOrbitZRange() const=0;
    virtual bool GetDashEnabled() const=0;
    virtual bool GetDashOnButtonRelease() const=0;
    virtual float GetDashButtonHoldCancelTime() const=0;
    virtual float GetDashStrafeInputThreshold() const=0;
    virtual float GetSidewaysDoubleJumpImpulse() const=0;
    virtual float GetSidewaysVerticalDoubleJumpAccel() const=0;
    virtual float GetSidewaysHorizontalDoubleJumpAccel() const=0;
    virtual float GetScanningRange() const=0; // x218
    virtual bool GetScanRetention() const=0;
    virtual bool GetScanFreezesGame() const=0; // x21c_25
    virtual bool GetOrbitWhileScanning() const=0;
    virtual bool GetFallingDoubleJump() const=0;
    virtual bool GetImpulseDoubleJump() const=0;
    virtual bool GetFiringCancelsCameraPitch() const=0;
    virtual bool GetAssistedAimingIgnoreHorizontal() const=0;
    virtual bool GetAssistedAimingIgnoreVertical() const=0;
    virtual float GetAimMaxDistance() const=0;
    virtual float GetAimThresholdDistance() const=0;
    virtual float GetAimBoxWidth() const=0;
    virtual float GetAimBoxHeight() const=0;
    virtual float GetAimTargetTimer() const=0;
    virtual float GetAimAssistHorizontalAngle() const=0;
    virtual float GetAimAssistVerticalAngle() const=0;
    virtual float GetScanMaxTargetDistance() const=0;
    virtual float GetScanMaxLockDistance() const=0;
    virtual bool GetMoveDuringFreeLook() const=0;
    virtual bool GetHoldButtonsForFreeLook() const=0;
    virtual bool GetTwoButtonsForFreeLook() const=0;
    virtual bool GetAimWhenOrbitingPoint() const=0;
    virtual bool GetStayInFreeLookWhileFiring() const=0;
    virtual bool GetOrbitFixedOffset() const=0;
    virtual bool GetGunButtonTogglesHolster() const=0;
    virtual bool GetGunNotFiringHolstersGun() const=0;
    virtual float GetPlayerHeight() const=0; // x26c
    virtual float GetPlayerXYHalfExtent() const=0; // x270
    virtual bool GetFreeLookTurnsPlayer() const=0; // x228_24
    virtual float GetStepUpHeight() const=0; // x274
    virtual float GetStepDownHeight() const=0; // x278
    virtual float GetPlayerBallHalfExtent() const=0; // x27c
    virtual float GetOrbitDistanceMax() const=0;
    virtual float GetGrappleSwingLength() const=0;
    virtual float GetGrappleSwingPeriod() const=0;
    virtual float GetGrapplePullSpeedMin() const=0;
    virtual float GetMaxGrappleLockedTurnAlignDistance() const=0;
    virtual float GetGrapplePullSpeedProportion() const=0;
    virtual float GetGrapplePullSpeedMax() const=0;
    virtual float GetGrappleLookCenterSpeed() const=0;
    virtual float GetMaxGrappleTurnSpeed() const=0;
    virtual float GetGrappleJumpForce() const=0;
    virtual float GetGrappleReleaseTime() const=0;
    virtual uint32_t GetGrappleJumpMode() const=0;
    virtual bool GetOrbitReleaseBreaksGrapple() const=0;
    virtual bool GetInvertGrappleTurn() const=0;
    virtual float GetGrappleBeamSpeed() const=0;
    virtual float GetGrappleBeamXWaveAmplitude() const=0;
    virtual float GetGrappleBeamZWaveAmplitude() const=0;
    virtual float GetGrappleBeamAnglePhaseDelta() const=0;
    virtual float GetHorizontalFreeLookAngleVel() const=0;
    virtual float GetVerticalFreeLookAngleVel() const=0; // x134
    virtual float GetOrbitCameraSpeed() const=0; // x184
    virtual float GetOrbitPreventionTime() const=0;
    virtual float GetJumpCameraPitchDownStart() const=0; // x288
    virtual float GetJumpCameraPitchDownFull() const=0; // x28c
    virtual float GetJumpCameraPitchDownAngle() const=0; // x290
    virtual float GetFallCameraPitchDownStart() const=0; // x294
    virtual float GetFallCameraPitchDownFull() const=0; // x298
    virtual float GetFallCameraPitchDownAngle() const=0; // x29c
    virtual float GetFirstPersonCameraSpeed() const=0; // x280
    virtual float GetGrappleCameraSpeed() const=0; // x2b0
    virtual float GetFreeLookDampenFactor() const=0; // x14c
    virtual float GetLeftLogicalThreshold() const=0;
    virtual float GetRightLogicalThreshold() const=0;
    virtual float GetOrbitMinDistance(int type) const=0;
    virtual float GetOrbitNormalDistance(int type) const=0;
    virtual float GetOrbitMaxDistance(int type) const=0;
    virtual float GetFrozenTimeout() const=0;
    virtual uint32_t GetIceBreakJumpCount() const=0;
    virtual float GetVariaDamageReduction() const=0;
    virtual float GetGravityDamageReduction() const=0;
    virtual float GetPhazonDamageReduction() const=0;
};

}

#endif // __DNACOMMON_ITWEAKPLAYER_HPP__
