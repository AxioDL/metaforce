#include "CTweakPlayer.hpp"
#include "zeus/Math.hpp"

#include <hecl/CVar.hpp>
#include <hecl/CVarManager.hpp>

#define DEFINE_CVAR_GLOBAL(name)                                                                                       \
  constexpr std::string_view sk##name = std::string_view("tweak.player." #name);                                       \
  hecl::CVar* tw_##name = nullptr;

#define CREATE_CVAR(name, help, value, flags)                                                                          \
  tw_##name = mgr->findOrMakeCVar(sk##name, help, value, flags);                                                       \
  if (tw_##name->wasDeserialized()) {                                                                                  \
    tw_##name->toValue(value);                                                                                         \
  }                                                                                                                    \
  tw_##name->addListener([this](hecl::CVar* cv) { _tweakListener(cv); });

#define CREATE_CVAR_BITFIELD(name, help, value, flags)                                                                 \
  {                                                                                                                    \
    bool tmp = value;                                                                                                  \
    CREATE_CVAR(name, help, tmp, flags)                                                                                \
  }

#define UPDATE_CVAR(name, cv, value)                                                                                   \
  if ((cv) == tw_##name) {                                                                                             \
    (cv)->toValue(value);                                                                                              \
    return;                                                                                                            \
  }

#define UPDATE_CVAR_BITFIELD(name, cv, value)                                                                          \
  {                                                                                                                    \
    bool tmp = value;                                                                                                  \
    UPDATE_CVAR(name, cv, tmp)                                                                                         \
    (value) = tmp;                                                                                                     \
  }

