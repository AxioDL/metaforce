#include "Runtime/MP1/Tweaks/CTweakBall.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce::MP1 {
CTweakBall::CTweakBall(CInputStream& in) {
  LoadTweaks(in);
  x6c_ = -x6c_;
  x70_ = -x70_;
  x74_ballCameraAnglePerSecond = zeus::degToRad(x74_ballCameraAnglePerSecond);
  x90_ = zeus::degToRad(x90_);
  xe4_ballGravity = -xe4_ballGravity;
  xe8_ballWaterGravity = -xe8_ballWaterGravity;
  x15c_ = zeus::degToRad(x15c_);
  x16c_ = zeus::degToRad(x16c_);
  x174_ = zeus::degToRad(x174_);
  x17c_ballCameraChaseDampenAngle = zeus::degToRad(x17c_ballCameraChaseDampenAngle);
  x184_ballCameraChaseYawSpeed = zeus::degToRad(x184_ballCameraChaseYawSpeed);
  x188_ballCameraChaseAnglePerSecond = zeus::degToRad(x188_ballCameraChaseAnglePerSecond);
  x1a8_ballCameraBoostDampenAngle = zeus::degToRad(x1a8_ballCameraBoostDampenAngle);
  x1b0_ballCameraBoostYawSpeed = zeus::degToRad(x1b0_ballCameraBoostYawSpeed);
  x1b4_ballCameraBoostAnglePerSecond = zeus::degToRad(x1b4_ballCameraBoostAnglePerSecond);
  x1ec_maxLeanAngle = zeus::degToRad(x1ec_maxLeanAngle);
}

