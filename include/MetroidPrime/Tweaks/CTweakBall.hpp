#ifndef _CTWEAKBALL
#define _CTWEAKBALL

#include "types.h"

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/string.hpp"

class CTweakBall;

class CTweakBall : public ITweakObject, public TOneStatic< CTweakBall > {
public:
  ~CTweakBall() override;
  CTweakBall(CInputStream&);

  float GetMaxBallTranslationAcceleration(int s) const { return x4_maxTranslationAcceleration[s]; }
  float GetBallTranslationFriction(int s) const { return x24_translationFriction[s]; }
  float GetBallTranslationMaxSpeed(int s) const { return x44_translationMaxSpeed[s]; }
  float GetBallCameraElevation() const { return 2.736f; }
  float GetBallCameraAnglePerSecond() const { return x74_ballCameraAnglePerSecond; }
  const CVector3f& GetBallCameraOffset() const { return x78_ballCameraOffset; }
  float GetBallCameraMinSpeedDistance() const { return x84_ballCameraMinSpeedDistance; }
  float GetBallCameraMaxSpeedDistance() const { return x88_ballCameraMaxSpeedDistance; }
  float GetBallCameraBackwardsDistance() const { return x8c_ballCameraBackwardsDistance; }
  float GetBallCameraSpringConstant() const { return x94_ballCameraSpringConstant; }
  float GetBallCameraSpringMax() const { return x98_ballCameraSpringMax; }
  float GetBallCameraSpringTardis() const { return x9c_ballCameraSpringTardis; }
  float GetBallCameraCentroidSpringConstant() const { return xa0_ballCameraCentroidSpringConstant; }
  float GetBallCameraCentroidSpringMax() const { return xa4_ballCameraCentroidSpringMax; }
  float GetBallCameraCentroidSpringTardis() const { return xa8_ballCameraCentroidSpringTardis; }
  float GetBallCameraCentroidDistanceSpringConstant() const {
    return xac_ballCameraCentroidDistanceSpringConstant;
  }
  float GetBallCameraCentroidDistanceSpringMax() const {
    return xb0_ballCameraCentroidDistanceSpringMax;
  }
  float GetBallCameraCentroidDistanceSpringTardis() const {
    return xb4_ballCameraCentroidDistanceSpringTardis;
  }
  float GetBallCameraLookAtSpringConstant() const { return xb8_ballCameraLookAtSpringConstant; }
  float GetBallCameraLookAtSpringMax() const { return xbc_ballCameraLookAtSpringMax; }
  float GetBallCameraLookAtSpringTardis() const { return xc0_ballCameraLookAtSpringTardis; }
  float GetBallForwardBrakingAcceleration(int s) const {
    return xc4_ballForwardBrakingAcceleration[s];
  }
  float GetBallGravity() const { return xe4_ballGravity; }
  float GetBallWaterGravity() const { return xe8_ballWaterGravity; }
  float GetBallSlipFactor(int s) const { return x12c_ballSlipFactor[s]; }
  float GetConservativeDoorCameraDistance() const { return x170_conservativeDoorCamDistance; }
  float GetBallCameraChaseElevation() const { return x178_ballCameraChaseElevation; }
  float GetBallCameraChaseDampenAngle() const { return x17c_ballCameraChaseDampenAngle; }
  float GetBallCameraChaseDistance() const { return x180_ballCameraChaseDistance; }
  float GetBallCameraChaseYawSpeed() const { return x184_ballCameraChaseYawSpeed; }
  float GetBallCameraChaseAnglePerSecond() const { return x188_ballCameraChaseAnglePerSecond; }
  const CVector3f& GetBallCameraChaseLookAtOffset() const {
    return x18c_ballCameraChaseLookAtOffset;
  }
  float GetBallCameraChaseSpringConstant() const { return x198_ballCameraChaseSpringConstant; }
  float GetBallCameraChaseSpringMax() const { return x19c_ballCameraChaseSpringMax; }
  float GetBallCameraChaseSpringTardis() const { return x1a0_ballCameraChaseSpringTardis; }
  float GetBallCameraBoostElevation() const { return x1a4_ballCameraBoostElevation; }
  float GetBallCameraBoostDampenAngle() const { return x1a8_ballCameraBoostDampenAngle; }
  float GetBallCameraBoostDistance() const { return x1ac_ballCameraBoostDistance; }
  float GetBallCameraBoostYawSpeed() const { return x1b0_ballCameraBoostYawSpeed; }
  float GetBallCameraBoostAnglePerSecond() const { return x1b4_ballCameraBoostAnglePerSecond; }
  const CVector3f& GetBallCameraBoostLookAtOffset() const {
    return x1b8_ballCameraBoostLookAtOffset;
  }
  float GetBallCameraBoostSpringConstant() const { return x1c4_ballCameraBoostSpringConstant; }
  float GetBallCameraBoostSpringMax() const { return x1c8_ballCameraBoostSpringMax; }
  float GetBallCameraBoostSpringTardis() const { return x1cc_ballCameraBoostSpringTardis; }
  float GetBallCameraControlDistance() const { return x1d0_ballCameraControlDistance; }
  float GetMinimumAlignmentSpeed() const { return x1dc_minimumAlignmentSpeed; }
  float GetTireness() const { return x1e0_tireness; }
  const float& GetMaxLeanAngle() const { return x1ec_maxLeanAngle; }
  float GetTireToMarbleThresholdSpeed() const { return x1f0_tireToMarbleThresholdSpeed; }
  float GetMarbleToTireThresholdSpeed() const { return x1f4_marbleToTireThresholdSpeed; }
  float GetForceToLeanGain() const { return x1f8_forceToLeanGain; }
  float GetLeanTrackingGain() const { return x1fc_leanTrackingGain; }
  float GetLeftStickDivisor() const { return x1e4_leftStickDivisor; }
  float GetRightStickDivisor() const { return x1e8_rightStickDivisor; }
  float GetBallTouchRadius() const { return x204_ballTouchRadius; }
  float GetBoostBallDrainTime() const { return x20c_boostBallDrainTime; }
  float GetBoostBallMaxChargeTime() const { return x210_boostBallMaxChargeTime; }
  float GetBoostBallMinChargeTime() const { return x218_boostBallMinChargeTime; }
  float GetBoostBallMinRelativeSpeedForDamage() const {
    return x21c_boostBallMinRelativeSpeedForDamage;
  }
  float GetBoostBallChargeTimeTable(int i) const {
    switch (i) {
    default:
    case 0:
      return x220_boostBallChargeTime0;
    case 1:
      return x224_boostBallChargeTime1;
    case 2:
      return x228_boostBallChargeTime2;
    }
  }
  float GetBoostBallIncrementalSpeedTable(int i) const {
    switch (i) {
    default:
    case 0:
      return x22c_boostBallIncrementalSpeed0;
    case 1:
      return x230_boostBallIncrementalSpeed1;
    case 2:
      return x234_boostBallIncrementalSpeed2;
    }
  }

private:
  float x4_maxTranslationAcceleration[8];
  float x24_translationFriction[8];
  float x44_translationMaxSpeed[8];
  float x64_;
  float x68_;
  float x6c_;
  float x70_;
  float x74_ballCameraAnglePerSecond;
  CVector3f x78_ballCameraOffset;
  float x84_ballCameraMinSpeedDistance;
  float x88_ballCameraMaxSpeedDistance;
  float x8c_ballCameraBackwardsDistance;
  float x90_;
  float x94_ballCameraSpringConstant;
  float x98_ballCameraSpringMax;
  float x9c_ballCameraSpringTardis;
  float xa0_ballCameraCentroidSpringConstant;
  float xa4_ballCameraCentroidSpringMax;
  float xa8_ballCameraCentroidSpringTardis;
  float xac_ballCameraCentroidDistanceSpringConstant;
  float xb0_ballCameraCentroidDistanceSpringMax;
  float xb4_ballCameraCentroidDistanceSpringTardis;
  float xb8_ballCameraLookAtSpringConstant;
  float xbc_ballCameraLookAtSpringMax;
  float xc0_ballCameraLookAtSpringTardis;
  float xc4_ballForwardBrakingAcceleration[8];
  float xe4_ballGravity;
  float xe8_ballWaterGravity;
  float xec_;
  float xf0_;
  float xf4_;
  float xf8_;
  float xfc_;
  float x100_;
  float x104_;
  float x108_;
  float x10c_;
  float x110_;
  float x114_;
  float x118_;
  float x11c_;
  float x120_;
  float x124_;
  float x128_;
  float x12c_ballSlipFactor[8];
  float x14c_;
  float x150_;
  float x154_;
  float x158_;
  float x15c_;
  float x160_;
  float x164_;
  float x168_;
  float x16c_;
  float x170_conservativeDoorCamDistance;
  float x174_;
  float x178_ballCameraChaseElevation;
  float x17c_ballCameraChaseDampenAngle;
  float x180_ballCameraChaseDistance;
  float x184_ballCameraChaseYawSpeed;
  float x188_ballCameraChaseAnglePerSecond;
  CVector3f x18c_ballCameraChaseLookAtOffset;
  float x198_ballCameraChaseSpringConstant;
  float x19c_ballCameraChaseSpringMax;
  float x1a0_ballCameraChaseSpringTardis;
  float x1a4_ballCameraBoostElevation;
  float x1a8_ballCameraBoostDampenAngle;
  float x1ac_ballCameraBoostDistance;
  float x1b0_ballCameraBoostYawSpeed;
  float x1b4_ballCameraBoostAnglePerSecond;
  CVector3f x1b8_ballCameraBoostLookAtOffset;
  float x1c4_ballCameraBoostSpringConstant;
  float x1c8_ballCameraBoostSpringMax;
  float x1cc_ballCameraBoostSpringTardis;
  float x1d0_ballCameraControlDistance;
  float x1d4_;
  float x1d8_;
  float x1dc_minimumAlignmentSpeed;
  float x1e0_tireness;
  float x1e4_leftStickDivisor;
  float x1e8_rightStickDivisor;
  float x1ec_maxLeanAngle;
  float x1f0_tireToMarbleThresholdSpeed;
  float x1f4_marbleToTireThresholdSpeed;
  float x1f8_forceToLeanGain;
  float x1fc_leanTrackingGain;
  float x200_;
  float x204_ballTouchRadius;
  float x208_;
  float x20c_boostBallDrainTime;
  float x210_boostBallMaxChargeTime;
  float x214_;
  float x218_boostBallMinChargeTime;
  float x21c_boostBallMinRelativeSpeedForDamage;
  float x220_boostBallChargeTime0;
  float x224_boostBallChargeTime1;
  float x228_boostBallChargeTime2;
  float x22c_boostBallIncrementalSpeed0;
  float x230_boostBallIncrementalSpeed1;
  float x234_boostBallIncrementalSpeed2;
};
CHECK_SIZEOF(CTweakBall, 0x238)

extern CTweakBall* gpTweakBall;

#endif // _CTWEAKBALL