namespace DataSpec::DNAMP1 {
namespace {
static constexpr hecl::CVar::EFlags skDefaultFlags =
    hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Cheat | hecl::CVar::EFlags::Archive;
DEFINE_CVAR_GLOBAL(MaxTranslationAccelerationNormal);
DEFINE_CVAR_GLOBAL(MaxTranslationAccelerationAir);
DEFINE_CVAR_GLOBAL(MaxTranslationAccelerationIce);
DEFINE_CVAR_GLOBAL(MaxTranslationAccelerationOrganic);
DEFINE_CVAR_GLOBAL(MaxTranslationAccelerationWater);
DEFINE_CVAR_GLOBAL(MaxTranslationAccelerationLava);
DEFINE_CVAR_GLOBAL(MaxTranslationAccelerationPhazon);
DEFINE_CVAR_GLOBAL(MaxTranslationAccelerationShrubbery);
DEFINE_CVAR_GLOBAL(MaxRotationAccelerationNormal);
DEFINE_CVAR_GLOBAL(MaxRotationAccelerationAir);
DEFINE_CVAR_GLOBAL(MaxRotationAccelerationIce);
DEFINE_CVAR_GLOBAL(MaxRotationAccelerationOrganic);
DEFINE_CVAR_GLOBAL(MaxRotationAccelerationWater);
DEFINE_CVAR_GLOBAL(MaxRotationAccelerationLava);
DEFINE_CVAR_GLOBAL(MaxRotationAccelerationPhazon);
DEFINE_CVAR_GLOBAL(MaxRotationAccelerationShrubbery);
DEFINE_CVAR_GLOBAL(TranslationFrictionNormal);
DEFINE_CVAR_GLOBAL(TranslationFrictionAir);
DEFINE_CVAR_GLOBAL(TranslationFrictionIce);
DEFINE_CVAR_GLOBAL(TranslationFrictionOrganic);
DEFINE_CVAR_GLOBAL(TranslationFrictionWater);
DEFINE_CVAR_GLOBAL(TranslationFrictionLava);
DEFINE_CVAR_GLOBAL(TranslationFrictionPhazon);
DEFINE_CVAR_GLOBAL(TranslationFrictionShrubbery);
DEFINE_CVAR_GLOBAL(RotationFrictionNormal);
DEFINE_CVAR_GLOBAL(RotationFrictionAir);
DEFINE_CVAR_GLOBAL(RotationFrictionIce);
DEFINE_CVAR_GLOBAL(RotationFrictionOrganic);
DEFINE_CVAR_GLOBAL(RotationFrictionWater);
DEFINE_CVAR_GLOBAL(RotationFrictionLava);
DEFINE_CVAR_GLOBAL(RotationFrictionPhazon);
DEFINE_CVAR_GLOBAL(RotationFrictionShrubbery);
DEFINE_CVAR_GLOBAL(RotationMaxSpeedNormal);
DEFINE_CVAR_GLOBAL(RotationMaxSpeedAir);
DEFINE_CVAR_GLOBAL(RotationMaxSpeedIce);
DEFINE_CVAR_GLOBAL(RotationMaxSpeedOrganic);
DEFINE_CVAR_GLOBAL(RotationMaxSpeedWater);
DEFINE_CVAR_GLOBAL(RotationMaxSpeedLava);
DEFINE_CVAR_GLOBAL(RotationMaxSpeedPhazon);
DEFINE_CVAR_GLOBAL(RotationMaxSpeedShrubbery);
DEFINE_CVAR_GLOBAL(TranslationMaxSpeedNormal);
DEFINE_CVAR_GLOBAL(TranslationMaxSpeedAir);
DEFINE_CVAR_GLOBAL(TranslationMaxSpeedIce);
DEFINE_CVAR_GLOBAL(TranslationMaxSpeedOrganic);
DEFINE_CVAR_GLOBAL(TranslationMaxSpeedWater);
DEFINE_CVAR_GLOBAL(TranslationMaxSpeedLava);
DEFINE_CVAR_GLOBAL(TranslationMaxSpeedPhazon);
DEFINE_CVAR_GLOBAL(TranslationMaxSpeedShrubbery);
DEFINE_CVAR_GLOBAL(NormalGravityAcceleration);
DEFINE_CVAR_GLOBAL(FluidGravityAcceleration);
DEFINE_CVAR_GLOBAL(VerticalJumpAcceleration);
DEFINE_CVAR_GLOBAL(HorizontalJumpAcceleration);
DEFINE_CVAR_GLOBAL(VerticalDoubleJumpAcceleration);
DEFINE_CVAR_GLOBAL(HorizontalDoubleJumpAcceleration);
DEFINE_CVAR_GLOBAL(WaterJumpFactor);
DEFINE_CVAR_GLOBAL(WaterBallJumpFactor);
DEFINE_CVAR_GLOBAL(LavaJumpFactor);
DEFINE_CVAR_GLOBAL(LavaBallJumpFactor);
DEFINE_CVAR_GLOBAL(PhazonJumpFactor);
DEFINE_CVAR_GLOBAL(PhazonBallJumpFactor);
DEFINE_CVAR_GLOBAL(AllowedJumpTime);
DEFINE_CVAR_GLOBAL(AllowedDoubleJumpTime);
DEFINE_CVAR_GLOBAL(MinDoubleJumpWindow);
DEFINE_CVAR_GLOBAL(MaxDoubleJumpWindow)
// DEFINE_CVAR_GLOBAL(); // x104_
DEFINE_CVAR_GLOBAL(MinJumpTime);
DEFINE_CVAR_GLOBAL(MinDoubleJumpTime);
DEFINE_CVAR_GLOBAL(AllowedLedgeTime);
DEFINE_CVAR_GLOBAL(DoubleJumpImpulse);
DEFINE_CVAR_GLOBAL(BackwardsForceMultiplier);
DEFINE_CVAR_GLOBAL(BombJumpRadius);
DEFINE_CVAR_GLOBAL(BombJumpHeight);
DEFINE_CVAR_GLOBAL(EyeOffset);
DEFINE_CVAR_GLOBAL(TurnSpeedMultiplier);
DEFINE_CVAR_GLOBAL(FreeLookTurnSpeedMultiplier);
DEFINE_CVAR_GLOBAL(HorizontalFreeLookAngleVelocity);
DEFINE_CVAR_GLOBAL(VerticalFreeLookAngleVelocity);
DEFINE_CVAR_GLOBAL(FreeLookSpeed);
DEFINE_CVAR_GLOBAL(FreeLookSnapSpeed);
// DEFINE_CVAR_GLOBAL(); // x140_
DEFINE_CVAR_GLOBAL(FreeLookCenteredThresholdAngle);
DEFINE_CVAR_GLOBAL(FreeLookCenteredTime);
DEFINE_CVAR_GLOBAL(FreeLookDampenFactor);
DEFINE_CVAR_GLOBAL(LeftDivisor);
DEFINE_CVAR_GLOBAL(RightDivisor);
DEFINE_CVAR_GLOBAL(OrbitMinDistanceClose);
DEFINE_CVAR_GLOBAL(OrbitMinDistanceFar);
DEFINE_CVAR_GLOBAL(OrbitMinDistanceDefault);
DEFINE_CVAR_GLOBAL(OrbitNormalDistanceClose);
DEFINE_CVAR_GLOBAL(OrbitNormalDistanceFar);
DEFINE_CVAR_GLOBAL(OrbitNormalDistanceDefault);
DEFINE_CVAR_GLOBAL(OrbitMaxDistanceClose);
DEFINE_CVAR_GLOBAL(OrbitMaxDistanceFar);
DEFINE_CVAR_GLOBAL(OrbitMaxDistanceDefault);
// DEFINE_CVAR_GLOBAL(); // x17c_
DEFINE_CVAR_GLOBAL(OrbitmodeTimer);
DEFINE_CVAR_GLOBAL(OrbitCameraSpeed);
DEFINE_CVAR_GLOBAL(OrbitUpperAngle);
DEFINE_CVAR_GLOBAL(OrbitLowerAngle);
DEFINE_CVAR_GLOBAL(OrbitHorizontalAngle);
// DEFINE_CVAR_GLOBAL(); // x194_
// DEFINE_CVAR_GLOBAL(); // x198_
DEFINE_CVAR_GLOBAL(OrbitMaxTargetDistance);
DEFINE_CVAR_GLOBAL(OrbitMaxLockDistance);
DEFINE_CVAR_GLOBAL(OrbitDistanceThreshold);
DEFINE_CVAR_GLOBAL(OrbitScreenTargetingBoxHalfExtentX);
DEFINE_CVAR_GLOBAL(OrbitScreenScanBoxHalfExtentX);
DEFINE_CVAR_GLOBAL(OrbitScreenTargetingBoxHalfExtentY);
DEFINE_CVAR_GLOBAL(OrbitScreenScanBoxHalfExtentY);
DEFINE_CVAR_GLOBAL(OrbitScreenTargetingBoxCenterX);
DEFINE_CVAR_GLOBAL(OrbitScreenScanBoxCenterX);
DEFINE_CVAR_GLOBAL(OrbitScreenTargetingBoxCenterY);
DEFINE_CVAR_GLOBAL(OrbitScreenScanBoxCenterY);
DEFINE_CVAR_GLOBAL(OrbitZoneTargetingIdealX);
DEFINE_CVAR_GLOBAL(OrbitZoneScanIdealX);
DEFINE_CVAR_GLOBAL(OrbitZoneTargetingIdealY);
DEFINE_CVAR_GLOBAL(OrbitZoneScanIdealY);
DEFINE_CVAR_GLOBAL(OrbitNearX);
DEFINE_CVAR_GLOBAL(OrbitNearZ);
// DEFINE_CVAR_GLOBAL(); // x1e0_
// DEFINE_CVAR_GLOBAL(); // x1e4_
DEFINE_CVAR_GLOBAL(OrbitFixedOffsetZDiff);
DEFINE_CVAR_GLOBAL(OrbitZRange);
// DEFINE_CVAR_GLOBAL(); // x1f0_
// DEFINE_CVAR_GLOBAL(); // x1f4_
// DEFINE_CVAR_GLOBAL(); // x1f8_
DEFINE_CVAR_GLOBAL(OrbitPreventionTime);
DEFINE_CVAR_GLOBAL(DashEnabled);
DEFINE_CVAR_GLOBAL(DashOnButtonRelease);
DEFINE_CVAR_GLOBAL(DashButtonHoldCancelTime);
DEFINE_CVAR_GLOBAL(DashStrafeInputThreshold);
DEFINE_CVAR_GLOBAL(SidewaysDoubleJumpImpulse);
DEFINE_CVAR_GLOBAL(SidewaysVerticalDoubleJumpAccel);
DEFINE_CVAR_GLOBAL(SidewaysHorizontalDoubleJumpAccel);
DEFINE_CVAR_GLOBAL(ScanningRange);
DEFINE_CVAR_GLOBAL(ScanRetention);
DEFINE_CVAR_GLOBAL(ScanFreezesGame);
DEFINE_CVAR_GLOBAL(OrbitWhileScanning);
DEFINE_CVAR_GLOBAL(ScanMaxTargetDistance);
DEFINE_CVAR_GLOBAL(ScanMaxLockDistance)
DEFINE_CVAR_GLOBAL(FreeLookTurnsPlayer);
// DEFINE_CVAR_GLOBAL(); // x228_25_
// DEFINE_CVAR_GLOBAL(); // x228_26_
DEFINE_CVAR_GLOBAL(MoveDuringFreelook);
DEFINE_CVAR_GLOBAL(HoldButtonsForFreeLook);
// DEFINE_CVAR_GLOBAL(); // x228_30_
// DEFINE_CVAR_GLOBAL(); // x228_31_
// DEFINE_CVAR_GLOBAL(); // x229_24_
DEFINE_CVAR_GLOBAL(AimWhenOrbitingPoint);
DEFINE_CVAR_GLOBAL(StayInFreeLookWhileFiring);
// DEFINE_CVAR_GLOBAL(); // x229_27_
// DEFINE_CVAR_GLOBAL(); // x229_28_
DEFINE_CVAR_GLOBAL(OrbitFixedOffset);
DEFINE_CVAR_GLOBAL(GunButtonTogglesHolster);
DEFINE_CVAR_GLOBAL(GunNotFiringHolstersGun);
DEFINE_CVAR_GLOBAL(FallingDoubleJump);
DEFINE_CVAR_GLOBAL(ImpulseDoubleJump);
DEFINE_CVAR_GLOBAL(FiringCancelsCameraPitch);
DEFINE_CVAR_GLOBAL(AssistedAimingIgnoreHorizontal);
DEFINE_CVAR_GLOBAL(AssistedAimingIgnoreVertical);
// DEFINE_CVAR_GLOBAL(); // x22c
// DEFINE_CVAR_GLOBAL(); // x230_
DEFINE_CVAR_GLOBAL(AimMaxDistance);
// DEFINE_CVAR_GLOBAL(); // x238_
// DEFINE_CVAR_GLOBAL(); // x23c_
// DEFINE_CVAR_GLOBAL(); // x240_
// DEFINE_CVAR_GLOBAL(); // x244_
// DEFINE_CVAR_GLOBAL(); // x248_
DEFINE_CVAR_GLOBAL(AimThresholdDistance);
// DEFINE_CVAR_GLOBAL(); // x250_
// DEFINE_CVAR_GLOBAL(); // x254_
DEFINE_CVAR_GLOBAL(AimBoxWidth);
DEFINE_CVAR_GLOBAL(AimBoxHeight);
DEFINE_CVAR_GLOBAL(AimTargetTimer);
DEFINE_CVAR_GLOBAL(AimAssistHorizontalAngle);
DEFINE_CVAR_GLOBAL(AimAssistVerticalAngle);
DEFINE_CVAR_GLOBAL(PlayerHeight);
DEFINE_CVAR_GLOBAL(PlayerXYHalfExtent);
DEFINE_CVAR_GLOBAL(StepUpHeight);
DEFINE_CVAR_GLOBAL(StepDownHeight);
DEFINE_CVAR_GLOBAL(PlayerBallHalfExtent);
DEFINE_CVAR_GLOBAL(FirstPersonCameraSpeed);
// DEFINE_CVAR_GLOBAL(); // x284_
DEFINE_CVAR_GLOBAL(JumpCameraPitchDownStart);
DEFINE_CVAR_GLOBAL(JumpCameraPitchDownFull);
DEFINE_CVAR_GLOBAL(JumpCameraPitchDownAngle);
DEFINE_CVAR_GLOBAL(FallCameraPitchDownStart);
DEFINE_CVAR_GLOBAL(FallCameraPitchDownFull);
DEFINE_CVAR_GLOBAL(FallCameraPitchDownAngle);
DEFINE_CVAR_GLOBAL(OrbitDistanceMax);
DEFINE_CVAR_GLOBAL(GrappleSwingLength);
DEFINE_CVAR_GLOBAL(GrappleSwingPeriod);
DEFINE_CVAR_GLOBAL(GrapplePullSpeedMin);
DEFINE_CVAR_GLOBAL(GrappleCameraSpeed);
DEFINE_CVAR_GLOBAL(MaxGrappleLockedTurnAlignDistance);
DEFINE_CVAR_GLOBAL(GrapplePullSpeedProportion);
DEFINE_CVAR_GLOBAL(GrapplePullSpeedMax);
DEFINE_CVAR_GLOBAL(GrappleLookCenterSpeed);
DEFINE_CVAR_GLOBAL(MaxGrappleTurnSpeed);
DEFINE_CVAR_GLOBAL(GrappleJumpForce);
DEFINE_CVAR_GLOBAL(GrappleReleaseTime);
DEFINE_CVAR_GLOBAL(GrappleJumpMode);
DEFINE_CVAR_GLOBAL(OrbitReleaseBreaksGrapple);
DEFINE_CVAR_GLOBAL(InvertGrappleTurn);
DEFINE_CVAR_GLOBAL(GrappleBeamSpeed);
DEFINE_CVAR_GLOBAL(GrappleBeamXWaveAmplitude);
DEFINE_CVAR_GLOBAL(GrappleBeamZWaveAmplitude);
DEFINE_CVAR_GLOBAL(GrappleBeamAnglePhaseDelta);
// DEFINE_CVAR_GLOBAL(); // x2e8_
// DEFINE_CVAR_GLOBAL(); // x2ec_
// DEFINE_CVAR_GLOBAL(); // x2f0_
// DEFINE_CVAR_GLOBAL(); // x2f4_
DEFINE_CVAR_GLOBAL(FrozenTimeout);
DEFINE_CVAR_GLOBAL(IceBreakJumpCount);
DEFINE_CVAR_GLOBAL(VariaDamageReduction);
DEFINE_CVAR_GLOBAL(GravityDamageReduction);
DEFINE_CVAR_GLOBAL(PhazonDamageReduction);
} // namespace
template <>
void CTweakPlayer::Enumerate<BigDNA::Read>(athena::io::IStreamReader& __dna_reader) {
  /* x4_maxTranslationalAcceleration[0] */
  x4_maxTranslationalAcceleration[0] = __dna_reader.readFloatBig();
  /* x4_maxTranslationalAcceleration[1] */
  x4_maxTranslationalAcceleration[1] = __dna_reader.readFloatBig();
  /* x4_maxTranslationalAcceleration[2] */
  x4_maxTranslationalAcceleration[2] = __dna_reader.readFloatBig();
  /* x4_maxTranslationalAcceleration[3] */
  x4_maxTranslationalAcceleration[3] = __dna_reader.readFloatBig();
  /* x4_maxTranslationalAcceleration[4] */
  x4_maxTranslationalAcceleration[4] = __dna_reader.readFloatBig();
  /* x4_maxTranslationalAcceleration[5] */
  x4_maxTranslationalAcceleration[5] = __dna_reader.readFloatBig();
  /* x4_maxTranslationalAcceleration[6] */
  x4_maxTranslationalAcceleration[6] = __dna_reader.readFloatBig();
  /* x4_maxTranslationalAcceleration[7] */
  x4_maxTranslationalAcceleration[7] = __dna_reader.readFloatBig();
  /* x24_maxRotationalAcceleration[0] */
  x24_maxRotationalAcceleration[0] = __dna_reader.readFloatBig();
  /* x24_maxRotationalAcceleration[1] */
  x24_maxRotationalAcceleration[1] = __dna_reader.readFloatBig();
  /* x24_maxRotationalAcceleration[2] */
  x24_maxRotationalAcceleration[2] = __dna_reader.readFloatBig();
  /* x24_maxRotationalAcceleration[3] */
  x24_maxRotationalAcceleration[3] = __dna_reader.readFloatBig();
  /* x24_maxRotationalAcceleration[4] */
  x24_maxRotationalAcceleration[4] = __dna_reader.readFloatBig();
  /* x24_maxRotationalAcceleration[5] */
  x24_maxRotationalAcceleration[5] = __dna_reader.readFloatBig();
  /* x24_maxRotationalAcceleration[6] */
  x24_maxRotationalAcceleration[6] = __dna_reader.readFloatBig();
  /* x24_maxRotationalAcceleration[7] */
  x24_maxRotationalAcceleration[7] = __dna_reader.readFloatBig();
  /* x44_translationFriction[0] */
  x44_translationFriction[0] = __dna_reader.readFloatBig();
  /* x44_translationFriction[1] */
  x44_translationFriction[1] = __dna_reader.readFloatBig();
  /* x44_translationFriction[2] */
  x44_translationFriction[2] = __dna_reader.readFloatBig();
  /* x44_translationFriction[3] */
  x44_translationFriction[3] = __dna_reader.readFloatBig();
  /* x44_translationFriction[4] */
  x44_translationFriction[4] = __dna_reader.readFloatBig();
  /* x44_translationFriction[5] */
  x44_translationFriction[5] = __dna_reader.readFloatBig();
  /* x44_translationFriction[6] */
  x44_translationFriction[6] = __dna_reader.readFloatBig();
  /* x44_translationFriction[7] */
  x44_translationFriction[7] = __dna_reader.readFloatBig();
  /* x64_rotationFriction[0] */
  x64_rotationFriction[0] = __dna_reader.readFloatBig();
  /* x64_rotationFriction[1] */
  x64_rotationFriction[1] = __dna_reader.readFloatBig();
  /* x64_rotationFriction[2] */
  x64_rotationFriction[2] = __dna_reader.readFloatBig();
  /* x64_rotationFriction[3] */
  x64_rotationFriction[3] = __dna_reader.readFloatBig();
  /* x64_rotationFriction[4] */
  x64_rotationFriction[4] = __dna_reader.readFloatBig();
  /* x64_rotationFriction[5] */
  x64_rotationFriction[5] = __dna_reader.readFloatBig();
  /* x64_rotationFriction[6] */
  x64_rotationFriction[6] = __dna_reader.readFloatBig();
  /* x64_rotationFriction[7] */
  x64_rotationFriction[7] = __dna_reader.readFloatBig();
  /* x84_rotationMaxSpeed[0] */
  x84_rotationMaxSpeed[0] = __dna_reader.readFloatBig();
  /* x84_rotationMaxSpeed[1] */
  x84_rotationMaxSpeed[1] = __dna_reader.readFloatBig();
  /* x84_rotationMaxSpeed[2] */
  x84_rotationMaxSpeed[2] = __dna_reader.readFloatBig();
  /* x84_rotationMaxSpeed[3] */
  x84_rotationMaxSpeed[3] = __dna_reader.readFloatBig();
  /* x84_rotationMaxSpeed[4] */
  x84_rotationMaxSpeed[4] = __dna_reader.readFloatBig();
  /* x84_rotationMaxSpeed[5] */
  x84_rotationMaxSpeed[5] = __dna_reader.readFloatBig();
  /* x84_rotationMaxSpeed[6] */
  x84_rotationMaxSpeed[6] = __dna_reader.readFloatBig();
  /* x84_rotationMaxSpeed[7] */
  x84_rotationMaxSpeed[7] = __dna_reader.readFloatBig();
  /* xa4_translationMaxSpeed[0] */
  xa4_translationMaxSpeed[0] = __dna_reader.readFloatBig();
  /* xa4_translationMaxSpeed[1] */
  xa4_translationMaxSpeed[1] = __dna_reader.readFloatBig();
  /* xa4_translationMaxSpeed[2] */
  xa4_translationMaxSpeed[2] = __dna_reader.readFloatBig();
  /* xa4_translationMaxSpeed[3] */
  xa4_translationMaxSpeed[3] = __dna_reader.readFloatBig();
  /* xa4_translationMaxSpeed[4] */
  xa4_translationMaxSpeed[4] = __dna_reader.readFloatBig();
  /* xa4_translationMaxSpeed[5] */
  xa4_translationMaxSpeed[5] = __dna_reader.readFloatBig();
  /* xa4_translationMaxSpeed[6] */
  xa4_translationMaxSpeed[6] = __dna_reader.readFloatBig();
  /* xa4_translationMaxSpeed[7] */
  xa4_translationMaxSpeed[7] = __dna_reader.readFloatBig();
  /* xc4_normalGravAccel */
  xc4_normalGravAccel = __dna_reader.readFloatBig();
  /* xc8_fluidGravAccel */
  xc8_fluidGravAccel = __dna_reader.readFloatBig();
  /* xcc_verticalJumpAccel */
  xcc_verticalJumpAccel = __dna_reader.readFloatBig();
  /* xd0_horizontalJumpAccel */
  xd0_horizontalJumpAccel = __dna_reader.readFloatBig();
  /* xd4_verticalDoubleJumpAccel */
  xd4_verticalDoubleJumpAccel = __dna_reader.readFloatBig();
  /* xd8_horizontalDoubleJumpAccel */
  xd8_horizontalDoubleJumpAccel = __dna_reader.readFloatBig();
  /* xdc_waterJumpFactor */
  xdc_waterJumpFactor = __dna_reader.readFloatBig();
  /* xe0_waterBallJumpFactor */
  xe0_waterBallJumpFactor = __dna_reader.readFloatBig();
  /* xe4_lavaJumpFactor */
  xe4_lavaJumpFactor = __dna_reader.readFloatBig();
  /* xe8_lavaBallJumpFactor */
  xe8_lavaBallJumpFactor = __dna_reader.readFloatBig();
  /* xec_phazonJumpFactor */
  xec_phazonJumpFactor = __dna_reader.readFloatBig();
  /* xf0_phazonBallJumpFactor */
  xf0_phazonBallJumpFactor = __dna_reader.readFloatBig();
  /* xf4_allowedJumpTime */
  xf4_allowedJumpTime = __dna_reader.readFloatBig();
  /* xf8_allowedDoubleJumpTime */
  xf8_allowedDoubleJumpTime = __dna_reader.readFloatBig();
  /* xfc_minDoubleJumpWindow */
  xfc_minDoubleJumpWindow = __dna_reader.readFloatBig();
  /* x100_maxDoubleJumpWindow */
  x100_maxDoubleJumpWindow = __dna_reader.readFloatBig();
  /* x104_ */
  x104_ = __dna_reader.readFloatBig();
  /* x108_minJumpTime */
  x108_minJumpTime = __dna_reader.readFloatBig();
  /* x10c_minDoubleJumpTime */
  x10c_minDoubleJumpTime = __dna_reader.readFloatBig();
  /* x110_allowedLedgeTime */
  x110_allowedLedgeTime = __dna_reader.readFloatBig();
  /* x114_doubleJumpImpulse */
  x114_doubleJumpImpulse = __dna_reader.readFloatBig();
  /* x118_backwardsForceMultiplier */
  x118_backwardsForceMultiplier = __dna_reader.readFloatBig();
  /* x11c_bombJumpRadius */
  x11c_bombJumpRadius = __dna_reader.readFloatBig();
  /* x120_bombJumpHeight */
  x120_bombJumpHeight = __dna_reader.readFloatBig();
  /* x124_eyeOffset */
  x124_eyeOffset = __dna_reader.readFloatBig();
  /* x128_turnSpeedMultiplier */
  x128_turnSpeedMultiplier = __dna_reader.readFloatBig();
  /* x12c_freeLookTurnSpeedMultiplier */
  x12c_freeLookTurnSpeedMultiplier = __dna_reader.readFloatBig();
  /* x130_horizontalFreeLookAngleVel */
  x130_horizontalFreeLookAngleVel = __dna_reader.readFloatBig();
  /* x134_verticalFreeLookAngleVel */
  x134_verticalFreeLookAngleVel = __dna_reader.readFloatBig();
  /* x138_freeLookSpeed */
  x138_freeLookSpeed = __dna_reader.readFloatBig();
  /* x13c_freeLookSnapSpeed */
  x13c_freeLookSnapSpeed = __dna_reader.readFloatBig();
  /* x140_ */
  x140_ = __dna_reader.readFloatBig();
  /* x144_freeLookCenteredThresholdAngle */
  x144_freeLookCenteredThresholdAngle = __dna_reader.readFloatBig();
  /* x148_freeLookCenteredTime */
  x148_freeLookCenteredTime = __dna_reader.readFloatBig();
  /* x14c_freeLookDampenFactor */
  x14c_freeLookDampenFactor = __dna_reader.readFloatBig();
  /* x150_leftDiv */
  x150_leftDiv = __dna_reader.readFloatBig();
  /* x154_rightDiv */
  x154_rightDiv = __dna_reader.readFloatBig();
  /* x228_24_freelookTurnsPlayer */
  x228_24_freelookTurnsPlayer = __dna_reader.readBool();
  /* x228_25_ */
  x228_25_ = __dna_reader.readBool();
  /* x228_26_ */
  x228_26_ = __dna_reader.readBool();
  /* x228_27_moveDuringFreeLook */
  x228_27_moveDuringFreeLook = __dna_reader.readBool();
  /* x228_28_holdButtonsForFreeLook */
  x228_28_holdButtonsForFreeLook = __dna_reader.readBool();
  /* x228_29_twoButtonsForFreeLook */
  x228_29_twoButtonsForFreeLook = __dna_reader.readBool();
  /* x228_30_ */
  x228_30_ = __dna_reader.readBool();
  /* x228_31_ */
  x228_31_ = __dna_reader.readBool();
  /* x229_24_ */
  x229_24_ = __dna_reader.readBool();
  /* x229_25_aimWhenOrbitingPoint */
  x229_25_aimWhenOrbitingPoint = __dna_reader.readBool();
  /* x229_26_stayInFreeLookWhileFiring */
  x229_26_stayInFreeLookWhileFiring = __dna_reader.readBool();
  /* x229_27_ */
  x229_27_ = __dna_reader.readBool();
  /* x229_28_ */
  x229_28_ = __dna_reader.readBool();
  /* x229_29_orbitFixedOffset */
  x229_29_orbitFixedOffset = __dna_reader.readBool();
  /* x229_30_gunButtonTogglesHolster */
  x229_30_gunButtonTogglesHolster = __dna_reader.readBool();
  /* x229_31_gunNotFiringHolstersGun */
  x229_31_gunNotFiringHolstersGun = __dna_reader.readBool();
  /* x22a_24_fallingDoubleJump */
  x22a_24_fallingDoubleJump = __dna_reader.readBool();
  /* x22a_25_impulseDoubleJump */
  x22a_25_impulseDoubleJump = __dna_reader.readBool();
  /* x22a_26_firingCancelsCameraPitch */
  x22a_26_firingCancelsCameraPitch = __dna_reader.readBool();
  /* x22a_27_assistedAimingIgnoreHorizontal */
  x22a_27_assistedAimingIgnoreHorizontal = __dna_reader.readBool();
  /* x22a_28_assistedAimingIgnoreVertical */
  x22a_28_assistedAimingIgnoreVertical = __dna_reader.readBool();
  /* x22c_ */
  x22c_ = __dna_reader.readFloatBig();
  /* x230_ */
  x230_ = __dna_reader.readFloatBig();
  /* x234_aimMaxDistance */
  x234_aimMaxDistance = __dna_reader.readFloatBig();
  /* x238_ */
  x238_ = __dna_reader.readFloatBig();
  /* x23c_ */
  x23c_ = __dna_reader.readFloatBig();
  /* x240_ */
  x240_ = __dna_reader.readFloatBig();
  /* x244_ */
  x244_ = __dna_reader.readFloatBig();
  /* x248_ */
  x248_ = __dna_reader.readFloatBig();
  /* x24c_aimThresholdDistance */
  x24c_aimThresholdDistance = __dna_reader.readFloatBig();
  /* x250_ */
  x250_ = __dna_reader.readFloatBig();
  /* x254_ */
  x254_ = __dna_reader.readFloatBig();
  /* x258_aimBoxWidth */
  x258_aimBoxWidth = __dna_reader.readFloatBig();
  /* x25c_aimBoxHeight */
  x25c_aimBoxHeight = __dna_reader.readFloatBig();
  /* x260_aimTargetTimer */
  x260_aimTargetTimer = __dna_reader.readFloatBig();
  /* x264_aimAssistHorizontalAngle */
  x264_aimAssistHorizontalAngle = __dna_reader.readFloatBig();
  /* x268_aimAssistVerticalAngle */
  x268_aimAssistVerticalAngle = __dna_reader.readFloatBig();
  /* x158_orbitMinDistance[0] */
  x158_orbitMinDistance[0] = __dna_reader.readFloatBig();
  /* x164_orbitNormalDistance[0] */
  x164_orbitNormalDistance[0] = __dna_reader.readFloatBig();
  /* x170_orbitMaxDistance[0] */
  x170_orbitMaxDistance[0] = __dna_reader.readFloatBig();
  /* x158_orbitMinDistance[1] */
  x158_orbitMinDistance[1] = __dna_reader.readFloatBig();
  /* x164_orbitNormalDistance[1] */
  x164_orbitNormalDistance[1] = __dna_reader.readFloatBig();
  /* x170_orbitMaxDistance[1] */
  x170_orbitMaxDistance[1] = __dna_reader.readFloatBig();
  /* x158_orbitMinDistance[2] */
  x158_orbitMinDistance[2] = __dna_reader.readFloatBig();
  /* x164_orbitNormalDistance[2] */
  x164_orbitNormalDistance[2] = __dna_reader.readFloatBig();
  /* x170_orbitMaxDistance[2] */
  x170_orbitMaxDistance[2] = __dna_reader.readFloatBig();
  /* x17c_ */
  x17c_ = __dna_reader.readFloatBig();
  /* x180_orbitModeTimer */
  x180_orbitModeTimer = __dna_reader.readFloatBig();
  /* x184_orbitCameraSpeed */
  x184_orbitCameraSpeed = __dna_reader.readFloatBig();
  /* x188_orbitUpperAngle */
  x188_orbitUpperAngle = __dna_reader.readFloatBig();
  /* x18c_orbitLowerAngle */
  x18c_orbitLowerAngle = __dna_reader.readFloatBig();
  /* x190_orbitHorizAngle */
  x190_orbitHorizAngle = __dna_reader.readFloatBig();
  /* x194_ */
  x194_ = __dna_reader.readFloatBig();
  /* x198_ */
  x198_ = __dna_reader.readFloatBig();
  /* x19c_orbitMaxTargetDistance */
  x19c_orbitMaxTargetDistance = __dna_reader.readFloatBig();
  /* x1a0_orbitMaxLockDistance */
  x1a0_orbitMaxLockDistance = __dna_reader.readFloatBig();
  /* x1a4_orbitDistanceThreshold */
  x1a4_orbitDistanceThreshold = __dna_reader.readFloatBig();
  /* x1a8_orbitScreenBoxHalfExtentX[0] */
  x1a8_orbitScreenBoxHalfExtentX[0] = __dna_reader.readUint32Big();
  /* x1b0_orbitScreenBoxHalfExtentY[0] */
  x1b0_orbitScreenBoxHalfExtentY[0] = __dna_reader.readUint32Big();
  /* x1b8_orbitScreenBoxCenterX[0] */
  x1b8_orbitScreenBoxCenterX[0] = __dna_reader.readUint32Big();
  /* x1c0_orbitScreenBoxCenterY[0] */
  x1c0_orbitScreenBoxCenterY[0] = __dna_reader.readUint32Big();
  /* x1c8_orbitZoneIdealX[0] */
  x1c8_orbitZoneIdealX[0] = __dna_reader.readUint32Big();
  /* x1d0_orbitZoneIdealY[0] */
  x1d0_orbitZoneIdealY[0] = __dna_reader.readUint32Big();
  /* x1a8_orbitScreenBoxHalfExtentX[1] */
  x1a8_orbitScreenBoxHalfExtentX[1] = __dna_reader.readUint32Big();
  /* x1b0_orbitScreenBoxHalfExtentY[1] */
  x1b0_orbitScreenBoxHalfExtentY[1] = __dna_reader.readUint32Big();
  /* x1b8_orbitScreenBoxCenterX[1] */
  x1b8_orbitScreenBoxCenterX[1] = __dna_reader.readUint32Big();
  /* x1c0_orbitScreenBoxCenterY[1] */
  x1c0_orbitScreenBoxCenterY[1] = __dna_reader.readUint32Big();
  /* x1c8_orbitZoneIdealX[1] */
  x1c8_orbitZoneIdealX[1] = __dna_reader.readUint32Big();
  /* x1d0_orbitZoneIdealY[1] */
  x1d0_orbitZoneIdealY[1] = __dna_reader.readUint32Big();
  /* x1d8_orbitNearX */
  x1d8_orbitNearX = __dna_reader.readFloatBig();
  /* x1dc_orbitNearZ */
  x1dc_orbitNearZ = __dna_reader.readFloatBig();
  /* x1e0_ */
  x1e0_ = __dna_reader.readFloatBig();
  /* x1e4_ */
  x1e4_ = __dna_reader.readFloatBig();
  /* x1e8_orbitFixedOffsetZDiff */
  x1e8_orbitFixedOffsetZDiff = __dna_reader.readFloatBig();
  /* x1ec_orbitZRange */
  x1ec_orbitZRange = __dna_reader.readFloatBig();
  /* x1f0_ */
  x1f0_ = __dna_reader.readFloatBig();
  /* x1f4_ */
  x1f4_ = __dna_reader.readFloatBig();
  /* x1f8_ */
  x1f8_ = __dna_reader.readFloatBig();
  /* x1fc_orbitPreventionTime */
  x1fc_orbitPreventionTime = __dna_reader.readFloatBig();
  /* x200_24_dashEnabled */
  x200_24_dashEnabled = __dna_reader.readBool();
  /* x200_25_dashOnButtonRelease */
  x200_25_dashOnButtonRelease = __dna_reader.readBool();
  /* x204_dashButtonHoldCancelTime */
  x204_dashButtonHoldCancelTime = __dna_reader.readFloatBig();
  /* x208_dashStrafeInputThreshold */
  x208_dashStrafeInputThreshold = __dna_reader.readFloatBig();
  /* x20c_sidewaysDoubleJumpImpulse */
  x20c_sidewaysDoubleJumpImpulse = __dna_reader.readFloatBig();
  /* x210_sidewaysVerticalDoubleJumpAccel */
  x210_sidewaysVerticalDoubleJumpAccel = __dna_reader.readFloatBig();
  /* x214_sidewaysHorizontalDoubleJumpAccel */
  x214_sidewaysHorizontalDoubleJumpAccel = __dna_reader.readFloatBig();
  /* x218_scanningRange */
  x218_scanningRange = __dna_reader.readFloatBig();
  /* x21c_24_scanRetention */
  x21c_24_scanRetention = __dna_reader.readBool();
  /* x21c_25_scanFreezesGame */
  x21c_25_scanFreezesGame = __dna_reader.readBool();
  /* x21c_26_orbitWhileScanning */
  x21c_26_orbitWhileScanning = __dna_reader.readBool();
  /* x220_scanMaxTargetDistance */
  x220_scanMaxTargetDistance = __dna_reader.readFloatBig();
  /* x224_scanMaxLockDistance */
  x224_scanMaxLockDistance = __dna_reader.readFloatBig();
  /* x2a0_orbitDistanceMax */
  x2a0_orbitDistanceMax = __dna_reader.readFloatBig();
  /* x2a4_grappleSwingLength */
  x2a4_grappleSwingLength = __dna_reader.readFloatBig();
  /* x2a8_grappleSwingPeriod */
  x2a8_grappleSwingPeriod = __dna_reader.readFloatBig();
  /* x2ac_grapplePullSpeedMin */
  x2ac_grapplePullSpeedMin = __dna_reader.readFloatBig();
  /* x2b0_grappleCameraSpeed */
  x2b0_grappleCameraSpeed = __dna_reader.readFloatBig();
  /* x2b4_maxGrappleLockedTurnAlignDistance */
  x2b4_maxGrappleLockedTurnAlignDistance = __dna_reader.readFloatBig();
  /* x2b8_grapplePullSpeedProportion */
  x2b8_grapplePullSpeedProportion = __dna_reader.readFloatBig();
  /* x2bc_grapplePullSpeedMax */
  x2bc_grapplePullSpeedMax = __dna_reader.readFloatBig();
  /* x2c0_grappleLookCenterSpeed */
  x2c0_grappleLookCenterSpeed = __dna_reader.readFloatBig();
  /* x2c4_maxGrappleTurnSpeed */
  x2c4_maxGrappleTurnSpeed = __dna_reader.readFloatBig();
  /* x2c8_grappleJumpForce */
  x2c8_grappleJumpForce = __dna_reader.readFloatBig();
  /* x2cc_grappleReleaseTime */
  x2cc_grappleReleaseTime = __dna_reader.readFloatBig();
  /* x2d0_grappleJumpMode */
  x2d0_grappleJumpMode = __dna_reader.readUint32Big();
  /* x2d4_orbitReleaseBreaksGrapple */
  x2d4_orbitReleaseBreaksGrapple = __dna_reader.readBool();
  /* x2d5_invertGrappleTurn */
  x2d5_invertGrappleTurn = __dna_reader.readBool();
  /* x2d8_grappleBeamSpeed */
  x2d8_grappleBeamSpeed = __dna_reader.readFloatBig();
  /* x2dc_grappleBeamXWaveAmplitude */
  x2dc_grappleBeamXWaveAmplitude = __dna_reader.readFloatBig();
  /* x2e0_grappleBeamZWaveAmplitude */
  x2e0_grappleBeamZWaveAmplitude = __dna_reader.readFloatBig();
  /* x2e4_grappleBeamAnglePhaseDelta */
  x2e4_grappleBeamAnglePhaseDelta = __dna_reader.readFloatBig();
  /* x26c_playerHeight */
  x26c_playerHeight = __dna_reader.readFloatBig();
  /* x270_playerXYHalfExtent */
  x270_playerXYHalfExtent = __dna_reader.readFloatBig();
  /* x274_stepUpHeight */
  x274_stepUpHeight = __dna_reader.readFloatBig();
  /* x278_stepDownHeight */
  x278_stepDownHeight = __dna_reader.readFloatBig();
  /* x27c_playerBallHalfExtent */
  x27c_playerBallHalfExtent = __dna_reader.readFloatBig();
  /* x280_ */
  x280_firstPersonCameraSpeed = __dna_reader.readFloatBig();
  /* x284_ */
  x284_ = __dna_reader.readFloatBig();
  /* x288_jumpCameraPitchDownStart */
  x288_jumpCameraPitchDownStart = __dna_reader.readFloatBig();
  /* x28c_jumpCameraPitchDownFull */
  x28c_jumpCameraPitchDownFull = __dna_reader.readFloatBig();
  /* x290_jumpCameraPitchDownAngle */
  x290_jumpCameraPitchDownAngle = __dna_reader.readFloatBig();
  /* x294_fallCameraPitchDownStart */
  x294_fallCameraPitchDownStart = __dna_reader.readFloatBig();
  /* x298_fallCameraPitchDownFull */
  x298_fallCameraPitchDownFull = __dna_reader.readFloatBig();
  /* x29c_fallCameraPitchDownAngle */
  x29c_fallCameraPitchDownAngle = __dna_reader.readFloatBig();
  /* x2e8_ */
  x2e8_ = __dna_reader.readFloatBig();
  /* x2ec_ */
  x2ec_ = __dna_reader.readFloatBig();
  /* x2f0_ */
  x2f0_ = __dna_reader.readFloatBig();
  /* x2f4_ */
  x2f4_ = __dna_reader.readBool();
  /* x2f8_frozenTimeout */
  x2f8_frozenTimeout = __dna_reader.readFloatBig();
  /* x2fc_iceBreakJumpCount */
  x2fc_iceBreakJumpCount = __dna_reader.readUint32Big();
  /* x300_variaDamageReduction */
  x300_variaDamageReduction = __dna_reader.readFloatBig();
  /* x304_gravityDamageReduction */
  x304_gravityDamageReduction = __dna_reader.readFloatBig();
  /* x308_phazonDamageReduction */
  x308_phazonDamageReduction = __dna_reader.readFloatBig();
}

template <>
void CTweakPlayer::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& __dna_writer) {
  /* x4_maxTranslationalAcceleration[0] */
  __dna_writer.writeFloatBig(x4_maxTranslationalAcceleration[0]);
  /* x4_maxTranslationalAcceleration[1] */
  __dna_writer.writeFloatBig(x4_maxTranslationalAcceleration[1]);
  /* x4_maxTranslationalAcceleration[2] */
  __dna_writer.writeFloatBig(x4_maxTranslationalAcceleration[2]);
  /* x4_maxTranslationalAcceleration[3] */
  __dna_writer.writeFloatBig(x4_maxTranslationalAcceleration[3]);
  /* x4_maxTranslationalAcceleration[4] */
  __dna_writer.writeFloatBig(x4_maxTranslationalAcceleration[4]);
  /* x4_maxTranslationalAcceleration[5] */
  __dna_writer.writeFloatBig(x4_maxTranslationalAcceleration[5]);
  /* x4_maxTranslationalAcceleration[6] */
  __dna_writer.writeFloatBig(x4_maxTranslationalAcceleration[6]);
  /* x4_maxTranslationalAcceleration[7] */
  __dna_writer.writeFloatBig(x4_maxTranslationalAcceleration[7]);
  /* x24_maxRotationalAcceleration[0] */
  __dna_writer.writeFloatBig(x24_maxRotationalAcceleration[0]);
  /* x24_maxRotationalAcceleration[1] */
  __dna_writer.writeFloatBig(x24_maxRotationalAcceleration[1]);
  /* x24_maxRotationalAcceleration[2] */
  __dna_writer.writeFloatBig(x24_maxRotationalAcceleration[2]);
  /* x24_maxRotationalAcceleration[3] */
  __dna_writer.writeFloatBig(x24_maxRotationalAcceleration[3]);
  /* x24_maxRotationalAcceleration[4] */
  __dna_writer.writeFloatBig(x24_maxRotationalAcceleration[4]);
  /* x24_maxRotationalAcceleration[5] */
  __dna_writer.writeFloatBig(x24_maxRotationalAcceleration[5]);
  /* x24_maxRotationalAcceleration[6] */
  __dna_writer.writeFloatBig(x24_maxRotationalAcceleration[6]);
  /* x24_maxRotationalAcceleration[7] */
  __dna_writer.writeFloatBig(x24_maxRotationalAcceleration[7]);
  /* x44_translationFriction[0] */
  __dna_writer.writeFloatBig(x44_translationFriction[0]);
  /* x44_translationFriction[1] */
  __dna_writer.writeFloatBig(x44_translationFriction[1]);
  /* x44_translationFriction[2] */
  __dna_writer.writeFloatBig(x44_translationFriction[2]);
  /* x44_translationFriction[3] */
  __dna_writer.writeFloatBig(x44_translationFriction[3]);
  /* x44_translationFriction[4] */
  __dna_writer.writeFloatBig(x44_translationFriction[4]);
  /* x44_translationFriction[5] */
  __dna_writer.writeFloatBig(x44_translationFriction[5]);
  /* x44_translationFriction[6] */
  __dna_writer.writeFloatBig(x44_translationFriction[6]);
  /* x44_translationFriction[7] */
  __dna_writer.writeFloatBig(x44_translationFriction[7]);
  /* x64_rotationFriction[0] */
  __dna_writer.writeFloatBig(x64_rotationFriction[0]);
  /* x64_rotationFriction[1] */
  __dna_writer.writeFloatBig(x64_rotationFriction[1]);
  /* x64_rotationFriction[2] */
  __dna_writer.writeFloatBig(x64_rotationFriction[2]);
  /* x64_rotationFriction[3] */
  __dna_writer.writeFloatBig(x64_rotationFriction[3]);
  /* x64_rotationFriction[4] */
  __dna_writer.writeFloatBig(x64_rotationFriction[4]);
  /* x64_rotationFriction[5] */
  __dna_writer.writeFloatBig(x64_rotationFriction[5]);
  /* x64_rotationFriction[6] */
  __dna_writer.writeFloatBig(x64_rotationFriction[6]);
  /* x64_rotationFriction[7] */
  __dna_writer.writeFloatBig(x64_rotationFriction[7]);
  /* x84_rotationMaxSpeed[0] */
  __dna_writer.writeFloatBig(x84_rotationMaxSpeed[0]);
  /* x84_rotationMaxSpeed[1] */
  __dna_writer.writeFloatBig(x84_rotationMaxSpeed[1]);
  /* x84_rotationMaxSpeed[2] */
  __dna_writer.writeFloatBig(x84_rotationMaxSpeed[2]);
  /* x84_rotationMaxSpeed[3] */
  __dna_writer.writeFloatBig(x84_rotationMaxSpeed[3]);
  /* x84_rotationMaxSpeed[4] */
  __dna_writer.writeFloatBig(x84_rotationMaxSpeed[4]);
  /* x84_rotationMaxSpeed[5] */
  __dna_writer.writeFloatBig(x84_rotationMaxSpeed[5]);
  /* x84_rotationMaxSpeed[6] */
  __dna_writer.writeFloatBig(x84_rotationMaxSpeed[6]);
  /* x84_rotationMaxSpeed[7] */
  __dna_writer.writeFloatBig(x84_rotationMaxSpeed[7]);
  /* xa4_translationMaxSpeed[0] */
  __dna_writer.writeFloatBig(xa4_translationMaxSpeed[0]);
  /* xa4_translationMaxSpeed[1] */
  __dna_writer.writeFloatBig(xa4_translationMaxSpeed[1]);
  /* xa4_translationMaxSpeed[2] */
  __dna_writer.writeFloatBig(xa4_translationMaxSpeed[2]);
  /* xa4_translationMaxSpeed[3] */
  __dna_writer.writeFloatBig(xa4_translationMaxSpeed[3]);
  /* xa4_translationMaxSpeed[4] */
  __dna_writer.writeFloatBig(xa4_translationMaxSpeed[4]);
  /* xa4_translationMaxSpeed[5] */
  __dna_writer.writeFloatBig(xa4_translationMaxSpeed[5]);
  /* xa4_translationMaxSpeed[6] */
  __dna_writer.writeFloatBig(xa4_translationMaxSpeed[6]);
  /* xa4_translationMaxSpeed[7] */
  __dna_writer.writeFloatBig(xa4_translationMaxSpeed[7]);
  /* xc4_normalGravAccel */
  __dna_writer.writeFloatBig(xc4_normalGravAccel);
  /* xc8_fluidGravAccel */
  __dna_writer.writeFloatBig(xc8_fluidGravAccel);
  /* xcc_verticalJumpAccel */
  __dna_writer.writeFloatBig(xcc_verticalJumpAccel);
  /* xd0_horizontalJumpAccel */
  __dna_writer.writeFloatBig(xd0_horizontalJumpAccel);
  /* xd4_verticalDoubleJumpAccel */
  __dna_writer.writeFloatBig(xd4_verticalDoubleJumpAccel);
  /* xd8_horizontalDoubleJumpAccel */
  __dna_writer.writeFloatBig(xd8_horizontalDoubleJumpAccel);
  /* xdc_waterJumpFactor */
  __dna_writer.writeFloatBig(xdc_waterJumpFactor);
  /* xe0_waterBallJumpFactor */
  __dna_writer.writeFloatBig(xe0_waterBallJumpFactor);
  /* xe4_lavaJumpFactor */
  __dna_writer.writeFloatBig(xe4_lavaJumpFactor);
  /* xe8_lavaBallJumpFactor */
  __dna_writer.writeFloatBig(xe8_lavaBallJumpFactor);
  /* xec_phazonJumpFactor */
  __dna_writer.writeFloatBig(xec_phazonJumpFactor);
  /* xf0_phazonBallJumpFactor */
  __dna_writer.writeFloatBig(xf0_phazonBallJumpFactor);
  /* xf4_allowedJumpTime */
  __dna_writer.writeFloatBig(xf4_allowedJumpTime);
  /* xf8_allowedDoubleJumpTime */
  __dna_writer.writeFloatBig(xf8_allowedDoubleJumpTime);
  /* xfc_minDoubleJumpWindow */
  __dna_writer.writeFloatBig(xfc_minDoubleJumpWindow);
  /* x100_maxDoubleJumpWindow */
  __dna_writer.writeFloatBig(x100_maxDoubleJumpWindow);
  /* x104_ */
  __dna_writer.writeFloatBig(x104_);
  /* x108_minJumpTime */
  __dna_writer.writeFloatBig(x108_minJumpTime);
  /* x10c_minDoubleJumpTime */
  __dna_writer.writeFloatBig(x10c_minDoubleJumpTime);
  /* x110_allowedLedgeTime */
  __dna_writer.writeFloatBig(x110_allowedLedgeTime);
  /* x114_doubleJumpImpulse */
  __dna_writer.writeFloatBig(x114_doubleJumpImpulse);
  /* x118_backwardsForceMultiplier */
  __dna_writer.writeFloatBig(x118_backwardsForceMultiplier);
  /* x11c_bombJumpRadius */
  __dna_writer.writeFloatBig(x11c_bombJumpRadius);
  /* x120_bombJumpHeight */
  __dna_writer.writeFloatBig(x120_bombJumpHeight);
  /* x124_eyeOffset */
  __dna_writer.writeFloatBig(x124_eyeOffset);
  /* x128_turnSpeedMultiplier */
  __dna_writer.writeFloatBig(x128_turnSpeedMultiplier);
  /* x12c_freeLookTurnSpeedMultiplier */
  __dna_writer.writeFloatBig(x12c_freeLookTurnSpeedMultiplier);
  /* x130_horizontalFreeLookAngleVel */
  __dna_writer.writeFloatBig(x130_horizontalFreeLookAngleVel);
  /* x134_verticalFreeLookAngleVel */
  __dna_writer.writeFloatBig(x134_verticalFreeLookAngleVel);
  /* x138_freeLookSpeed */
  __dna_writer.writeFloatBig(x138_freeLookSpeed);
  /* x13c_freeLookSnapSpeed */
  __dna_writer.writeFloatBig(x13c_freeLookSnapSpeed);
  /* x140_ */
  __dna_writer.writeFloatBig(x140_);
  /* x144_freeLookCenteredThresholdAngle */
  __dna_writer.writeFloatBig(x144_freeLookCenteredThresholdAngle);
  /* x148_freeLookCenteredTime */
  __dna_writer.writeFloatBig(x148_freeLookCenteredTime);
  /* x14c_freeLookDampenFactor */
  __dna_writer.writeFloatBig(x14c_freeLookDampenFactor);
  /* x150_leftDiv */
  __dna_writer.writeFloatBig(x150_leftDiv);
  /* x154_rightDiv */
  __dna_writer.writeFloatBig(x154_rightDiv);
  /* x228_24_freelookTurnsPlayer */
  __dna_writer.writeBool(x228_24_freelookTurnsPlayer);
  /* x228_25_ */
  __dna_writer.writeBool(x228_25_);
  /* x228_26_ */
  __dna_writer.writeBool(x228_26_);
  /* x228_27_moveDuringFreeLook */
  __dna_writer.writeBool(x228_27_moveDuringFreeLook);
  /* x228_28_holdButtonsForFreeLook */
  __dna_writer.writeBool(x228_28_holdButtonsForFreeLook);
  /* x228_29_twoButtonsForFreeLook */
  __dna_writer.writeBool(x228_29_twoButtonsForFreeLook);
  /* x228_30_ */
  __dna_writer.writeBool(x228_30_);
  /* x228_31_ */
  __dna_writer.writeBool(x228_31_);
  /* x229_24_ */
  __dna_writer.writeBool(x229_24_);
  /* x229_25_aimWhenOrbitingPoint */
  __dna_writer.writeBool(x229_25_aimWhenOrbitingPoint);
  /* x229_26_stayInFreeLookWhileFiring */
  __dna_writer.writeBool(x229_26_stayInFreeLookWhileFiring);
  /* x229_27_ */
  __dna_writer.writeBool(x229_27_);
  /* x229_28_ */
  __dna_writer.writeBool(x229_28_);
  /* x229_29_orbitFixedOffset */
  __dna_writer.writeBool(x229_29_orbitFixedOffset);
  /* x229_30_gunButtonTogglesHolster */
  __dna_writer.writeBool(x229_30_gunButtonTogglesHolster);
  /* x229_31_gunNotFiringHolstersGun */
  __dna_writer.writeBool(x229_31_gunNotFiringHolstersGun);
  /* x22a_24_fallingDoubleJump */
  __dna_writer.writeBool(x22a_24_fallingDoubleJump);
  /* x22a_25_impulseDoubleJump */
  __dna_writer.writeBool(x22a_25_impulseDoubleJump);
  /* x22a_26_firingCancelsCameraPitch */
  __dna_writer.writeBool(x22a_26_firingCancelsCameraPitch);
  /* x22a_27_assistedAimingIgnoreHorizontal */
  __dna_writer.writeBool(x22a_27_assistedAimingIgnoreHorizontal);
  /* x22a_28_assistedAimingIgnoreVertical */
  __dna_writer.writeBool(x22a_28_assistedAimingIgnoreVertical);
  /* x22c_ */
  __dna_writer.writeFloatBig(x22c_);
  /* x230_ */
  __dna_writer.writeFloatBig(x230_);
  /* x234_aimMaxDistance */
  __dna_writer.writeFloatBig(x234_aimMaxDistance);
  /* x238_ */
  __dna_writer.writeFloatBig(x238_);
  /* x23c_ */
  __dna_writer.writeFloatBig(x23c_);
  /* x240_ */
  __dna_writer.writeFloatBig(x240_);
  /* x244_ */
  __dna_writer.writeFloatBig(x244_);
  /* x248_ */
  __dna_writer.writeFloatBig(x248_);
  /* x24c_aimThresholdDistance */
  __dna_writer.writeFloatBig(x24c_aimThresholdDistance);
  /* x250_ */
  __dna_writer.writeFloatBig(x250_);
  /* x254_ */
  __dna_writer.writeFloatBig(x254_);
  /* x258_aimBoxWidth */
  __dna_writer.writeFloatBig(x258_aimBoxWidth);
  /* x25c_aimBoxHeight */
  __dna_writer.writeFloatBig(x25c_aimBoxHeight);
  /* x260_aimTargetTimer */
  __dna_writer.writeFloatBig(x260_aimTargetTimer);
  /* x264_aimAssistHorizontalAngle */
  __dna_writer.writeFloatBig(x264_aimAssistHorizontalAngle);
  /* x268_aimAssistVerticalAngle */
  __dna_writer.writeFloatBig(x268_aimAssistVerticalAngle);
  /* x158_orbitMinDistance[0] */
  __dna_writer.writeFloatBig(x158_orbitMinDistance[0]);
  /* x164_orbitNormalDistance[0] */
  __dna_writer.writeFloatBig(x164_orbitNormalDistance[0]);
  /* x170_orbitMaxDistance[0] */
  __dna_writer.writeFloatBig(x170_orbitMaxDistance[0]);
  /* x158_orbitMinDistance[1] */
  __dna_writer.writeFloatBig(x158_orbitMinDistance[1]);
  /* x164_orbitNormalDistance[1] */
  __dna_writer.writeFloatBig(x164_orbitNormalDistance[1]);
  /* x170_orbitMaxDistance[1] */
  __dna_writer.writeFloatBig(x170_orbitMaxDistance[1]);
  /* x158_orbitMinDistance[2] */
  __dna_writer.writeFloatBig(x158_orbitMinDistance[2]);
  /* x164_orbitNormalDistance[2] */
  __dna_writer.writeFloatBig(x164_orbitNormalDistance[2]);
  /* x170_orbitMaxDistance[2] */
  __dna_writer.writeFloatBig(x170_orbitMaxDistance[2]);
  /* x17c_ */
  __dna_writer.writeFloatBig(x17c_);
  /* x180_orbitModeTimer */
  __dna_writer.writeFloatBig(x180_orbitModeTimer);
  /* x184_orbitCameraSpeed */
  __dna_writer.writeFloatBig(x184_orbitCameraSpeed);
  /* x188_orbitUpperAngle */
  __dna_writer.writeFloatBig(x188_orbitUpperAngle);
  /* x18c_orbitLowerAngle */
  __dna_writer.writeFloatBig(x18c_orbitLowerAngle);
  /* x190_orbitHorizAngle */
  __dna_writer.writeFloatBig(x190_orbitHorizAngle);
  /* x194_ */
  __dna_writer.writeFloatBig(x194_);
  /* x198_ */
  __dna_writer.writeFloatBig(x198_);
  /* x19c_orbitMaxTargetDistance */
  __dna_writer.writeFloatBig(x19c_orbitMaxTargetDistance);
  /* x1a0_orbitMaxLockDistance */
  __dna_writer.writeFloatBig(x1a0_orbitMaxLockDistance);
  /* x1a4_orbitDistanceThreshold */
  __dna_writer.writeFloatBig(x1a4_orbitDistanceThreshold);
  /* x1a8_orbitScreenBoxHalfExtentX[0] */
  __dna_writer.writeUint32Big(x1a8_orbitScreenBoxHalfExtentX[0]);
  /* x1b0_orbitScreenBoxHalfExtentY[0] */
  __dna_writer.writeUint32Big(x1b0_orbitScreenBoxHalfExtentY[0]);
  /* x1b8_orbitScreenBoxCenterX[0] */
  __dna_writer.writeUint32Big(x1b8_orbitScreenBoxCenterX[0]);
  /* x1c0_orbitScreenBoxCenterY[0] */
  __dna_writer.writeUint32Big(x1c0_orbitScreenBoxCenterY[0]);
  /* x1c8_orbitZoneIdealX[0] */
  __dna_writer.writeUint32Big(x1c8_orbitZoneIdealX[0]);
  /* x1d0_orbitZoneIdealY[0] */
  __dna_writer.writeUint32Big(x1d0_orbitZoneIdealY[0]);
  /* x1a8_orbitScreenBoxHalfExtentX[1] */
  __dna_writer.writeUint32Big(x1a8_orbitScreenBoxHalfExtentX[1]);
  /* x1b0_orbitScreenBoxHalfExtentY[1] */
  __dna_writer.writeUint32Big(x1b0_orbitScreenBoxHalfExtentY[1]);
  /* x1b8_orbitScreenBoxCenterX[1] */
  __dna_writer.writeUint32Big(x1b8_orbitScreenBoxCenterX[1]);
  /* x1c0_orbitScreenBoxCenterY[1] */
  __dna_writer.writeUint32Big(x1c0_orbitScreenBoxCenterY[1]);
  /* x1c8_orbitZoneIdealX[1] */
  __dna_writer.writeUint32Big(x1c8_orbitZoneIdealX[1]);
  /* x1d0_orbitZoneIdealY[1] */
  __dna_writer.writeUint32Big(x1d0_orbitZoneIdealY[1]);
  /* x1d8_orbitNearX */
  __dna_writer.writeFloatBig(x1d8_orbitNearX);
  /* x1dc_orbitNearZ */
  __dna_writer.writeFloatBig(x1dc_orbitNearZ);
  /* x1e0_ */
  __dna_writer.writeFloatBig(x1e0_);
  /* x1e4_ */
  __dna_writer.writeFloatBig(x1e4_);
  /* x1e8_orbitFixedOffsetZDiff */
  __dna_writer.writeFloatBig(x1e8_orbitFixedOffsetZDiff);
  /* x1ec_orbitZRange */
  __dna_writer.writeFloatBig(x1ec_orbitZRange);
  /* x1f0_ */
  __dna_writer.writeFloatBig(x1f0_);
  /* x1f4_ */
  __dna_writer.writeFloatBig(x1f4_);
  /* x1f8_ */
  __dna_writer.writeFloatBig(x1f8_);
  /* x1fc_orbitPreventionTime */
  __dna_writer.writeFloatBig(x1fc_orbitPreventionTime);
  /* x200_24_dashEnabled */
  __dna_writer.writeBool(x200_24_dashEnabled);
  /* x200_25_dashOnButtonRelease */
  __dna_writer.writeBool(x200_25_dashOnButtonRelease);
  /* x204_dashButtonHoldCancelTime */
  __dna_writer.writeFloatBig(x204_dashButtonHoldCancelTime);
  /* x208_dashStrafeInputThreshold */
  __dna_writer.writeFloatBig(x208_dashStrafeInputThreshold);
  /* x20c_sidewaysDoubleJumpImpulse */
  __dna_writer.writeFloatBig(x20c_sidewaysDoubleJumpImpulse);
  /* x210_sidewaysVerticalDoubleJumpAccel */
  __dna_writer.writeFloatBig(x210_sidewaysVerticalDoubleJumpAccel);
  /* x214_sidewaysHorizontalDoubleJumpAccel */
  __dna_writer.writeFloatBig(x214_sidewaysHorizontalDoubleJumpAccel);
  /* x218_scanningRange */
  __dna_writer.writeFloatBig(x218_scanningRange);
  /* x21c_24_scanRetention */
  __dna_writer.writeBool(x21c_24_scanRetention);
  /* x21c_25_scanFreezesGame */
  __dna_writer.writeBool(x21c_25_scanFreezesGame);
  /* x21c_26_orbitWhileScanning */
  __dna_writer.writeBool(x21c_26_orbitWhileScanning);
  /* x220_scanMaxTargetDistance */
  __dna_writer.writeFloatBig(x220_scanMaxTargetDistance);
  /* x224_scanMaxLockDistance */
  __dna_writer.writeFloatBig(x224_scanMaxLockDistance);
  /* x2a0_orbitDistanceMax */
  __dna_writer.writeFloatBig(x2a0_orbitDistanceMax);
  /* x2a4_grappleSwingLength */
  __dna_writer.writeFloatBig(x2a4_grappleSwingLength);
  /* x2a8_grappleSwingPeriod */
  __dna_writer.writeFloatBig(x2a8_grappleSwingPeriod);
  /* x2ac_grapplePullSpeedMin */
  __dna_writer.writeFloatBig(x2ac_grapplePullSpeedMin);
  /* x2b0_grappleCameraSpeed */
  __dna_writer.writeFloatBig(x2b0_grappleCameraSpeed);
  /* x2b4_maxGrappleLockedTurnAlignDistance */
  __dna_writer.writeFloatBig(x2b4_maxGrappleLockedTurnAlignDistance);
  /* x2b8_grapplePullSpeedProportion */
  __dna_writer.writeFloatBig(x2b8_grapplePullSpeedProportion);
  /* x2bc_grapplePullSpeedMax */
  __dna_writer.writeFloatBig(x2bc_grapplePullSpeedMax);
  /* x2c0_grappleLookCenterSpeed */
  __dna_writer.writeFloatBig(x2c0_grappleLookCenterSpeed);
  /* x2c4_maxGrappleTurnSpeed */
  __dna_writer.writeFloatBig(x2c4_maxGrappleTurnSpeed);
  /* x2c8_grappleJumpForce */
  __dna_writer.writeFloatBig(x2c8_grappleJumpForce);
  /* x2cc_grappleReleaseTime */
  __dna_writer.writeFloatBig(x2cc_grappleReleaseTime);
  /* x2d0_grappleJumpMode */
  __dna_writer.writeUint32Big(x2d0_grappleJumpMode);
  /* x2d4_orbitReleaseBreaksGrapple */
  __dna_writer.writeBool(x2d4_orbitReleaseBreaksGrapple);
  /* x2d5_invertGrappleTurn */
  __dna_writer.writeBool(x2d5_invertGrappleTurn);
  /* x2d8_grappleBeamSpeed */
  __dna_writer.writeFloatBig(x2d8_grappleBeamSpeed);
  /* x2dc_grappleBeamXWaveAmplitude */
  __dna_writer.writeFloatBig(x2dc_grappleBeamXWaveAmplitude);
  /* x2e0_grappleBeamZWaveAmplitude */
  __dna_writer.writeFloatBig(x2e0_grappleBeamZWaveAmplitude);
  /* x2e4_grappleBeamAnglePhaseDelta */
  __dna_writer.writeFloatBig(x2e4_grappleBeamAnglePhaseDelta);
  /* x26c_playerHeight */
  __dna_writer.writeFloatBig(x26c_playerHeight);
  /* x270_playerXYHalfExtent */
  __dna_writer.writeFloatBig(x270_playerXYHalfExtent);
  /* x274_stepUpHeight */
  __dna_writer.writeFloatBig(x274_stepUpHeight);
  /* x278_stepDownHeight */
  __dna_writer.writeFloatBig(x278_stepDownHeight);
  /* x27c_playerBallHalfExtent */
  __dna_writer.writeFloatBig(x27c_playerBallHalfExtent);
  /* x280_ */
  __dna_writer.writeFloatBig(x280_firstPersonCameraSpeed);
  /* x284_ */
  __dna_writer.writeFloatBig(x284_);
  /* x288_jumpCameraPitchDownStart */
  __dna_writer.writeFloatBig(x288_jumpCameraPitchDownStart);
  /* x28c_jumpCameraPitchDownFull */
  __dna_writer.writeFloatBig(x28c_jumpCameraPitchDownFull);
  /* x290_jumpCameraPitchDownAngle */
  __dna_writer.writeFloatBig(x290_jumpCameraPitchDownAngle);
  /* x294_fallCameraPitchDownStart */
  __dna_writer.writeFloatBig(x294_fallCameraPitchDownStart);
  /* x298_fallCameraPitchDownFull */
  __dna_writer.writeFloatBig(x298_fallCameraPitchDownFull);
  /* x29c_fallCameraPitchDownAngle */
  __dna_writer.writeFloatBig(x29c_fallCameraPitchDownAngle);
  /* x2e8_ */
  __dna_writer.writeFloatBig(x2e8_);
  /* x2ec_ */
  __dna_writer.writeFloatBig(x2ec_);
  /* x2f0_ */
  __dna_writer.writeFloatBig(x2f0_);
  /* x2f4_ */
  __dna_writer.writeBool(x2f4_);
  /* x2f8_frozenTimeout */
  __dna_writer.writeFloatBig(x2f8_frozenTimeout);
  /* x2fc_iceBreakJumpCount */
  __dna_writer.writeUint32Big(x2fc_iceBreakJumpCount);
  /* x300_variaDamageReduction */
  __dna_writer.writeFloatBig(x300_variaDamageReduction);
  /* x304_gravityDamageReduction */
  __dna_writer.writeFloatBig(x304_gravityDamageReduction);
  /* x308_phazonDamageReduction */
  __dna_writer.writeFloatBig(x308_phazonDamageReduction);
}

