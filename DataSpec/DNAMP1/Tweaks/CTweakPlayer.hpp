#pragma once

#include "../../DNACommon/Tweaks/ITweakPlayer.hpp"

namespace DataSpec::DNAMP1 {

struct CTweakPlayer final : ITweakPlayer {
  AT_DECL_EXPLICIT_DNA_YAML
  Value<float> x4_maxTranslationalAcceleration[8];
  Value<float> x24_maxRotationalAcceleration[8];
  Value<float> x44_translationFriction[8];
  Value<float> x64_rotationFriction[8];
  Value<float> x84_rotationMaxSpeed[8];
  Value<float> xa4_translationMaxSpeed[8];
  Value<float> xc4_normalGravAccel;
  Value<float> xc8_fluidGravAccel;
  Value<float> xcc_verticalJumpAccel;
  Value<float> xd0_horizontalJumpAccel;
  Value<float> xd4_verticalDoubleJumpAccel;
  Value<float> xd8_horizontalDoubleJumpAccel;
  Value<float> xdc_waterJumpFactor;
  Value<float> xe0_waterBallJumpFactor;
  Value<float> xe4_lavaJumpFactor;
  Value<float> xe8_lavaBallJumpFactor;
  Value<float> xec_phazonJumpFactor;
  Value<float> xf0_phazonBallJumpFactor;
  Value<float> xf4_allowedJumpTime;
  Value<float> xf8_allowedDoubleJumpTime;
  Value<float> xfc_minDoubleJumpWindow;
  Value<float> x100_maxDoubleJumpWindow;
  Value<float> x104_;
  Value<float> x108_minJumpTime;
  Value<float> x10c_minDoubleJumpTime;
  Value<float> x110_allowedLedgeTime;
  Value<float> x114_doubleJumpImpulse;
  Value<float> x118_backwardsForceMultiplier;
  Value<float> x11c_bombJumpRadius;
  Value<float> x120_bombJumpHeight;
  Value<float> x124_eyeOffset;
  Value<float> x128_turnSpeedMultiplier;
  Value<float> x12c_freeLookTurnSpeedMultiplier;
  Value<float> x130_horizontalFreeLookAngleVel;
  Value<float> x134_verticalFreeLookAngleVel;
  Value<float> x138_freeLookSpeed;
  Value<float> x13c_freeLookSnapSpeed;
  Value<float> x140_;
  Value<float> x144_freeLookCenteredThresholdAngle;
  Value<float> x148_freeLookCenteredTime;
  Value<float> x14c_freeLookDampenFactor;
  Value<float> x150_leftDiv;
  Value<float> x154_rightDiv;
  Value<float> x158_orbitMinDistance[3];
  Value<float> x164_orbitNormalDistance[3];
  Value<float> x170_orbitMaxDistance[3];
  Value<float> x17c_;
  Value<float> x180_orbitModeTimer;
  Value<float> x184_orbitCameraSpeed;
  Value<float> x188_orbitUpperAngle;
  Value<float> x18c_orbitLowerAngle;
  Value<float> x190_orbitHorizAngle;
  Value<float> x194_;
  Value<float> x198_;
  Value<float> x19c_orbitMaxTargetDistance;
  Value<float> x1a0_orbitMaxLockDistance;
  Value<float> x1a4_orbitDistanceThreshold;
  Value<atUint32> x1a8_orbitScreenBoxHalfExtentX[2];
  Value<atUint32> x1b0_orbitScreenBoxHalfExtentY[2];
  Value<atUint32> x1b8_orbitScreenBoxCenterX[2];
  Value<atUint32> x1c0_orbitScreenBoxCenterY[2];
  Value<atUint32> x1c8_orbitZoneIdealX[2];
  Value<atUint32> x1d0_orbitZoneIdealY[2];
  Value<float> x1d8_orbitNearX;
  Value<float> x1dc_orbitNearZ;
  Value<float> x1e0_;
  Value<float> x1e4_;
  Value<float> x1e8_orbitFixedOffsetZDiff;
  Value<float> x1ec_orbitZRange;
  Value<float> x1f0_;
  Value<float> x1f4_;
  Value<float> x1f8_;
  Value<float> x1fc_orbitPreventionTime;
  Value<bool> x200_24_dashEnabled : 1;
  Value<bool> x200_25_dashOnButtonRelease : 1;
  Value<float> x204_dashButtonHoldCancelTime;
  Value<float> x208_dashStrafeInputThreshold;
  Value<float> x20c_sidewaysDoubleJumpImpulse;
  Value<float> x210_sidewaysVerticalDoubleJumpAccel;
  Value<float> x214_sidewaysHorizontalDoubleJumpAccel;
  Value<float> x218_scanningRange;
  Value<bool> x21c_24_scanRetention : 1;
  Value<bool> x21c_25_scanFreezesGame : 1;
  Value<bool> x21c_26_orbitWhileScanning : 1;
  Value<float> x220_scanMaxTargetDistance;
  Value<float> x224_scanMaxLockDistance;
  Value<bool> x228_24_freelookTurnsPlayer : 1;
  Value<bool> x228_25_ : 1;
  Value<bool> x228_26_ : 1;
  Value<bool> x228_27_moveDuringFreeLook : 1;
  Value<bool> x228_28_holdButtonsForFreeLook : 1;
  Value<bool> x228_29_twoButtonsForFreeLook : 1;
  Value<bool> x228_30_ : 1;
  Value<bool> x228_31_ : 1;
  Value<bool> x229_24_ : 1;
  Value<bool> x229_25_aimWhenOrbitingPoint : 1;
  Value<bool> x229_26_stayInFreeLookWhileFiring : 1;
  Value<bool> x229_27_ : 1;
  Value<bool> x229_28_ : 1;
  Value<bool> x229_29_orbitFixedOffset : 1;
  Value<bool> x229_30_gunButtonTogglesHolster : 1;
  Value<bool> x229_31_gunNotFiringHolstersGun : 1;
  Value<bool> x22a_24_fallingDoubleJump : 1;
  Value<bool> x22a_25_impulseDoubleJump : 1;
  Value<bool> x22a_26_firingCancelsCameraPitch : 1;
  Value<bool> x22a_27_assistedAimingIgnoreHorizontal : 1;
  Value<bool> x22a_28_assistedAimingIgnoreVertical : 1;
  Value<float> x22c_;
  Value<float> x230_;
  Value<float> x234_aimMaxDistance;
  Value<float> x238_;
  Value<float> x23c_;
  Value<float> x240_;
  Value<float> x244_;
  Value<float> x248_;
  Value<float> x24c_aimThresholdDistance;
  Value<float> x250_;
  Value<float> x254_;
  Value<float> x258_aimBoxWidth;
  Value<float> x25c_aimBoxHeight;
  Value<float> x260_aimTargetTimer;
  Value<float> x264_aimAssistHorizontalAngle;
  Value<float> x268_aimAssistVerticalAngle;
  Value<float> x26c_playerHeight;
  Value<float> x270_playerXYHalfExtent;
  Value<float> x274_stepUpHeight;
  Value<float> x278_stepDownHeight;
  Value<float> x27c_playerBallHalfExtent;
  Value<float> x280_firstPersonCameraSpeed;
  Value<float> x284_;
  Value<float> x288_jumpCameraPitchDownStart;
  Value<float> x28c_jumpCameraPitchDownFull;
  Value<float> x290_jumpCameraPitchDownAngle;
  Value<float> x294_fallCameraPitchDownStart;
  Value<float> x298_fallCameraPitchDownFull;
  Value<float> x29c_fallCameraPitchDownAngle;
  Value<float> x2a0_orbitDistanceMax;
  Value<float> x2a4_grappleSwingLength;
  Value<float> x2a8_grappleSwingPeriod;
  Value<float> x2ac_grapplePullSpeedMin;
  Value<float> x2b0_grappleCameraSpeed;
  Value<float> x2b4_maxGrappleLockedTurnAlignDistance;
  Value<float> x2b8_grapplePullSpeedProportion;
  Value<float> x2bc_grapplePullSpeedMax;
  Value<float> x2c0_grappleLookCenterSpeed;
  Value<float> x2c4_maxGrappleTurnSpeed;
  Value<float> x2c8_grappleJumpForce;
  Value<float> x2cc_grappleReleaseTime;
  Value<atUint32> x2d0_grappleJumpMode;
  Value<bool> x2d4_orbitReleaseBreaksGrapple;
  Value<bool> x2d5_invertGrappleTurn;
  Value<float> x2d8_grappleBeamSpeed;
  Value<float> x2dc_grappleBeamXWaveAmplitude;
  Value<float> x2e0_grappleBeamZWaveAmplitude;
  Value<float> x2e4_grappleBeamAnglePhaseDelta;
  Value<float> x2e8_;
  Value<float> x2ec_;
  Value<float> x2f0_;
  Value<bool> x2f4_;
  Value<float> x2f8_frozenTimeout;
  Value<atUint32> x2fc_iceBreakJumpCount;
  Value<float> x300_variaDamageReduction;
  Value<float> x304_gravityDamageReduction;
  Value<float> x308_phazonDamageReduction;
  float GetMaxTranslationalAcceleration(int s) const override { return x4_maxTranslationalAcceleration[s]; }
  float GetMaxRotationalAcceleration(int s) const override { return x24_maxRotationalAcceleration[s]; }
  float GetPlayerTranslationFriction(int s) const override { return x44_translationFriction[s]; }
  float GetPlayerRotationFriction(int s) const override { return x64_rotationFriction[s]; }
  float GetPlayerRotationMaxSpeed(int s) const override { return x84_rotationMaxSpeed[s]; }
  float GetPlayerTranslationMaxSpeed(int s) const override { return xa4_translationMaxSpeed[s]; }
  float GetNormalGravAccel() const override { return xc4_normalGravAccel; }
  float GetFluidGravAccel() const override { return xc8_fluidGravAccel; }
  float GetVerticalJumpAccel() const override { return xcc_verticalJumpAccel; }
  float GetHorizontalJumpAccel() const override { return xd0_horizontalJumpAccel; }
  float GetVerticalDoubleJumpAccel() const override { return xd4_verticalDoubleJumpAccel; }
  float GetHorizontalDoubleJumpAccel() const override { return xd8_horizontalDoubleJumpAccel; }
  float GetWaterJumpFactor() const override { return xdc_waterJumpFactor; }
  float GetWaterBallJumpFactor() const override { return xe0_waterBallJumpFactor; }
  float GetLavaJumpFactor() const override { return xe4_lavaJumpFactor; }
  float GetLavaBallJumpFactor() const override { return xe8_lavaBallJumpFactor; }
  float GetPhazonJumpFactor() const override { return xec_phazonJumpFactor; }
  float GetPhazonBallJumpFactor() const override { return xf0_phazonBallJumpFactor; }
  float GetAllowedJumpTime() const override { return xf4_allowedJumpTime; }
  float GetAllowedDoubleJumpTime() const override { return xf8_allowedDoubleJumpTime; }
  float GetMinDoubleJumpWindow() const override { return xfc_minDoubleJumpWindow; }
  float GetMaxDoubleJumpWindow() const override { return x100_maxDoubleJumpWindow; }
  float GetMinJumpTime() const override { return x108_minJumpTime; }
  float GetMinDoubleJumpTime() const override { return x10c_minDoubleJumpTime; }
  float GetAllowedLedgeTime() const override { return x110_allowedLedgeTime; }
  float GetDoubleJumpImpulse() const override { return x114_doubleJumpImpulse; }
  float GetBackwardsForceMultiplier() const override { return x118_backwardsForceMultiplier; }
  float GetBombJumpRadius() const override { return x11c_bombJumpRadius; }
  float GetBombJumpHeight() const override { return x120_bombJumpHeight; }
  float GetEyeOffset() const override { return x124_eyeOffset; }
  float GetTurnSpeedMultiplier() const override { return x128_turnSpeedMultiplier; }
  float GetFreeLookTurnSpeedMultiplier() const override { return x12c_freeLookTurnSpeedMultiplier; }
  float GetFreeLookSpeed() const override { return x138_freeLookSpeed; }
  float GetFreeLookSnapSpeed() const override { return x13c_freeLookSnapSpeed; }
  float GetFreeLookCenteredThresholdAngle() const override { return x144_freeLookCenteredThresholdAngle; }
  float GetFreeLookCenteredTime() const override { return x148_freeLookCenteredTime; }
  float GetOrbitModeTimer() const override { return x180_orbitModeTimer; }
  float GetOrbitUpperAngle() const override { return x188_orbitUpperAngle; }
  float GetOrbitLowerAngle() const override { return x18c_orbitLowerAngle; }
  float GetOrbitHorizAngle() const override { return x190_orbitHorizAngle; }
  float GetOrbitMaxTargetDistance() const override { return x19c_orbitMaxTargetDistance; }
  float GetOrbitMaxLockDistance() const override { return x1a0_orbitMaxLockDistance; }
  float GetOrbitDistanceThreshold() const override { return x1a4_orbitDistanceThreshold; }
  uint32_t GetOrbitScreenBoxHalfExtentX(int zone) const override { return x1a8_orbitScreenBoxHalfExtentX[zone]; }
  uint32_t GetOrbitScreenBoxHalfExtentY(int zone) const override { return x1b0_orbitScreenBoxHalfExtentY[zone]; }
  uint32_t GetOrbitScreenBoxCenterX(int zone) const override { return x1b8_orbitScreenBoxCenterX[zone]; }
  uint32_t GetOrbitScreenBoxCenterY(int zone) const override { return x1c0_orbitScreenBoxCenterY[zone]; }
  uint32_t GetOrbitZoneIdealX(int zone) const override { return x1c8_orbitZoneIdealX[zone]; }
  uint32_t GetOrbitZoneIdealY(int zone) const override { return x1d0_orbitZoneIdealY[zone]; }
  float GetOrbitNearX() const override { return x1d8_orbitNearX; }
  float GetOrbitNearZ() const override { return x1dc_orbitNearZ; }
  float GetOrbitFixedOffsetZDiff() const override { return x1e8_orbitFixedOffsetZDiff; }
  float GetOrbitZRange() const override { return x1ec_orbitZRange; }
  bool GetDashEnabled() const override { return x200_24_dashEnabled; }
  bool GetDashOnButtonRelease() const override { return x200_25_dashOnButtonRelease; }
  float GetDashButtonHoldCancelTime() const override { return x204_dashButtonHoldCancelTime; }
  float GetDashStrafeInputThreshold() const override { return x208_dashStrafeInputThreshold; }
  float GetSidewaysDoubleJumpImpulse() const override { return x20c_sidewaysDoubleJumpImpulse; }
  float GetSidewaysVerticalDoubleJumpAccel() const override { return x210_sidewaysVerticalDoubleJumpAccel; }
  float GetSidewaysHorizontalDoubleJumpAccel() const override { return x214_sidewaysHorizontalDoubleJumpAccel; }
  float GetScanningRange() const override { return x218_scanningRange; }
  bool GetScanRetention() const override { return x21c_24_scanRetention; }
  bool GetScanFreezesGame() const override { return x21c_25_scanFreezesGame; }
  bool GetOrbitWhileScanning() const override { return x21c_26_orbitWhileScanning; }
  float GetScanMaxTargetDistance() const override { return x220_scanMaxTargetDistance; }
  float GetScanMaxLockDistance() const override { return x224_scanMaxLockDistance; }
  bool GetMoveDuringFreeLook() const override { return x228_27_moveDuringFreeLook; }
  bool GetHoldButtonsForFreeLook() const override { return x228_28_holdButtonsForFreeLook; }
  bool GetTwoButtonsForFreeLook() const override { return x228_29_twoButtonsForFreeLook; }
  bool GetAimWhenOrbitingPoint() const override { return x229_25_aimWhenOrbitingPoint; }
  bool GetStayInFreeLookWhileFiring() const override { return x229_26_stayInFreeLookWhileFiring; }
  bool GetOrbitFixedOffset() const override { return x229_29_orbitFixedOffset; }
  bool GetGunButtonTogglesHolster() const override { return x229_30_gunButtonTogglesHolster; }
  bool GetGunNotFiringHolstersGun() const override { return x229_31_gunNotFiringHolstersGun; }
  bool GetFallingDoubleJump() const override { return x22a_24_fallingDoubleJump; }
  bool GetImpulseDoubleJump() const override { return x22a_25_impulseDoubleJump; }
  bool GetFiringCancelsCameraPitch() const override { return x22a_26_firingCancelsCameraPitch; }
  bool GetAssistedAimingIgnoreHorizontal() const override { return x22a_27_assistedAimingIgnoreHorizontal; }
  bool GetAssistedAimingIgnoreVertical() const override { return x22a_28_assistedAimingIgnoreVertical; }
  float GetAimMaxDistance() const override { return x234_aimMaxDistance; }
  float GetAimThresholdDistance() const override { return x24c_aimThresholdDistance; }
  float GetAimBoxWidth() const override { return x258_aimBoxWidth; }
  float GetAimBoxHeight() const override { return x25c_aimBoxHeight; }
  float GetAimTargetTimer() const override { return x260_aimTargetTimer; }
  float GetAimAssistHorizontalAngle() const override { return x264_aimAssistHorizontalAngle; }
  float GetAimAssistVerticalAngle() const override { return x268_aimAssistVerticalAngle; }
  float GetPlayerHeight() const override { return x26c_playerHeight; }
  float GetPlayerXYHalfExtent() const override { return x270_playerXYHalfExtent; }
  float GetStepUpHeight() const override { return x274_stepUpHeight; }
  float GetStepDownHeight() const override { return x278_stepDownHeight; }
  float GetPlayerBallHalfExtent() const override { return x27c_playerBallHalfExtent; }
  float GetOrbitDistanceMax() const override { return x2a0_orbitDistanceMax; }
  float GetGrappleSwingLength() const override { return x2a4_grappleSwingLength; }
  float GetGrappleSwingPeriod() const override { return x2a8_grappleSwingPeriod; }
  float GetGrapplePullSpeedMin() const override { return x2ac_grapplePullSpeedMin; }
  float GetMaxGrappleLockedTurnAlignDistance() const override { return x2b4_maxGrappleLockedTurnAlignDistance; }
  float GetGrapplePullSpeedProportion() const override { return x2b8_grapplePullSpeedProportion; }
  float GetGrapplePullSpeedMax() const override { return x2bc_grapplePullSpeedMax; }
  float GetGrappleLookCenterSpeed() const override { return x2c0_grappleLookCenterSpeed; }
  float GetMaxGrappleTurnSpeed() const override { return x2c4_maxGrappleTurnSpeed; }
  float GetGrappleJumpForce() const override { return x2c8_grappleJumpForce; }
  float GetGrappleReleaseTime() const override { return x2cc_grappleReleaseTime; }
  uint32_t GetGrappleJumpMode() const override { return x2d0_grappleJumpMode; }
  bool GetOrbitReleaseBreaksGrapple() const override { return x2d4_orbitReleaseBreaksGrapple; }
  bool GetInvertGrappleTurn() const override { return x2d5_invertGrappleTurn; }
  float GetGrappleBeamSpeed() const override { return x2d8_grappleBeamSpeed; }
  float GetGrappleBeamXWaveAmplitude() const override { return x2dc_grappleBeamXWaveAmplitude; }
  float GetGrappleBeamZWaveAmplitude() const override { return x2e0_grappleBeamZWaveAmplitude; }
  float GetGrappleBeamAnglePhaseDelta() const override { return x2e4_grappleBeamAnglePhaseDelta; }
  float GetHorizontalFreeLookAngleVel() const override { return x130_horizontalFreeLookAngleVel; }
  float GetVerticalFreeLookAngleVel() const override { return x134_verticalFreeLookAngleVel; }
  float GetOrbitCameraSpeed() const override { return x184_orbitCameraSpeed; }
  float GetOrbitPreventionTime() const override { return x1fc_orbitPreventionTime; }
  bool GetFreeLookTurnsPlayer() const override { return x228_24_freelookTurnsPlayer; }
  float GetJumpCameraPitchDownStart() const override { return x288_jumpCameraPitchDownStart; }
  float GetJumpCameraPitchDownFull() const override { return x28c_jumpCameraPitchDownFull; }
  float GetJumpCameraPitchDownAngle() const override { return x290_jumpCameraPitchDownAngle; }
  float GetFallCameraPitchDownStart() const override { return x294_fallCameraPitchDownStart; }
  float GetFallCameraPitchDownFull() const override { return x298_fallCameraPitchDownFull; }
  float GetFallCameraPitchDownAngle() const override { return x29c_fallCameraPitchDownAngle; }
  float GetFirstPersonCameraSpeed() const override { return x280_firstPersonCameraSpeed; }
  float GetGrappleCameraSpeed() const override { return x2b0_grappleCameraSpeed; }
  float GetFreeLookDampenFactor() const override { return x14c_freeLookDampenFactor; }
  float GetLeftLogicalThreshold() const override { return x150_leftDiv; }
  float GetRightLogicalThreshold() const override { return x154_rightDiv; }
  float GetOrbitMinDistance(int type) const override { return x158_orbitMinDistance[type]; }
  float GetOrbitNormalDistance(int type) const override { return x164_orbitNormalDistance[type]; }
  float GetOrbitMaxDistance(int type) const override { return x170_orbitMaxDistance[type]; }
  float GetFrozenTimeout() const override { return x2f8_frozenTimeout; }
  uint32_t GetIceBreakJumpCount() const override { return x2fc_iceBreakJumpCount; }
  float GetVariaDamageReduction() const override { return x300_variaDamageReduction; }
  float GetGravityDamageReduction() const override { return x304_gravityDamageReduction; }
  float GetPhazonDamageReduction() const override { return x308_phazonDamageReduction; }
  CTweakPlayer() = default;
  CTweakPlayer(athena::io::IStreamReader& reader) {
    read(reader);
    FixupValues();
  }

  void FixupValues();
};

} // namespace DataSpec::DNAMP1
