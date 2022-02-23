#pragma once

#include "Runtime/Tweaks/ITweakBall.hpp"
#include "zeus/CVector3f.hpp"

namespace metaforce::MP1 {
struct CTweakBall final : public Tweaks::ITweakBall {
  float x4_maxTranslationAcceleration[8];
  float x24_translationFriction[8];
  float x44_translationMaxSpeed[8];
  float x64_;
  float x68_;
  float x6c_;
  float x70_;
  float x74_ballCameraAnglePerSecond;
  zeus::CVector3f x78_ballCameraOffset;
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
  float xec_ = 10000.f;
  float xf0_ = 1000.f;
  float xf4_ = 40000.f;
  float xf8_ = 40000.f;
  float xfc_ = 40000.f;
  float x100_ = 40000.f;
  float x104_ = 40000.f;
  float x108_ = 40000.f;
  float x10c_ = 10000.f;
  float x110_ = 1000.f;
  float x114_ = 40000.f;
  float x118_ = 40000.f;
  float x11c_ = 40000.f;
  float x120_ = 40000.f;
  float x124_ = 40000.f;
  float x128_ = 40000.f;
  float x12c_ballSlipFactor[8] = {10000.f, 10000.f, 1000.f, 10000.f, 2000.f, 2000.f, 2000.f, 2000.f};
  float x14c_;
  float x150_;
  float x158_;
  float x154_;
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
  zeus::CVector3f x18c_ballCameraChaseLookAtOffset;
  float x198_ballCameraChaseSpringConstant;
  float x19c_ballCameraChaseSpringMax;
  float x1a0_ballCameraChaseSpringTardis;
  float x1a4_ballCameraBoostElevation;
  float x1a8_ballCameraBoostDampenAngle;
  float x1ac_ballCameraBoostDistance;
  float x1b0_ballCameraBoostYawSpeed;
  float x1b4_ballCameraBoostAnglePerSecond;
  zeus::CVector3f x1b8_ballCameraBoostLookAtOffset;
  float x1c4_ballCameraBoostSpringConstant;
  float x1c8_ballCameraBoostSpringMax;
  float x1cc_ballCameraBoostSpringTardis;
  float x1d0_ballCameraControlDistance;
  float x1d4_;
  float x1d8_;
  float x1dc_minimumAlignmentSpeed;
  float x1e0_tireness;
  float x1ec_maxLeanAngle;
  float x1f0_tireToMarbleThresholdSpeed;
  float x1f4_marbleToTireThresholdSpeed;
  float x1f8_forceToLeanGain;
  float x1fc_leanTrackingGain;
  float x1e4_leftStickDivisor;
  float x1e8_rightStickDivisor;
  float x200_;
  float x204_ballTouchRadius;
  float x208_;
  float x20c_boostBallDrainTime;
  float x218_boostBallMinChargeTime;
  float x21c_boostBallMinRelativeSpeedForDamage;
  float x220_boostBallChargeTime0;
  float x224_boostBallChargeTime1;
  float x228_boostBallChargeTime2;
  float x210_boostBallMaxChargeTime;
  float x22c_boostBallIncrementalSpeed0;
  float x230_boostBallIncrementalSpeed1;
  float x234_boostBallIncrementalSpeed2;