template <>
void CTweakPlayer::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& __dna_docin) {
  /* x4_maxTranslationalAcceleration */
  size_t __x4_Count;
  if (auto v = __dna_docin.enterSubVector("x4_maxTranslationalAcceleration", __x4_Count)) {
    /* x4_maxTranslationalAcceleration[0] */
    x4_maxTranslationalAcceleration[0] = __dna_docin.readFloat("x4_maxTranslationalAcceleration");
    /* x4_maxTranslationalAcceleration[1] */
    x4_maxTranslationalAcceleration[1] = __dna_docin.readFloat("x4_maxTranslationalAcceleration");
    /* x4_maxTranslationalAcceleration[2] */
    x4_maxTranslationalAcceleration[2] = __dna_docin.readFloat("x4_maxTranslationalAcceleration");
    /* x4_maxTranslationalAcceleration[3] */
    x4_maxTranslationalAcceleration[3] = __dna_docin.readFloat("x4_maxTranslationalAcceleration");
    /* x4_maxTranslationalAcceleration[4] */
    x4_maxTranslationalAcceleration[4] = __dna_docin.readFloat("x4_maxTranslationalAcceleration");
    /* x4_maxTranslationalAcceleration[5] */
    x4_maxTranslationalAcceleration[5] = __dna_docin.readFloat("x4_maxTranslationalAcceleration");
    /* x4_maxTranslationalAcceleration[6] */
    x4_maxTranslationalAcceleration[6] = __dna_docin.readFloat("x4_maxTranslationalAcceleration");
    /* x4_maxTranslationalAcceleration[7] */
    x4_maxTranslationalAcceleration[7] = __dna_docin.readFloat("x4_maxTranslationalAcceleration");
  }
  /* x24_maxRotationalAcceleration */
  size_t __x24_Count;
  if (auto v = __dna_docin.enterSubVector("x24_maxRotationalAcceleration", __x24_Count)) {
    /* x24_maxRotationalAcceleration[0] */
    x24_maxRotationalAcceleration[0] = __dna_docin.readFloat("x24_maxRotationalAcceleration");
    /* x24_maxRotationalAcceleration[1] */
    x24_maxRotationalAcceleration[1] = __dna_docin.readFloat("x24_maxRotationalAcceleration");
    /* x24_maxRotationalAcceleration[2] */
    x24_maxRotationalAcceleration[2] = __dna_docin.readFloat("x24_maxRotationalAcceleration");
    /* x24_maxRotationalAcceleration[3] */
    x24_maxRotationalAcceleration[3] = __dna_docin.readFloat("x24_maxRotationalAcceleration");
    /* x24_maxRotationalAcceleration[4] */
    x24_maxRotationalAcceleration[4] = __dna_docin.readFloat("x24_maxRotationalAcceleration");
    /* x24_maxRotationalAcceleration[5] */
    x24_maxRotationalAcceleration[5] = __dna_docin.readFloat("x24_maxRotationalAcceleration");
    /* x24_maxRotationalAcceleration[6] */
    x24_maxRotationalAcceleration[6] = __dna_docin.readFloat("x24_maxRotationalAcceleration");
    /* x24_maxRotationalAcceleration[7] */
    x24_maxRotationalAcceleration[7] = __dna_docin.readFloat("x24_maxRotationalAcceleration");
  }
  /* x44_translationFriction */
  size_t __x44_Count;
  if (auto v = __dna_docin.enterSubVector("x44_translationFriction", __x44_Count)) {
    /* x44_translationFriction[0] */
    x44_translationFriction[0] = __dna_docin.readFloat("x44_translationFriction");
    /* x44_translationFriction[1] */
    x44_translationFriction[1] = __dna_docin.readFloat("x44_translationFriction");
    /* x44_translationFriction[2] */
    x44_translationFriction[2] = __dna_docin.readFloat("x44_translationFriction");
    /* x44_translationFriction[3] */
    x44_translationFriction[3] = __dna_docin.readFloat("x44_translationFriction");
    /* x44_translationFriction[4] */
    x44_translationFriction[4] = __dna_docin.readFloat("x44_translationFriction");
    /* x44_translationFriction[5] */
    x44_translationFriction[5] = __dna_docin.readFloat("x44_translationFriction");
    /* x44_translationFriction[6] */
    x44_translationFriction[6] = __dna_docin.readFloat("x44_translationFriction");
    /* x44_translationFriction[7] */
    x44_translationFriction[7] = __dna_docin.readFloat("x44_translationFriction");
  }
  /* x64_rotationFriction */
  size_t __x64_Count;
  if (auto v = __dna_docin.enterSubVector("x64_rotationFriction", __x64_Count)) {
    /* x64_rotationFriction[0] */
    x64_rotationFriction[0] = __dna_docin.readFloat("x64_rotationFriction");
    /* x64_rotationFriction[1] */
    x64_rotationFriction[1] = __dna_docin.readFloat("x64_rotationFriction");
    /* x64_rotationFriction[2] */
    x64_rotationFriction[2] = __dna_docin.readFloat("x64_rotationFriction");
    /* x64_rotationFriction[3] */
    x64_rotationFriction[3] = __dna_docin.readFloat("x64_rotationFriction");
    /* x64_rotationFriction[4] */
    x64_rotationFriction[4] = __dna_docin.readFloat("x64_rotationFriction");
    /* x64_rotationFriction[5] */
    x64_rotationFriction[5] = __dna_docin.readFloat("x64_rotationFriction");
    /* x64_rotationFriction[6] */
    x64_rotationFriction[6] = __dna_docin.readFloat("x64_rotationFriction");
    /* x64_rotationFriction[7] */
    x64_rotationFriction[7] = __dna_docin.readFloat("x64_rotationFriction");
  }
  /* x84_rotationMaxSpeed */
  size_t __x84_Count;
  if (auto v = __dna_docin.enterSubVector("x84_rotationMaxSpeed", __x84_Count)) {
    /* x84_rotationMaxSpeed[0] */
    x84_rotationMaxSpeed[0] = __dna_docin.readFloat("x84_rotationMaxSpeed");
    /* x84_rotationMaxSpeed[1] */
    x84_rotationMaxSpeed[1] = __dna_docin.readFloat("x84_rotationMaxSpeed");
    /* x84_rotationMaxSpeed[2] */
    x84_rotationMaxSpeed[2] = __dna_docin.readFloat("x84_rotationMaxSpeed");
    /* x84_rotationMaxSpeed[3] */
    x84_rotationMaxSpeed[3] = __dna_docin.readFloat("x84_rotationMaxSpeed");
    /* x84_rotationMaxSpeed[4] */
    x84_rotationMaxSpeed[4] = __dna_docin.readFloat("x84_rotationMaxSpeed");
    /* x84_rotationMaxSpeed[5] */
    x84_rotationMaxSpeed[5] = __dna_docin.readFloat("x84_rotationMaxSpeed");
    /* x84_rotationMaxSpeed[6] */
    x84_rotationMaxSpeed[6] = __dna_docin.readFloat("x84_rotationMaxSpeed");
    /* x84_rotationMaxSpeed[7] */
    x84_rotationMaxSpeed[7] = __dna_docin.readFloat("x84_rotationMaxSpeed");
  }
  /* xa4_translationMaxSpeed */
  size_t __xa4_Count;
  if (auto v = __dna_docin.enterSubVector("xa4_translationMaxSpeed", __xa4_Count)) {
    /* xa4_translationMaxSpeed[0] */
    xa4_translationMaxSpeed[0] = __dna_docin.readFloat("xa4_translationMaxSpeed");
    /* xa4_translationMaxSpeed[1] */
    xa4_translationMaxSpeed[1] = __dna_docin.readFloat("xa4_translationMaxSpeed");
    /* xa4_translationMaxSpeed[2] */
    xa4_translationMaxSpeed[2] = __dna_docin.readFloat("xa4_translationMaxSpeed");
    /* xa4_translationMaxSpeed[3] */
    xa4_translationMaxSpeed[3] = __dna_docin.readFloat("xa4_translationMaxSpeed");
    /* xa4_translationMaxSpeed[4] */
    xa4_translationMaxSpeed[4] = __dna_docin.readFloat("xa4_translationMaxSpeed");
    /* xa4_translationMaxSpeed[5] */
    xa4_translationMaxSpeed[5] = __dna_docin.readFloat("xa4_translationMaxSpeed");
    /* xa4_translationMaxSpeed[6] */
    xa4_translationMaxSpeed[6] = __dna_docin.readFloat("xa4_translationMaxSpeed");
    /* xa4_translationMaxSpeed[7] */
    xa4_translationMaxSpeed[7] = __dna_docin.readFloat("xa4_translationMaxSpeed");
  }
  /* xc4_normalGravAccel */
  xc4_normalGravAccel = __dna_docin.readFloat("xc4_normalGravAccel");
  /* xc8_fluidGravAccel */
  xc8_fluidGravAccel = __dna_docin.readFloat("xc8_fluidGravAccel");
  /* xcc_verticalJumpAccel */
  xcc_verticalJumpAccel = __dna_docin.readFloat("xcc_verticalJumpAccel");
  /* xd0_horizontalJumpAccel */
  xd0_horizontalJumpAccel = __dna_docin.readFloat("xd0_horizontalJumpAccel");
  /* xd4_verticalDoubleJumpAccel */
  xd4_verticalDoubleJumpAccel = __dna_docin.readFloat("xd4_verticalDoubleJumpAccel");
  /* xd8_horizontalDoubleJumpAccel */
  xd8_horizontalDoubleJumpAccel = __dna_docin.readFloat("xd8_horizontalDoubleJumpAccel");
  /* xdc_waterJumpFactor */
  xdc_waterJumpFactor = __dna_docin.readFloat("xdc_waterJumpFactor");
  /* xe0_waterBallJumpFactor */
  xe0_waterBallJumpFactor = __dna_docin.readFloat("xe0_waterBallJumpFactor");
  /* xe4_lavaJumpFactor */
  xe4_lavaJumpFactor = __dna_docin.readFloat("xe4_lavaJumpFactor");
  /* xe8_lavaBallJumpFactor */
  xe8_lavaBallJumpFactor = __dna_docin.readFloat("xe8_lavaBallJumpFactor");
  /* xec_phazonJumpFactor */
  xec_phazonJumpFactor = __dna_docin.readFloat("xec_phazonJumpFactor");
  /* xf0_phazonBallJumpFactor */
  xf0_phazonBallJumpFactor = __dna_docin.readFloat("xf0_phazonBallJumpFactor");
  /* xf4_allowedJumpTime */
  xf4_allowedJumpTime = __dna_docin.readFloat("xf4_allowedJumpTime");
  /* xf8_allowedDoubleJumpTime */
  xf8_allowedDoubleJumpTime = __dna_docin.readFloat("xf8_allowedDoubleJumpTime");
  /* xfc_minDoubleJumpWindow */
  xfc_minDoubleJumpWindow = __dna_docin.readFloat("xfc_minDoubleJumpWindow");
  /* x100_maxDoubleJumpWindow */
  x100_maxDoubleJumpWindow = __dna_docin.readFloat("x100_maxDoubleJumpWindow");
  /* x104_ */
  x104_ = __dna_docin.readFloat("x104_");
  /* x108_minJumpTime */
  x108_minJumpTime = __dna_docin.readFloat("x108_minJumpTime");
  /* x10c_minDoubleJumpTime */
  x10c_minDoubleJumpTime = __dna_docin.readFloat("x10c_minDoubleJumpTime");
  /* x110_allowedLedgeTime */
  x110_allowedLedgeTime = __dna_docin.readFloat("x110_allowedLedgeTime");
  /* x114_doubleJumpImpulse */
  x114_doubleJumpImpulse = __dna_docin.readFloat("x114_doubleJumpImpulse");
  /* x118_backwardsForceMultiplier */
  x118_backwardsForceMultiplier = __dna_docin.readFloat("x118_backwardsForceMultiplier");
  /* x11c_bombJumpRadius */
  x11c_bombJumpRadius = __dna_docin.readFloat("x11c_bombJumpRadius");
  /* x120_bombJumpHeight */
  x120_bombJumpHeight = __dna_docin.readFloat("x120_bombJumpHeight");
  /* x124_eyeOffset */
  x124_eyeOffset = __dna_docin.readFloat("x124_eyeOffset");
  /* x128_turnSpeedMultiplier */
  x128_turnSpeedMultiplier = __dna_docin.readFloat("x128_turnSpeedMultiplier");
  /* x12c_freeLookTurnSpeedMultiplier */
  x12c_freeLookTurnSpeedMultiplier = __dna_docin.readFloat("x12c_freeLookTurnSpeedMultiplier");
  /* x130_horizontalFreeLookAngleVel */
  x130_horizontalFreeLookAngleVel = __dna_docin.readFloat("x130_horizontalFreeLookAngleVel");
  /* x134_verticalFreeLookAngleVel */
  x134_verticalFreeLookAngleVel = __dna_docin.readFloat("x134_verticalFreeLookAngleVel");
  /* x138_freeLookSpeed */
  x138_freeLookSpeed = __dna_docin.readFloat("x138_freeLookSpeed");
  /* x13c_freeLookSnapSpeed */
  x13c_freeLookSnapSpeed = __dna_docin.readFloat("x13c_freeLookSnapSpeed");
  /* x140_ */
  x140_ = __dna_docin.readFloat("x140_");
  /* x144_freeLookCenteredThresholdAngle */
  x144_freeLookCenteredThresholdAngle = __dna_docin.readFloat("x144_freeLookCenteredThresholdAngle");
  /* x148_freeLookCenteredTime */
  x148_freeLookCenteredTime = __dna_docin.readFloat("x148_freeLookCenteredTime");
  /* x14c_freeLookDampenFactor */
  x14c_freeLookDampenFactor = __dna_docin.readFloat("x14c_freeLookDampenFactor");
  /* x150_leftDiv */
  x150_leftDiv = __dna_docin.readFloat("x150_leftDiv");
  /* x154_rightDiv */
  x154_rightDiv = __dna_docin.readFloat("x154_rightDiv");
  /* x228_24_freelookTurnsPlayer */
  x228_24_freelookTurnsPlayer = __dna_docin.readBool("x228_24_freelookTurnsPlayer");
  /* x228_25_ */
  x228_25_ = __dna_docin.readBool("x228_25_");
  /* x228_26_ */
  x228_26_ = __dna_docin.readBool("x228_26_");
  /* x228_27_moveDuringFreeLook */
  x228_27_moveDuringFreeLook = __dna_docin.readBool("x228_27_moveDuringFreeLook");
  /* x228_28_holdButtonsForFreeLook */
  x228_28_holdButtonsForFreeLook = __dna_docin.readBool("x228_28_holdButtonsForFreeLook");
  /* x228_29_twoButtonsForFreeLook */
  x228_29_twoButtonsForFreeLook = __dna_docin.readBool("x228_29_twoButtonsForFreeLook");
  /* x228_30_ */
  x228_30_ = __dna_docin.readBool("x228_30_");
  /* x228_31_ */
  x228_31_ = __dna_docin.readBool("x228_31_");
  /* x229_24_ */
  x229_24_ = __dna_docin.readBool("x229_24_");
  /* x229_25_aimWhenOrbitingPoint */
  x229_25_aimWhenOrbitingPoint = __dna_docin.readBool("x229_25_aimWhenOrbitingPoint");
  /* x229_26_stayInFreeLookWhileFiring */
  x229_26_stayInFreeLookWhileFiring = __dna_docin.readBool("x229_26_stayInFreeLookWhileFiring");
  /* x229_27_ */
  x229_27_ = __dna_docin.readBool("x229_27_");
  /* x229_28_ */
  x229_28_ = __dna_docin.readBool("x229_28_");
  /* x229_29_orbitFixedOffset */
  x229_29_orbitFixedOffset = __dna_docin.readBool("x229_29_orbitFixedOffset");
  /* x229_30_gunButtonTogglesHolster */
  x229_30_gunButtonTogglesHolster = __dna_docin.readBool("x229_30_gunButtonTogglesHolster");
  /* x229_31_gunNotFiringHolstersGun */
  x229_31_gunNotFiringHolstersGun = __dna_docin.readBool("x229_31_gunNotFiringHolstersGun");
  /* x22a_24_fallingDoubleJump */
  x22a_24_fallingDoubleJump = __dna_docin.readBool("x22a_24_fallingDoubleJump");
  /* x22a_25_impulseDoubleJump */
  x22a_25_impulseDoubleJump = __dna_docin.readBool("x22a_25_impulseDoubleJump");
  /* x22a_26_firingCancelsCameraPitch */
  x22a_26_firingCancelsCameraPitch = __dna_docin.readBool("x22a_26_firingCancelsCameraPitch");
  /* x22a_27_assistedAimingIgnoreHorizontal */
  x22a_27_assistedAimingIgnoreHorizontal = __dna_docin.readBool("x22a_27_assistedAimingIgnoreHorizontal");
  /* x22a_28_assistedAimingIgnoreVertical */
  x22a_28_assistedAimingIgnoreVertical = __dna_docin.readBool("x22a_28_assistedAimingIgnoreVertical");
  /* x22c_ */
  x22c_ = __dna_docin.readFloat("x22c_");
  /* x230_ */
  x230_ = __dna_docin.readFloat("x230_");
  /* x234_aimMaxDistance */
  x234_aimMaxDistance = __dna_docin.readFloat("x234_aimMaxDistance");
  /* x238_ */
  x238_ = __dna_docin.readFloat("x238_");
  /* x23c_ */
  x23c_ = __dna_docin.readFloat("x23c_");
  /* x240_ */
  x240_ = __dna_docin.readFloat("x240_");
  /* x244_ */
  x244_ = __dna_docin.readFloat("x244_");
  /* x248_ */
  x248_ = __dna_docin.readFloat("x248_");
  /* x24c_aimThresholdDistance */
  x24c_aimThresholdDistance = __dna_docin.readFloat("x24c_aimThresholdDistance");
  /* x250_ */
  x250_ = __dna_docin.readFloat("x250_");
  /* x254_ */
  x254_ = __dna_docin.readFloat("x254_");
  /* x258_aimBoxWidth */
  x258_aimBoxWidth = __dna_docin.readFloat("x258_aimBoxWidth");
  /* x25c_aimBoxHeight */
  x25c_aimBoxHeight = __dna_docin.readFloat("x25c_aimBoxHeight");
  /* x260_aimTargetTimer */
  x260_aimTargetTimer = __dna_docin.readFloat("x260_aimTargetTimer");
  /* x264_aimAssistHorizontalAngle */
  x264_aimAssistHorizontalAngle = __dna_docin.readFloat("x264_aimAssistHorizontalAngle");
  /* x268_aimAssistVerticalAngle */
  x268_aimAssistVerticalAngle = __dna_docin.readFloat("x268_aimAssistVerticalAngle");
  /* x158_orbitMinDistance */
  size_t __x158_Count;
  if (auto v = __dna_docin.enterSubVector("x158_orbitMinDistance", __x158_Count)) {
    /* x158_orbitMinDistance[0] */
    x158_orbitMinDistance[0] = __dna_docin.readFloat("x158_orbitMinDistance");
    /* x158_orbitMinDistance[1] */
    x158_orbitMinDistance[1] = __dna_docin.readFloat("x158_orbitMinDistance");
    /* x158_orbitMinDistance[2] */
    x158_orbitMinDistance[2] = __dna_docin.readFloat("x158_orbitMinDistance");
  }
  /* x164_orbitNormalDistance */
  size_t __x164_Count;
  if (auto v = __dna_docin.enterSubVector("x164_orbitNormalDistance", __x164_Count)) {
    /* x164_orbitNormalDistance[0] */
    x164_orbitNormalDistance[0] = __dna_docin.readFloat("x164_orbitNormalDistance");
    /* x164_orbitNormalDistance[1] */
    x164_orbitNormalDistance[1] = __dna_docin.readFloat("x164_orbitNormalDistance");
    /* x164_orbitNormalDistance[2] */
    x164_orbitNormalDistance[2] = __dna_docin.readFloat("x164_orbitNormalDistance");
  }
  /* x170_orbitMaxDistance */
  size_t __x170_Count;
  if (auto v = __dna_docin.enterSubVector("x170_orbitMaxDistance", __x170_Count)) {
    /* x170_orbitMaxDistance[0] */
    x170_orbitMaxDistance[0] = __dna_docin.readFloat("x170_orbitMaxDistance");
    /* x170_orbitMaxDistance[1] */
    x170_orbitMaxDistance[1] = __dna_docin.readFloat("x170_orbitMaxDistance");
    /* x170_orbitMaxDistance[2] */
    x170_orbitMaxDistance[2] = __dna_docin.readFloat("x170_orbitMaxDistance");
  }
  /* x17c_ */
  x17c_ = __dna_docin.readFloat("x17c_");
  /* x180_orbitModeTimer */
  x180_orbitModeTimer = __dna_docin.readFloat("x180_orbitModeTimer");
  /* x184_orbitCameraSpeed */
  x184_orbitCameraSpeed = __dna_docin.readFloat("x184_orbitCameraSpeed");
  /* x188_orbitUpperAngle */
  x188_orbitUpperAngle = __dna_docin.readFloat("x188_orbitUpperAngle");
  /* x18c_orbitLowerAngle */
  x18c_orbitLowerAngle = __dna_docin.readFloat("x18c_orbitLowerAngle");
  /* x190_orbitHorizAngle */
  x190_orbitHorizAngle = __dna_docin.readFloat("x190_orbitHorizAngle");
  /* x194_ */
  x194_ = __dna_docin.readFloat("x194_");
  /* x198_ */
  x198_ = __dna_docin.readFloat("x198_");
  /* x19c_orbitMaxTargetDistance */
  x19c_orbitMaxTargetDistance = __dna_docin.readFloat("x19c_orbitMaxTargetDistance");
  /* x1a0_orbitMaxLockDistance */
  x1a0_orbitMaxLockDistance = __dna_docin.readFloat("x1a0_orbitMaxLockDistance");
  /* x1a4_orbitDistanceThreshold */
  x1a4_orbitDistanceThreshold = __dna_docin.readFloat("x1a4_orbitDistanceThreshold");
  /* x1a8_orbitScreenBoxHalfExtentX */
  size_t __x1a8_Count;
  if (auto v = __dna_docin.enterSubVector("x1a8_orbitScreenBoxHalfExtentX", __x1a8_Count)) {
    /* x1a8_orbitScreenBoxHalfExtentX[0] */
    x1a8_orbitScreenBoxHalfExtentX[0] = __dna_docin.readUint32("x1a8_orbitScreenBoxHalfExtentX");
    /* x1a8_orbitScreenBoxHalfExtentX[1] */
    x1a8_orbitScreenBoxHalfExtentX[1] = __dna_docin.readUint32("x1a8_orbitScreenBoxHalfExtentX");
  }
  /* x1b0_orbitScreenBoxHalfExtentY */
  size_t __x1b0_Count;
  if (auto v = __dna_docin.enterSubVector("x1b0_orbitScreenBoxHalfExtentY", __x1b0_Count)) {
    /* x1b0_orbitScreenBoxHalfExtentY[0] */
    x1b0_orbitScreenBoxHalfExtentY[0] = __dna_docin.readUint32("x1b0_orbitScreenBoxHalfExtentY");
    /* x1b0_orbitScreenBoxHalfExtentY[1] */
    x1b0_orbitScreenBoxHalfExtentY[1] = __dna_docin.readUint32("x1b0_orbitScreenBoxHalfExtentY");
  }
  /* x1b8_orbitScreenBoxCenterX */
  size_t __x1b8_Count;
  if (auto v = __dna_docin.enterSubVector("x1b8_orbitScreenBoxCenterX", __x1b8_Count)) {
    /* x1b8_orbitScreenBoxCenterX[0] */
    x1b8_orbitScreenBoxCenterX[0] = __dna_docin.readUint32("x1b8_orbitScreenBoxCenterX");
    /* x1b8_orbitScreenBoxCenterX[1] */
    x1b8_orbitScreenBoxCenterX[1] = __dna_docin.readUint32("x1b8_orbitScreenBoxCenterX");
  }
  /* x1c0_orbitScreenBoxCenterY */
  size_t __x1c0_Count;
  if (auto v = __dna_docin.enterSubVector("x1c0_orbitScreenBoxCenterY", __x1c0_Count)) {
    /* x1c0_orbitScreenBoxCenterY[0] */
    x1c0_orbitScreenBoxCenterY[0] = __dna_docin.readUint32("x1c0_orbitScreenBoxCenterY");
    /* x1c0_orbitScreenBoxCenterY[1] */
    x1c0_orbitScreenBoxCenterY[1] = __dna_docin.readUint32("x1c0_orbitScreenBoxCenterY");
  }
  /* x1c8_orbitZoneIdealX */
  size_t __x1c8_Count;
  if (auto v = __dna_docin.enterSubVector("x1c8_orbitZoneIdealX", __x1c8_Count)) {
    /* x1c8_orbitZoneIdealX[0] */
    x1c8_orbitZoneIdealX[0] = __dna_docin.readUint32("x1c8_orbitZoneIdealX");
    /* x1c8_orbitZoneIdealX[1] */
    x1c8_orbitZoneIdealX[1] = __dna_docin.readUint32("x1c8_orbitZoneIdealX");
  }
  /* x1d0_orbitZoneIdealY */
  size_t __x1d0_Count;
  if (auto v = __dna_docin.enterSubVector("x1d0_orbitZoneIdealY", __x1d0_Count)) {
    /* x1d0_orbitZoneIdealY[0] */
    x1d0_orbitZoneIdealY[0] = __dna_docin.readUint32("x1d0_orbitZoneIdealY");
    /* x1d0_orbitZoneIdealY[1] */
    x1d0_orbitZoneIdealY[1] = __dna_docin.readUint32("x1d0_orbitZoneIdealY");
  }
  /* x1d8_orbitNearX */
  x1d8_orbitNearX = __dna_docin.readFloat("x1d8_orbitNearX");
  /* x1dc_orbitNearZ */
  x1dc_orbitNearZ = __dna_docin.readFloat("x1dc_orbitNearZ");
  /* x1e0_ */
  x1e0_ = __dna_docin.readFloat("x1e0_");
  /* x1e4_ */
  x1e4_ = __dna_docin.readFloat("x1e4_");
  /* x1e8_orbitFixedOffsetZDiff */
  x1e8_orbitFixedOffsetZDiff = __dna_docin.readFloat("x1e8_orbitFixedOffsetZDiff");
  /* x1ec_orbitZRange */
  x1ec_orbitZRange = __dna_docin.readFloat("x1ec_orbitZRange");
  /* x1f0_ */
  x1f0_ = __dna_docin.readFloat("x1f0_");
  /* x1f4_ */
  x1f4_ = __dna_docin.readFloat("x1f4_");
  /* x1f8_ */
  x1f8_ = __dna_docin.readFloat("x1f8_");
  /* x1fc_orbitPreventionTime */
  x1fc_orbitPreventionTime = __dna_docin.readFloat("x1fc_orbitPreventionTime");
  /* x200_24_dashEnabled */
  x200_24_dashEnabled = __dna_docin.readBool("x200_24_dashEnabled");
  /* x200_25_dashOnButtonRelease */
  x200_25_dashOnButtonRelease = __dna_docin.readBool("x200_25_dashOnButtonRelease");
  /* x204_dashButtonHoldCancelTime */
  x204_dashButtonHoldCancelTime = __dna_docin.readFloat("x204_dashButtonHoldCancelTime");
  /* x208_dashStrafeInputThreshold */
  x208_dashStrafeInputThreshold = __dna_docin.readFloat("x208_dashStrafeInputThreshold");
  /* x20c_sidewaysDoubleJumpImpulse */
  x20c_sidewaysDoubleJumpImpulse = __dna_docin.readFloat("x20c_sidewaysDoubleJumpImpulse");
  /* x210_sidewaysVerticalDoubleJumpAccel */
  x210_sidewaysVerticalDoubleJumpAccel = __dna_docin.readFloat("x210_sidewaysVerticalDoubleJumpAccel");
  /* x214_sidewaysHorizontalDoubleJumpAccel */
  x214_sidewaysHorizontalDoubleJumpAccel = __dna_docin.readFloat("x214_sidewaysHorizontalDoubleJumpAccel");
  /* x218_scanningRange */
  x218_scanningRange = __dna_docin.readFloat("x218_scanningRange");
  /* x21c_24_scanRetention */
  x21c_24_scanRetention = __dna_docin.readBool("x21c_24_scanRetention");
  /* x21c_25_scanFreezesGame */
  x21c_25_scanFreezesGame = __dna_docin.readBool("x21c_25_scanFreezesGame");
  /* x21c_26_orbitWhileScanning */
  x21c_26_orbitWhileScanning = __dna_docin.readBool("x21c_26_orbitWhileScanning");
  /* x220_scanMaxTargetDistance */
  x220_scanMaxTargetDistance = __dna_docin.readFloat("x220_scanMaxTargetDistance");
  /* x224_scanMaxLockDistance */
  x224_scanMaxLockDistance = __dna_docin.readFloat("x224_scanMaxLockDistance");
  /* x2a0_orbitDistanceMax */
  x2a0_orbitDistanceMax = __dna_docin.readFloat("x2a0_orbitDistanceMax");
  /* x2a4_grappleSwingLength */
  x2a4_grappleSwingLength = __dna_docin.readFloat("x2a4_grappleSwingLength");
  /* x2a8_grappleSwingPeriod */
  x2a8_grappleSwingPeriod = __dna_docin.readFloat("x2a8_grappleSwingPeriod");
  /* x2ac_grapplePullSpeedMin */
  x2ac_grapplePullSpeedMin = __dna_docin.readFloat("x2ac_grapplePullSpeedMin");
  /* x2b0_grappleCameraSpeed */
  x2b0_grappleCameraSpeed = __dna_docin.readFloat("x2b0_grappleCameraSpeed");
  /* x2b4_maxGrappleLockedTurnAlignDistance */
  x2b4_maxGrappleLockedTurnAlignDistance = __dna_docin.readFloat("x2b4_maxGrappleLockedTurnAlignDistance");
  /* x2b8_grapplePullSpeedProportion */
  x2b8_grapplePullSpeedProportion = __dna_docin.readFloat("x2b8_grapplePullSpeedProportion");
  /* x2bc_grapplePullSpeedMax */
  x2bc_grapplePullSpeedMax = __dna_docin.readFloat("x2bc_grapplePullSpeedMax");
  /* x2c0_grappleLookCenterSpeed */
  x2c0_grappleLookCenterSpeed = __dna_docin.readFloat("x2c0_grappleLookCenterSpeed");
  /* x2c4_maxGrappleTurnSpeed */
  x2c4_maxGrappleTurnSpeed = __dna_docin.readFloat("x2c4_maxGrappleTurnSpeed");
  /* x2c8_grappleJumpForce */
  x2c8_grappleJumpForce = __dna_docin.readFloat("x2c8_grappleJumpForce");
  /* x2cc_grappleReleaseTime */
  x2cc_grappleReleaseTime = __dna_docin.readFloat("x2cc_grappleReleaseTime");
  /* x2d0_grappleJumpMode */
  x2d0_grappleJumpMode = __dna_docin.readUint32("x2d0_grappleJumpMode");
  /* x2d4_orbitReleaseBreaksGrapple */
  x2d4_orbitReleaseBreaksGrapple = __dna_docin.readBool("x2d4_orbitReleaseBreaksGrapple");
  /* x2d5_invertGrappleTurn */
  x2d5_invertGrappleTurn = __dna_docin.readBool("x2d5_invertGrappleTurn");
  /* x2d8_grappleBeamSpeed */
  x2d8_grappleBeamSpeed = __dna_docin.readFloat("x2d8_grappleBeamSpeed");
  /* x2dc_grappleBeamXWaveAmplitude */
  x2dc_grappleBeamXWaveAmplitude = __dna_docin.readFloat("x2dc_grappleBeamXWaveAmplitude");
  /* x2e0_grappleBeamZWaveAmplitude */
  x2e0_grappleBeamZWaveAmplitude = __dna_docin.readFloat("x2e0_grappleBeamZWaveAmplitude");
  /* x2e4_grappleBeamAnglePhaseDelta */
  x2e4_grappleBeamAnglePhaseDelta = __dna_docin.readFloat("x2e4_grappleBeamAnglePhaseDelta");
  /* x26c_playerHeight */
  x26c_playerHeight = __dna_docin.readFloat("x26c_playerHeight");
  /* x270_playerXYHalfExtent */
  x270_playerXYHalfExtent = __dna_docin.readFloat("x270_playerXYHalfExtent");
  /* x274_stepUpHeight */
  x274_stepUpHeight = __dna_docin.readFloat("x274_stepUpHeight");
  /* x278_stepDownHeight */
  x278_stepDownHeight = __dna_docin.readFloat("x278_stepDownHeight");
  /* x27c_playerBallHalfExtent */
  x27c_playerBallHalfExtent = __dna_docin.readFloat("x27c_playerBallHalfExtent");
  /* x280_ */
  x280_firstPersonCameraSpeed = __dna_docin.readFloat("x280_");
  /* x284_ */
  x284_ = __dna_docin.readFloat("x284_");
  /* x288_jumpCameraPitchDownStart */
  x288_jumpCameraPitchDownStart = __dna_docin.readFloat("x288_jumpCameraPitchDownStart");
  /* x28c_jumpCameraPitchDownFull */
  x28c_jumpCameraPitchDownFull = __dna_docin.readFloat("x28c_jumpCameraPitchDownFull");
  /* x290_jumpCameraPitchDownAngle */
  x290_jumpCameraPitchDownAngle = __dna_docin.readFloat("x290_jumpCameraPitchDownAngle");
  /* x294_fallCameraPitchDownStart */
  x294_fallCameraPitchDownStart = __dna_docin.readFloat("x294_fallCameraPitchDownStart");
  /* x298_fallCameraPitchDownFull */
  x298_fallCameraPitchDownFull = __dna_docin.readFloat("x298_fallCameraPitchDownFull");
  /* x29c_fallCameraPitchDownAngle */
  x29c_fallCameraPitchDownAngle = __dna_docin.readFloat("x29c_fallCameraPitchDownAngle");
  /* x2e8_ */
  x2e8_ = __dna_docin.readFloat("x2e8_");
  /* x2ec_ */
  x2ec_ = __dna_docin.readFloat("x2ec_");
  /* x2f0_ */
  x2f0_ = __dna_docin.readFloat("x2f0_");
  /* x2f4_ */
  x2f4_ = __dna_docin.readBool("x2f4_");
  /* x2f8_frozenTimeout */
  x2f8_frozenTimeout = __dna_docin.readFloat("x2f8_frozenTimeout");
  /* x2fc_iceBreakJumpCount */
  x2fc_iceBreakJumpCount = __dna_docin.readUint32("x2fc_iceBreakJumpCount");
  /* x300_variaDamageReduction */
  x300_variaDamageReduction = __dna_docin.readFloat("x300_variaDamageReduction");
  /* x304_gravityDamageReduction */
  x304_gravityDamageReduction = __dna_docin.readFloat("x304_gravityDamageReduction");
  /* x308_phazonDamageReduction */
  x308_phazonDamageReduction = __dna_docin.readFloat("x308_phazonDamageReduction");
}

