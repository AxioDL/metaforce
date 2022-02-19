#pragma once

#include "Runtime/Tweaks/ITweakPlayer.hpp"

namespace hecl {
class CVar;
}

namespace metaforce::MP1 {

struct CTweakPlayer final : Tweaks::ITweakPlayer {
  float x4_maxTranslationalAcceleration[8]{};
  float x24_maxRotationalAcceleration[8]{};
  float x44_translationFriction[8]{};
  float x64_rotationFriction[8]{};
  float x84_rotationMaxSpeed[8]{};
  float xa4_translationMaxSpeed[8]{};
  float xc4_normalGravAccel{};
  float xc8_fluidGravAccel{};
  float xcc_verticalJumpAccel{};
  float xd0_horizontalJumpAccel{};
  float xd4_verticalDoubleJumpAccel{};
  float xd8_horizontalDoubleJumpAccel{};
  float xdc_waterJumpFactor{};
  float xe0_waterBallJumpFactor{};
  float xe4_lavaJumpFactor{};
  float xe8_lavaBallJumpFactor{};
  float xec_phazonJumpFactor{};
  float xf0_phazonBallJumpFactor{};
  float xf4_allowedJumpTime{};
  float xf8_allowedDoubleJumpTime{};
  float xfc_minDoubleJumpWindow{};
  float x100_maxDoubleJumpWindow{};
  float x104_{};
  float x108_minJumpTime{};
  float x10c_minDoubleJumpTime{};
  float x110_allowedLedgeTime{};
  float x114_doubleJumpImpulse{};
  float x118_backwardsForceMultiplier{};
  float x11c_bombJumpRadius{};
  float x120_bombJumpHeight{};
  float x124_eyeOffset{};
  float x128_turnSpeedMultiplier{};
  float x12c_freeLookTurnSpeedMultiplier{};
  float x130_horizontalFreeLookAngleVel{};
  float x134_verticalFreeLookAngleVel{};
  float x138_freeLookSpeed{};
  float x13c_freeLookSnapSpeed{};
  float x140_{};
  float x144_freeLookCenteredThresholdAngle{};
  float x148_freeLookCenteredTime{};
  float x14c_freeLookDampenFactor{};
  float x150_leftDiv{};
  float x154_rightDiv{};
  float x158_orbitMinDistance[3]{};
  float x164_orbitNormalDistance[3]{};
  float x170_orbitMaxDistance[3]{};
  float x17c_{};
  float x180_orbitModeTimer{};
  float x184_orbitCameraSpeed{};
  float x188_orbitUpperAngle{};
  float x18c_orbitLowerAngle{};
  float x190_orbitHorizAngle{};
  float x194_{};
  float x198_{};
  float x19c_orbitMaxTargetDistance{};
  float x1a0_orbitMaxLockDistance{};
  float x1a4_orbitDistanceThreshold{};
  u32 x1a8_orbitScreenBoxHalfExtentX[2]{};
  u32 x1b0_orbitScreenBoxHalfExtentY[2]{};
  u32 x1b8_orbitScreenBoxCenterX[2]{};
  u32 x1c0_orbitScreenBoxCenterY[2]{};
  u32 x1c8_orbitZoneIdealX[2]{};
  u32 x1d0_orbitZoneIdealY[2]{};
  float x1d8_orbitNearX{};
  float x1dc_orbitNearZ{};
  float x1e0_{};
  float x1e4_{};
  float x1e8_orbitFixedOffsetZDiff{};
  float x1ec_orbitZRange{};
  float x1f0_{};
  float x1f4_{};
  float x1f8_{};
  float x1fc_orbitPreventionTime{};
  bool x200_24_dashEnabled : 1{};
  bool x200_25_dashOnButtonRelease : 1{};
  float x204_dashButtonHoldCancelTime{};
  float x208_dashStrafeInputThreshold{};
  float x20c_sidewaysDoubleJumpImpulse{};
  float x210_sidewaysVerticalDoubleJumpAccel{};
  float x214_sidewaysHorizontalDoubleJumpAccel{};
  float x218_scanningRange{};
  bool x21c_24_scanRetention : 1{};
  bool x21c_25_scanFreezesGame : 1{};
  bool x21c_26_orbitWhileScanning : 1{};
  float x220_scanMaxTargetDistance{};
  float x224_scanMaxLockDistance{};
  bool x228_24_freelookTurnsPlayer : 1{};
  bool x228_25_ : 1{};
  bool x228_26_ : 1{};
  bool x228_27_moveDuringFreeLook : 1{};
  bool x228_28_holdButtonsForFreeLook : 1{};
  bool x228_29_twoButtonsForFreeLook : 1{};
  bool x228_30_ : 1{};
  bool x228_31_ : 1{};
  bool x229_24_ : 1{};
  bool x229_25_aimWhenOrbitingPoint : 1{};
  bool x229_26_stayInFreeLookWhileFiring : 1{};
  bool x229_27_ : 1{};
  bool x229_28_ : 1{};
  bool x229_29_orbitFixedOffset : 1{};
  bool x229_30_gunButtonTogglesHolster : 1{};
  bool x229_31_gunNotFiringHolstersGun : 1{};
  bool x22a_24_fallingDoubleJump : 1{};
  bool x22a_25_impulseDoubleJump : 1{};
  bool x22a_26_firingCancelsCameraPitch : 1{};
  bool x22a_27_assistedAimingIgnoreHorizontal : 1{};
  bool x22a_28_assistedAimingIgnoreVertical : 1{};
  float x22c_{};
  float x230_{};
  float x234_aimMaxDistance{};
  float x238_{};
  float x23c_{};
  float x240_{};
  float x244_{};
  float x248_{};
  float x24c_aimThresholdDistance{};
  float x250_{};
  float x254_{};
  float x258_aimBoxWidth{};
  float x25c_aimBoxHeight{};
  float x260_aimTargetTimer{};
  float x264_aimAssistHorizontalAngle{};
  float x268_aimAssistVerticalAngle{};
  float x26c_playerHeight{};
  float x270_playerXYHalfExtent{};
  float x274_stepUpHeight{};
  float x278_stepDownHeight{};
  float x27c_playerBallHalfExtent{};
  float x280_firstPersonCameraSpeed{};
  float x284_{};
  float x288_jumpCameraPitchDownStart{};
  float x28c_jumpCameraPitchDownFull{};
  float x290_jumpCameraPitchDownAngle{};
  float x294_fallCameraPitchDownStart{};
  float x298_fallCameraPitchDownFull{};
  float x29c_fallCameraPitchDownAngle{};
  float x2a0_orbitDistanceMax{};
  float x2a4_grappleSwingLength{};
  float x2a8_grappleSwingPeriod{};
  float x2ac_grapplePullSpeedMin{};
  float x2b0_grappleCameraSpeed{};
  float x2b4_maxGrappleLockedTurnAlignDistance{};
  float x2b8_grapplePullSpeedProportion{};
  float x2bc_grapplePullSpeedMax{};
  float x2c0_grappleLookCenterSpeed{};
  float x2c4_maxGrappleTurnSpeed{};
  float x2c8_grappleJumpForce{};
  float x2cc_grappleReleaseTime{};
  u32 x2d0_grappleJumpMode{};
  bool x2d4_orbitReleaseBreaksGrapple{};
  bool x2d5_invertGrappleTurn{};
  float x2d8_grappleBeamSpeed{};
  float x2dc_grappleBeamXWaveAmplitude{};
  float x2e0_grappleBeamZWaveAmplitude{};
  float x2e4_grappleBeamAnglePhaseDelta{};
  float x2e8_{};
  float x2ec_{};
  float x2f0_{};
  bool x2f4_{};
  float x2f8_frozenTimeout{};
  u32 x2fc_iceBreakJumpCount{};
  float x300_variaDamageReduction{};
  float x304_gravityDamageReduction{};
  float x308_phazonDamageReduction{};
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
  CTweakPlayer(CInputStream& in);
  void PutTo(COutputStream& out);
  void FixupValues();
  void initCVars(hecl::CVarManager* mgr) override;
  void _tweakListener(hecl::CVar* cv);
};

} // namespace DataSpec::DNAMP1