void CTweakBall::LoadTweaks(CInputStream& in) {
  /* x4_maxTranslationAcceleration[0] */
  x4_maxTranslationAcceleration[0] = in.ReadFloat();
  /* x4_maxTranslationAcceleration[1] */
  x4_maxTranslationAcceleration[1] = in.ReadFloat();
  /* x4_maxTranslationAcceleration[2] */
  x4_maxTranslationAcceleration[2] = in.ReadFloat();
  /* x4_maxTranslationAcceleration[3] */
  x4_maxTranslationAcceleration[3] = in.ReadFloat();
  /* x4_maxTranslationAcceleration[4] */
  x4_maxTranslationAcceleration[4] = in.ReadFloat();
  /* x4_maxTranslationAcceleration[5] */
  x4_maxTranslationAcceleration[5] = in.ReadFloat();
  /* x4_maxTranslationAcceleration[6] */
  x4_maxTranslationAcceleration[6] = in.ReadFloat();
  /* x4_maxTranslationAcceleration[7] */
  x4_maxTranslationAcceleration[7] = in.ReadFloat();
  /* x24_translationFriction[0] */
  x24_translationFriction[0] = in.ReadFloat();
  /* x24_translationFriction[1] */
  x24_translationFriction[1] = in.ReadFloat();
  /* x24_translationFriction[2] */
  x24_translationFriction[2] = in.ReadFloat();
  /* x24_translationFriction[3] */
  x24_translationFriction[3] = in.ReadFloat();
  /* x24_translationFriction[4] */
  x24_translationFriction[4] = in.ReadFloat();
  /* x24_translationFriction[5] */
  x24_translationFriction[5] = in.ReadFloat();
  /* x24_translationFriction[6] */
  x24_translationFriction[6] = in.ReadFloat();
  /* x24_translationFriction[7] */
  x24_translationFriction[7] = in.ReadFloat();
  /* x44_translationMaxSpeed[0] */
  x44_translationMaxSpeed[0] = in.ReadFloat();
  /* x44_translationMaxSpeed[1] */
  x44_translationMaxSpeed[1] = in.ReadFloat();
  /* x44_translationMaxSpeed[2] */
  x44_translationMaxSpeed[2] = in.ReadFloat();
  /* x44_translationMaxSpeed[3] */
  x44_translationMaxSpeed[3] = in.ReadFloat();
  /* x44_translationMaxSpeed[4] */
  x44_translationMaxSpeed[4] = in.ReadFloat();
  /* x44_translationMaxSpeed[5] */
  x44_translationMaxSpeed[5] = in.ReadFloat();
  /* x44_translationMaxSpeed[6] */
  x44_translationMaxSpeed[6] = in.ReadFloat();
  /* x44_translationMaxSpeed[7] */
  x44_translationMaxSpeed[7] = in.ReadFloat();
  /* x64_ */
  x64_ = in.ReadFloat();
  /* x68_ */
  x68_ = in.ReadFloat();
  /* x6c_ */
  x6c_ = in.ReadFloat();
  /* x70_ */
  x70_ = in.ReadFloat();
  /* xc4_ballForwardBrakingAcceleration[0] */
  xc4_ballForwardBrakingAcceleration[0] = in.ReadFloat();
  /* xc4_ballForwardBrakingAcceleration[1] */
  xc4_ballForwardBrakingAcceleration[1] = in.ReadFloat();
  /* xc4_ballForwardBrakingAcceleration[2] */
  xc4_ballForwardBrakingAcceleration[2] = in.ReadFloat();
  /* xc4_ballForwardBrakingAcceleration[3] */
  xc4_ballForwardBrakingAcceleration[3] = in.ReadFloat();
  /* xc4_ballForwardBrakingAcceleration[4] */
  xc4_ballForwardBrakingAcceleration[4] = in.ReadFloat();
  /* xc4_ballForwardBrakingAcceleration[5] */
  xc4_ballForwardBrakingAcceleration[5] = in.ReadFloat();
  /* xc4_ballForwardBrakingAcceleration[6] */
  xc4_ballForwardBrakingAcceleration[6] = in.ReadFloat();
  /* xc4_ballForwardBrakingAcceleration[7] */
  xc4_ballForwardBrakingAcceleration[7] = in.ReadFloat();
  /* xe4_ballGravity */
  xe4_ballGravity = in.ReadFloat();
  /* xe8_ballWaterGravity */
  xe8_ballWaterGravity = in.ReadFloat();
  /* x14c_ */
  x14c_ = in.ReadFloat();
  /* x150_ */
  x150_ = in.ReadFloat();
  /* x158_ */
  x158_ = in.ReadFloat();
  /* x1dc_minimumAlignmentSpeed */
  x1dc_minimumAlignmentSpeed = in.ReadFloat();
  /* x1e0_tireness */
  x1e0_tireness = in.ReadFloat();
  /* x1ec_maxLeanAngle */
  x1ec_maxLeanAngle = in.ReadFloat();
  /* x1f0_tireToMarbleThresholdSpeed */
  x1f0_tireToMarbleThresholdSpeed = in.ReadFloat();
  /* x1f4_marbleToTireThresholdSpeed */
  x1f4_marbleToTireThresholdSpeed = in.ReadFloat();
  /* x1f8_forceToLeanGain */
  x1f8_forceToLeanGain = in.ReadFloat();
  /* x1fc_leanTrackingGain */
  x1fc_leanTrackingGain = in.ReadFloat();
  /* x74_ballCameraAnglePerSecond */
  x74_ballCameraAnglePerSecond = in.ReadFloat();
  /* x78_ballCameraOffset */
  x78_ballCameraOffset = in.Get<zeus::CVector3f>();
  /* x84_ballCameraMinSpeedDistance */
  x84_ballCameraMinSpeedDistance = in.ReadFloat();
  /* x88_ballCameraMaxSpeedDistance */
  x88_ballCameraMaxSpeedDistance = in.ReadFloat();
  /* x8c_ballCameraBackwardsDistance */
  x8c_ballCameraBackwardsDistance = in.ReadFloat();
  /* x90_ */
  x90_ = in.ReadFloat();
  /* x94_ballCameraSpringConstant */
  x94_ballCameraSpringConstant = in.ReadFloat();
  /* x98_ballCameraSpringMax */
  x98_ballCameraSpringMax = in.ReadFloat();
  /* x9c_ballCameraSpringTardis */
  x9c_ballCameraSpringTardis = in.ReadFloat();
  /* xa0_ballCameraCentroidSpringConstant */
  xa0_ballCameraCentroidSpringConstant = in.ReadFloat();
  /* xa4_ballCameraCentroidSpringMax */
  xa4_ballCameraCentroidSpringMax = in.ReadFloat();
  /* xa8_ballCameraCentroidSpringTardis */
  xa8_ballCameraCentroidSpringTardis = in.ReadFloat();
  /* xac_ballCameraCentroidDistanceSpringConstant */
  xac_ballCameraCentroidDistanceSpringConstant = in.ReadFloat();
  /* xb0_ballCameraCentroidDistanceSpringMax */
  xb0_ballCameraCentroidDistanceSpringMax = in.ReadFloat();
  /* xb4_ballCameraCentroidDistanceSpringTardis */
  xb4_ballCameraCentroidDistanceSpringTardis = in.ReadFloat();
  /* xb8_ballCameraLookAtSpringConstant */
  xb8_ballCameraLookAtSpringConstant = in.ReadFloat();
  /* xbc_ballCameraLookAtSpringMax */
  xbc_ballCameraLookAtSpringMax = in.ReadFloat();
  /* xc0_ballCameraLookAtSpringTardis */
  xc0_ballCameraLookAtSpringTardis = in.ReadFloat();
  /* x154_ */
  x154_ = in.ReadFloat();
  /* x15c_ */
  x15c_ = in.ReadFloat();
  /* x160_ */
  x160_ = in.ReadFloat();
  /* x164_ */
  x164_ = in.ReadFloat();
  /* x168_ */
  x168_ = in.ReadFloat();
  /* x16c_ */
  x16c_ = in.ReadFloat();
  /* x170_conservativeDoorCamDistance */
  x170_conservativeDoorCamDistance = in.ReadFloat();
  /* x174_ */
  x174_ = in.ReadFloat();
  /* x178_ballCameraChaseElevation */
  x178_ballCameraChaseElevation = in.ReadFloat();
  /* x17c_ballCameraChaseDampenAngle */
  x17c_ballCameraChaseDampenAngle = in.ReadFloat();
  /* x180_ballCameraChaseDistance */
  x180_ballCameraChaseDistance = in.ReadFloat();
  /* x184_ballCameraChaseYawSpeed */
  x184_ballCameraChaseYawSpeed = in.ReadFloat();
  /* x188_ballCameraChaseAnglePerSecond */
  x188_ballCameraChaseAnglePerSecond = in.ReadFloat();
  /* x18c_ballCameraChaseLookAtOffset */
  x18c_ballCameraChaseLookAtOffset = in.Get<zeus::CVector3f>();
  /* x198_ballCameraChaseSpringConstant */
  x198_ballCameraChaseSpringConstant = in.ReadFloat();
  /* x19c_ballCameraChaseSpringMax */
  x19c_ballCameraChaseSpringMax = in.ReadFloat();
  /* x1a0_ballCameraChaseSpringTardis */
  x1a0_ballCameraChaseSpringTardis = in.ReadFloat();
  /* x1a4_ballCameraBoostElevation */
  x1a4_ballCameraBoostElevation = in.ReadFloat();
  /* x1a8_ballCameraBoostDampenAngle */
  x1a8_ballCameraBoostDampenAngle = in.ReadFloat();
  /* x1ac_ballCameraBoostDistance */
  x1ac_ballCameraBoostDistance = in.ReadFloat();
  /* x1b0_ballCameraBoostYawSpeed */
  x1b0_ballCameraBoostYawSpeed = in.ReadFloat();
  /* x1b4_ballCameraBoostAnglePerSecond */
  x1b4_ballCameraBoostAnglePerSecond = in.ReadFloat();
  /* x1b8_ballCameraBoostLookAtOffset */
  x1b8_ballCameraBoostLookAtOffset = in.Get<zeus::CVector3f>();
  /* x1c4_ballCameraBoostSpringConstant */
  x1c4_ballCameraBoostSpringConstant = in.ReadFloat();
  /* x1c8_ballCameraBoostSpringMax */
  x1c8_ballCameraBoostSpringMax = in.ReadFloat();
  /* x1cc_ballCameraBoostSpringTardis */
  x1cc_ballCameraBoostSpringTardis = in.ReadFloat();
  /* x1d0_ballCameraControlDistance */
  x1d0_ballCameraControlDistance = in.ReadFloat();
  /* x1d4_ */
  x1d4_ = in.ReadFloat();
  /* x1d8_ */
  x1d8_ = in.ReadFloat();
  /* x1e4_leftStickDivisor */
  x1e4_leftStickDivisor = in.ReadFloat();
  /* x1e8_rightStickDivisor */
  x1e8_rightStickDivisor = in.ReadFloat();
  /* x200_ */
  x200_ = in.ReadFloat();
  /* x204_ballTouchRadius */
  x204_ballTouchRadius = in.ReadFloat();
  /* x20c_boostBallDrainTime */
  x20c_boostBallDrainTime = in.ReadFloat();
  /* x218_boostBallMinChargeTime */
  x218_boostBallMinChargeTime = in.ReadFloat();
  /* x21c_boostBallMinRelativeSpeedForDamage */
  x21c_boostBallMinRelativeSpeedForDamage = in.ReadFloat();
  /* x220_boostBallChargeTime0 */
  x220_boostBallChargeTime0 = in.ReadFloat();
  /* x224_boostBallChargeTime1 */
  x224_boostBallChargeTime1 = in.ReadFloat();
  /* x210_boostBallMaxChargeTime */
  x228_boostBallChargeTime2 = x210_boostBallMaxChargeTime = in.ReadFloat();
  /* x22c_boostBallIncrementalSpeed0 */
  x22c_boostBallIncrementalSpeed0 = in.ReadFloat();
  /* x230_boostBallIncrementalSpeed1 */
  x230_boostBallIncrementalSpeed1 = in.ReadFloat();
  /* x234_boostBallIncrementalSpeed2 */
  x234_boostBallIncrementalSpeed2 = in.ReadFloat();
}