template <>
void CTweakPlayer::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& __dna_docout) {
  /* x4_maxTranslationalAcceleration */
  if (auto v = __dna_docout.enterSubVector("x4_maxTranslationalAcceleration")) {
    /* x4_maxTranslationalAcceleration[0] */
    __dna_docout.writeFloat("x4_maxTranslationalAcceleration", x4_maxTranslationalAcceleration[0]);
    /* x4_maxTranslationalAcceleration[1] */
    __dna_docout.writeFloat("x4_maxTranslationalAcceleration", x4_maxTranslationalAcceleration[1]);
    /* x4_maxTranslationalAcceleration[2] */
    __dna_docout.writeFloat("x4_maxTranslationalAcceleration", x4_maxTranslationalAcceleration[2]);
    /* x4_maxTranslationalAcceleration[3] */
    __dna_docout.writeFloat("x4_maxTranslationalAcceleration", x4_maxTranslationalAcceleration[3]);
    /* x4_maxTranslationalAcceleration[4] */
    __dna_docout.writeFloat("x4_maxTranslationalAcceleration", x4_maxTranslationalAcceleration[4]);
    /* x4_maxTranslationalAcceleration[5] */
    __dna_docout.writeFloat("x4_maxTranslationalAcceleration", x4_maxTranslationalAcceleration[5]);
    /* x4_maxTranslationalAcceleration[6] */
    __dna_docout.writeFloat("x4_maxTranslationalAcceleration", x4_maxTranslationalAcceleration[6]);
    /* x4_maxTranslationalAcceleration[7] */
    __dna_docout.writeFloat("x4_maxTranslationalAcceleration", x4_maxTranslationalAcceleration[7]);
  }
  /* x24_maxRotationalAcceleration */
  if (auto v = __dna_docout.enterSubVector("x24_maxRotationalAcceleration")) {
    /* x24_maxRotationalAcceleration[0] */
    __dna_docout.writeFloat("x24_maxRotationalAcceleration", x24_maxRotationalAcceleration[0]);
    /* x24_maxRotationalAcceleration[1] */
    __dna_docout.writeFloat("x24_maxRotationalAcceleration", x24_maxRotationalAcceleration[1]);
    /* x24_maxRotationalAcceleration[2] */
    __dna_docout.writeFloat("x24_maxRotationalAcceleration", x24_maxRotationalAcceleration[2]);
    /* x24_maxRotationalAcceleration[3] */
    __dna_docout.writeFloat("x24_maxRotationalAcceleration", x24_maxRotationalAcceleration[3]);
    /* x24_maxRotationalAcceleration[4] */
    __dna_docout.writeFloat("x24_maxRotationalAcceleration", x24_maxRotationalAcceleration[4]);
    /* x24_maxRotationalAcceleration[5] */
    __dna_docout.writeFloat("x24_maxRotationalAcceleration", x24_maxRotationalAcceleration[5]);
    /* x24_maxRotationalAcceleration[6] */
    __dna_docout.writeFloat("x24_maxRotationalAcceleration", x24_maxRotationalAcceleration[6]);
    /* x24_maxRotationalAcceleration[7] */
    __dna_docout.writeFloat("x24_maxRotationalAcceleration", x24_maxRotationalAcceleration[7]);
  }
  /* x44_translationFriction */
  if (auto v = __dna_docout.enterSubVector("x44_translationFriction")) {
    /* x44_translationFriction[0] */
    __dna_docout.writeFloat("x44_translationFriction", x44_translationFriction[0]);
    /* x44_translationFriction[1] */
    __dna_docout.writeFloat("x44_translationFriction", x44_translationFriction[1]);
    /* x44_translationFriction[2] */
    __dna_docout.writeFloat("x44_translationFriction", x44_translationFriction[2]);
    /* x44_translationFriction[3] */
    __dna_docout.writeFloat("x44_translationFriction", x44_translationFriction[3]);
    /* x44_translationFriction[4] */
    __dna_docout.writeFloat("x44_translationFriction", x44_translationFriction[4]);
    /* x44_translationFriction[5] */
    __dna_docout.writeFloat("x44_translationFriction", x44_translationFriction[5]);
    /* x44_translationFriction[6] */
    __dna_docout.writeFloat("x44_translationFriction", x44_translationFriction[6]);
    /* x44_translationFriction[7] */
    __dna_docout.writeFloat("x44_translationFriction", x44_translationFriction[7]);
  }
  /* x64_rotationFriction */
  if (auto v = __dna_docout.enterSubVector("x64_rotationFriction")) {
    /* x64_rotationFriction[0] */
    __dna_docout.writeFloat("x64_rotationFriction", x64_rotationFriction[0]);
    /* x64_rotationFriction[1] */
    __dna_docout.writeFloat("x64_rotationFriction", x64_rotationFriction[1]);
    /* x64_rotationFriction[2] */
    __dna_docout.writeFloat("x64_rotationFriction", x64_rotationFriction[2]);
    /* x64_rotationFriction[3] */
    __dna_docout.writeFloat("x64_rotationFriction", x64_rotationFriction[3]);
    /* x64_rotationFriction[4] */
    __dna_docout.writeFloat("x64_rotationFriction", x64_rotationFriction[4]);
    /* x64_rotationFriction[5] */
    __dna_docout.writeFloat("x64_rotationFriction", x64_rotationFriction[5]);
    /* x64_rotationFriction[6] */
    __dna_docout.writeFloat("x64_rotationFriction", x64_rotationFriction[6]);
    /* x64_rotationFriction[7] */
    __dna_docout.writeFloat("x64_rotationFriction", x64_rotationFriction[7]);
  }
  /* x84_rotationMaxSpeed */
  if (auto v = __dna_docout.enterSubVector("x84_rotationMaxSpeed")) {
    /* x84_rotationMaxSpeed[0] */
    __dna_docout.writeFloat("x84_rotationMaxSpeed", x84_rotationMaxSpeed[0]);
    /* x84_rotationMaxSpeed[1] */
    __dna_docout.writeFloat("x84_rotationMaxSpeed", x84_rotationMaxSpeed[1]);
    /* x84_rotationMaxSpeed[2] */
    __dna_docout.writeFloat("x84_rotationMaxSpeed", x84_rotationMaxSpeed[2]);
    /* x84_rotationMaxSpeed[3] */
    __dna_docout.writeFloat("x84_rotationMaxSpeed", x84_rotationMaxSpeed[3]);
    /* x84_rotationMaxSpeed[4] */
    __dna_docout.writeFloat("x84_rotationMaxSpeed", x84_rotationMaxSpeed[4]);
    /* x84_rotationMaxSpeed[5] */
    __dna_docout.writeFloat("x84_rotationMaxSpeed", x84_rotationMaxSpeed[5]);
    /* x84_rotationMaxSpeed[6] */
    __dna_docout.writeFloat("x84_rotationMaxSpeed", x84_rotationMaxSpeed[6]);
    /* x84_rotationMaxSpeed[7] */
    __dna_docout.writeFloat("x84_rotationMaxSpeed", x84_rotationMaxSpeed[7]);
  }
  /* xa4_translationMaxSpeed */
  if (auto v = __dna_docout.enterSubVector("xa4_translationMaxSpeed")) {
    /* xa4_translationMaxSpeed[0] */
    __dna_docout.writeFloat("xa4_translationMaxSpeed", xa4_translationMaxSpeed[0]);
    /* xa4_translationMaxSpeed[1] */
    __dna_docout.writeFloat("xa4_translationMaxSpeed", xa4_translationMaxSpeed[1]);
    /* xa4_translationMaxSpeed[2] */
    __dna_docout.writeFloat("xa4_translationMaxSpeed", xa4_translationMaxSpeed[2]);
    /* xa4_translationMaxSpeed[3] */
    __dna_docout.writeFloat("xa4_translationMaxSpeed", xa4_translationMaxSpeed[3]);
    /* xa4_translationMaxSpeed[4] */
    __dna_docout.writeFloat("xa4_translationMaxSpeed", xa4_translationMaxSpeed[4]);
    /* xa4_translationMaxSpeed[5] */
    __dna_docout.writeFloat("xa4_translationMaxSpeed", xa4_translationMaxSpeed[5]);
    /* xa4_translationMaxSpeed[6] */
    __dna_docout.writeFloat("xa4_translationMaxSpeed", xa4_translationMaxSpeed[6]);
    /* xa4_translationMaxSpeed[7] */
    __dna_docout.writeFloat("xa4_translationMaxSpeed", xa4_translationMaxSpeed[7]);
  }
  /* xc4_normalGravAccel */
  __dna_docout.writeFloat("xc4_normalGravAccel", xc4_normalGravAccel);
  /* xc8_fluidGravAccel */
  __dna_docout.writeFloat("xc8_fluidGravAccel", xc8_fluidGravAccel);
  /* xcc_verticalJumpAccel */
  __dna_docout.writeFloat("xcc_verticalJumpAccel", xcc_verticalJumpAccel);
  /* xd0_horizontalJumpAccel */
  __dna_docout.writeFloat("xd0_horizontalJumpAccel", xd0_horizontalJumpAccel);
  /* xd4_verticalDoubleJumpAccel */
  __dna_docout.writeFloat("xd4_verticalDoubleJumpAccel", xd4_verticalDoubleJumpAccel);
  /* xd8_horizontalDoubleJumpAccel */
  __dna_docout.writeFloat("xd8_horizontalDoubleJumpAccel", xd8_horizontalDoubleJumpAccel);
  /* xdc_waterJumpFactor */
  __dna_docout.writeFloat("xdc_waterJumpFactor", xdc_waterJumpFactor);
  /* xe0_waterBallJumpFactor */
  __dna_docout.writeFloat("xe0_waterBallJumpFactor", xe0_waterBallJumpFactor);
  /* xe4_lavaJumpFactor */
  __dna_docout.writeFloat("xe4_lavaJumpFactor", xe4_lavaJumpFactor);
  /* xe8_lavaBallJumpFactor */
  __dna_docout.writeFloat("xe8_lavaBallJumpFactor", xe8_lavaBallJumpFactor);
  /* xec_phazonJumpFactor */
  __dna_docout.writeFloat("xec_phazonJumpFactor", xec_phazonJumpFactor);
  /* xf0_phazonBallJumpFactor */
  __dna_docout.writeFloat("xf0_phazonBallJumpFactor", xf0_phazonBallJumpFactor);
  /* xf4_allowedJumpTime */
  __dna_docout.writeFloat("xf4_allowedJumpTime", xf4_allowedJumpTime);
  /* xf8_allowedDoubleJumpTime */
  __dna_docout.writeFloat("xf8_allowedDoubleJumpTime", xf8_allowedDoubleJumpTime);
  /* xfc_minDoubleJumpWindow */
  __dna_docout.writeFloat("xfc_minDoubleJumpWindow", xfc_minDoubleJumpWindow);
  /* x100_maxDoubleJumpWindow */
  __dna_docout.writeFloat("x100_maxDoubleJumpWindow", x100_maxDoubleJumpWindow);
  /* x104_ */
  __dna_docout.writeFloat("x104_", x104_);
  /* x108_minJumpTime */
  __dna_docout.writeFloat("x108_minJumpTime", x108_minJumpTime);
  /* x10c_minDoubleJumpTime */
  __dna_docout.writeFloat("x10c_minDoubleJumpTime", x10c_minDoubleJumpTime);
  /* x110_allowedLedgeTime */
  __dna_docout.writeFloat("x110_allowedLedgeTime", x110_allowedLedgeTime);
  /* x114_doubleJumpImpulse */
  __dna_docout.writeFloat("x114_doubleJumpImpulse", x114_doubleJumpImpulse);
  /* x118_backwardsForceMultiplier */
  __dna_docout.writeFloat("x118_backwardsForceMultiplier", x118_backwardsForceMultiplier);
  /* x11c_bombJumpRadius */
  __dna_docout.writeFloat("x11c_bombJumpRadius", x11c_bombJumpRadius);
  /* x120_bombJumpHeight */
  __dna_docout.writeFloat("x120_bombJumpHeight", x120_bombJumpHeight);
  /* x124_eyeOffset */
  __dna_docout.writeFloat("x124_eyeOffset", x124_eyeOffset);
  /* x128_turnSpeedMultiplier */
  __dna_docout.writeFloat("x128_turnSpeedMultiplier", x128_turnSpeedMultiplier);
  /* x12c_freeLookTurnSpeedMultiplier */
  __dna_docout.writeFloat("x12c_freeLookTurnSpeedMultiplier", x12c_freeLookTurnSpeedMultiplier);
  /* x130_horizontalFreeLookAngleVel */
  __dna_docout.writeFloat("x130_horizontalFreeLookAngleVel", x130_horizontalFreeLookAngleVel);
  /* x134_verticalFreeLookAngleVel */
  __dna_docout.writeFloat("x134_verticalFreeLookAngleVel", x134_verticalFreeLookAngleVel);
  /* x138_freeLookSpeed */
  __dna_docout.writeFloat("x138_freeLookSpeed", x138_freeLookSpeed);
  /* x13c_freeLookSnapSpeed */
  __dna_docout.writeFloat("x13c_freeLookSnapSpeed", x13c_freeLookSnapSpeed);
  /* x140_ */
  __dna_docout.writeFloat("x140_", x140_);
  /* x144_freeLookCenteredThresholdAngle */
  __dna_docout.writeFloat("x144_freeLookCenteredThresholdAngle", x144_freeLookCenteredThresholdAngle);
  /* x148_freeLookCenteredTime */
  __dna_docout.writeFloat("x148_freeLookCenteredTime", x148_freeLookCenteredTime);
  /* x14c_freeLookDampenFactor */
  __dna_docout.writeFloat("x14c_freeLookDampenFactor", x14c_freeLookDampenFactor);
  /* x150_leftDiv */
  __dna_docout.writeFloat("x150_leftDiv", x150_leftDiv);
  /* x154_rightDiv */
  __dna_docout.writeFloat("x154_rightDiv", x154_rightDiv);
  /* x228_24_freelookTurnsPlayer */
  __dna_docout.writeBool("x228_24_freelookTurnsPlayer", x228_24_freelookTurnsPlayer);
  /* x228_25_ */
  __dna_docout.writeBool("x228_25_", x228_25_);
  /* x228_26_ */
  __dna_docout.writeBool("x228_26_", x228_26_);
  /* x228_27_moveDuringFreeLook */
  __dna_docout.writeBool("x228_27_moveDuringFreeLook", x228_27_moveDuringFreeLook);
  /* x228_28_holdButtonsForFreeLook */
  __dna_docout.writeBool("x228_28_holdButtonsForFreeLook", x228_28_holdButtonsForFreeLook);
  /* x228_29_twoButtonsForFreeLook */
  __dna_docout.writeBool("x228_29_twoButtonsForFreeLook", x228_29_twoButtonsForFreeLook);
  /* x228_30_ */
  __dna_docout.writeBool("x228_30_", x228_30_);
  /* x228_31_ */
  __dna_docout.writeBool("x228_31_", x228_31_);
  /* x229_24_ */
  __dna_docout.writeBool("x229_24_", x229_24_);
  /* x229_25_aimWhenOrbitingPoint */
  __dna_docout.writeBool("x229_25_aimWhenOrbitingPoint", x229_25_aimWhenOrbitingPoint);
  /* x229_26_stayInFreeLookWhileFiring */
  __dna_docout.writeBool("x229_26_stayInFreeLookWhileFiring", x229_26_stayInFreeLookWhileFiring);
  /* x229_27_ */
  __dna_docout.writeBool("x229_27_", x229_27_);
  /* x229_28_ */
  __dna_docout.writeBool("x229_28_", x229_28_);
  /* x229_29_orbitFixedOffset */
  __dna_docout.writeBool("x229_29_orbitFixedOffset", x229_29_orbitFixedOffset);
  /* x229_30_gunButtonTogglesHolster */
  __dna_docout.writeBool("x229_30_gunButtonTogglesHolster", x229_30_gunButtonTogglesHolster);
  /* x229_31_gunNotFiringHolstersGun */
  __dna_docout.writeBool("x229_31_gunNotFiringHolstersGun", x229_31_gunNotFiringHolstersGun);
  /* x22a_24_fallingDoubleJump */
  __dna_docout.writeBool("x22a_24_fallingDoubleJump", x22a_24_fallingDoubleJump);
  /* x22a_25_impulseDoubleJump */
  __dna_docout.writeBool("x22a_25_impulseDoubleJump", x22a_25_impulseDoubleJump);
  /* x22a_26_firingCancelsCameraPitch */
  __dna_docout.writeBool("x22a_26_firingCancelsCameraPitch", x22a_26_firingCancelsCameraPitch);
  /* x22a_27_assistedAimingIgnoreHorizontal */
  __dna_docout.writeBool("x22a_27_assistedAimingIgnoreHorizontal", x22a_27_assistedAimingIgnoreHorizontal);
  /* x22a_28_assistedAimingIgnoreVertical */
  __dna_docout.writeBool("x22a_28_assistedAimingIgnoreVertical", x22a_28_assistedAimingIgnoreVertical);
  /* x22c_ */
  __dna_docout.writeFloat("x22c_", x22c_);
  /* x230_ */
  __dna_docout.writeFloat("x230_", x230_);
  /* x234_aimMaxDistance */
  __dna_docout.writeFloat("x234_aimMaxDistance", x234_aimMaxDistance);
  /* x238_ */
  __dna_docout.writeFloat("x238_", x238_);
  /* x23c_ */
  __dna_docout.writeFloat("x23c_", x23c_);
  /* x240_ */
  __dna_docout.writeFloat("x240_", x240_);
  /* x244_ */
  __dna_docout.writeFloat("x244_", x244_);
  /* x248_ */
  __dna_docout.writeFloat("x248_", x248_);
  /* x24c_aimThresholdDistance */
  __dna_docout.writeFloat("x24c_aimThresholdDistance", x24c_aimThresholdDistance);
  /* x250_ */
  __dna_docout.writeFloat("x250_", x250_);
  /* x254_ */
  __dna_docout.writeFloat("x254_", x254_);
  /* x258_aimBoxWidth */
  __dna_docout.writeFloat("x258_aimBoxWidth", x258_aimBoxWidth);
  /* x25c_aimBoxHeight */
  __dna_docout.writeFloat("x25c_aimBoxHeight", x25c_aimBoxHeight);
  /* x260_aimTargetTimer */
  __dna_docout.writeFloat("x260_aimTargetTimer", x260_aimTargetTimer);
  /* x264_aimAssistHorizontalAngle */
  __dna_docout.writeFloat("x264_aimAssistHorizontalAngle", x264_aimAssistHorizontalAngle);
  /* x268_aimAssistVerticalAngle */
  __dna_docout.writeFloat("x268_aimAssistVerticalAngle", x268_aimAssistVerticalAngle);
  /* x158_orbitMinDistance */
  if (auto v = __dna_docout.enterSubVector("x158_orbitMinDistance")) {
    /* x158_orbitMinDistance[0] */
    __dna_docout.writeFloat("x158_orbitMinDistance", x158_orbitMinDistance[0]);
    /* x158_orbitMinDistance[1] */
    __dna_docout.writeFloat("x158_orbitMinDistance", x158_orbitMinDistance[1]);
    /* x158_orbitMinDistance[2] */
    __dna_docout.writeFloat("x158_orbitMinDistance", x158_orbitMinDistance[2]);
  }
  /* x164_orbitNormalDistance */
  if (auto v = __dna_docout.enterSubVector("x164_orbitNormalDistance")) {
    /* x164_orbitNormalDistance[0] */
    __dna_docout.writeFloat("x164_orbitNormalDistance", x164_orbitNormalDistance[0]);
    /* x164_orbitNormalDistance[1] */
    __dna_docout.writeFloat("x164_orbitNormalDistance", x164_orbitNormalDistance[1]);
    /* x164_orbitNormalDistance[2] */
    __dna_docout.writeFloat("x164_orbitNormalDistance", x164_orbitNormalDistance[2]);
  }
  /* x170_orbitMaxDistance */
  if (auto v = __dna_docout.enterSubVector("x170_orbitMaxDistance")) {
    /* x170_orbitMaxDistance[0] */
    __dna_docout.writeFloat("x170_orbitMaxDistance", x170_orbitMaxDistance[0]);
    /* x170_orbitMaxDistance[1] */
    __dna_docout.writeFloat("x170_orbitMaxDistance", x170_orbitMaxDistance[1]);
    /* x170_orbitMaxDistance[2] */
    __dna_docout.writeFloat("x170_orbitMaxDistance", x170_orbitMaxDistance[2]);
  }
  /* x17c_ */
  __dna_docout.writeFloat("x17c_", x17c_);
  /* x180_orbitModeTimer */
  __dna_docout.writeFloat("x180_orbitModeTimer", x180_orbitModeTimer);
  /* x184_orbitCameraSpeed */
  __dna_docout.writeFloat("x184_orbitCameraSpeed", x184_orbitCameraSpeed);
  /* x188_orbitUpperAngle */
  __dna_docout.writeFloat("x188_orbitUpperAngle", x188_orbitUpperAngle);
  /* x18c_orbitLowerAngle */
  __dna_docout.writeFloat("x18c_orbitLowerAngle", x18c_orbitLowerAngle);
  /* x190_orbitHorizAngle */
  __dna_docout.writeFloat("x190_orbitHorizAngle", x190_orbitHorizAngle);
  /* x194_ */
  __dna_docout.writeFloat("x194_", x194_);
  /* x198_ */
  __dna_docout.writeFloat("x198_", x198_);
  /* x19c_orbitMaxTargetDistance */
  __dna_docout.writeFloat("x19c_orbitMaxTargetDistance", x19c_orbitMaxTargetDistance);
  /* x1a0_orbitMaxLockDistance */
  __dna_docout.writeFloat("x1a0_orbitMaxLockDistance", x1a0_orbitMaxLockDistance);
  /* x1a4_orbitDistanceThreshold */
  __dna_docout.writeFloat("x1a4_orbitDistanceThreshold", x1a4_orbitDistanceThreshold);
  /* x1a8_orbitScreenBoxHalfExtentX */
  if (auto v = __dna_docout.enterSubVector("x1a8_orbitScreenBoxHalfExtentX")) {
    /* x1a8_orbitScreenBoxHalfExtentX[0] */
    __dna_docout.writeUint32("x1a8_orbitScreenBoxHalfExtentX", x1a8_orbitScreenBoxHalfExtentX[0]);
    /* x1a8_orbitScreenBoxHalfExtentX[1] */
    __dna_docout.writeUint32("x1a8_orbitScreenBoxHalfExtentX", x1a8_orbitScreenBoxHalfExtentX[1]);
  }
  /* x1b0_orbitScreenBoxHalfExtentY */
  if (auto v = __dna_docout.enterSubVector("x1b0_orbitScreenBoxHalfExtentY")) {
    /* x1b0_orbitScreenBoxHalfExtentY[0] */
    __dna_docout.writeUint32("x1b0_orbitScreenBoxHalfExtentY", x1b0_orbitScreenBoxHalfExtentY[0]);
    /* x1b0_orbitScreenBoxHalfExtentY[1] */
    __dna_docout.writeUint32("x1b0_orbitScreenBoxHalfExtentY", x1b0_orbitScreenBoxHalfExtentY[1]);
  }
  /* x1b8_orbitScreenBoxCenterX */
  if (auto v = __dna_docout.enterSubVector("x1b8_orbitScreenBoxCenterX")) {
    /* x1b8_orbitScreenBoxCenterX[0] */
    __dna_docout.writeUint32("x1b8_orbitScreenBoxCenterX", x1b8_orbitScreenBoxCenterX[0]);
    /* x1b8_orbitScreenBoxCenterX[1] */
    __dna_docout.writeUint32("x1b8_orbitScreenBoxCenterX", x1b8_orbitScreenBoxCenterX[1]);
  }
  /* x1c0_orbitScreenBoxCenterY */
  if (auto v = __dna_docout.enterSubVector("x1c0_orbitScreenBoxCenterY")) {
    /* x1c0_orbitScreenBoxCenterY[0] */
    __dna_docout.writeUint32("x1c0_orbitScreenBoxCenterY", x1c0_orbitScreenBoxCenterY[0]);
    /* x1c0_orbitScreenBoxCenterY[1] */
    __dna_docout.writeUint32("x1c0_orbitScreenBoxCenterY", x1c0_orbitScreenBoxCenterY[1]);
  }
  /* x1c8_orbitZoneIdealX */
  if (auto v = __dna_docout.enterSubVector("x1c8_orbitZoneIdealX")) {
    /* x1c8_orbitZoneIdealX[0] */
    __dna_docout.writeUint32("x1c8_orbitZoneIdealX", x1c8_orbitZoneIdealX[0]);
    /* x1c8_orbitZoneIdealX[1] */
    __dna_docout.writeUint32("x1c8_orbitZoneIdealX", x1c8_orbitZoneIdealX[1]);
  }
  /* x1d0_orbitZoneIdealY */
  if (auto v = __dna_docout.enterSubVector("x1d0_orbitZoneIdealY")) {
    /* x1d0_orbitZoneIdealY[0] */
    __dna_docout.writeUint32("x1d0_orbitZoneIdealY", x1d0_orbitZoneIdealY[0]);
    /* x1d0_orbitZoneIdealY[1] */
    __dna_docout.writeUint32("x1d0_orbitZoneIdealY", x1d0_orbitZoneIdealY[1]);
  }
  /* x1d8_orbitNearX */
  __dna_docout.writeFloat("x1d8_orbitNearX", x1d8_orbitNearX);
  /* x1dc_orbitNearZ */
  __dna_docout.writeFloat("x1dc_orbitNearZ", x1dc_orbitNearZ);
  /* x1e0_ */
  __dna_docout.writeFloat("x1e0_", x1e0_);
  /* x1e4_ */
  __dna_docout.writeFloat("x1e4_", x1e4_);
  /* x1e8_orbitFixedOffsetZDiff */
  __dna_docout.writeFloat("x1e8_orbitFixedOffsetZDiff", x1e8_orbitFixedOffsetZDiff);
  /* x1ec_orbitZRange */
  __dna_docout.writeFloat("x1ec_orbitZRange", x1ec_orbitZRange);
  /* x1f0_ */
  __dna_docout.writeFloat("x1f0_", x1f0_);
  /* x1f4_ */
  __dna_docout.writeFloat("x1f4_", x1f4_);
  /* x1f8_ */
  __dna_docout.writeFloat("x1f8_", x1f8_);
  /* x1fc_orbitPreventionTime */
  __dna_docout.writeFloat("x1fc_orbitPreventionTime", x1fc_orbitPreventionTime);
  /* x200_24_dashEnabled */
  __dna_docout.writeBool("x200_24_dashEnabled", x200_24_dashEnabled);
  /* x200_25_dashOnButtonRelease */
  __dna_docout.writeBool("x200_25_dashOnButtonRelease", x200_25_dashOnButtonRelease);
  /* x204_dashButtonHoldCancelTime */
  __dna_docout.writeFloat("x204_dashButtonHoldCancelTime", x204_dashButtonHoldCancelTime);
  /* x208_dashStrafeInputThreshold */
  __dna_docout.writeFloat("x208_dashStrafeInputThreshold", x208_dashStrafeInputThreshold);
  /* x20c_sidewaysDoubleJumpImpulse */
  __dna_docout.writeFloat("x20c_sidewaysDoubleJumpImpulse", x20c_sidewaysDoubleJumpImpulse);
  /* x210_sidewaysVerticalDoubleJumpAccel */
  __dna_docout.writeFloat("x210_sidewaysVerticalDoubleJumpAccel", x210_sidewaysVerticalDoubleJumpAccel);
  /* x214_sidewaysHorizontalDoubleJumpAccel */
  __dna_docout.writeFloat("x214_sidewaysHorizontalDoubleJumpAccel", x214_sidewaysHorizontalDoubleJumpAccel);
  /* x218_scanningRange */
  __dna_docout.writeFloat("x218_scanningRange", x218_scanningRange);
  /* x21c_24_scanRetention */
  __dna_docout.writeBool("x21c_24_scanRetention", x21c_24_scanRetention);
  /* x21c_25_scanFreezesGame */
  __dna_docout.writeBool("x21c_25_scanFreezesGame", x21c_25_scanFreezesGame);
  /* x21c_26_orbitWhileScanning */
  __dna_docout.writeBool("x21c_26_orbitWhileScanning", x21c_26_orbitWhileScanning);
  /* x220_scanMaxTargetDistance */
  __dna_docout.writeFloat("x220_scanMaxTargetDistance", x220_scanMaxTargetDistance);
  /* x224_scanMaxLockDistance */
  __dna_docout.writeFloat("x224_scanMaxLockDistance", x224_scanMaxLockDistance);
  /* x2a0_orbitDistanceMax */
  __dna_docout.writeFloat("x2a0_orbitDistanceMax", x2a0_orbitDistanceMax);
  /* x2a4_grappleSwingLength */
  __dna_docout.writeFloat("x2a4_grappleSwingLength", x2a4_grappleSwingLength);
  /* x2a8_grappleSwingPeriod */
  __dna_docout.writeFloat("x2a8_grappleSwingPeriod", x2a8_grappleSwingPeriod);
  /* x2ac_grapplePullSpeedMin */
  __dna_docout.writeFloat("x2ac_grapplePullSpeedMin", x2ac_grapplePullSpeedMin);
  /* x2b0_grappleCameraSpeed */
  __dna_docout.writeFloat("x2b0_grappleCameraSpeed", x2b0_grappleCameraSpeed);
  /* x2b4_maxGrappleLockedTurnAlignDistance */
  __dna_docout.writeFloat("x2b4_maxGrappleLockedTurnAlignDistance", x2b4_maxGrappleLockedTurnAlignDistance);
  /* x2b8_grapplePullSpeedProportion */
  __dna_docout.writeFloat("x2b8_grapplePullSpeedProportion", x2b8_grapplePullSpeedProportion);
  /* x2bc_grapplePullSpeedMax */
  __dna_docout.writeFloat("x2bc_grapplePullSpeedMax", x2bc_grapplePullSpeedMax);
  /* x2c0_grappleLookCenterSpeed */
  __dna_docout.writeFloat("x2c0_grappleLookCenterSpeed", x2c0_grappleLookCenterSpeed);
  /* x2c4_maxGrappleTurnSpeed */
  __dna_docout.writeFloat("x2c4_maxGrappleTurnSpeed", x2c4_maxGrappleTurnSpeed);
  /* x2c8_grappleJumpForce */
  __dna_docout.writeFloat("x2c8_grappleJumpForce", x2c8_grappleJumpForce);
  /* x2cc_grappleReleaseTime */
  __dna_docout.writeFloat("x2cc_grappleReleaseTime", x2cc_grappleReleaseTime);
  /* x2d0_grappleJumpMode */
  __dna_docout.writeUint32("x2d0_grappleJumpMode", x2d0_grappleJumpMode);
  /* x2d4_orbitReleaseBreaksGrapple */
  __dna_docout.writeBool("x2d4_orbitReleaseBreaksGrapple", x2d4_orbitReleaseBreaksGrapple);
  /* x2d5_invertGrappleTurn */
  __dna_docout.writeBool("x2d5_invertGrappleTurn", x2d5_invertGrappleTurn);
  /* x2d8_grappleBeamSpeed */
  __dna_docout.writeFloat("x2d8_grappleBeamSpeed", x2d8_grappleBeamSpeed);
  /* x2dc_grappleBeamXWaveAmplitude */
  __dna_docout.writeFloat("x2dc_grappleBeamXWaveAmplitude", x2dc_grappleBeamXWaveAmplitude);
  /* x2e0_grappleBeamZWaveAmplitude */
  __dna_docout.writeFloat("x2e0_grappleBeamZWaveAmplitude", x2e0_grappleBeamZWaveAmplitude);
  /* x2e4_grappleBeamAnglePhaseDelta */
  __dna_docout.writeFloat("x2e4_grappleBeamAnglePhaseDelta", x2e4_grappleBeamAnglePhaseDelta);
  /* x26c_playerHeight */
  __dna_docout.writeFloat("x26c_playerHeight", x26c_playerHeight);
  /* x270_playerXYHalfExtent */
  __dna_docout.writeFloat("x270_playerXYHalfExtent", x270_playerXYHalfExtent);
  /* x274_stepUpHeight */
  __dna_docout.writeFloat("x274_stepUpHeight", x274_stepUpHeight);
  /* x278_stepDownHeight */
  __dna_docout.writeFloat("x278_stepDownHeight", x278_stepDownHeight);
  /* x27c_playerBallHalfExtent */
  __dna_docout.writeFloat("x27c_playerBallHalfExtent", x27c_playerBallHalfExtent);
  /* x280_ */
  __dna_docout.writeFloat("x280_", x280_firstPersonCameraSpeed);
  /* x284_ */
  __dna_docout.writeFloat("x284_", x284_);
  /* x288_jumpCameraPitchDownStart */
  __dna_docout.writeFloat("x288_jumpCameraPitchDownStart", x288_jumpCameraPitchDownStart);
  /* x28c_jumpCameraPitchDownFull */
  __dna_docout.writeFloat("x28c_jumpCameraPitchDownFull", x28c_jumpCameraPitchDownFull);
  /* x290_jumpCameraPitchDownAngle */
  __dna_docout.writeFloat("x290_jumpCameraPitchDownAngle", x290_jumpCameraPitchDownAngle);
  /* x294_fallCameraPitchDownStart */
  __dna_docout.writeFloat("x294_fallCameraPitchDownStart", x294_fallCameraPitchDownStart);
  /* x298_fallCameraPitchDownFull */
  __dna_docout.writeFloat("x298_fallCameraPitchDownFull", x298_fallCameraPitchDownFull);
  /* x29c_fallCameraPitchDownAngle */
  __dna_docout.writeFloat("x29c_fallCameraPitchDownAngle", x29c_fallCameraPitchDownAngle);
  /* x2e8_ */
  __dna_docout.writeFloat("x2e8_", x2e8_);
  /* x2ec_ */
  __dna_docout.writeFloat("x2ec_", x2ec_);
  /* x2f0_ */
  __dna_docout.writeFloat("x2f0_", x2f0_);
  /* x2f4_ */
  __dna_docout.writeBool("x2f4_", x2f4_);
  /* x2f8_frozenTimeout */
  __dna_docout.writeFloat("x2f8_frozenTimeout", x2f8_frozenTimeout);
  /* x2fc_iceBreakJumpCount */
  __dna_docout.writeUint32("x2fc_iceBreakJumpCount", x2fc_iceBreakJumpCount);
  /* x300_variaDamageReduction */
  __dna_docout.writeFloat("x300_variaDamageReduction", x300_variaDamageReduction);
  /* x304_gravityDamageReduction */
  __dna_docout.writeFloat("x304_gravityDamageReduction", x304_gravityDamageReduction);
  /* x308_phazonDamageReduction */
  __dna_docout.writeFloat("x308_phazonDamageReduction", x308_phazonDamageReduction);
}

