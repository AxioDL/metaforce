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
  float GetMaxTranslationalAcceleration(int s) const { return x4_maxTranslationalAcceleration[s]; }
  float GetMaxRotationalAcceleration(int s) const { return x24_maxRotationalAcceleration[s]; }
  float GetPlayerTranslationFriction(int s) const { return x44_translationFriction[s]; }
  float GetPlayerRotationFriction(int s) const { return x64_rotationFriction[s]; }
  float GetPlayerRotationMaxSpeed(int s) const { return x84_rotationMaxSpeed[s]; }
  float GetPlayerTranslationMaxSpeed(int s) const { return xa4_translationMaxSpeed[s]; }
  float GetNormalGravAccel() const { return xc4_normalGravAccel; }
  float GetFluidGravAccel() const { return xc8_fluidGravAccel; }
  float GetVerticalJumpAccel() const { return xcc_verticalJumpAccel; }
  float GetHorizontalJumpAccel() const { return xd0_horizontalJumpAccel; }
  float GetVerticalDoubleJumpAccel() const { return xd4_verticalDoubleJumpAccel; }
  float GetHorizontalDoubleJumpAccel() const { return xd8_horizontalDoubleJumpAccel; }
  float GetWaterJumpFactor() const { return xdc_waterJumpFactor; }
  float GetWaterBallJumpFactor() const { return xe0_waterBallJumpFactor; }
  float GetLavaJumpFactor() const { return xe4_lavaJumpFactor; }
  float GetLavaBallJumpFactor() const { return xe8_lavaBallJumpFactor; }
  float GetPhazonJumpFactor() const { return xec_phazonJumpFactor; }
  float GetPhazonBallJumpFactor() const { return xf0_phazonBallJumpFactor; }
  float GetAllowedJumpTime() const { return xf4_allowedJumpTime; }
  float GetAllowedDoubleJumpTime() const { return xf8_allowedDoubleJumpTime; }
  float GetMinDoubleJumpWindow() const { return xfc_minDoubleJumpWindow; }
  float GetMaxDoubleJumpWindow() const { return x100_maxDoubleJumpWindow; }
  float GetMinJumpTime() const { return x108_minJumpTime; }
  float GetMinDoubleJumpTime() const { return x10c_minDoubleJumpTime; }
  float GetAllowedLedgeTime() const { return x110_allowedLedgeTime; }
  float GetDoubleJumpImpulse() const { return x114_doubleJumpImpulse; }
  float GetBackwardsForceMultiplier() const { return x118_backwardsForceMultiplier; }
  float GetBombJumpRadius() const { return x11c_bombJumpRadius; }
  float GetBombJumpHeight() const { return x120_bombJumpHeight; }
  float GetEyeOffset() const { return x124_eyeOffset; }
  float GetTurnSpeedMultiplier() const { return x128_turnSpeedMultiplier; }
  float GetFreeLookTurnSpeedMultiplier() const { return x12c_freeLookTurnSpeedMultiplier; }
  float GetFreeLookSpeed() const { return x138_freeLookSpeed; }
  float GetFreeLookSnapSpeed() const { return x13c_freeLookSnapSpeed; }
  float GetFreeLookCenteredThresholdAngle() const { return x144_freeLookCenteredThresholdAngle; }
  float GetFreeLookCenteredTime() const { return x148_freeLookCenteredTime; }
  float GetOrbitModeTimer() const { return x180_orbitModeTimer; }
  float GetOrbitUpperAngle() const { return x188_orbitUpperAngle; }
  float GetOrbitLowerAngle() const { return x18c_orbitLowerAngle; }
  float GetOrbitHorizAngle() const { return x190_orbitHorizAngle; }
  float GetOrbitMaxTargetDistance() const { return x19c_orbitMaxTargetDistance; }
  float GetOrbitMaxLockDistance() const { return x1a0_orbitMaxLockDistance; }
  float GetOrbitDistanceThreshold() const { return x1a4_orbitDistanceThreshold; }
  uint32_t GetOrbitScreenBoxHalfExtentX(int zone) const { return x1a8_orbitScreenBoxHalfExtentX[zone]; }
  uint32_t GetOrbitScreenBoxHalfExtentY(int zone) const { return x1b0_orbitScreenBoxHalfExtentY[zone]; }
  uint32_t GetOrbitScreenBoxCenterX(int zone) const { return x1b8_orbitScreenBoxCenterX[zone]; }
  uint32_t GetOrbitScreenBoxCenterY(int zone) const { return x1c0_orbitScreenBoxCenterY[zone]; }
  uint32_t GetOrbitZoneIdealX(int zone) const { return x1c8_orbitZoneIdealX[zone]; }
  uint32_t GetOrbitZoneIdealY(int zone) const { return x1d0_orbitZoneIdealY[zone]; }
  float GetOrbitNearX() const { return x1d8_orbitNearX; }
  float GetOrbitNearZ() const { return x1dc_orbitNearZ; }
  float GetOrbitFixedOffsetZDiff() const { return x1e8_orbitFixedOffsetZDiff; }
  float GetOrbitZRange() const { return x1ec_orbitZRange; }
  bool GetDashEnabled() const { return x200_24_dashEnabled; }
  bool GetDashOnButtonRelease() const { return x200_25_dashOnButtonRelease; }
  float GetDashButtonHoldCancelTime() const { return x204_dashButtonHoldCancelTime; }
  float GetDashStrafeInputThreshold() const { return x208_dashStrafeInputThreshold; }
  float GetSidewaysDoubleJumpImpulse() const { return x20c_sidewaysDoubleJumpImpulse; }
  float GetSidewaysVerticalDoubleJumpAccel() const { return x210_sidewaysVerticalDoubleJumpAccel; }
  float GetSidewaysHorizontalDoubleJumpAccel() const { return x214_sidewaysHorizontalDoubleJumpAccel; }
  float GetScanningRange() const { return x218_scanningRange; }
  bool GetScanRetention() const { return x21c_24_scanRetention; }
  bool GetScanFreezesGame() const { return x21c_25_scanFreezesGame; }
  bool GetOrbitWhileScanning() const { return x21c_26_orbitWhileScanning; }
  float GetScanMaxTargetDistance() const { return x220_scanMaxTargetDistance; }
  float GetScanMaxLockDistance() const { return x224_scanMaxLockDistance; }
  bool GetMoveDuringFreeLook() const { return x228_27_moveDuringFreeLook; }
  bool GetHoldButtonsForFreeLook() const { return x228_28_holdButtonsForFreeLook; }
  bool GetTwoButtonsForFreeLook() const { return x228_29_twoButtonsForFreeLook; }
  bool GetAimWhenOrbitingPoint() const { return x229_25_aimWhenOrbitingPoint; }
  bool GetStayInFreeLookWhileFiring() const { return x229_26_stayInFreeLookWhileFiring; }
  bool GetOrbitFixedOffset() const { return x229_29_orbitFixedOffset; }
  bool GetGunButtonTogglesHolster() const { return x229_30_gunButtonTogglesHolster; }
  bool GetGunNotFiringHolstersGun() const { return x229_31_gunNotFiringHolstersGun; }
  bool GetFallingDoubleJump() const { return x22a_24_fallingDoubleJump; }
  bool GetImpulseDoubleJump() const { return x22a_25_impulseDoubleJump; }
  bool GetFiringCancelsCameraPitch() const { return x22a_26_firingCancelsCameraPitch; }
  bool GetAssistedAimingIgnoreHorizontal() const { return x22a_27_assistedAimingIgnoreHorizontal; }
  bool GetAssistedAimingIgnoreVertical() const { return x22a_28_assistedAimingIgnoreVertical; }
  float GetAimMaxDistance() const { return x234_aimMaxDistance; }
  float GetAimThresholdDistance() const { return x24c_aimThresholdDistance; }
  float GetAimBoxWidth() const { return x258_aimBoxWidth; }
  float GetAimBoxHeight() const { return x25c_aimBoxHeight; }
  float GetAimTargetTimer() const { return x260_aimTargetTimer; }
  float GetAimAssistHorizontalAngle() const { return x264_aimAssistHorizontalAngle; }
  float GetAimAssistVerticalAngle() const { return x268_aimAssistVerticalAngle; }
  float GetPlayerHeight() const { return x26c_playerHeight; }
  float GetPlayerXYHalfExtent() const { return x270_playerXYHalfExtent; }
  float GetStepUpHeight() const { return x274_stepUpHeight; }
  float GetStepDownHeight() const { return x278_stepDownHeight; }
  float GetPlayerBallHalfExtent() const { return x27c_playerBallHalfExtent; }
  float GetOrbitDistanceMax() const { return x2a0_orbitDistanceMax; }
  float GetGrappleSwingLength() const { return x2a4_grappleSwingLength; }
  float GetGrappleSwingPeriod() const { return x2a8_grappleSwingPeriod; }
  float GetGrapplePullSpeedMin() const { return x2ac_grapplePullSpeedMin; }
  float GetMaxGrappleLockedTurnAlignDistance() const { return x2b4_maxGrappleLockedTurnAlignDistance; }
  float GetGrapplePullSpeedProportion() const { return x2b8_grapplePullSpeedProportion; }
  float GetGrapplePullSpeedMax() const { return x2bc_grapplePullSpeedMax; }
  float GetGrappleLookCenterSpeed() const { return x2c0_grappleLookCenterSpeed; }
  float GetMaxGrappleTurnSpeed() const { return x2c4_maxGrappleTurnSpeed; }
  float GetGrappleJumpForce() const { return x2c8_grappleJumpForce; }
  float GetGrappleReleaseTime() const { return x2cc_grappleReleaseTime; }
  uint32_t GetGrappleJumpMode() const { return x2d0_grappleJumpMode; }
  bool GetOrbitReleaseBreaksGrapple() const { return x2d4_orbitReleaseBreaksGrapple; }
  bool GetInvertGrappleTurn() const { return x2d5_invertGrappleTurn; }
  float GetGrappleBeamSpeed() const { return x2d8_grappleBeamSpeed; }
  float GetGrappleBeamXWaveAmplitude() const { return x2dc_grappleBeamXWaveAmplitude; }
  float GetGrappleBeamZWaveAmplitude() const { return x2e0_grappleBeamZWaveAmplitude; }
  float GetGrappleBeamAnglePhaseDelta() const { return x2e4_grappleBeamAnglePhaseDelta; }
  float GetHorizontalFreeLookAngleVel() const { return x130_horizontalFreeLookAngleVel; }
  float GetVerticalFreeLookAngleVel() const { return x134_verticalFreeLookAngleVel; }
  float GetOrbitCameraSpeed() const { return x184_orbitCameraSpeed; }
  float GetOrbitPreventionTime() const { return x1fc_orbitPreventionTime; }
  bool GetFreeLookTurnsPlayer() const { return x228_24_freelookTurnsPlayer; }
  float GetJumpCameraPitchDownStart() const { return x288_jumpCameraPitchDownStart; }
  float GetJumpCameraPitchDownFull() const { return x28c_jumpCameraPitchDownFull; }
  float GetJumpCameraPitchDownAngle() const { return x290_jumpCameraPitchDownAngle; }
  float GetFallCameraPitchDownStart() const { return x294_fallCameraPitchDownStart; }
  float GetFallCameraPitchDownFull() const { return x298_fallCameraPitchDownFull; }
  float GetFallCameraPitchDownAngle() const { return x29c_fallCameraPitchDownAngle; }
  float GetFirstPersonCameraSpeed() const { return x280_firstPersonCameraSpeed; }
  float GetGrappleCameraSpeed() const { return x2b0_grappleCameraSpeed; }
  float GetFreeLookDampenFactor() const { return x14c_freeLookDampenFactor; }
  float GetLeftLogicalThreshold() const { return x150_leftDiv; }
  float GetRightLogicalThreshold() const { return x154_rightDiv; }
  float GetOrbitMinDistance(int type) const { return x158_orbitMinDistance[type]; }
  float GetOrbitNormalDistance(int type) const { return x164_orbitNormalDistance[type]; }
  float GetOrbitMaxDistance(int type) const { return x170_orbitMaxDistance[type]; }
  float GetFrozenTimeout() const { return x2f8_frozenTimeout; }
  uint32_t GetIceBreakJumpCount() const { return x2fc_iceBreakJumpCount; }
  float GetVariaDamageReduction() const { return x300_variaDamageReduction; }
  float GetGravityDamageReduction() const { return x304_gravityDamageReduction; }
  float GetPhazonDamageReduction() const { return x308_phazonDamageReduction; }
  CTweakPlayer() = default;
  CTweakPlayer(athena::io::IStreamReader& reader) {
    read(reader);
    FixupValues();
  }

  void FixupValues();
};

} // namespace DataSpec::DNAMP1
