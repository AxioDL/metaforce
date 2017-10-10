#include "CTweakBall.hpp"

namespace DataSpec
{
namespace DNAMP1
{

void CTweakBall::read(athena::io::IStreamReader& __dna_reader)
{
    /* x4_maxTranslationAcceleration[0] */
    x4_maxTranslationAcceleration[0] = __dna_reader.readFloatBig();
    /* x4_maxTranslationAcceleration[1] */
    x4_maxTranslationAcceleration[1] = __dna_reader.readFloatBig();
    /* x4_maxTranslationAcceleration[2] */
    x4_maxTranslationAcceleration[2] = __dna_reader.readFloatBig();
    /* x4_maxTranslationAcceleration[3] */
    x4_maxTranslationAcceleration[3] = __dna_reader.readFloatBig();
    /* x4_maxTranslationAcceleration[4] */
    x4_maxTranslationAcceleration[4] = __dna_reader.readFloatBig();
    /* x4_maxTranslationAcceleration[5] */
    x4_maxTranslationAcceleration[5] = __dna_reader.readFloatBig();
    /* x4_maxTranslationAcceleration[6] */
    x4_maxTranslationAcceleration[6] = __dna_reader.readFloatBig();
    /* x4_maxTranslationAcceleration[7] */
    x4_maxTranslationAcceleration[7] = __dna_reader.readFloatBig();
    /* x24_translationFriction[0] */
    x24_translationFriction[0] = __dna_reader.readFloatBig();
    /* x24_translationFriction[1] */
    x24_translationFriction[1] = __dna_reader.readFloatBig();
    /* x24_translationFriction[2] */
    x24_translationFriction[2] = __dna_reader.readFloatBig();
    /* x24_translationFriction[3] */
    x24_translationFriction[3] = __dna_reader.readFloatBig();
    /* x24_translationFriction[4] */
    x24_translationFriction[4] = __dna_reader.readFloatBig();
    /* x24_translationFriction[5] */
    x24_translationFriction[5] = __dna_reader.readFloatBig();
    /* x24_translationFriction[6] */
    x24_translationFriction[6] = __dna_reader.readFloatBig();
    /* x24_translationFriction[7] */
    x24_translationFriction[7] = __dna_reader.readFloatBig();
    /* x44_translationMaxSpeed[0] */
    x44_translationMaxSpeed[0] = __dna_reader.readFloatBig();
    /* x44_translationMaxSpeed[1] */
    x44_translationMaxSpeed[1] = __dna_reader.readFloatBig();
    /* x44_translationMaxSpeed[2] */
    x44_translationMaxSpeed[2] = __dna_reader.readFloatBig();
    /* x44_translationMaxSpeed[3] */
    x44_translationMaxSpeed[3] = __dna_reader.readFloatBig();
    /* x44_translationMaxSpeed[4] */
    x44_translationMaxSpeed[4] = __dna_reader.readFloatBig();
    /* x44_translationMaxSpeed[5] */
    x44_translationMaxSpeed[5] = __dna_reader.readFloatBig();
    /* x44_translationMaxSpeed[6] */
    x44_translationMaxSpeed[6] = __dna_reader.readFloatBig();
    /* x44_translationMaxSpeed[7] */
    x44_translationMaxSpeed[7] = __dna_reader.readFloatBig();
    /* x64_ */
    x64_ = __dna_reader.readFloatBig();
    /* x68_ */
    x68_ = __dna_reader.readFloatBig();
    /* x6c_ */
    x6c_ = __dna_reader.readFloatBig();
    /* x70_ */
    x70_ = __dna_reader.readFloatBig();
    /* xc4_ballForwardBrakingAcceleration[0] */
    xc4_ballForwardBrakingAcceleration[0] = __dna_reader.readFloatBig();
    /* xc4_ballForwardBrakingAcceleration[1] */
    xc4_ballForwardBrakingAcceleration[1] = __dna_reader.readFloatBig();
    /* xc4_ballForwardBrakingAcceleration[2] */
    xc4_ballForwardBrakingAcceleration[2] = __dna_reader.readFloatBig();
    /* xc4_ballForwardBrakingAcceleration[3] */
    xc4_ballForwardBrakingAcceleration[3] = __dna_reader.readFloatBig();
    /* xc4_ballForwardBrakingAcceleration[4] */
    xc4_ballForwardBrakingAcceleration[4] = __dna_reader.readFloatBig();
    /* xc4_ballForwardBrakingAcceleration[5] */
    xc4_ballForwardBrakingAcceleration[5] = __dna_reader.readFloatBig();
    /* xc4_ballForwardBrakingAcceleration[6] */
    xc4_ballForwardBrakingAcceleration[6] = __dna_reader.readFloatBig();
    /* xc4_ballForwardBrakingAcceleration[7] */
    xc4_ballForwardBrakingAcceleration[7] = __dna_reader.readFloatBig();
    /* xe4_ballGravity */
    xe4_ballGravity = __dna_reader.readFloatBig();
    /* xe8_ballWaterGravity */
    xe8_ballWaterGravity = __dna_reader.readFloatBig();
    /* x14c_ */
    x14c_ = __dna_reader.readFloatBig();
    /* x150_ */
    x150_ = __dna_reader.readFloatBig();
    /* x158_ */
    x158_ = __dna_reader.readFloatBig();
    /* x1dc_minimumAlignmentSpeed */
    x1dc_minimumAlignmentSpeed = __dna_reader.readFloatBig();
    /* x1e0_tireness */
    x1e0_tireness = __dna_reader.readFloatBig();
    /* x1ec_maxLeanAngle */
    x1ec_maxLeanAngle = __dna_reader.readFloatBig();
    /* x1f0_tireToMarbleThresholdSpeed */
    x1f0_tireToMarbleThresholdSpeed = __dna_reader.readFloatBig();
    /* x1f4_marbleToTireThresholdSpeed */
    x1f4_marbleToTireThresholdSpeed = __dna_reader.readFloatBig();
    /* x1f8_forceToLeanGain */
    x1f8_forceToLeanGain = __dna_reader.readFloatBig();
    /* x1fc_leanTrackingGain */
    x1fc_leanTrackingGain = __dna_reader.readFloatBig();
    /* x74_ballCameraAnglePerSecond */
    x74_ballCameraAnglePerSecond = __dna_reader.readFloatBig();
    /* x78_ballCameraOffset */
    x78_ballCameraOffset = __dna_reader.readVec3fBig();
    /* x84_ballCameraMinSpeedDistance */
    x84_ballCameraMinSpeedDistance = __dna_reader.readFloatBig();
    /* x88_ballCameraMaxSpeedDistance */
    x88_ballCameraMaxSpeedDistance = __dna_reader.readFloatBig();
    /* x8c_ballCameraBackwardsDistance */
    x8c_ballCameraBackwardsDistance = __dna_reader.readFloatBig();
    /* x90_ */
    x90_ = __dna_reader.readFloatBig();
    /* x94_ballCameraSpringConstant */
    x94_ballCameraSpringConstant = __dna_reader.readFloatBig();
    /* x98_ballCameraSpringMax */
    x98_ballCameraSpringMax = __dna_reader.readFloatBig();
    /* x9c_ballCameraSpringTardis */
    x9c_ballCameraSpringTardis = __dna_reader.readFloatBig();
    /* xa0_ballCameraCentroidSpringConstant */
    xa0_ballCameraCentroidSpringConstant = __dna_reader.readFloatBig();
    /* xa4_ballCameraCentroidSpringMax */
    xa4_ballCameraCentroidSpringMax = __dna_reader.readFloatBig();
    /* xa8_ballCameraCentroidSpringTardis */
    xa8_ballCameraCentroidSpringTardis = __dna_reader.readFloatBig();
    /* xac_ballCameraCentroidDistanceSpringConstant */
    xac_ballCameraCentroidDistanceSpringConstant = __dna_reader.readFloatBig();
    /* xb0_ballCameraCentroidDistanceSpringMax */
    xb0_ballCameraCentroidDistanceSpringMax = __dna_reader.readFloatBig();
    /* xb4_ballCameraCentroidDistanceSpringTardis */
    xb4_ballCameraCentroidDistanceSpringTardis = __dna_reader.readFloatBig();
    /* xb8_ballCameraLookAtSpringConstant */
    xb8_ballCameraLookAtSpringConstant = __dna_reader.readFloatBig();
    /* xbc_ballCameraLookAtSpringMax */
    xbc_ballCameraLookAtSpringMax = __dna_reader.readFloatBig();
    /* xc0_ballCameraLookAtSpringTardis */
    xc0_ballCameraLookAtSpringTardis = __dna_reader.readFloatBig();
    /* x154_ */
    x154_ = __dna_reader.readFloatBig();
    /* x15c_ */
    x15c_ = __dna_reader.readFloatBig();
    /* x160_ */
    x160_ = __dna_reader.readFloatBig();
    /* x164_ */
    x164_ = __dna_reader.readFloatBig();
    /* x168_ */
    x168_ = __dna_reader.readFloatBig();
    /* x16c_ */
    x16c_ = __dna_reader.readFloatBig();
    /* x170_ */
    x170_ = __dna_reader.readFloatBig();
    /* x174_ */
    x174_ = __dna_reader.readFloatBig();
    /* x178_ballCameraChaseElevation */
    x178_ballCameraChaseElevation = __dna_reader.readFloatBig();
    /* x17c_ballCameraChaseDampenAngle */
    x17c_ballCameraChaseDampenAngle = __dna_reader.readFloatBig();
    /* x180_ballCameraChaseDistance */
    x180_ballCameraChaseDistance = __dna_reader.readFloatBig();
    /* x184_ballCameraChaseYawSpeed */
    x184_ballCameraChaseYawSpeed = __dna_reader.readFloatBig();
    /* x188_ballCameraChaseAnglePerSecond */
    x188_ballCameraChaseAnglePerSecond = __dna_reader.readFloatBig();
    /* x18c_ballCameraChaseLookAtOffset */
    x18c_ballCameraChaseLookAtOffset = __dna_reader.readVec3fBig();
    /* x198_ballCameraChaseSpringConstant */
    x198_ballCameraChaseSpringConstant = __dna_reader.readFloatBig();
    /* x19c_ballCameraChaseSpringMax */
    x19c_ballCameraChaseSpringMax = __dna_reader.readFloatBig();
    /* x1a0_ballCameraChaseSpringTardis */
    x1a0_ballCameraChaseSpringTardis = __dna_reader.readFloatBig();
    /* x1a4_ballCameraBoostElevation */
    x1a4_ballCameraBoostElevation = __dna_reader.readFloatBig();
    /* x1a8_ballCameraBoostDampenAngle */
    x1a8_ballCameraBoostDampenAngle = __dna_reader.readFloatBig();
    /* x1ac_ballCameraBoostDistance */
    x1ac_ballCameraBoostDistance = __dna_reader.readFloatBig();
    /* x1b0_ballCameraBoostYawSpeed */
    x1b0_ballCameraBoostYawSpeed = __dna_reader.readFloatBig();
    /* x1b4_ballCameraBoostAnglePerSecond */
    x1b4_ballCameraBoostAnglePerSecond = __dna_reader.readFloatBig();
    /* x1b8_ballCameraBoostLookAtOffset */
    x1b8_ballCameraBoostLookAtOffset = __dna_reader.readVec3fBig();
    /* x1c4_ballCameraBoostSpringConstant */
    x1c4_ballCameraBoostSpringConstant = __dna_reader.readFloatBig();
    /* x1c8_ballCameraBoostSpringMax */
    x1c8_ballCameraBoostSpringMax = __dna_reader.readFloatBig();
    /* x1cc_ballCameraBoostSpringTardis */
    x1cc_ballCameraBoostSpringTardis = __dna_reader.readFloatBig();
    /* x1d0_ballCameraControlDistance */
    x1d0_ballCameraControlDistance = __dna_reader.readFloatBig();
    /* x1d4_ */
    x1d4_ = __dna_reader.readFloatBig();
    /* x1d8_ */
    x1d8_ = __dna_reader.readFloatBig();
    /* x1e4_leftStickDivisor */
    x1e4_leftStickDivisor = __dna_reader.readFloatBig();
    /* x1e8_rightStickDivisor */
    x1e8_rightStickDivisor = __dna_reader.readFloatBig();
    /* x200_ */
    x200_ = __dna_reader.readFloatBig();
    /* x204_ballTouchRadius */
    x204_ballTouchRadius = __dna_reader.readFloatBig();
    /* x20c_boostBallDrainTime */
    x20c_boostBallDrainTime = __dna_reader.readFloatBig();
    /* x218_boostBallMinChargeTime */
    x218_boostBallMinChargeTime = __dna_reader.readFloatBig();
    /* x21c_boostBallMinRelativeSpeedForDamage */
    x21c_boostBallMinRelativeSpeedForDamage = __dna_reader.readFloatBig();
    /* x220_boostBallChargeTime0 */
    x220_boostBallChargeTime0 = __dna_reader.readFloatBig();
    /* x224_boostBallChargeTime1 */
    x224_boostBallChargeTime1 = __dna_reader.readFloatBig();
    /* x210_boostBallMaxChargeTime */
    x228_boostBallChargeTime2 = x210_boostBallMaxChargeTime = __dna_reader.readFloatBig();
    /* x22c_boostBallIncrementalSpeed0 */
    x22c_boostBallIncrementalSpeed0 = __dna_reader.readFloatBig();
    /* x230_boostBallIncrementalSpeed1 */
    x230_boostBallIncrementalSpeed1 = __dna_reader.readFloatBig();
    /* x234_boostBallIncrementalSpeed2 */
    x234_boostBallIncrementalSpeed2 = __dna_reader.readFloatBig();
}

void CTweakBall::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* x4_maxTranslationAcceleration[0] */
    __dna_writer.writeFloatBig(x4_maxTranslationAcceleration[0]);
    /* x4_maxTranslationAcceleration[1] */
    __dna_writer.writeFloatBig(x4_maxTranslationAcceleration[1]);
    /* x4_maxTranslationAcceleration[2] */
    __dna_writer.writeFloatBig(x4_maxTranslationAcceleration[2]);
    /* x4_maxTranslationAcceleration[3] */
    __dna_writer.writeFloatBig(x4_maxTranslationAcceleration[3]);
    /* x4_maxTranslationAcceleration[4] */
    __dna_writer.writeFloatBig(x4_maxTranslationAcceleration[4]);
    /* x4_maxTranslationAcceleration[5] */
    __dna_writer.writeFloatBig(x4_maxTranslationAcceleration[5]);
    /* x4_maxTranslationAcceleration[6] */
    __dna_writer.writeFloatBig(x4_maxTranslationAcceleration[6]);
    /* x4_maxTranslationAcceleration[7] */
    __dna_writer.writeFloatBig(x4_maxTranslationAcceleration[7]);
    /* x24_translationFriction[0] */
    __dna_writer.writeFloatBig(x24_translationFriction[0]);
    /* x24_translationFriction[1] */
    __dna_writer.writeFloatBig(x24_translationFriction[1]);
    /* x24_translationFriction[2] */
    __dna_writer.writeFloatBig(x24_translationFriction[2]);
    /* x24_translationFriction[3] */
    __dna_writer.writeFloatBig(x24_translationFriction[3]);
    /* x24_translationFriction[4] */
    __dna_writer.writeFloatBig(x24_translationFriction[4]);
    /* x24_translationFriction[5] */
    __dna_writer.writeFloatBig(x24_translationFriction[5]);
    /* x24_translationFriction[6] */
    __dna_writer.writeFloatBig(x24_translationFriction[6]);
    /* x24_translationFriction[7] */
    __dna_writer.writeFloatBig(x24_translationFriction[7]);
    /* x44_translationMaxSpeed[0] */
    __dna_writer.writeFloatBig(x44_translationMaxSpeed[0]);
    /* x44_translationMaxSpeed[1] */
    __dna_writer.writeFloatBig(x44_translationMaxSpeed[1]);
    /* x44_translationMaxSpeed[2] */
    __dna_writer.writeFloatBig(x44_translationMaxSpeed[2]);
    /* x44_translationMaxSpeed[3] */
    __dna_writer.writeFloatBig(x44_translationMaxSpeed[3]);
    /* x44_translationMaxSpeed[4] */
    __dna_writer.writeFloatBig(x44_translationMaxSpeed[4]);
    /* x44_translationMaxSpeed[5] */
    __dna_writer.writeFloatBig(x44_translationMaxSpeed[5]);
    /* x44_translationMaxSpeed[6] */
    __dna_writer.writeFloatBig(x44_translationMaxSpeed[6]);
    /* x44_translationMaxSpeed[7] */
    __dna_writer.writeFloatBig(x44_translationMaxSpeed[7]);
    /* x64_ */
    __dna_writer.writeFloatBig(x64_);
    /* x68_ */
    __dna_writer.writeFloatBig(x68_);
    /* x6c_ */
    __dna_writer.writeFloatBig(x6c_);
    /* x70_ */
    __dna_writer.writeFloatBig(x70_);
    /* xc4_ballForwardBrakingAcceleration[0] */
    __dna_writer.writeFloatBig(xc4_ballForwardBrakingAcceleration[0]);
    /* xc4_ballForwardBrakingAcceleration[1] */
    __dna_writer.writeFloatBig(xc4_ballForwardBrakingAcceleration[1]);
    /* xc4_ballForwardBrakingAcceleration[2] */
    __dna_writer.writeFloatBig(xc4_ballForwardBrakingAcceleration[2]);
    /* xc4_ballForwardBrakingAcceleration[3] */
    __dna_writer.writeFloatBig(xc4_ballForwardBrakingAcceleration[3]);
    /* xc4_ballForwardBrakingAcceleration[4] */
    __dna_writer.writeFloatBig(xc4_ballForwardBrakingAcceleration[4]);
    /* xc4_ballForwardBrakingAcceleration[5] */
    __dna_writer.writeFloatBig(xc4_ballForwardBrakingAcceleration[5]);
    /* xc4_ballForwardBrakingAcceleration[6] */
    __dna_writer.writeFloatBig(xc4_ballForwardBrakingAcceleration[6]);
    /* xc4_ballForwardBrakingAcceleration[7] */
    __dna_writer.writeFloatBig(xc4_ballForwardBrakingAcceleration[7]);
    /* xe4_ballGravity */
    __dna_writer.writeFloatBig(xe4_ballGravity);
    /* xe8_ballWaterGravity */
    __dna_writer.writeFloatBig(xe8_ballWaterGravity);
    /* x14c_ */
    __dna_writer.writeFloatBig(x14c_);
    /* x150_ */
    __dna_writer.writeFloatBig(x150_);
    /* x158_ */
    __dna_writer.writeFloatBig(x158_);
    /* x1dc_minimumAlignmentSpeed */
    __dna_writer.writeFloatBig(x1dc_minimumAlignmentSpeed);
    /* x1e0_tireness */
    __dna_writer.writeFloatBig(x1e0_tireness);
    /* x1ec_maxLeanAngle */
    __dna_writer.writeFloatBig(x1ec_maxLeanAngle);
    /* x1f0_tireToMarbleThresholdSpeed */
    __dna_writer.writeFloatBig(x1f0_tireToMarbleThresholdSpeed);
    /* x1f4_marbleToTireThresholdSpeed */
    __dna_writer.writeFloatBig(x1f4_marbleToTireThresholdSpeed);
    /* x1f8_forceToLeanGain */
    __dna_writer.writeFloatBig(x1f8_forceToLeanGain);
    /* x1fc_leanTrackingGain */
    __dna_writer.writeFloatBig(x1fc_leanTrackingGain);
    /* x74_ballCameraAnglePerSecond */
    __dna_writer.writeFloatBig(x74_ballCameraAnglePerSecond);
    /* x78_ballCameraOffset */
    __dna_writer.writeVec3fBig(x78_ballCameraOffset);
    /* x84_ballCameraMinSpeedDistance */
    __dna_writer.writeFloatBig(x84_ballCameraMinSpeedDistance);
    /* x88_ballCameraMaxSpeedDistance */
    __dna_writer.writeFloatBig(x88_ballCameraMaxSpeedDistance);
    /* x8c_ballCameraBackwardsDistance */
    __dna_writer.writeFloatBig(x8c_ballCameraBackwardsDistance);
    /* x90_ */
    __dna_writer.writeFloatBig(x90_);
    /* x94_ballCameraSpringConstant */
    __dna_writer.writeFloatBig(x94_ballCameraSpringConstant);
    /* x98_ballCameraSpringMax */
    __dna_writer.writeFloatBig(x98_ballCameraSpringMax);
    /* x9c_ballCameraSpringTardis */
    __dna_writer.writeFloatBig(x9c_ballCameraSpringTardis);
    /* xa0_ballCameraCentroidSpringConstant */
    __dna_writer.writeFloatBig(xa0_ballCameraCentroidSpringConstant);
    /* xa4_ballCameraCentroidSpringMax */
    __dna_writer.writeFloatBig(xa4_ballCameraCentroidSpringMax);
    /* xa8_ballCameraCentroidSpringTardis */
    __dna_writer.writeFloatBig(xa8_ballCameraCentroidSpringTardis);
    /* xac_ballCameraCentroidDistanceSpringConstant */
    __dna_writer.writeFloatBig(xac_ballCameraCentroidDistanceSpringConstant);
    /* xb0_ballCameraCentroidDistanceSpringMax */
    __dna_writer.writeFloatBig(xb0_ballCameraCentroidDistanceSpringMax);
    /* xb4_ballCameraCentroidDistanceSpringTardis */
    __dna_writer.writeFloatBig(xb4_ballCameraCentroidDistanceSpringTardis);
    /* xb8_ballCameraLookAtSpringConstant */
    __dna_writer.writeFloatBig(xb8_ballCameraLookAtSpringConstant);
    /* xbc_ballCameraLookAtSpringMax */
    __dna_writer.writeFloatBig(xbc_ballCameraLookAtSpringMax);
    /* xc0_ballCameraLookAtSpringTardis */
    __dna_writer.writeFloatBig(xc0_ballCameraLookAtSpringTardis);
    /* x154_ */
    __dna_writer.writeFloatBig(x154_);
    /* x15c_ */
    __dna_writer.writeFloatBig(x15c_);
    /* x160_ */
    __dna_writer.writeFloatBig(x160_);
    /* x164_ */
    __dna_writer.writeFloatBig(x164_);
    /* x168_ */
    __dna_writer.writeFloatBig(x168_);
    /* x16c_ */
    __dna_writer.writeFloatBig(x16c_);
    /* x170_ */
    __dna_writer.writeFloatBig(x170_);
    /* x174_ */
    __dna_writer.writeFloatBig(x174_);
    /* x178_ballCameraChaseElevation */
    __dna_writer.writeFloatBig(x178_ballCameraChaseElevation);
    /* x17c_ballCameraChaseDampenAngle */
    __dna_writer.writeFloatBig(x17c_ballCameraChaseDampenAngle);
    /* x180_ballCameraChaseDistance */
    __dna_writer.writeFloatBig(x180_ballCameraChaseDistance);
    /* x184_ballCameraChaseYawSpeed */
    __dna_writer.writeFloatBig(x184_ballCameraChaseYawSpeed);
    /* x188_ballCameraChaseAnglePerSecond */
    __dna_writer.writeFloatBig(x188_ballCameraChaseAnglePerSecond);
    /* x18c_ballCameraChaseLookAtOffset */
    __dna_writer.writeVec3fBig(x18c_ballCameraChaseLookAtOffset);
    /* x198_ballCameraChaseSpringConstant */
    __dna_writer.writeFloatBig(x198_ballCameraChaseSpringConstant);
    /* x19c_ballCameraChaseSpringMax */
    __dna_writer.writeFloatBig(x19c_ballCameraChaseSpringMax);
    /* x1a0_ballCameraChaseSpringTardis */
    __dna_writer.writeFloatBig(x1a0_ballCameraChaseSpringTardis);
    /* x1a4_ballCameraBoostElevation */
    __dna_writer.writeFloatBig(x1a4_ballCameraBoostElevation);
    /* x1a8_ballCameraBoostDampenAngle */
    __dna_writer.writeFloatBig(x1a8_ballCameraBoostDampenAngle);
    /* x1ac_ballCameraBoostDistance */
    __dna_writer.writeFloatBig(x1ac_ballCameraBoostDistance);
    /* x1b0_ballCameraBoostYawSpeed */
    __dna_writer.writeFloatBig(x1b0_ballCameraBoostYawSpeed);
    /* x1b4_ballCameraBoostAnglePerSecond */
    __dna_writer.writeFloatBig(x1b4_ballCameraBoostAnglePerSecond);
    /* x1b8_ballCameraBoostLookAtOffset */
    __dna_writer.writeVec3fBig(x1b8_ballCameraBoostLookAtOffset);
    /* x1c4_ballCameraBoostSpringConstant */
    __dna_writer.writeFloatBig(x1c4_ballCameraBoostSpringConstant);
    /* x1c8_ballCameraBoostSpringMax */
    __dna_writer.writeFloatBig(x1c8_ballCameraBoostSpringMax);
    /* x1cc_ballCameraBoostSpringTardis */
    __dna_writer.writeFloatBig(x1cc_ballCameraBoostSpringTardis);
    /* x1d0_ballCameraControlDistance */
    __dna_writer.writeFloatBig(x1d0_ballCameraControlDistance);
    /* x1d4_ */
    __dna_writer.writeFloatBig(x1d4_);
    /* x1d8_ */
    __dna_writer.writeFloatBig(x1d8_);
    /* x1e4_leftStickDivisor */
    __dna_writer.writeFloatBig(x1e4_leftStickDivisor);
    /* x1e8_rightStickDivisor */
    __dna_writer.writeFloatBig(x1e8_rightStickDivisor);
    /* x200_ */
    __dna_writer.writeFloatBig(x200_);
    /* x204_ballTouchRadius */
    __dna_writer.writeFloatBig(x204_ballTouchRadius);
    /* x20c_boostBallDrainTime */
    __dna_writer.writeFloatBig(x20c_boostBallDrainTime);
    /* x218_boostBallMinChargeTime */
    __dna_writer.writeFloatBig(x218_boostBallMinChargeTime);
    /* x21c_boostBallMinRelativeSpeedForDamage */
    __dna_writer.writeFloatBig(x21c_boostBallMinRelativeSpeedForDamage);
    /* x220_boostBallChargeTime0 */
    __dna_writer.writeFloatBig(x220_boostBallChargeTime0);
    /* x224_boostBallChargeTime1 */
    __dna_writer.writeFloatBig(x224_boostBallChargeTime1);
    /* x210_boostBallMaxChargeTime */
    __dna_writer.writeFloatBig(x210_boostBallMaxChargeTime);
    /* x22c_boostBallIncrementalSpeed0 */
    __dna_writer.writeFloatBig(x22c_boostBallIncrementalSpeed0);
    /* x230_boostBallIncrementalSpeed1 */
    __dna_writer.writeFloatBig(x230_boostBallIncrementalSpeed1);
    /* x234_boostBallIncrementalSpeed2 */
    __dna_writer.writeFloatBig(x234_boostBallIncrementalSpeed2);
}

void CTweakBall::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* x4_maxTranslationAcceleration */
    size_t __x4_Count;
    if (auto v = __dna_docin.enterSubVector("x4_maxTranslationAcceleration", __x4_Count))
    {
    /* x4_maxTranslationAcceleration[0] */
    x4_maxTranslationAcceleration[0] = __dna_docin.readFloat("x4_maxTranslationAcceleration");
    /* x4_maxTranslationAcceleration[1] */
    x4_maxTranslationAcceleration[1] = __dna_docin.readFloat("x4_maxTranslationAcceleration");
    /* x4_maxTranslationAcceleration[2] */
    x4_maxTranslationAcceleration[2] = __dna_docin.readFloat("x4_maxTranslationAcceleration");
    /* x4_maxTranslationAcceleration[3] */
    x4_maxTranslationAcceleration[3] = __dna_docin.readFloat("x4_maxTranslationAcceleration");
    /* x4_maxTranslationAcceleration[4] */
    x4_maxTranslationAcceleration[4] = __dna_docin.readFloat("x4_maxTranslationAcceleration");
    /* x4_maxTranslationAcceleration[5] */
    x4_maxTranslationAcceleration[5] = __dna_docin.readFloat("x4_maxTranslationAcceleration");
    /* x4_maxTranslationAcceleration[6] */
    x4_maxTranslationAcceleration[6] = __dna_docin.readFloat("x4_maxTranslationAcceleration");
    /* x4_maxTranslationAcceleration[7] */
    x4_maxTranslationAcceleration[7] = __dna_docin.readFloat("x4_maxTranslationAcceleration");
    }
    /* x24_translationFriction */
    size_t __x24_Count;
    if (auto v = __dna_docin.enterSubVector("x24_translationFriction", __x24_Count))
    {
    /* x24_translationFriction[0] */
    x24_translationFriction[0] = __dna_docin.readFloat("x24_translationFriction");
    /* x24_translationFriction[1] */
    x24_translationFriction[1] = __dna_docin.readFloat("x24_translationFriction");
    /* x24_translationFriction[2] */
    x24_translationFriction[2] = __dna_docin.readFloat("x24_translationFriction");
    /* x24_translationFriction[3] */
    x24_translationFriction[3] = __dna_docin.readFloat("x24_translationFriction");
    /* x24_translationFriction[4] */
    x24_translationFriction[4] = __dna_docin.readFloat("x24_translationFriction");
    /* x24_translationFriction[5] */
    x24_translationFriction[5] = __dna_docin.readFloat("x24_translationFriction");
    /* x24_translationFriction[6] */
    x24_translationFriction[6] = __dna_docin.readFloat("x24_translationFriction");
    /* x24_translationFriction[7] */
    x24_translationFriction[7] = __dna_docin.readFloat("x24_translationFriction");
    }
    /* x44_translationMaxSpeed */
    size_t __x44_Count;
    if (auto v = __dna_docin.enterSubVector("x44_translationMaxSpeed", __x44_Count))
    {
    /* x44_translationMaxSpeed[0] */
    x44_translationMaxSpeed[0] = __dna_docin.readFloat("x44_translationMaxSpeed");
    /* x44_translationMaxSpeed[1] */
    x44_translationMaxSpeed[1] = __dna_docin.readFloat("x44_translationMaxSpeed");
    /* x44_translationMaxSpeed[2] */
    x44_translationMaxSpeed[2] = __dna_docin.readFloat("x44_translationMaxSpeed");
    /* x44_translationMaxSpeed[3] */
    x44_translationMaxSpeed[3] = __dna_docin.readFloat("x44_translationMaxSpeed");
    /* x44_translationMaxSpeed[4] */
    x44_translationMaxSpeed[4] = __dna_docin.readFloat("x44_translationMaxSpeed");
    /* x44_translationMaxSpeed[5] */
    x44_translationMaxSpeed[5] = __dna_docin.readFloat("x44_translationMaxSpeed");
    /* x44_translationMaxSpeed[6] */
    x44_translationMaxSpeed[6] = __dna_docin.readFloat("x44_translationMaxSpeed");
    /* x44_translationMaxSpeed[7] */
    x44_translationMaxSpeed[7] = __dna_docin.readFloat("x44_translationMaxSpeed");
    }
    /* x64_ */
    x64_ = __dna_docin.readFloat("x64_");
    /* x68_ */
    x68_ = __dna_docin.readFloat("x68_");
    /* x6c_ */
    x6c_ = __dna_docin.readFloat("x6c_");
    /* x70_ */
    x70_ = __dna_docin.readFloat("x70_");
    /* xc4_ballForwardBrakingAcceleration */
    size_t __xc4_Count;
    if (auto v = __dna_docin.enterSubVector("xc4_ballForwardBrakingAcceleration", __xc4_Count))
    {
    /* xc4_ballForwardBrakingAcceleration[0] */
    xc4_ballForwardBrakingAcceleration[0] = __dna_docin.readFloat("xc4_ballForwardBrakingAcceleration");
    /* xc4_ballForwardBrakingAcceleration[1] */
    xc4_ballForwardBrakingAcceleration[1] = __dna_docin.readFloat("xc4_ballForwardBrakingAcceleration");
    /* xc4_ballForwardBrakingAcceleration[2] */
    xc4_ballForwardBrakingAcceleration[2] = __dna_docin.readFloat("xc4_ballForwardBrakingAcceleration");
    /* xc4_ballForwardBrakingAcceleration[3] */
    xc4_ballForwardBrakingAcceleration[3] = __dna_docin.readFloat("xc4_ballForwardBrakingAcceleration");
    /* xc4_ballForwardBrakingAcceleration[4] */
    xc4_ballForwardBrakingAcceleration[4] = __dna_docin.readFloat("xc4_ballForwardBrakingAcceleration");
    /* xc4_ballForwardBrakingAcceleration[5] */
    xc4_ballForwardBrakingAcceleration[5] = __dna_docin.readFloat("xc4_ballForwardBrakingAcceleration");
    /* xc4_ballForwardBrakingAcceleration[6] */
    xc4_ballForwardBrakingAcceleration[6] = __dna_docin.readFloat("xc4_ballForwardBrakingAcceleration");
    /* xc4_ballForwardBrakingAcceleration[7] */
    xc4_ballForwardBrakingAcceleration[7] = __dna_docin.readFloat("xc4_ballForwardBrakingAcceleration");
    }
    /* xe4_ballGravity */
    xe4_ballGravity = __dna_docin.readFloat("xe4_ballGravity");
    /* xe8_ballWaterGravity */
    xe8_ballWaterGravity = __dna_docin.readFloat("xe8_ballWaterGravity");
    /* x14c_ */
    x14c_ = __dna_docin.readFloat("x14c_");
    /* x150_ */
    x150_ = __dna_docin.readFloat("x150_");
    /* x158_ */
    x158_ = __dna_docin.readFloat("x158_");
    /* x1dc_minimumAlignmentSpeed */
    x1dc_minimumAlignmentSpeed = __dna_docin.readFloat("x1dc_minimumAlignmentSpeed");
    /* x1e0_tireness */
    x1e0_tireness = __dna_docin.readFloat("x1e0_tireness");
    /* x1ec_maxLeanAngle */
    x1ec_maxLeanAngle = __dna_docin.readFloat("x1ec_maxLeanAngle");
    /* x1f0_tireToMarbleThresholdSpeed */
    x1f0_tireToMarbleThresholdSpeed = __dna_docin.readFloat("x1f0_tireToMarbleThresholdSpeed");
    /* x1f4_marbleToTireThresholdSpeed */
    x1f4_marbleToTireThresholdSpeed = __dna_docin.readFloat("x1f4_marbleToTireThresholdSpeed");
    /* x1f8_forceToLeanGain */
    x1f8_forceToLeanGain = __dna_docin.readFloat("x1f8_forceToLeanGain");
    /* x1fc_leanTrackingGain */
    x1fc_leanTrackingGain = __dna_docin.readFloat("x1fc_leanTrackingGain");
    /* x74_ballCameraAnglePerSecond */
    x74_ballCameraAnglePerSecond = __dna_docin.readFloat("x74_ballCameraAnglePerSecond");
    /* x78_ballCameraOffset */
    x78_ballCameraOffset = __dna_docin.readVec3f("x78_ballCameraOffset");
    /* x84_ballCameraMinSpeedDistance */
    x84_ballCameraMinSpeedDistance = __dna_docin.readFloat("x84_ballCameraMinSpeedDistance");
    /* x88_ballCameraMaxSpeedDistance */
    x88_ballCameraMaxSpeedDistance = __dna_docin.readFloat("x88_ballCameraMaxSpeedDistance");
    /* x8c_ballCameraBackwardsDistance */
    x8c_ballCameraBackwardsDistance = __dna_docin.readFloat("x8c_ballCameraBackwardsDistance");
    /* x90_ */
    x90_ = __dna_docin.readFloat("x90_");
    /* x94_ballCameraSpringConstant */
    x94_ballCameraSpringConstant = __dna_docin.readFloat("x94_ballCameraSpringConstant");
    /* x98_ballCameraSpringMax */
    x98_ballCameraSpringMax = __dna_docin.readFloat("x98_ballCameraSpringMax");
    /* x9c_ballCameraSpringTardis */
    x9c_ballCameraSpringTardis = __dna_docin.readFloat("x9c_ballCameraSpringTardis");
    /* xa0_ballCameraCentroidSpringConstant */
    xa0_ballCameraCentroidSpringConstant = __dna_docin.readFloat("xa0_ballCameraCentroidSpringConstant");
    /* xa4_ballCameraCentroidSpringMax */
    xa4_ballCameraCentroidSpringMax = __dna_docin.readFloat("xa4_ballCameraCentroidSpringMax");
    /* xa8_ballCameraCentroidSpringTardis */
    xa8_ballCameraCentroidSpringTardis = __dna_docin.readFloat("xa8_ballCameraCentroidSpringTardis");
    /* xac_ballCameraCentroidDistanceSpringConstant */
    xac_ballCameraCentroidDistanceSpringConstant = __dna_docin.readFloat("xac_ballCameraCentroidDistanceSpringConstant");
    /* xb0_ballCameraCentroidDistanceSpringMax */
    xb0_ballCameraCentroidDistanceSpringMax = __dna_docin.readFloat("xb0_ballCameraCentroidDistanceSpringMax");
    /* xb4_ballCameraCentroidDistanceSpringTardis */
    xb4_ballCameraCentroidDistanceSpringTardis = __dna_docin.readFloat("xb4_ballCameraCentroidDistanceSpringTardis");
    /* xb8_ballCameraLookAtSpringConstant */
    xb8_ballCameraLookAtSpringConstant = __dna_docin.readFloat("xb8_ballCameraLookAtSpringConstant");
    /* xbc_ballCameraLookAtSpringMax */
    xbc_ballCameraLookAtSpringMax = __dna_docin.readFloat("xbc_ballCameraLookAtSpringMax");
    /* xc0_ballCameraLookAtSpringTardis */
    xc0_ballCameraLookAtSpringTardis = __dna_docin.readFloat("xc0_ballCameraLookAtSpringTardis");
    /* x154_ */
    x154_ = __dna_docin.readFloat("x154_");
    /* x15c_ */
    x15c_ = __dna_docin.readFloat("x15c_");
    /* x160_ */
    x160_ = __dna_docin.readFloat("x160_");
    /* x164_ */
    x164_ = __dna_docin.readFloat("x164_");
    /* x168_ */
    x168_ = __dna_docin.readFloat("x168_");
    /* x16c_ */
    x16c_ = __dna_docin.readFloat("x16c_");
    /* x170_ */
    x170_ = __dna_docin.readFloat("x170_");
    /* x174_ */
    x174_ = __dna_docin.readFloat("x174_");
    /* x178_ballCameraChaseElevation */
    x178_ballCameraChaseElevation = __dna_docin.readFloat("x178_ballCameraChaseElevation");
    /* x17c_ballCameraChaseDampenAngle */
    x17c_ballCameraChaseDampenAngle = __dna_docin.readFloat("x17c_ballCameraChaseDampenAngle");
    /* x180_ballCameraChaseDistance */
    x180_ballCameraChaseDistance = __dna_docin.readFloat("x180_ballCameraChaseDistance");
    /* x184_ballCameraChaseYawSpeed */
    x184_ballCameraChaseYawSpeed = __dna_docin.readFloat("x184_ballCameraChaseYawSpeed");
    /* x188_ballCameraChaseAnglePerSecond */
    x188_ballCameraChaseAnglePerSecond = __dna_docin.readFloat("x188_ballCameraChaseAnglePerSecond");
    /* x18c_ballCameraChaseLookAtOffset */
    x18c_ballCameraChaseLookAtOffset = __dna_docin.readVec3f("x18c_ballCameraChaseLookAtOffset");
    /* x198_ballCameraChaseSpringConstant */
    x198_ballCameraChaseSpringConstant = __dna_docin.readFloat("x198_ballCameraChaseSpringConstant");
    /* x19c_ballCameraChaseSpringMax */
    x19c_ballCameraChaseSpringMax = __dna_docin.readFloat("x19c_ballCameraChaseSpringMax");
    /* x1a0_ballCameraChaseSpringTardis */
    x1a0_ballCameraChaseSpringTardis = __dna_docin.readFloat("x1a0_ballCameraChaseSpringTardis");
    /* x1a4_ballCameraBoostElevation */
    x1a4_ballCameraBoostElevation = __dna_docin.readFloat("x1a4_ballCameraBoostElevation");
    /* x1a8_ballCameraBoostDampenAngle */
    x1a8_ballCameraBoostDampenAngle = __dna_docin.readFloat("x1a8_ballCameraBoostDampenAngle");
    /* x1ac_ballCameraBoostDistance */
    x1ac_ballCameraBoostDistance = __dna_docin.readFloat("x1ac_ballCameraBoostDistance");
    /* x1b0_ballCameraBoostYawSpeed */
    x1b0_ballCameraBoostYawSpeed = __dna_docin.readFloat("x1b0_ballCameraBoostYawSpeed");
    /* x1b4_ballCameraBoostAnglePerSecond */
    x1b4_ballCameraBoostAnglePerSecond = __dna_docin.readFloat("x1b4_ballCameraBoostAnglePerSecond");
    /* x1b8_ballCameraBoostLookAtOffset */
    x1b8_ballCameraBoostLookAtOffset = __dna_docin.readVec3f("x1b8_ballCameraBoostLookAtOffset");
    /* x1c4_ballCameraBoostSpringConstant */
    x1c4_ballCameraBoostSpringConstant = __dna_docin.readFloat("x1c4_ballCameraBoostSpringConstant");
    /* x1c8_ballCameraBoostSpringMax */
    x1c8_ballCameraBoostSpringMax = __dna_docin.readFloat("x1c8_ballCameraBoostSpringMax");
    /* x1cc_ballCameraBoostSpringTardis */
    x1cc_ballCameraBoostSpringTardis = __dna_docin.readFloat("x1cc_ballCameraBoostSpringTardis");
    /* x1d0_ballCameraControlDistance */
    x1d0_ballCameraControlDistance = __dna_docin.readFloat("x1d0_ballCameraControlDistance");
    /* x1d4_ */
    x1d4_ = __dna_docin.readFloat("x1d4_");
    /* x1d8_ */
    x1d8_ = __dna_docin.readFloat("x1d8_");
    /* x1e4_leftStickDivisor */
    x1e4_leftStickDivisor = __dna_docin.readFloat("x1e4_leftStickDivisor");
    /* x1e8_rightStickDivisor */
    x1e8_rightStickDivisor = __dna_docin.readFloat("x1e8_rightStickDivisor");
    /* x200_ */
    x200_ = __dna_docin.readFloat("x200_");
    /* x204_ballTouchRadius */
    x204_ballTouchRadius = __dna_docin.readFloat("x204_ballTouchRadius");
    /* x20c_boostBallDrainTime */
    x20c_boostBallDrainTime = __dna_docin.readFloat("x20c_boostBallDrainTime");
    /* x218_boostBallMinChargeTime */
    x218_boostBallMinChargeTime = __dna_docin.readFloat("x218_boostBallMinChargeTime");
    /* x21c_boostBallMinRelativeSpeedForDamage */
    x21c_boostBallMinRelativeSpeedForDamage = __dna_docin.readFloat("x21c_boostBallMinRelativeSpeedForDamage");
    /* x220_boostBallChargeTime0 */
    x220_boostBallChargeTime0 = __dna_docin.readFloat("x220_boostBallChargeTime0");
    /* x224_boostBallChargeTime1 */
    x224_boostBallChargeTime1 = __dna_docin.readFloat("x224_boostBallChargeTime1");
    /* x210_boostBallMaxChargeTime */
    x228_boostBallChargeTime2 = x210_boostBallMaxChargeTime = __dna_docin.readFloat("x210_boostBallMaxChargeTime");
    /* x22c_boostBallIncrementalSpeed0 */
    x22c_boostBallIncrementalSpeed0 = __dna_docin.readFloat("x22c_boostBallIncrementalSpeed0");
    /* x230_boostBallIncrementalSpeed1 */
    x230_boostBallIncrementalSpeed1 = __dna_docin.readFloat("x230_boostBallIncrementalSpeed1");
    /* x234_boostBallIncrementalSpeed2 */
    x234_boostBallIncrementalSpeed2 = __dna_docin.readFloat("x234_boostBallIncrementalSpeed2");
}