void CTweakPlayer::FixupValues() {
  x130_horizontalFreeLookAngleVel = zeus::degToRad(x130_horizontalFreeLookAngleVel);
  x134_verticalFreeLookAngleVel = zeus::degToRad(x134_verticalFreeLookAngleVel);
  x138_freeLookSpeed = zeus::degToRad(x138_freeLookSpeed);
  x13c_freeLookSnapSpeed = zeus::degToRad(x13c_freeLookSnapSpeed);
  x140_ = zeus::degToRad(x140_);
  x144_freeLookCenteredThresholdAngle = zeus::degToRad(x144_freeLookCenteredThresholdAngle);
  x23c_ = zeus::degToRad(x23c_);
  x240_ = zeus::degToRad(x240_);
  x244_ = zeus::degToRad(x244_);
  x248_ = zeus::degToRad(x248_);
  x250_ = zeus::degToRad(x250_);
  x264_aimAssistHorizontalAngle = zeus::degToRad(x264_aimAssistHorizontalAngle);
  x268_aimAssistVerticalAngle = zeus::degToRad(x268_aimAssistVerticalAngle);
  x17c_ = zeus::degToRad(x17c_);
  x184_orbitCameraSpeed = zeus::degToRad(x184_orbitCameraSpeed);
  x188_orbitUpperAngle = zeus::degToRad(x188_orbitUpperAngle);
  x18c_orbitLowerAngle = zeus::degToRad(x18c_orbitLowerAngle);
  x190_orbitHorizAngle = zeus::degToRad(x190_orbitHorizAngle);
  x194_ = zeus::degToRad(x194_);
  x198_ = zeus::degToRad(x198_);
  x1f0_ = zeus::degToRad(x1f0_);
  x1f4_ = zeus::degToRad(x1f4_);
  x2b0_grappleCameraSpeed = zeus::degToRad(x2b0_grappleCameraSpeed);
  x2c0_grappleLookCenterSpeed = zeus::degToRad(x2c0_grappleLookCenterSpeed);
  x280_firstPersonCameraSpeed = zeus::degToRad(x280_firstPersonCameraSpeed);
  x284_ = zeus::degToRad(x284_);
  x290_jumpCameraPitchDownAngle = zeus::degToRad(x290_jumpCameraPitchDownAngle);
  x29c_fallCameraPitchDownAngle = zeus::degToRad(x29c_fallCameraPitchDownAngle);
}