void CTweakBall::PutTo(COutputStream& out) {
  /* x4_maxTranslationAcceleration[0] */
  out.Put(x4_maxTranslationAcceleration[0]);
  /* x4_maxTranslationAcceleration[1] */
  out.Put(x4_maxTranslationAcceleration[1]);
  /* x4_maxTranslationAcceleration[2] */
  out.Put(x4_maxTranslationAcceleration[2]);
  /* x4_maxTranslationAcceleration[3] */
  out.Put(x4_maxTranslationAcceleration[3]);
  /* x4_maxTranslationAcceleration[4] */
  out.Put(x4_maxTranslationAcceleration[4]);
  /* x4_maxTranslationAcceleration[5] */
  out.Put(x4_maxTranslationAcceleration[5]);
  /* x4_maxTranslationAcceleration[6] */
  out.Put(x4_maxTranslationAcceleration[6]);
  /* x4_maxTranslationAcceleration[7] */
  out.Put(x4_maxTranslationAcceleration[7]);
  /* x24_translationFriction[0] */
  out.Put(x24_translationFriction[0]);
  /* x24_translationFriction[1] */
  out.Put(x24_translationFriction[1]);
  /* x24_translationFriction[2] */
  out.Put(x24_translationFriction[2]);
  /* x24_translationFriction[3] */
  out.Put(x24_translationFriction[3]);
  /* x24_translationFriction[4] */
  out.Put(x24_translationFriction[4]);
  /* x24_translationFriction[5] */
  out.Put(x24_translationFriction[5]);
  /* x24_translationFriction[6] */
  out.Put(x24_translationFriction[6]);
  /* x24_translationFriction[7] */
  out.Put(x24_translationFriction[7]);
  /* x44_translationMaxSpeed[0] */
  out.Put(x44_translationMaxSpeed[0]);
  /* x44_translationMaxSpeed[1] */
  out.Put(x44_translationMaxSpeed[1]);
  /* x44_translationMaxSpeed[2] */
  out.Put(x44_translationMaxSpeed[2]);
  /* x44_translationMaxSpeed[3] */
  out.Put(x44_translationMaxSpeed[3]);
  /* x44_translationMaxSpeed[4] */
  out.Put(x44_translationMaxSpeed[4]);
  /* x44_translationMaxSpeed[5] */
  out.Put(x44_translationMaxSpeed[5]);
  /* x44_translationMaxSpeed[6] */
  out.Put(x44_translationMaxSpeed[6]);
  /* x44_translationMaxSpeed[7] */
  out.Put(x44_translationMaxSpeed[7]);
  /* x64_ */
  out.Put(x64_);
  /* x68_ */
  out.Put(x68_);
  /* x6c_ */
  out.Put(x6c_);
  /* x70_ */
  out.Put(x70_);
  /* xc4_ballForwardBrakingAcceleration[0] */
  out.Put(xc4_ballForwardBrakingAcceleration[0]);
  /* xc4_ballForwardBrakingAcceleration[1] */
  out.Put(xc4_ballForwardBrakingAcceleration[1]);
  /* xc4_ballForwardBrakingAcceleration[2] */
  out.Put(xc4_ballForwardBrakingAcceleration[2]);
  /* xc4_ballForwardBrakingAcceleration[3] */
  out.Put(xc4_ballForwardBrakingAcceleration[3]);
  /* xc4_ballForwardBrakingAcceleration[4] */
  out.Put(xc4_ballForwardBrakingAcceleration[4]);
  /* xc4_ballForwardBrakingAcceleration[5] */
  out.Put(xc4_ballForwardBrakingAcceleration[5]);
  /* xc4_ballForwardBrakingAcceleration[6] */
  out.Put(xc4_ballForwardBrakingAcceleration[6]);
  /* xc4_ballForwardBrakingAcceleration[7] */
  out.Put(xc4_ballForwardBrakingAcceleration[7]);
  /* xe4_ballGravity */
  out.Put(xe4_ballGravity);
  /* xe8_ballWaterGravity */
  out.Put(xe8_ballWaterGravity);
  /* x14c_ */
  out.Put(x14c_);
  /* x150_ */
  out.Put(x150_);
  /* x158_ */
  out.Put(x158_);
  /* x1dc_minimumAlignmentSpeed */
  out.Put(x1dc_minimumAlignmentSpeed);
  /* x1e0_tireness */
  out.Put(x1e0_tireness);
  /* x1ec_maxLeanAngle */
  out.Put(x1ec_maxLeanAngle);
  /* x1f0_tireToMarbleThresholdSpeed */
  out.Put(x1f0_tireToMarbleThresholdSpeed);
  /* x1f4_marbleToTireThresholdSpeed */
  out.Put(x1f4_marbleToTireThresholdSpeed);
  /* x1f8_forceToLeanGain */
  out.Put(x1f8_forceToLeanGain);
  /* x1fc_leanTrackingGain */
  out.Put(x1fc_leanTrackingGain);
  /* x74_ballCameraAnglePerSecond */
  out.Put(x74_ballCameraAnglePerSecond);
  /* x78_ballCameraOffset */
  out.Put(x78_ballCameraOffset);
  /* x84_ballCameraMinSpeedDistance */
  out.Put(x84_ballCameraMinSpeedDistance);
  /* x88_ballCameraMaxSpeedDistance */
  out.Put(x88_ballCameraMaxSpeedDistance);
  /* x8c_ballCameraBackwardsDistance */
  out.Put(x8c_ballCameraBackwardsDistance);
  /* x90_ */
  out.Put(x90_);
  /* x94_ballCameraSpringConstant */
  out.Put(x94_ballCameraSpringConstant);
  /* x98_ballCameraSpringMax */
  out.Put(x98_ballCameraSpringMax);
  /* x9c_ballCameraSpringTardis */
  out.Put(x9c_ballCameraSpringTardis);
  /* xa0_ballCameraCentroidSpringConstant */
  out.Put(xa0_ballCameraCentroidSpringConstant);
  /* xa4_ballCameraCentroidSpringMax */
  out.Put(xa4_ballCameraCentroidSpringMax);
  /* xa8_ballCameraCentroidSpringTardis */
  out.Put(xa8_ballCameraCentroidSpringTardis);
  /* xac_ballCameraCentroidDistanceSpringConstant */
  out.Put(xac_ballCameraCentroidDistanceSpringConstant);
  /* xb0_ballCameraCentroidDistanceSpringMax */
  out.Put(xb0_ballCameraCentroidDistanceSpringMax);
  /* xb4_ballCameraCentroidDistanceSpringTardis */
  out.Put(xb4_ballCameraCentroidDistanceSpringTardis);
  /* xb8_ballCameraLookAtSpringConstant */
  out.Put(xb8_ballCameraLookAtSpringConstant);
  /* xbc_ballCameraLookAtSpringMax */
  out.Put(xbc_ballCameraLookAtSpringMax);
  /* xc0_ballCameraLookAtSpringTardis */
  out.Put(xc0_ballCameraLookAtSpringTardis);
  /* x154_ */
  out.Put(x154_);
  /* x15c_ */
  out.Put(x15c_);
  /* x160_ */
  out.Put(x160_);
  /* x164_ */
  out.Put(x164_);
  /* x168_ */
  out.Put(x168_);
  /* x16c_ */
  out.Put(x16c_);
  /* x170_conservativeDoorCamDistance */
  out.Put(x170_conservativeDoorCamDistance);
  /* x174_ */
  out.Put(x174_);
  /* x178_ballCameraChaseElevation */
  out.Put(x178_ballCameraChaseElevation);
  /* x17c_ballCameraChaseDampenAngle */
  out.Put(x17c_ballCameraChaseDampenAngle);
  /* x180_ballCameraChaseDistance */
  out.Put(x180_ballCameraChaseDistance);
  /* x184_ballCameraChaseYawSpeed */
  out.Put(x184_ballCameraChaseYawSpeed);
  /* x188_ballCameraChaseAnglePerSecond */
  out.Put(x188_ballCameraChaseAnglePerSecond);
  /* x18c_ballCameraChaseLookAtOffset */
  out.Put(x18c_ballCameraChaseLookAtOffset);
  /* x198_ballCameraChaseSpringConstant */
  out.Put(x198_ballCameraChaseSpringConstant);
  /* x19c_ballCameraChaseSpringMax */
  out.Put(x19c_ballCameraChaseSpringMax);
  /* x1a0_ballCameraChaseSpringTardis */
  out.Put(x1a0_ballCameraChaseSpringTardis);
  /* x1a4_ballCameraBoostElevation */
  out.Put(x1a4_ballCameraBoostElevation);
  /* x1a8_ballCameraBoostDampenAngle */
  out.Put(x1a8_ballCameraBoostDampenAngle);
  /* x1ac_ballCameraBoostDistance */
  out.Put(x1ac_ballCameraBoostDistance);
  /* x1b0_ballCameraBoostYawSpeed */
  out.Put(x1b0_ballCameraBoostYawSpeed);
  /* x1b4_ballCameraBoostAnglePerSecond */
  out.Put(x1b4_ballCameraBoostAnglePerSecond);
  /* x1b8_ballCameraBoostLookAtOffset */
  out.Put(x1b8_ballCameraBoostLookAtOffset);
  /* x1c4_ballCameraBoostSpringConstant */
  out.Put(x1c4_ballCameraBoostSpringConstant);
  /* x1c8_ballCameraBoostSpringMax */
  out.Put(x1c8_ballCameraBoostSpringMax);
  /* x1cc_ballCameraBoostSpringTardis */
  out.Put(x1cc_ballCameraBoostSpringTardis);
  /* x1d0_ballCameraControlDistance */
  out.Put(x1d0_ballCameraControlDistance);
  /* x1d4_ */
  out.Put(x1d4_);
  /* x1d8_ */
  out.Put(x1d8_);
  /* x1e4_leftStickDivisor */
  out.Put(x1e4_leftStickDivisor);
  /* x1e8_rightStickDivisor */
  out.Put(x1e8_rightStickDivisor);
  /* x200_ */
  out.Put(x200_);
  /* x204_ballTouchRadius */
  out.Put(x204_ballTouchRadius);
  /* x20c_boostBallDrainTime */
  out.Put(x20c_boostBallDrainTime);
  /* x218_boostBallMinChargeTime */
  out.Put(x218_boostBallMinChargeTime);
  /* x21c_boostBallMinRelativeSpeedForDamage */
  out.Put(x21c_boostBallMinRelativeSpeedForDamage);
  /* x220_boostBallChargeTime0 */
  out.Put(x220_boostBallChargeTime0);
  /* x224_boostBallChargeTime1 */
  out.Put(x224_boostBallChargeTime1);
  /* x210_boostBallMaxChargeTime */
  out.Put(x210_boostBallMaxChargeTime);
  /* x22c_boostBallIncrementalSpeed0 */
  out.Put(x22c_boostBallIncrementalSpeed0);
  /* x230_boostBallIncrementalSpeed1 */
  out.Put(x230_boostBallIncrementalSpeed1);
  /* x234_boostBallIncrementalSpeed2 */
  out.Put(x234_boostBallIncrementalSpeed2);
}
} // namespace metaforce::MP1