void CTweakBall::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* x4_maxTranslationAcceleration */
    if (auto v = __dna_docout.enterSubVector("x4_maxTranslationAcceleration"))
    {
    /* x4_maxTranslationAcceleration[0] */
    __dna_docout.writeFloat("x4_maxTranslationAcceleration", x4_maxTranslationAcceleration[0]);
    /* x4_maxTranslationAcceleration[1] */
    __dna_docout.writeFloat("x4_maxTranslationAcceleration", x4_maxTranslationAcceleration[1]);
    /* x4_maxTranslationAcceleration[2] */
    __dna_docout.writeFloat("x4_maxTranslationAcceleration", x4_maxTranslationAcceleration[2]);
    /* x4_maxTranslationAcceleration[3] */
    __dna_docout.writeFloat("x4_maxTranslationAcceleration", x4_maxTranslationAcceleration[3]);
    /* x4_maxTranslationAcceleration[4] */
    __dna_docout.writeFloat("x4_maxTranslationAcceleration", x4_maxTranslationAcceleration[4]);
    /* x4_maxTranslationAcceleration[5] */
    __dna_docout.writeFloat("x4_maxTranslationAcceleration", x4_maxTranslationAcceleration[5]);
    /* x4_maxTranslationAcceleration[6] */
    __dna_docout.writeFloat("x4_maxTranslationAcceleration", x4_maxTranslationAcceleration[6]);
    /* x4_maxTranslationAcceleration[7] */
    __dna_docout.writeFloat("x4_maxTranslationAcceleration", x4_maxTranslationAcceleration[7]);
    }
    /* x24_translationFriction */
    if (auto v = __dna_docout.enterSubVector("x24_translationFriction"))
    {
    /* x24_translationFriction[0] */
    __dna_docout.writeFloat("x24_translationFriction", x24_translationFriction[0]);
    /* x24_translationFriction[1] */
    __dna_docout.writeFloat("x24_translationFriction", x24_translationFriction[1]);
    /* x24_translationFriction[2] */
    __dna_docout.writeFloat("x24_translationFriction", x24_translationFriction[2]);
    /* x24_translationFriction[3] */
    __dna_docout.writeFloat("x24_translationFriction", x24_translationFriction[3]);
    /* x24_translationFriction[4] */
    __dna_docout.writeFloat("x24_translationFriction", x24_translationFriction[4]);
    /* x24_translationFriction[5] */
    __dna_docout.writeFloat("x24_translationFriction", x24_translationFriction[5]);
    /* x24_translationFriction[6] */
    __dna_docout.writeFloat("x24_translationFriction", x24_translationFriction[6]);
    /* x24_translationFriction[7] */
    __dna_docout.writeFloat("x24_translationFriction", x24_translationFriction[7]);
    }
    /* x44_translationMaxSpeed */
    if (auto v = __dna_docout.enterSubVector("x44_translationMaxSpeed"))
    {
    /* x44_translationMaxSpeed[0] */
    __dna_docout.writeFloat("x44_translationMaxSpeed", x44_translationMaxSpeed[0]);
    /* x44_translationMaxSpeed[1] */
    __dna_docout.writeFloat("x44_translationMaxSpeed", x44_translationMaxSpeed[1]);
    /* x44_translationMaxSpeed[2] */
    __dna_docout.writeFloat("x44_translationMaxSpeed", x44_translationMaxSpeed[2]);
    /* x44_translationMaxSpeed[3] */
    __dna_docout.writeFloat("x44_translationMaxSpeed", x44_translationMaxSpeed[3]);
    /* x44_translationMaxSpeed[4] */
    __dna_docout.writeFloat("x44_translationMaxSpeed", x44_translationMaxSpeed[4]);
    /* x44_translationMaxSpeed[5] */
    __dna_docout.writeFloat("x44_translationMaxSpeed", x44_translationMaxSpeed[5]);
    /* x44_translationMaxSpeed[6] */
    __dna_docout.writeFloat("x44_translationMaxSpeed", x44_translationMaxSpeed[6]);
    /* x44_translationMaxSpeed[7] */
    __dna_docout.writeFloat("x44_translationMaxSpeed", x44_translationMaxSpeed[7]);
    }
    /* x64_ */
    __dna_docout.writeFloat("x64_", x64_);
    /* x68_ */
    __dna_docout.writeFloat("x68_", x68_);
    /* x6c_ */
    __dna_docout.writeFloat("x6c_", x6c_);
    /* x70_ */
    __dna_docout.writeFloat("x70_", x70_);
    /* xc4_ballForwardBrakingAcceleration */
    if (auto v = __dna_docout.enterSubVector("xc4_ballForwardBrakingAcceleration"))
    {
    /* xc4_ballForwardBrakingAcceleration[0] */
    __dna_docout.writeFloat("xc4_ballForwardBrakingAcceleration", xc4_ballForwardBrakingAcceleration[0]);
    /* xc4_ballForwardBrakingAcceleration[1] */
    __dna_docout.writeFloat("xc4_ballForwardBrakingAcceleration", xc4_ballForwardBrakingAcceleration[1]);
    /* xc4_ballForwardBrakingAcceleration[2] */
    __dna_docout.writeFloat("xc4_ballForwardBrakingAcceleration", xc4_ballForwardBrakingAcceleration[2]);
    /* xc4_ballForwardBrakingAcceleration[3] */
    __dna_docout.writeFloat("xc4_ballForwardBrakingAcceleration", xc4_ballForwardBrakingAcceleration[3]);
    /* xc4_ballForwardBrakingAcceleration[4] */
    __dna_docout.writeFloat("xc4_ballForwardBrakingAcceleration", xc4_ballForwardBrakingAcceleration[4]);
    /* xc4_ballForwardBrakingAcceleration[5] */
    __dna_docout.writeFloat("xc4_ballForwardBrakingAcceleration", xc4_ballForwardBrakingAcceleration[5]);
    /* xc4_ballForwardBrakingAcceleration[6] */
    __dna_docout.writeFloat("xc4_ballForwardBrakingAcceleration", xc4_ballForwardBrakingAcceleration[6]);
    /* xc4_ballForwardBrakingAcceleration[7] */
    __dna_docout.writeFloat("xc4_ballForwardBrakingAcceleration", xc4_ballForwardBrakingAcceleration[7]);
    }
    /* xe4_ballGravity */
    __dna_docout.writeFloat("xe4_ballGravity", xe4_ballGravity);
    /* xe8_ballWaterGravity */
    __dna_docout.writeFloat("xe8_ballWaterGravity", xe8_ballWaterGravity);
    /* x14c_ */
    __dna_docout.writeFloat("x14c_", x14c_);
    /* x150_ */
    __dna_docout.writeFloat("x150_", x150_);
    /* x158_ */
    __dna_docout.writeFloat("x158_", x158_);
    /* x1dc_minimumAlignmentSpeed */
    __dna_docout.writeFloat("x1dc_minimumAlignmentSpeed", x1dc_minimumAlignmentSpeed);
    /* x1e0_tireness */
    __dna_docout.writeFloat("x1e0_tireness", x1e0_tireness);
    /* x1ec_maxLeanAngle */
    __dna_docout.writeFloat("x1ec_maxLeanAngle", x1ec_maxLeanAngle);
    /* x1f0_tireToMarbleThresholdSpeed */
    __dna_docout.writeFloat("x1f0_tireToMarbleThresholdSpeed", x1f0_tireToMarbleThresholdSpeed);
    /* x1f4_marbleToTireThresholdSpeed */
    __dna_docout.writeFloat("x1f4_marbleToTireThresholdSpeed", x1f4_marbleToTireThresholdSpeed);
    /* x1f8_forceToLeanGain */
    __dna_docout.writeFloat("x1f8_forceToLeanGain", x1f8_forceToLeanGain);
    /* x1fc_leanTrackingGain */
    __dna_docout.writeFloat("x1fc_leanTrackingGain", x1fc_leanTrackingGain);
    /* x74_ballCameraAnglePerSecond */
    __dna_docout.writeFloat("x74_ballCameraAnglePerSecond", x74_ballCameraAnglePerSecond);
    /* x78_ballCameraOffset */
    __dna_docout.writeVec3f("x78_ballCameraOffset", x78_ballCameraOffset);
    /* x84_ballCameraMinSpeedDistance */
    __dna_docout.writeFloat("x84_ballCameraMinSpeedDistance", x84_ballCameraMinSpeedDistance);
    /* x88_ballCameraMaxSpeedDistance */
    __dna_docout.writeFloat("x88_ballCameraMaxSpeedDistance", x88_ballCameraMaxSpeedDistance);
    /* x8c_ballCameraBackwardsDistance */
    __dna_docout.writeFloat("x8c_ballCameraBackwardsDistance", x8c_ballCameraBackwardsDistance);
    /* x90_ */
    __dna_docout.writeFloat("x90_", x90_);
    /* x94_ballCameraSpringConstant */
    __dna_docout.writeFloat("x94_ballCameraSpringConstant", x94_ballCameraSpringConstant);
    /* x98_ballCameraSpringMax */
    __dna_docout.writeFloat("x98_ballCameraSpringMax", x98_ballCameraSpringMax);
    /* x9c_ballCameraSpringTardis */
    __dna_docout.writeFloat("x9c_ballCameraSpringTardis", x9c_ballCameraSpringTardis);
    /* xa0_ballCameraCentroidSpringConstant */
    __dna_docout.writeFloat("xa0_ballCameraCentroidSpringConstant", xa0_ballCameraCentroidSpringConstant);
    /* xa4_ballCameraCentroidSpringMax */
    __dna_docout.writeFloat("xa4_ballCameraCentroidSpringMax", xa4_ballCameraCentroidSpringMax);
    /* xa8_ballCameraCentroidSpringTardis */
    __dna_docout.writeFloat("xa8_ballCameraCentroidSpringTardis", xa8_ballCameraCentroidSpringTardis);
    /* xac_ballCameraCentroidDistanceSpringConstant */
    __dna_docout.writeFloat("xac_ballCameraCentroidDistanceSpringConstant", xac_ballCameraCentroidDistanceSpringConstant);
    /* xb0_ballCameraCentroidDistanceSpringMax */
    __dna_docout.writeFloat("xb0_ballCameraCentroidDistanceSpringMax", xb0_ballCameraCentroidDistanceSpringMax);
    /* xb4_ballCameraCentroidDistanceSpringTardis */
    __dna_docout.writeFloat("xb4_ballCameraCentroidDistanceSpringTardis", xb4_ballCameraCentroidDistanceSpringTardis);
    /* xb8_ballCameraLookAtSpringConstant */
    __dna_docout.writeFloat("xb8_ballCameraLookAtSpringConstant", xb8_ballCameraLookAtSpringConstant);
    /* xbc_ballCameraLookAtSpringMax */
    __dna_docout.writeFloat("xbc_ballCameraLookAtSpringMax", xbc_ballCameraLookAtSpringMax);
    /* xc0_ballCameraLookAtSpringTardis */
    __dna_docout.writeFloat("xc0_ballCameraLookAtSpringTardis", xc0_ballCameraLookAtSpringTardis);
    /* x154_ */
    __dna_docout.writeFloat("x154_", x154_);
    /* x15c_ */
    __dna_docout.writeFloat("x15c_", x15c_);
    /* x160_ */
    __dna_docout.writeFloat("x160_", x160_);
    /* x164_ */
    __dna_docout.writeFloat("x164_", x164_);
    /* x168_ */
    __dna_docout.writeFloat("x168_", x168_);
    /* x16c_ */
    __dna_docout.writeFloat("x16c_", x16c_);
    /* x170_ */
    __dna_docout.writeFloat("x170_", x170_);
    /* x174_ */
    __dna_docout.writeFloat("x174_", x174_);
    /* x178_ballCameraChaseElevation */
    __dna_docout.writeFloat("x178_ballCameraChaseElevation", x178_ballCameraChaseElevation);
    /* x17c_ballCameraChaseDampenAngle */
    __dna_docout.writeFloat("x17c_ballCameraChaseDampenAngle", x17c_ballCameraChaseDampenAngle);
    /* x180_ballCameraChaseDistance */
    __dna_docout.writeFloat("x180_ballCameraChaseDistance", x180_ballCameraChaseDistance);
    /* x184_ballCameraChaseYawSpeed */
    __dna_docout.writeFloat("x184_ballCameraChaseYawSpeed", x184_ballCameraChaseYawSpeed);
    /* x188_ballCameraChaseAnglePerSecond */
    __dna_docout.writeFloat("x188_ballCameraChaseAnglePerSecond", x188_ballCameraChaseAnglePerSecond);
    /* x18c_ballCameraChaseLookAtOffset */
    __dna_docout.writeVec3f("x18c_ballCameraChaseLookAtOffset", x18c_ballCameraChaseLookAtOffset);
    /* x198_ballCameraChaseSpringConstant */
    __dna_docout.writeFloat("x198_ballCameraChaseSpringConstant", x198_ballCameraChaseSpringConstant);
    /* x19c_ballCameraChaseSpringMax */
    __dna_docout.writeFloat("x19c_ballCameraChaseSpringMax", x19c_ballCameraChaseSpringMax);
    /* x1a0_ballCameraChaseSpringTardis */
    __dna_docout.writeFloat("x1a0_ballCameraChaseSpringTardis", x1a0_ballCameraChaseSpringTardis);
    /* x1a4_ballCameraBoostElevation */
    __dna_docout.writeFloat("x1a4_ballCameraBoostElevation", x1a4_ballCameraBoostElevation);
    /* x1a8_ballCameraBoostDampenAngle */
    __dna_docout.writeFloat("x1a8_ballCameraBoostDampenAngle", x1a8_ballCameraBoostDampenAngle);
    /* x1ac_ballCameraBoostDistance */
    __dna_docout.writeFloat("x1ac_ballCameraBoostDistance", x1ac_ballCameraBoostDistance);
    /* x1b0_ballCameraBoostYawSpeed */
    __dna_docout.writeFloat("x1b0_ballCameraBoostYawSpeed", x1b0_ballCameraBoostYawSpeed);
    /* x1b4_ballCameraBoostAnglePerSecond */
    __dna_docout.writeFloat("x1b4_ballCameraBoostAnglePerSecond", x1b4_ballCameraBoostAnglePerSecond);
    /* x1b8_ballCameraBoostLookAtOffset */
    __dna_docout.writeVec3f("x1b8_ballCameraBoostLookAtOffset", x1b8_ballCameraBoostLookAtOffset);
    /* x1c4_ballCameraBoostSpringConstant */
    __dna_docout.writeFloat("x1c4_ballCameraBoostSpringConstant", x1c4_ballCameraBoostSpringConstant);
    /* x1c8_ballCameraBoostSpringMax */
    __dna_docout.writeFloat("x1c8_ballCameraBoostSpringMax", x1c8_ballCameraBoostSpringMax);
    /* x1cc_ballCameraBoostSpringTardis */
    __dna_docout.writeFloat("x1cc_ballCameraBoostSpringTardis", x1cc_ballCameraBoostSpringTardis);
    /* x1d0_ballCameraControlDistance */
    __dna_docout.writeFloat("x1d0_ballCameraControlDistance", x1d0_ballCameraControlDistance);
    /* x1d4_ */
    __dna_docout.writeFloat("x1d4_", x1d4_);
    /* x1d8_ */
    __dna_docout.writeFloat("x1d8_", x1d8_);
    /* x1e4_leftStickDivisor */
    __dna_docout.writeFloat("x1e4_leftStickDivisor", x1e4_leftStickDivisor);
    /* x1e8_rightStickDivisor */
    __dna_docout.writeFloat("x1e8_rightStickDivisor", x1e8_rightStickDivisor);
    /* x200_ */
    __dna_docout.writeFloat("x200_", x200_);
    /* x204_ballTouchRadius */
    __dna_docout.writeFloat("x204_ballTouchRadius", x204_ballTouchRadius);
    /* x20c_boostBallDrainTime */
    __dna_docout.writeFloat("x20c_boostBallDrainTime", x20c_boostBallDrainTime);
    /* x218_boostBallMinChargeTime */
    __dna_docout.writeFloat("x218_boostBallMinChargeTime", x218_boostBallMinChargeTime);
    /* x21c_boostBallMinRelativeSpeedForDamage */
    __dna_docout.writeFloat("x21c_boostBallMinRelativeSpeedForDamage", x21c_boostBallMinRelativeSpeedForDamage);
    /* x220_boostBallChargeTime0 */
    __dna_docout.writeFloat("x220_boostBallChargeTime0", x220_boostBallChargeTime0);
    /* x224_boostBallChargeTime1 */
    __dna_docout.writeFloat("x224_boostBallChargeTime1", x224_boostBallChargeTime1);
    /* x210_boostBallMaxChargeTime */
    __dna_docout.writeFloat("x210_boostBallMaxChargeTime", x210_boostBallMaxChargeTime);
    /* x22c_boostBallIncrementalSpeed0 */
    __dna_docout.writeFloat("x22c_boostBallIncrementalSpeed0", x22c_boostBallIncrementalSpeed0);
    /* x230_boostBallIncrementalSpeed1 */
    __dna_docout.writeFloat("x230_boostBallIncrementalSpeed1", x230_boostBallIncrementalSpeed1);
    /* x234_boostBallIncrementalSpeed2 */
    __dna_docout.writeFloat("x234_boostBallIncrementalSpeed2", x234_boostBallIncrementalSpeed2);
}

const char* CTweakBall::DNAType()
{
    return "DataSpec::DNAMP1::CTweakBall";
}

size_t CTweakBall::binarySize(size_t __isz) const
{
    return __isz + 456;
}

}
}