std::string_view CTweakPlayer::DNAType() { return "DataSpec::DNAMP1::CTweakPlayer"sv; }
template <>
void CTweakPlayer::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += 785;
}

void CTweakPlayer::_tweakListener(hecl::CVar* cv) {
  UPDATE_CVAR(MaxTranslationAccelerationNormal, cv, x4_maxTranslationalAcceleration[0]);
  UPDATE_CVAR(MaxTranslationAccelerationAir, cv, x4_maxTranslationalAcceleration[1]);
  UPDATE_CVAR(MaxTranslationAccelerationIce, cv, x4_maxTranslationalAcceleration[2]);
  UPDATE_CVAR(MaxTranslationAccelerationOrganic, cv, x4_maxTranslationalAcceleration[3]);
  UPDATE_CVAR(MaxTranslationAccelerationWater, cv, x4_maxTranslationalAcceleration[4]);
  UPDATE_CVAR(MaxTranslationAccelerationLava, cv, x4_maxTranslationalAcceleration[5]);
  UPDATE_CVAR(MaxTranslationAccelerationPhazon, cv, x4_maxTranslationalAcceleration[6]);
  UPDATE_CVAR(MaxRotationAccelerationShrubbery, cv, x24_maxRotationalAcceleration[7]);
  UPDATE_CVAR(MaxRotationAccelerationNormal, cv, x24_maxRotationalAcceleration[0]);
  UPDATE_CVAR(MaxRotationAccelerationAir, cv, x24_maxRotationalAcceleration[1]);
  UPDATE_CVAR(MaxRotationAccelerationIce, cv, x24_maxRotationalAcceleration[2]);
  UPDATE_CVAR(MaxRotationAccelerationOrganic, cv, x24_maxRotationalAcceleration[3]);
  UPDATE_CVAR(MaxRotationAccelerationWater, cv, x24_maxRotationalAcceleration[4]);
  UPDATE_CVAR(MaxRotationAccelerationLava, cv, x24_maxRotationalAcceleration[5]);
  UPDATE_CVAR(MaxRotationAccelerationPhazon, cv, x24_maxRotationalAcceleration[6]);
  UPDATE_CVAR(MaxRotationAccelerationShrubbery, cv, x24_maxRotationalAcceleration[7]);
  UPDATE_CVAR(TranslationFrictionNormal, cv, x44_translationFriction[0]);
  UPDATE_CVAR(TranslationFrictionAir, cv, x44_translationFriction[1]);
  UPDATE_CVAR(TranslationFrictionIce, cv, x44_translationFriction[2]);
  UPDATE_CVAR(TranslationFrictionOrganic, cv, x44_translationFriction[3]);
  UPDATE_CVAR(TranslationFrictionWater, cv, x44_translationFriction[4]);
  UPDATE_CVAR(TranslationFrictionLava, cv, x44_translationFriction[5]);
  UPDATE_CVAR(TranslationFrictionPhazon, cv, x44_translationFriction[6]);
  UPDATE_CVAR(TranslationFrictionShrubbery, cv, x44_translationFriction[7]);
  UPDATE_CVAR(RotationFrictionNormal, cv, x44_translationFriction[2]);
  UPDATE_CVAR(RotationFrictionIce, cv, x44_translationFriction[2]);
  UPDATE_CVAR(RotationFrictionOrganic, cv, x44_translationFriction[3]);
  UPDATE_CVAR(RotationFrictionWater, cv, x44_translationFriction[4]);
  UPDATE_CVAR(RotationFrictionLava, cv, x44_translationFriction[5]);
  UPDATE_CVAR(RotationFrictionPhazon, cv, x44_translationFriction[6]);
  UPDATE_CVAR(RotationFrictionShrubbery, cv, x44_translationFriction[7]);
  UPDATE_CVAR(RotationMaxSpeedNormal, cv, x84_rotationMaxSpeed[2]);
  UPDATE_CVAR(RotationMaxSpeedIce, cv, x84_rotationMaxSpeed[2]);
  UPDATE_CVAR(RotationMaxSpeedOrganic, cv, x84_rotationMaxSpeed[3]);
  UPDATE_CVAR(RotationMaxSpeedWater, cv, x84_rotationMaxSpeed[4]);
  UPDATE_CVAR(RotationMaxSpeedLava, cv, x84_rotationMaxSpeed[5]);
  UPDATE_CVAR(RotationMaxSpeedPhazon, cv, x84_rotationMaxSpeed[6]);
  UPDATE_CVAR(RotationMaxSpeedShrubbery, cv, x84_rotationMaxSpeed[7]);
  UPDATE_CVAR(TranslationMaxSpeedNormal, cv, xa4_translationMaxSpeed[2]);
  UPDATE_CVAR(TranslationMaxSpeedIce, cv, xa4_translationMaxSpeed[2]);
  UPDATE_CVAR(TranslationMaxSpeedOrganic, cv, xa4_translationMaxSpeed[3]);
  UPDATE_CVAR(TranslationMaxSpeedWater, cv, xa4_translationMaxSpeed[4]);
  UPDATE_CVAR(TranslationMaxSpeedLava, cv, xa4_translationMaxSpeed[5]);
  UPDATE_CVAR(TranslationMaxSpeedPhazon, cv, xa4_translationMaxSpeed[6]);
  UPDATE_CVAR(TranslationMaxSpeedShrubbery, cv, xa4_translationMaxSpeed[7]);
  UPDATE_CVAR(NormalGravityAcceleration, cv, xc4_normalGravAccel);
  UPDATE_CVAR(FluidGravityAcceleration, cv, xc8_fluidGravAccel);
  UPDATE_CVAR(VerticalJumpAcceleration, cv, xcc_verticalJumpAccel);
  UPDATE_CVAR(HorizontalJumpAcceleration, cv, xd0_horizontalJumpAccel);
  UPDATE_CVAR(VerticalDoubleJumpAcceleration, cv, xd4_verticalDoubleJumpAccel);
  UPDATE_CVAR(HorizontalDoubleJumpAcceleration, cv, xd8_horizontalDoubleJumpAccel);
  UPDATE_CVAR(WaterJumpFactor, cv, xdc_waterJumpFactor);
  UPDATE_CVAR(WaterBallJumpFactor, cv, xe0_waterBallJumpFactor);
  UPDATE_CVAR(LavaJumpFactor, cv, xe4_lavaJumpFactor);
  UPDATE_CVAR(LavaBallJumpFactor, cv, xe8_lavaBallJumpFactor);
  UPDATE_CVAR(PhazonJumpFactor, cv, xec_phazonJumpFactor);
  UPDATE_CVAR(PhazonBallJumpFactor, cv, xf0_phazonBallJumpFactor);
  UPDATE_CVAR(AllowedJumpTime, cv, xf4_allowedJumpTime);
  UPDATE_CVAR(AllowedDoubleJumpTime, cv, xf8_allowedDoubleJumpTime);
  UPDATE_CVAR(MinDoubleJumpWindow, cv, xfc_minDoubleJumpWindow);
  UPDATE_CVAR(MaxDoubleJumpWindow, cv, x100_maxDoubleJumpWindow);
  // UPDATE_CVAR(); // x104_
  UPDATE_CVAR(MinJumpTime, cv, x108_minJumpTime);
  UPDATE_CVAR(MinDoubleJumpTime, cv, x10c_minDoubleJumpTime);
  UPDATE_CVAR(AllowedLedgeTime, cv, x110_allowedLedgeTime);
  UPDATE_CVAR(DoubleJumpImpulse, cv, x114_doubleJumpImpulse);
  UPDATE_CVAR(BackwardsForceMultiplier, cv, x118_backwardsForceMultiplier);
  UPDATE_CVAR(BombJumpRadius, cv, x11c_bombJumpRadius);
  UPDATE_CVAR(BombJumpHeight, cv, x120_bombJumpHeight);
  UPDATE_CVAR(EyeOffset, cv, x124_eyeOffset);
  UPDATE_CVAR(TurnSpeedMultiplier, cv, x128_turnSpeedMultiplier);
  UPDATE_CVAR(FreeLookTurnSpeedMultiplier, cv, x12c_freeLookTurnSpeedMultiplier);
  UPDATE_CVAR(HorizontalFreeLookAngleVelocity, cv, x130_horizontalFreeLookAngleVel);
  UPDATE_CVAR(VerticalFreeLookAngleVelocity, cv, x134_verticalFreeLookAngleVel);
  UPDATE_CVAR(FreeLookSpeed, cv, x138_freeLookSpeed);
  UPDATE_CVAR(FreeLookSnapSpeed, cv, x13c_freeLookSnapSpeed);
  // UPDATE_CVAR(); // x140_
  UPDATE_CVAR(FreeLookCenteredThresholdAngle, cv, x144_freeLookCenteredThresholdAngle);
  UPDATE_CVAR(FreeLookCenteredTime, cv, x148_freeLookCenteredTime);
  UPDATE_CVAR(FreeLookDampenFactor, cv, x14c_freeLookDampenFactor);
  UPDATE_CVAR(LeftDivisor, cv, x150_leftDiv);
  UPDATE_CVAR(RightDivisor, cv, x154_rightDiv);
  UPDATE_CVAR(OrbitMinDistanceClose, cv, x158_orbitMinDistance[0]);
  UPDATE_CVAR(OrbitMinDistanceFar, cv, x158_orbitMinDistance[1]);
  UPDATE_CVAR(OrbitMinDistanceDefault, cv, x158_orbitMinDistance[2]);
  UPDATE_CVAR(OrbitNormalDistanceClose, cv, x164_orbitNormalDistance[0]);
  UPDATE_CVAR(OrbitNormalDistanceFar, cv, x164_orbitNormalDistance[1]);
  UPDATE_CVAR(OrbitNormalDistanceDefault, cv, x164_orbitNormalDistance[2]);
  UPDATE_CVAR(OrbitMaxDistanceClose, cv, x170_orbitMaxDistance[0]);
  UPDATE_CVAR(OrbitMaxDistanceFar, cv, x170_orbitMaxDistance[1]);
  UPDATE_CVAR(OrbitMaxDistanceDefault, cv, x170_orbitMaxDistance[2]);
  // UPDATE_CVAR(); // x17c_
  UPDATE_CVAR(OrbitmodeTimer, cv, x180_orbitModeTimer);
  UPDATE_CVAR(OrbitCameraSpeed, cv, x184_orbitCameraSpeed);
  UPDATE_CVAR(OrbitUpperAngle, cv, x188_orbitUpperAngle);
  UPDATE_CVAR(OrbitLowerAngle, cv, x18c_orbitLowerAngle);
  UPDATE_CVAR(OrbitHorizontalAngle, cv, x190_orbitHorizAngle);
  // UPDATE_CVAR(); // x194_
  // UPDATE_CVAR(); // x198_
  UPDATE_CVAR(OrbitMaxTargetDistance, cv, x19c_orbitMaxTargetDistance);
  UPDATE_CVAR(OrbitMaxLockDistance, cv, x1a0_orbitMaxLockDistance);
  UPDATE_CVAR(OrbitDistanceThreshold, cv, x1a4_orbitDistanceThreshold);
  UPDATE_CVAR(OrbitScreenTargetingBoxHalfExtentX, cv, x1a8_orbitScreenBoxHalfExtentX[0]);
  UPDATE_CVAR(OrbitScreenScanBoxHalfExtentX, cv, x1a8_orbitScreenBoxHalfExtentX[1]);
  UPDATE_CVAR(OrbitScreenTargetingBoxHalfExtentY, cv, x1b0_orbitScreenBoxHalfExtentY[0]);
  UPDATE_CVAR(OrbitScreenScanBoxHalfExtentY, cv, x1b0_orbitScreenBoxHalfExtentY[1]);
  UPDATE_CVAR(OrbitScreenTargetingBoxCenterX, cv, x1b8_orbitScreenBoxCenterX[0]);
  UPDATE_CVAR(OrbitScreenScanBoxCenterX, cv, x1b8_orbitScreenBoxCenterX[1]);
  UPDATE_CVAR(OrbitScreenTargetingBoxCenterY, cv, x1c0_orbitScreenBoxCenterY[0]);
  UPDATE_CVAR(OrbitScreenScanBoxCenterY, cv, x1c0_orbitScreenBoxCenterY[1]);
  UPDATE_CVAR(OrbitZoneTargetingIdealX, cv, x1c8_orbitZoneIdealX[0]);
  UPDATE_CVAR(OrbitZoneScanIdealX, cv, x1c8_orbitZoneIdealX[1]);
  UPDATE_CVAR(OrbitZoneTargetingIdealY, cv, x1d0_orbitZoneIdealY[0]);
  UPDATE_CVAR(OrbitZoneScanIdealY, cv, x1d0_orbitZoneIdealY[1]);
  UPDATE_CVAR(OrbitNearX, cv, x1d8_orbitNearX);
  UPDATE_CVAR(OrbitNearZ, cv, x1dc_orbitNearZ);
  // UPDATE_CVAR(); // x1e0_
  // UPDATE_CVAR(); // x1e4_
  UPDATE_CVAR(OrbitFixedOffsetZDiff, cv, x1e8_orbitFixedOffsetZDiff);
  UPDATE_CVAR(OrbitZRange, cv, x1ec_orbitZRange);
  // UPDATE_CVAR(); // x1f0_
  // UPDATE_CVAR(); // x1f4_
  // UPDATE_CVAR(); // x1f8_
  UPDATE_CVAR(OrbitPreventionTime, cv, x1fc_orbitPreventionTime);
  UPDATE_CVAR_BITFIELD(DashEnabled, cv, x200_24_dashEnabled);
  UPDATE_CVAR_BITFIELD(DashOnButtonRelease, cv, x200_25_dashOnButtonRelease);
  UPDATE_CVAR(DashButtonHoldCancelTime, cv, x204_dashButtonHoldCancelTime);
  UPDATE_CVAR(DashStrafeInputThreshold, cv, x208_dashStrafeInputThreshold);
  UPDATE_CVAR(SidewaysDoubleJumpImpulse, cv, x20c_sidewaysDoubleJumpImpulse);
  UPDATE_CVAR(SidewaysVerticalDoubleJumpAccel, cv, x210_sidewaysVerticalDoubleJumpAccel);
  UPDATE_CVAR(SidewaysHorizontalDoubleJumpAccel, cv, x214_sidewaysHorizontalDoubleJumpAccel);
  UPDATE_CVAR(ScanningRange, cv, x218_scanningRange);
  UPDATE_CVAR_BITFIELD(ScanRetention, cv, x21c_24_scanRetention);
  UPDATE_CVAR_BITFIELD(ScanFreezesGame, cv, x21c_25_scanFreezesGame);
  UPDATE_CVAR_BITFIELD(OrbitWhileScanning, cv, x21c_26_orbitWhileScanning);
  UPDATE_CVAR(ScanMaxTargetDistance, cv, x220_scanMaxTargetDistance);
  UPDATE_CVAR(ScanMaxLockDistance, cv, x224_scanMaxLockDistance);
  UPDATE_CVAR_BITFIELD(FreeLookTurnsPlayer, cv, x228_24_freelookTurnsPlayer);
  // UPDATE_CVAR_BITFIELD(); // x228_25_
  // UPDATE_CVAR_BITFIELD(); // x228_26_
  UPDATE_CVAR_BITFIELD(MoveDuringFreelook, cv, x228_27_moveDuringFreeLook);
  UPDATE_CVAR_BITFIELD(HoldButtonsForFreeLook, cv, x228_28_holdButtonsForFreeLook);
  // UPDATE_CVAR_BITFIELD(); // x228_30_
  // UPDATE_CVAR_BITFIELD(); // x228_31_
  // UPDATE_CVAR_BITFIELD(); // x229_24_
  UPDATE_CVAR_BITFIELD(AimWhenOrbitingPoint, cv, x229_25_aimWhenOrbitingPoint);
  UPDATE_CVAR_BITFIELD(StayInFreeLookWhileFiring, cv, x229_26_stayInFreeLookWhileFiring);
  // UPDATE_CVAR_BITFIELD(); // x229_27_
  // UPDATE_CVAR_BITFIELD(); // x229_28_
  UPDATE_CVAR_BITFIELD(OrbitFixedOffset, cv, x229_29_orbitFixedOffset);
  UPDATE_CVAR_BITFIELD(GunButtonTogglesHolster, cv, x229_30_gunButtonTogglesHolster);
  UPDATE_CVAR_BITFIELD(GunNotFiringHolstersGun, cv, x229_31_gunNotFiringHolstersGun);
  UPDATE_CVAR_BITFIELD(FallingDoubleJump, cv, x22a_24_fallingDoubleJump);
  UPDATE_CVAR_BITFIELD(ImpulseDoubleJump, cv, x22a_25_impulseDoubleJump);
  UPDATE_CVAR_BITFIELD(FiringCancelsCameraPitch, cv, x22a_26_firingCancelsCameraPitch);
  UPDATE_CVAR_BITFIELD(AssistedAimingIgnoreHorizontal, cv, x22a_27_assistedAimingIgnoreHorizontal);
  UPDATE_CVAR_BITFIELD(AssistedAimingIgnoreVertical, cv, x22a_28_assistedAimingIgnoreVertical);
  // UPDATE_CVAR(); // x22c
  // UPDATE_CVAR(); // x230_
  UPDATE_CVAR(AimMaxDistance, cv, x234_aimMaxDistance);
  // UPDATE_CVAR(); // x238_
  // UPDATE_CVAR(); // x23c_
  // UPDATE_CVAR(); // x240_
  // UPDATE_CVAR(); // x244_
  // UPDATE_CVAR(); // x248_
  UPDATE_CVAR(AimThresholdDistance, cv, x24c_aimThresholdDistance);
  // UPDATE_CVAR(); // x250_
  // UPDATE_CVAR(); // x254_
  UPDATE_CVAR(AimBoxWidth, cv, x258_aimBoxWidth);
  UPDATE_CVAR(AimBoxHeight, cv, x25c_aimBoxHeight);
  UPDATE_CVAR(AimTargetTimer, cv, x260_aimTargetTimer);
  UPDATE_CVAR(AimAssistHorizontalAngle, cv, x264_aimAssistHorizontalAngle);
  UPDATE_CVAR(AimAssistVerticalAngle, cv, x268_aimAssistVerticalAngle);
  UPDATE_CVAR(PlayerHeight, cv, x26c_playerHeight);
  UPDATE_CVAR(PlayerXYHalfExtent, cv, x270_playerXYHalfExtent);
  UPDATE_CVAR(StepUpHeight, cv, x274_stepUpHeight);
  UPDATE_CVAR(StepDownHeight, cv, x278_stepDownHeight);
  UPDATE_CVAR(PlayerBallHalfExtent, cv, x27c_playerBallHalfExtent);
  UPDATE_CVAR(FirstPersonCameraSpeed, cv, x280_firstPersonCameraSpeed);
  // UPDATE_CVAR(); // x284_
  UPDATE_CVAR(JumpCameraPitchDownStart, cv, x288_jumpCameraPitchDownStart);
  UPDATE_CVAR(JumpCameraPitchDownFull, cv, x28c_jumpCameraPitchDownFull);
  UPDATE_CVAR(JumpCameraPitchDownAngle, cv, x290_jumpCameraPitchDownAngle);
  UPDATE_CVAR(FallCameraPitchDownStart, cv, x294_fallCameraPitchDownStart);
  UPDATE_CVAR(FallCameraPitchDownFull, cv, x298_fallCameraPitchDownFull);
  UPDATE_CVAR(FallCameraPitchDownAngle, cv, x29c_fallCameraPitchDownAngle);
  UPDATE_CVAR(OrbitDistanceMax, cv, x2a0_orbitDistanceMax);
  UPDATE_CVAR(GrappleSwingLength, cv, x2a4_grappleSwingLength);
  UPDATE_CVAR(GrappleSwingPeriod, cv, x2a8_grappleSwingPeriod);
  UPDATE_CVAR(GrapplePullSpeedMin, cv, x2ac_grapplePullSpeedMin);
  UPDATE_CVAR(GrappleCameraSpeed, cv, x2b0_grappleCameraSpeed);
  UPDATE_CVAR(MaxGrappleLockedTurnAlignDistance, cv, x2b4_maxGrappleLockedTurnAlignDistance);
  UPDATE_CVAR(GrapplePullSpeedProportion, cv, x2b8_grapplePullSpeedProportion);
  UPDATE_CVAR(GrapplePullSpeedMax, cv, x2bc_grapplePullSpeedMax);
  UPDATE_CVAR(GrappleLookCenterSpeed, cv, x2c0_grappleLookCenterSpeed);
  UPDATE_CVAR(MaxGrappleTurnSpeed, cv, x2c4_maxGrappleTurnSpeed);
  UPDATE_CVAR(GrappleJumpForce, cv, x2c8_grappleJumpForce);
  UPDATE_CVAR(GrappleReleaseTime, cv, x2cc_grappleReleaseTime);
  UPDATE_CVAR(GrappleJumpMode, cv, x2d0_grappleJumpMode);
  UPDATE_CVAR(OrbitReleaseBreaksGrapple, cv, x2d4_orbitReleaseBreaksGrapple);
  UPDATE_CVAR(InvertGrappleTurn, cv, x2d5_invertGrappleTurn);
  UPDATE_CVAR(GrappleBeamSpeed, cv, x2d8_grappleBeamSpeed);
  UPDATE_CVAR(GrappleBeamXWaveAmplitude, cv, x2dc_grappleBeamXWaveAmplitude);
  UPDATE_CVAR(GrappleBeamZWaveAmplitude, cv, x2e0_grappleBeamZWaveAmplitude);
  UPDATE_CVAR(GrappleBeamAnglePhaseDelta, cv, x2e4_grappleBeamAnglePhaseDelta);
  // UPDATE_CVAR(); // x2e8_
  // UPDATE_CVAR(); // x2ec_
  // UPDATE_CVAR(); // x2f0_
  // UPDATE_CVAR(); // x2f4_
  UPDATE_CVAR(FrozenTimeout, cv, x2f8_frozenTimeout);
  UPDATE_CVAR(IceBreakJumpCount, cv, x2fc_iceBreakJumpCount);
  UPDATE_CVAR(VariaDamageReduction, cv, x300_variaDamageReduction);
  UPDATE_CVAR(GravityDamageReduction, cv, x304_gravityDamageReduction);
  UPDATE_CVAR(PhazonDamageReduction, cv, x308_phazonDamageReduction);
}