  CTweakBall() = default;
  CTweakBall(CInputStream& r);
  float GetMaxBallTranslationAcceleration(int s) const override { return x4_maxTranslationAcceleration[s]; }
  float GetBallTranslationFriction(int s) const override { return x24_translationFriction[s]; }
  float GetBallTranslationMaxSpeed(int s) const override { return x44_translationMaxSpeed[s]; }
  float GetBallCameraElevation() const override { return 2.736f; }
  float GetBallCameraAnglePerSecond() const override { return x74_ballCameraAnglePerSecond; }
  const zeus::CVector3f& GetBallCameraOffset() const override { return x78_ballCameraOffset; }
  float GetBallCameraMinSpeedDistance() const override { return x84_ballCameraMinSpeedDistance; }
  float GetBallCameraMaxSpeedDistance() const override { return x88_ballCameraMaxSpeedDistance; }
  float GetBallCameraBackwardsDistance() const override { return x8c_ballCameraBackwardsDistance; }
  float GetBallCameraSpringConstant() const override { return x94_ballCameraSpringConstant; }
  float GetBallCameraSpringMax() const override { return x98_ballCameraSpringMax; }
  float GetBallCameraSpringTardis() const override { return x9c_ballCameraSpringTardis; }
  float GetBallCameraCentroidSpringConstant() const override { return xa0_ballCameraCentroidSpringConstant; }
  float GetBallCameraCentroidSpringMax() const override { return xa4_ballCameraCentroidSpringMax; }
  float GetBallCameraCentroidSpringTardis() const override { return xa8_ballCameraCentroidSpringTardis; }
  float GetBallCameraCentroidDistanceSpringConstant() const override {
    return xac_ballCameraCentroidDistanceSpringConstant;
  }
  float GetBallCameraCentroidDistanceSpringMax() const override { return xb0_ballCameraCentroidDistanceSpringMax; }
  float GetBallCameraCentroidDistanceSpringTardis() const override {
    return xb4_ballCameraCentroidDistanceSpringTardis;
  }
  float GetBallCameraLookAtSpringConstant() const override { return xb8_ballCameraLookAtSpringConstant; }
  float GetBallCameraLookAtSpringMax() const override { return xbc_ballCameraLookAtSpringMax; }
  float GetBallCameraLookAtSpringTardis() const override { return xc0_ballCameraLookAtSpringTardis; }
  float GetBallForwardBrakingAcceleration(int s) const override { return xc4_ballForwardBrakingAcceleration[s]; }
  float GetBallGravity() const override { return xe4_ballGravity; }
  float GetBallWaterGravity() const override { return xe8_ballWaterGravity; }
  float GetBallSlipFactor(int s) const override { return x12c_ballSlipFactor[s]; }
  float GetConservativeDoorCameraDistance() const override { return x170_conservativeDoorCamDistance; }
  float GetBallCameraChaseElevation() const override { return x178_ballCameraChaseElevation; }
  float GetBallCameraChaseDampenAngle() const override { return x17c_ballCameraChaseDampenAngle; }
  float GetBallCameraChaseDistance() const override { return x180_ballCameraChaseDistance; }
  float GetBallCameraChaseYawSpeed() const override { return x184_ballCameraChaseYawSpeed; }
  float GetBallCameraChaseAnglePerSecond() const override { return x188_ballCameraChaseAnglePerSecond; }
  const zeus::CVector3f& GetBallCameraChaseLookAtOffset() const override { return x18c_ballCameraChaseLookAtOffset; }
  float GetBallCameraChaseSpringConstant() const override { return x198_ballCameraChaseSpringConstant; }
  float GetBallCameraChaseSpringMax() const override { return x19c_ballCameraChaseSpringMax; }
  float GetBallCameraChaseSpringTardis() const override { return x1a0_ballCameraChaseSpringTardis; }
  float GetBallCameraBoostElevation() const override { return x1a4_ballCameraBoostElevation; }
  float GetBallCameraBoostDampenAngle() const override { return x1a8_ballCameraBoostDampenAngle; }
  float GetBallCameraBoostDistance() const override { return x1ac_ballCameraBoostDistance; }
  float GetBallCameraBoostYawSpeed() const override { return x1b0_ballCameraBoostYawSpeed; }
  float GetBallCameraBoostAnglePerSecond() const override { return x1b4_ballCameraBoostAnglePerSecond; }
  const zeus::CVector3f& GetBallCameraBoostLookAtOffset() const override { return x1b8_ballCameraBoostLookAtOffset; }
  float GetBallCameraBoostSpringConstant() const override { return x1c4_ballCameraBoostSpringConstant; }
  float GetBallCameraBoostSpringMax() const override { return x1c8_ballCameraBoostSpringMax; }
  float GetBallCameraBoostSpringTardis() const override { return x1cc_ballCameraBoostSpringTardis; }
  float GetBallCameraControlDistance() const override { return x1d0_ballCameraControlDistance; }
  float GetMinimumAlignmentSpeed() const override { return x1dc_minimumAlignmentSpeed; }
  float GetTireness() const override { return x1e0_tireness; }
  float GetMaxLeanAngle() const override { return x1ec_maxLeanAngle; }
  float GetTireToMarbleThresholdSpeed() const override { return x1f0_tireToMarbleThresholdSpeed; }
  float GetMarbleToTireThresholdSpeed() const override { return x1f4_marbleToTireThresholdSpeed; }
  float GetForceToLeanGain() const override { return x1f8_forceToLeanGain; }
  float GetLeanTrackingGain() const override { return x1fc_leanTrackingGain; }
  float GetLeftStickDivisor() const override { return x1e4_leftStickDivisor; }
  float GetRightStickDivisor() const override { return x1e8_rightStickDivisor; }
  float GetBallTouchRadius() const override { return x204_ballTouchRadius; }
  float GetBoostBallDrainTime() const override { return x20c_boostBallDrainTime; }
  float GetBoostBallMaxChargeTime() const override { return x210_boostBallMaxChargeTime; }
  float GetBoostBallMinChargeTime() const override { return x218_boostBallMinChargeTime; }
  float GetBoostBallMinRelativeSpeedForDamage() const override { return x21c_boostBallMinRelativeSpeedForDamage; }
  float GetBoostBallChargeTimeTable(int i) const override {
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
  float GetBoostBallIncrementalSpeedTable(int i) const override {
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

  void LoadTweaks(CInputStream& in);
  void PutTo(COutputStream& out);
};
} // namespace metaforce::MP1