void CTweakPlayer::initCVars(hecl::CVarManager* mgr) {
  CREATE_CVAR(MaxTranslationAccelerationNormal,
              "Max translation acceleration allowed to the player under normal circumstances",
              x4_maxTranslationalAcceleration[0], skDefaultFlags);
  CREATE_CVAR(MaxTranslationAccelerationAir, "Max translation acceleration allowed to the player while in air",
              x4_maxTranslationalAcceleration[1], skDefaultFlags);
  CREATE_CVAR(MaxTranslationAccelerationIce, "Max translation acceleration allowed to the player while on ice surfaces",
              x4_maxTranslationalAcceleration[2], skDefaultFlags);
  CREATE_CVAR(MaxTranslationAccelerationOrganic,
              "Max translation acceleration allowed to the player while on organic surfaces",
              x4_maxTranslationalAcceleration[3], skDefaultFlags);
  CREATE_CVAR(MaxTranslationAccelerationWater, "Max translation acceleration allowed to the player while in water",
              x4_maxTranslationalAcceleration[4], skDefaultFlags);
  CREATE_CVAR(MaxTranslationAccelerationLava, "Max translation acceleration allowed to the player while in lava",
              x4_maxTranslationalAcceleration[5], skDefaultFlags);
  CREATE_CVAR(MaxTranslationAccelerationPhazon, "Max translation acceleration allowed to the player while in phazon",
              x4_maxTranslationalAcceleration[6], skDefaultFlags);
  CREATE_CVAR(MaxTranslationAccelerationShrubbery,
              "Max translation acceleration allowed to the player while in shrubbery",
              x4_maxTranslationalAcceleration[7], skDefaultFlags);
  CREATE_CVAR(MaxRotationAccelerationNormal,
              "Max rotation acceleration allowed to the player under normal circumstances",
              x24_maxRotationalAcceleration[0], skDefaultFlags);
  CREATE_CVAR(MaxRotationAccelerationAir, "Max rotation acceleration allowed to the player while in air",
              x24_maxRotationalAcceleration[1], skDefaultFlags);
  CREATE_CVAR(MaxRotationAccelerationIce, "Max rotation acceleration allowed to the player while on ice surfaces",
              x24_maxRotationalAcceleration[2], skDefaultFlags);
  CREATE_CVAR(MaxRotationAccelerationOrganic,
              "Max rotation acceleration allowed to the player while on organic surfaces",
              x24_maxRotationalAcceleration[3], skDefaultFlags);
  CREATE_CVAR(MaxRotationAccelerationWater, "Max rotation acceleration allowed to the player while in water",
              x24_maxRotationalAcceleration[4], skDefaultFlags);
  CREATE_CVAR(MaxRotationAccelerationLava, "Max rotation acceleration allowed to the player while in lava",
              x24_maxRotationalAcceleration[5], skDefaultFlags);
  CREATE_CVAR(MaxRotationAccelerationPhazon, "Max rotation acceleration allowed to the player while in phazon",
              x24_maxRotationalAcceleration[6], skDefaultFlags);
  CREATE_CVAR(MaxRotationAccelerationShrubbery, "Max rotation acceleration allowed to the player while in shrubbery",
              x24_maxRotationalAcceleration[7], skDefaultFlags);
  CREATE_CVAR(TranslationFrictionNormal, "Translation friction allowed to the player under normal circumstances",
              x44_translationFriction[0], skDefaultFlags);
  CREATE_CVAR(TranslationFrictionAir, "Translation friction allowed to the player while in air",
              x44_translationFriction[1], skDefaultFlags);
  CREATE_CVAR(TranslationFrictionIce, "Translation friction allowed to the player while on ice surfaces",
              x44_translationFriction[2], skDefaultFlags);
  CREATE_CVAR(TranslationFrictionOrganic, "Translation friction allowed to the player while on organic surfaces",
              x44_translationFriction[3], skDefaultFlags);
  CREATE_CVAR(TranslationFrictionWater, "Translation friction allowed to the player while in water",
              x44_translationFriction[4], skDefaultFlags);
  CREATE_CVAR(TranslationFrictionLava, "Translation friction allowed to the player while in lava",
              x44_translationFriction[5], skDefaultFlags);
  CREATE_CVAR(TranslationFrictionPhazon, "Translation friction allowed to the player while in phazon",
              x44_translationFriction[6], skDefaultFlags);
  CREATE_CVAR(TranslationFrictionShrubbery, "Translation friction allowed to the player while in shrubbery",
              x44_translationFriction[7], skDefaultFlags);
  CREATE_CVAR(RotationFrictionNormal, "Rotation friction allowed to the player under normal circumstances",
              x44_translationFriction[0], skDefaultFlags);
  CREATE_CVAR(RotationFrictionAir, "Rotation friction allowed to the player while in air", x44_translationFriction[1],
              skDefaultFlags);
  CREATE_CVAR(RotationFrictionIce, "Rotation friction allowed to the player while on ice surfaces",
              x44_translationFriction[2], skDefaultFlags);
  CREATE_CVAR(RotationFrictionOrganic, "Rotation friction allowed to the player while on organic surfaces",
              x44_translationFriction[3], skDefaultFlags);
  CREATE_CVAR(RotationFrictionWater, "Rotation friction allowed to the player while in water",
              x44_translationFriction[4], skDefaultFlags);
  CREATE_CVAR(RotationFrictionLava, "Rotation friction allowed to the player while in lava", x44_translationFriction[5],
              skDefaultFlags);
  CREATE_CVAR(RotationFrictionPhazon, "Rotation friction allowed to the player while in phazon",
              x44_translationFriction[6], skDefaultFlags);
  CREATE_CVAR(RotationFrictionShrubbery, "Rotation friction allowed to the player while in shrubbery",
              x44_translationFriction[7], skDefaultFlags);
  CREATE_CVAR(RotationMaxSpeedNormal, "Rotation max speed allowed to the player under normal circumstances",
              x84_rotationMaxSpeed[0], skDefaultFlags);
  CREATE_CVAR(RotationMaxSpeedAir, "Rotation max speed allowed to the player while in air", x84_rotationMaxSpeed[1],
              skDefaultFlags);
  CREATE_CVAR(RotationMaxSpeedIce, "Rotation max speed allowed to the player while on ice surfaces",
              x84_rotationMaxSpeed[2], skDefaultFlags);
  CREATE_CVAR(RotationMaxSpeedOrganic, "Rotation max speed allowed to the player while on organic surfaces",
              x84_rotationMaxSpeed[3], skDefaultFlags);
  CREATE_CVAR(RotationMaxSpeedWater, "Rotation max speed allowed to the player while in water", x84_rotationMaxSpeed[4],
              skDefaultFlags);
  CREATE_CVAR(RotationMaxSpeedLava, "Rotation max speed allowed to the player while in lava", x84_rotationMaxSpeed[5],
              skDefaultFlags);
  CREATE_CVAR(RotationMaxSpeedPhazon, "Rotation max speed allowed to the player while in phazon",
              x84_rotationMaxSpeed[6], skDefaultFlags);
  CREATE_CVAR(RotationMaxSpeedShrubbery, "Rotation max speed allowed to the player while in shrubbery",
              x84_rotationMaxSpeed[7], skDefaultFlags);
  CREATE_CVAR(TranslationMaxSpeedNormal, "Translation max speed allowed to the player under normal circumstances",
              xa4_translationMaxSpeed[0], skDefaultFlags);
  CREATE_CVAR(TranslationMaxSpeedNormal, "Translation max speed allowed to the player under normal circumstances",
              xa4_translationMaxSpeed[1], skDefaultFlags);
  CREATE_CVAR(TranslationMaxSpeedIce, "Translation max speed allowed to the player while on ice surfaces",
              xa4_translationMaxSpeed[2], skDefaultFlags);
  CREATE_CVAR(TranslationMaxSpeedOrganic, "Translation max speed allowed to the player while on organic surfaces",
              xa4_translationMaxSpeed[3], skDefaultFlags);
  CREATE_CVAR(TranslationMaxSpeedWater, "Translation max speed allowed to the player while in water",
              xa4_translationMaxSpeed[4], skDefaultFlags);
  CREATE_CVAR(TranslationMaxSpeedLava, "Translation max speed allowed to the player while in lava",
              xa4_translationMaxSpeed[5], skDefaultFlags);
  CREATE_CVAR(TranslationMaxSpeedPhazon, "Translation max speed allowed to the player while in phazon",
              xa4_translationMaxSpeed[6], skDefaultFlags);
  CREATE_CVAR(TranslationMaxSpeedShrubbery, "Translation max speed allowed to the player while in shrubbery",
              xa4_translationMaxSpeed[7], skDefaultFlags);
  CREATE_CVAR(NormalGravityAcceleration, "Gravity applied to the player under normal circumstances",
              xc4_normalGravAccel, skDefaultFlags);
  CREATE_CVAR(FluidGravityAcceleration, "Gravity applied to the player while in water", xc8_fluidGravAccel,
              skDefaultFlags);
  CREATE_CVAR(VerticalJumpAcceleration, "Vertical acceleration applied while jumping", xcc_verticalJumpAccel,
              skDefaultFlags);
  CREATE_CVAR(HorizontalJumpAcceleration, "Horizontal acceleration while jumping", xd0_horizontalJumpAccel,
              skDefaultFlags);
  CREATE_CVAR(VerticalDoubleJumpAcceleration, "Vertical acceleration while double jumping", xd4_verticalDoubleJumpAccel,
              skDefaultFlags);
  CREATE_CVAR(HorizontalDoubleJumpAcceleration, "Horizontal acceleration while double jumping",
              xd8_horizontalDoubleJumpAccel, skDefaultFlags);
  CREATE_CVAR(WaterJumpFactor, "Jump Factor while in water", xdc_waterJumpFactor, skDefaultFlags);
  CREATE_CVAR(WaterBallJumpFactor, "Jump Factor while morphed in water", xe0_waterBallJumpFactor, skDefaultFlags);
  CREATE_CVAR(LavaJumpFactor, "Jump Factor while in lava", xe4_lavaJumpFactor, skDefaultFlags);
  CREATE_CVAR(LavaBallJumpFactor, "Jump Factor while morphed in lava", xe8_lavaBallJumpFactor, skDefaultFlags);
  CREATE_CVAR(PhazonJumpFactor, "Jump Factor while in phazon", xec_phazonJumpFactor, skDefaultFlags);
  CREATE_CVAR(PhazonBallJumpFactor, "Jump Factor while morphed in phazon", xf0_phazonBallJumpFactor, skDefaultFlags);
  CREATE_CVAR(AllowedJumpTime, "", xf4_allowedJumpTime, skDefaultFlags);
  CREATE_CVAR(AllowedDoubleJumpTime, "", xf8_allowedDoubleJumpTime, skDefaultFlags);
  CREATE_CVAR(MinDoubleJumpWindow, "", xfc_minDoubleJumpWindow, skDefaultFlags);
  CREATE_CVAR(MaxDoubleJumpWindow, "", x100_maxDoubleJumpWindow, skDefaultFlags);
  // CREATE_CVAR(); // x104_
  CREATE_CVAR(MinJumpTime, "", x108_minJumpTime, skDefaultFlags);
  CREATE_CVAR(MinDoubleJumpTime, "", x10c_minDoubleJumpTime, skDefaultFlags);
  CREATE_CVAR(AllowedLedgeTime, "", x110_allowedLedgeTime, skDefaultFlags);
  CREATE_CVAR(DoubleJumpImpulse, "", x114_doubleJumpImpulse, skDefaultFlags);
  CREATE_CVAR(BackwardsForceMultiplier, "", x118_backwardsForceMultiplier, skDefaultFlags);
  CREATE_CVAR(BombJumpRadius, "", x11c_bombJumpRadius, skDefaultFlags);
  CREATE_CVAR(BombJumpHeight, "", x120_bombJumpHeight, skDefaultFlags);
  CREATE_CVAR(EyeOffset, "", x124_eyeOffset, skDefaultFlags);
  CREATE_CVAR(TurnSpeedMultiplier, "", x128_turnSpeedMultiplier, skDefaultFlags);
  CREATE_CVAR(FreeLookTurnSpeedMultiplier, "", x12c_freeLookTurnSpeedMultiplier, skDefaultFlags);
  CREATE_CVAR(HorizontalFreeLookAngleVelocity, "", x130_horizontalFreeLookAngleVel, skDefaultFlags);
  CREATE_CVAR(VerticalFreeLookAngleVelocity, "", x134_verticalFreeLookAngleVel, skDefaultFlags);
  CREATE_CVAR(FreeLookSpeed, "", x138_freeLookSpeed, skDefaultFlags);
  CREATE_CVAR(FreeLookSnapSpeed, "", x13c_freeLookSnapSpeed, skDefaultFlags);
  // CREATE_CVAR(); // x140_
  CREATE_CVAR(FreeLookCenteredThresholdAngle, "", x144_freeLookCenteredThresholdAngle, skDefaultFlags);
  CREATE_CVAR(FreeLookCenteredTime, "", x148_freeLookCenteredTime, skDefaultFlags);
  CREATE_CVAR(FreeLookDampenFactor, "", x14c_freeLookDampenFactor, skDefaultFlags);
  CREATE_CVAR(LeftDivisor, "", x150_leftDiv, skDefaultFlags);
  CREATE_CVAR(RightDivisor, "", x154_rightDiv, skDefaultFlags);
  CREATE_CVAR(OrbitMinDistanceClose, "", x158_orbitMinDistance[0], skDefaultFlags);
  CREATE_CVAR(OrbitMinDistanceFar, "", x158_orbitMinDistance[1], skDefaultFlags);
  CREATE_CVAR(OrbitMinDistanceDefault, "", x158_orbitMinDistance[2], skDefaultFlags);
  CREATE_CVAR(OrbitNormalDistanceClose, "", x164_orbitNormalDistance[0], skDefaultFlags);
  CREATE_CVAR(OrbitNormalDistanceFar, "", x164_orbitNormalDistance[1], skDefaultFlags);
  CREATE_CVAR(OrbitNormalDistanceDefault, "", x164_orbitNormalDistance[2], skDefaultFlags);
  CREATE_CVAR(OrbitMaxDistanceClose, "", x170_orbitMaxDistance[0], skDefaultFlags);
  CREATE_CVAR(OrbitMaxDistanceFar, "", x170_orbitMaxDistance[1], skDefaultFlags);
  CREATE_CVAR(OrbitMaxDistanceDefault, "", x170_orbitMaxDistance[2], skDefaultFlags);
  // CREATE_CVAR(); // x17c_
  CREATE_CVAR(OrbitmodeTimer, "", x180_orbitModeTimer, skDefaultFlags);
  CREATE_CVAR(OrbitCameraSpeed, "", x184_orbitCameraSpeed, skDefaultFlags);
  CREATE_CVAR(OrbitUpperAngle, "", x184_orbitCameraSpeed, skDefaultFlags);
  CREATE_CVAR(OrbitLowerAngle, "", x184_orbitCameraSpeed, skDefaultFlags);
  CREATE_CVAR(OrbitHorizontalAngle, "", x184_orbitCameraSpeed, skDefaultFlags);
  // CREATE_CVAR(); // x194_
  // CREATE_CVAR(); // x198_
  CREATE_CVAR(OrbitMaxTargetDistance, "", x19c_orbitMaxTargetDistance, skDefaultFlags);
  CREATE_CVAR(OrbitMaxLockDistance, "", x1a0_orbitMaxLockDistance, skDefaultFlags);
  CREATE_CVAR(OrbitDistanceThreshold, "", x1a4_orbitDistanceThreshold, skDefaultFlags);
  CREATE_CVAR(OrbitScreenTargetingBoxHalfExtentX, "", x1a8_orbitScreenBoxHalfExtentX[0], skDefaultFlags);
  CREATE_CVAR(OrbitScreenScanBoxHalfExtentX, "", x1a8_orbitScreenBoxHalfExtentX[1], skDefaultFlags);
  CREATE_CVAR(OrbitScreenTargetingBoxHalfExtentY, "", x1b0_orbitScreenBoxHalfExtentY[0], skDefaultFlags);
  CREATE_CVAR(OrbitScreenScanBoxHalfExtentY, "", x1b0_orbitScreenBoxHalfExtentY[1], skDefaultFlags);
  CREATE_CVAR(OrbitScreenTargetingBoxCenterX, "", x1b8_orbitScreenBoxCenterX[0], skDefaultFlags);
  CREATE_CVAR(OrbitScreenScanBoxCenterX, "", x1b8_orbitScreenBoxCenterX[1], skDefaultFlags);
  CREATE_CVAR(OrbitScreenTargetingBoxCenterY, "", x1c0_orbitScreenBoxCenterY[0], skDefaultFlags);
  CREATE_CVAR(OrbitScreenScanBoxCenterY, "", x1c0_orbitScreenBoxCenterY[1], skDefaultFlags);
  CREATE_CVAR(OrbitZoneTargetingIdealX, "", x1c8_orbitZoneIdealX[0], skDefaultFlags);
  CREATE_CVAR(OrbitZoneScanIdealX, "", x1c8_orbitZoneIdealX[1], skDefaultFlags);
  CREATE_CVAR(OrbitZoneTargetingIdealY, "", x1d0_orbitZoneIdealY[0], skDefaultFlags);
  CREATE_CVAR(OrbitZoneScanIdealY, "", x1d0_orbitZoneIdealY[1], skDefaultFlags);
  CREATE_CVAR(OrbitNearX, "", x1d8_orbitNearX, skDefaultFlags);
  CREATE_CVAR(OrbitNearZ, "", x1dc_orbitNearZ, skDefaultFlags);
  // CREATE_CVAR(); // x1e0_
  // CREATE_CVAR(); // x1e4_
  CREATE_CVAR(OrbitFixedOffsetZDiff, "", x1e8_orbitFixedOffsetZDiff, skDefaultFlags);
  CREATE_CVAR(OrbitZRange, "", x1ec_orbitZRange, skDefaultFlags);
  // CREATE_CVAR(); // x1f0_
  // CREATE_CVAR(); // x1f4_
  // CREATE_CVAR(); // x1f8_
  CREATE_CVAR(OrbitPreventionTime, "", x1fc_orbitPreventionTime, skDefaultFlags);
  CREATE_CVAR_BITFIELD(DashEnabled, "", x200_24_dashEnabled, skDefaultFlags);
  CREATE_CVAR_BITFIELD(DashOnButtonRelease, "", x200_25_dashOnButtonRelease, skDefaultFlags);
  CREATE_CVAR(DashButtonHoldCancelTime, "", x204_dashButtonHoldCancelTime, skDefaultFlags);
  CREATE_CVAR(DashStrafeInputThreshold, "", x208_dashStrafeInputThreshold, skDefaultFlags);
  CREATE_CVAR(SidewaysDoubleJumpImpulse, "", x20c_sidewaysDoubleJumpImpulse, skDefaultFlags);
  CREATE_CVAR(SidewaysVerticalDoubleJumpAccel, "", x210_sidewaysVerticalDoubleJumpAccel, skDefaultFlags);
  CREATE_CVAR(SidewaysHorizontalDoubleJumpAccel, "", x214_sidewaysHorizontalDoubleJumpAccel, skDefaultFlags);
  CREATE_CVAR(ScanningRange, "", x218_scanningRange, skDefaultFlags);
  CREATE_CVAR_BITFIELD(ScanRetention, "", x21c_24_scanRetention, skDefaultFlags);
  CREATE_CVAR_BITFIELD(ScanFreezesGame, "", x21c_25_scanFreezesGame, skDefaultFlags);
  CREATE_CVAR_BITFIELD(OrbitWhileScanning, "", x21c_26_orbitWhileScanning, skDefaultFlags);
  CREATE_CVAR(ScanMaxTargetDistance, "", x220_scanMaxTargetDistance, skDefaultFlags);
  CREATE_CVAR(ScanMaxLockDistance, "", x224_scanMaxLockDistance, skDefaultFlags);
  CREATE_CVAR_BITFIELD(FreeLookTurnsPlayer, "", x228_24_freelookTurnsPlayer, skDefaultFlags);
  // CREATE_CVAR_BITFIELD(); // x228_25_
  // CREATE_CVAR_BITFIELD(); // x228_26_
  CREATE_CVAR_BITFIELD(MoveDuringFreelook, "", x228_27_moveDuringFreeLook, skDefaultFlags);
  CREATE_CVAR_BITFIELD(HoldButtonsForFreeLook, "", x228_28_holdButtonsForFreeLook, skDefaultFlags);
  // CREATE_CVAR_BITFIELD(); // x228_30_
  // CREATE_CVAR_BITFIELD(); // x228_31_
  // CREATE_CVAR(); // x229_24_
  CREATE_CVAR_BITFIELD(AimWhenOrbitingPoint, "", x229_25_aimWhenOrbitingPoint, skDefaultFlags);
  CREATE_CVAR_BITFIELD(StayInFreeLookWhileFiring, "", x229_26_stayInFreeLookWhileFiring, skDefaultFlags);
  // CREATE_CVAR_BITFIELD(); // x229_27_
  // CREATE_CVAR_BITFIELD(); // x229_28_
  CREATE_CVAR_BITFIELD(OrbitFixedOffset, "", x229_29_orbitFixedOffset, skDefaultFlags);
  CREATE_CVAR_BITFIELD(GunButtonTogglesHolster, "", x229_30_gunButtonTogglesHolster, skDefaultFlags);
  CREATE_CVAR_BITFIELD(GunNotFiringHolstersGun, "", x229_31_gunNotFiringHolstersGun, skDefaultFlags);
  CREATE_CVAR_BITFIELD(FallingDoubleJump, "", x22a_24_fallingDoubleJump, skDefaultFlags);
  CREATE_CVAR_BITFIELD(ImpulseDoubleJump, "", x22a_25_impulseDoubleJump, skDefaultFlags);
  CREATE_CVAR_BITFIELD(FiringCancelsCameraPitch, "", x22a_26_firingCancelsCameraPitch, skDefaultFlags);
  CREATE_CVAR_BITFIELD(AssistedAimingIgnoreHorizontal, "", x22a_27_assistedAimingIgnoreHorizontal, skDefaultFlags);
  CREATE_CVAR_BITFIELD(AssistedAimingIgnoreVertical, "", x22a_28_assistedAimingIgnoreVertical, skDefaultFlags);
  // CREATE_CVAR(); // x22c
  // CREATE_CVAR(); // x230_
  CREATE_CVAR(AimMaxDistance, "", x234_aimMaxDistance, skDefaultFlags);
  // CREATE_CVAR(); // x238_
  // CREATE_CVAR(); // x23c_
  // CREATE_CVAR(); // x240_
  // CREATE_CVAR(); // x244_
  // CREATE_CVAR(); // x248_
  CREATE_CVAR(AimThresholdDistance, "", x24c_aimThresholdDistance, skDefaultFlags);
  // CREATE_CVAR(); // x250_
  // CREATE_CVAR(); // x254_
  CREATE_CVAR(AimBoxWidth, "", x258_aimBoxWidth, skDefaultFlags);
  CREATE_CVAR(AimBoxHeight, "", x25c_aimBoxHeight, skDefaultFlags);
  CREATE_CVAR(AimTargetTimer, "", x260_aimTargetTimer, skDefaultFlags);
  CREATE_CVAR(AimAssistHorizontalAngle, "", x264_aimAssistHorizontalAngle, skDefaultFlags);
  CREATE_CVAR(AimAssistVerticalAngle, "", x268_aimAssistVerticalAngle, skDefaultFlags);
  CREATE_CVAR(PlayerHeight, "", x26c_playerHeight, skDefaultFlags);
  CREATE_CVAR(PlayerXYHalfExtent, "", x270_playerXYHalfExtent, skDefaultFlags);
  CREATE_CVAR(StepUpHeight, "", x274_stepUpHeight, skDefaultFlags);
  CREATE_CVAR(StepDownHeight, "", x278_stepDownHeight, skDefaultFlags);
  CREATE_CVAR(PlayerBallHalfExtent, "", x27c_playerBallHalfExtent, skDefaultFlags);
  CREATE_CVAR(FirstPersonCameraSpeed, "", x280_firstPersonCameraSpeed, skDefaultFlags);
  // CREATE_CVAR(); // x284_
  CREATE_CVAR(JumpCameraPitchDownStart, "", x288_jumpCameraPitchDownStart, skDefaultFlags);
  CREATE_CVAR(JumpCameraPitchDownFull, "", x28c_jumpCameraPitchDownFull, skDefaultFlags);
  CREATE_CVAR(JumpCameraPitchDownAngle, "", x290_jumpCameraPitchDownAngle, skDefaultFlags);
  CREATE_CVAR(FallCameraPitchDownStart, "", x294_fallCameraPitchDownStart, skDefaultFlags);
  CREATE_CVAR(FallCameraPitchDownFull, "", x298_fallCameraPitchDownFull, skDefaultFlags);
  CREATE_CVAR(FallCameraPitchDownAngle, "", x29c_fallCameraPitchDownAngle, skDefaultFlags);
  CREATE_CVAR(OrbitDistanceMax, "", x2a0_orbitDistanceMax, skDefaultFlags);
  CREATE_CVAR(GrappleSwingLength, "", x2a4_grappleSwingLength, skDefaultFlags);
  CREATE_CVAR(GrappleSwingPeriod, "", x2a8_grappleSwingPeriod, skDefaultFlags);
  CREATE_CVAR(GrapplePullSpeedMin, "", x2ac_grapplePullSpeedMin, skDefaultFlags);
  CREATE_CVAR(GrappleCameraSpeed, "", x2b0_grappleCameraSpeed, skDefaultFlags);
  CREATE_CVAR(MaxGrappleLockedTurnAlignDistance, "", x2b4_maxGrappleLockedTurnAlignDistance, skDefaultFlags);
  CREATE_CVAR(GrapplePullSpeedProportion, "", x2b8_grapplePullSpeedProportion, skDefaultFlags);
  CREATE_CVAR(GrapplePullSpeedMax, "", x2bc_grapplePullSpeedMax, skDefaultFlags);
  CREATE_CVAR(GrappleLookCenterSpeed, "", x2c0_grappleLookCenterSpeed, skDefaultFlags);
  CREATE_CVAR(MaxGrappleTurnSpeed, "", x2c4_maxGrappleTurnSpeed, skDefaultFlags);
  CREATE_CVAR(GrappleJumpForce, "", x2c8_grappleJumpForce, skDefaultFlags);
  CREATE_CVAR(GrappleReleaseTime, "", x2cc_grappleReleaseTime, skDefaultFlags);
  CREATE_CVAR(GrappleJumpMode, "", x2d0_grappleJumpMode, skDefaultFlags);
  CREATE_CVAR(OrbitReleaseBreaksGrapple, "", x2d4_orbitReleaseBreaksGrapple, skDefaultFlags);
  CREATE_CVAR(InvertGrappleTurn, "", x2d5_invertGrappleTurn, skDefaultFlags);
  CREATE_CVAR(GrappleBeamSpeed, "", x2d8_grappleBeamSpeed, skDefaultFlags);
  CREATE_CVAR(GrappleBeamXWaveAmplitude, "", x2dc_grappleBeamXWaveAmplitude, skDefaultFlags);
  CREATE_CVAR(GrappleBeamZWaveAmplitude, "", x2e0_grappleBeamZWaveAmplitude, skDefaultFlags);
  CREATE_CVAR(GrappleBeamAnglePhaseDelta, "", x2e4_grappleBeamAnglePhaseDelta, skDefaultFlags);
  // CREATE_CVAR(); // x2e8_
  // CREATE_CVAR(); // x2ec_
  // CREATE_CVAR(); // x2f0_
  // CREATE_CVAR(); // x2f4_
  CREATE_CVAR(FrozenTimeout, "", x2f8_frozenTimeout, skDefaultFlags);
  CREATE_CVAR(IceBreakJumpCount, "", x2fc_iceBreakJumpCount, skDefaultFlags);
  CREATE_CVAR(VariaDamageReduction, "", x300_variaDamageReduction, skDefaultFlags);
  CREATE_CVAR(GravityDamageReduction, "", x304_gravityDamageReduction, skDefaultFlags);
  CREATE_CVAR(PhazonDamageReduction, "", x308_phazonDamageReduction, skDefaultFlags);
}
} // namespace DataSpec::DNAMP1
