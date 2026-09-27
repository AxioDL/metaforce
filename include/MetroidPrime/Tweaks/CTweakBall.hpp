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

  float GetMaxBallTranslationAcceleration(int s) const { return mMaxTranslationAcceleration[s]; }
  float GetBallTranslationFriction(int s) const { return mTranslationFriction[s]; }
  float GetBallTranslationMaxSpeed(int s) const { return mTranslationMaxSpeed[s]; }
  float GetBallCameraElevation() const { return 2.736f; }
  float GetBallCameraAnglePerSecond() const { return mBallCameraAnglePerSecond; }
  const CVector3f& GetBallCameraOffset() const { return mBallCameraOffset; }
  float GetBallCameraMinSpeedDistance() const { return mBallCameraMinSpeedDistance; }
  float GetBallCameraMaxSpeedDistance() const { return mBallCameraMaxSpeedDistance; }
  float GetBallCameraBackwardsDistance() const { return mBallCameraBackwardsDistance; }
  float GetBallCameraSpringConstant() const { return mBallCameraSpringConstant; }
  float GetBallCameraSpringMax() const { return mBallCameraSpringMax; }
  float GetBallCameraSpringTardis() const { return mBallCameraSpringTardis; }
  float GetBallCameraCentroidSpringConstant() const { return mBallCameraCentroidSpringConstant; }
  float GetBallCameraCentroidSpringMax() const { return mBallCameraCentroidSpringMax; }
  float GetBallCameraCentroidSpringTardis() const { return mBallCameraCentroidSpringTardis; }
  float GetBallCameraCentroidDistanceSpringConstant() const {
    return mBallCameraCentroidDistanceSpringConstant;
  }
  float GetBallCameraCentroidDistanceSpringMax() const {
    return mBallCameraCentroidDistanceSpringMax;
  }
  float GetBallCameraCentroidDistanceSpringTardis() const {
    return mBallCameraCentroidDistanceSpringTardis;
  }
  float GetBallCameraLookAtSpringConstant() const { return mBallCameraLookAtSpringConstant; }
  float GetBallCameraLookAtSpringMax() const { return mBallCameraLookAtSpringMax; }
  float GetBallCameraLookAtSpringTardis() const { return mBallCameraLookAtSpringTardis; }
  float GetBallForwardBrakingAcceleration(int s) const {
    return mBallForwardBrakingAcceleration[s];
  }
  float GetBallGravity() const { return mBallGravity; }
  float GetBallWaterGravity() const { return mBallWaterGravity; }
  float GetBallSlipFactor(int s) const { return mBallSlipFactor[s]; }
  float GetConservativeDoorCameraDistance() const { return mConservativeDoorCamDistance; }
  float GetBallCameraChaseElevation() const { return mBallCameraChaseElevation; }
  float GetBallCameraChaseDampenAngle() const { return mBallCameraChaseDampenAngle; }
  float GetBallCameraChaseDistance() const { return mBallCameraChaseDistance; }
  float GetBallCameraChaseYawSpeed() const { return mBallCameraChaseYawSpeed; }
  float GetBallCameraChaseAnglePerSecond() const { return mBallCameraChaseAnglePerSecond; }
  const CVector3f& GetBallCameraChaseLookAtOffset() const {
    return mBallCameraChaseLookAtOffset;
  }
  float GetBallCameraChaseSpringConstant() const { return mBallCameraChaseSpringConstant; }
  float GetBallCameraChaseSpringMax() const { return mBallCameraChaseSpringMax; }
  float GetBallCameraChaseSpringTardis() const { return mBallCameraChaseSpringTardis; }
  float GetBallCameraBoostElevation() const { return mBallCameraBoostElevation; }
  float GetBallCameraBoostDampenAngle() const { return mBallCameraBoostDampenAngle; }
  float GetBallCameraBoostDistance() const { return mBallCameraBoostDistance; }
  float GetBallCameraBoostYawSpeed() const { return mBallCameraBoostYawSpeed; }
  float GetBallCameraBoostAnglePerSecond() const { return mBallCameraBoostAnglePerSecond; }
  const CVector3f& GetBallCameraBoostLookAtOffset() const {
    return mBallCameraBoostLookAtOffset;
  }
  float GetBallCameraBoostSpringConstant() const { return mBallCameraBoostSpringConstant; }
  float GetBallCameraBoostSpringMax() const { return mBallCameraBoostSpringMax; }
  float GetBallCameraBoostSpringTardis() const { return mBallCameraBoostSpringTardis; }
  float GetBallCameraControlDistance() const { return mBallCameraControlDistance; }
  float GetMinimumAlignmentSpeed() const { return mMinimumAlignmentSpeed; }
  float GetTireness() const { return mTireness; }
  const float& GetMaxLeanAngle() const { return mMaxLeanAngle; }
  float GetTireToMarbleThresholdSpeed() const { return mTireToMarbleThresholdSpeed; }
  float GetMarbleToTireThresholdSpeed() const { return mMarbleToTireThresholdSpeed; }
  float GetForceToLeanGain() const { return mForceToLeanGain; }
  float GetLeanTrackingGain() const { return mLeanTrackingGain; }
  float GetLeftStickDivisor() const { return mLeftStickDivisor; }
  float GetRightStickDivisor() const { return mRightStickDivisor; }
  float GetBallTouchRadius() const { return mBallTouchRadius; }
  float GetBoostBallDrainTime() const { return mBoostBallDrainTime; }
  float GetBoostBallMaxChargeTime() const { return mBoostBallMaxChargeTime; }
  float GetBoostBallMinChargeTime() const { return mBoostBallMinChargeTime; }
  float GetBoostBallMinRelativeSpeedForDamage() const {
    return mBoostBallMinRelativeSpeedForDamage;
  }
  float GetBoostBallChargeTimeTable(int i) const {
    switch (i) {
    default:
    case 0:
      return mBoostBallChargeTime0;
    case 1:
      return mBoostBallChargeTime1;
    case 2:
      return mBoostBallChargeTime2;
    }
  }
  float GetBoostBallIncrementalSpeedTable(int i) const {
    switch (i) {
    default:
    case 0:
      return mBoostBallIncrementalSpeed0;
    case 1:
      return mBoostBallIncrementalSpeed1;
    case 2:
      return mBoostBallIncrementalSpeed2;
    }
  }

private:
  float mMaxTranslationAcceleration[8];
  float mTranslationFriction[8];
  float mTranslationMaxSpeed[8];
  float x64_;
  float x68_;
  float x6c_;
  float x70_;
  float mBallCameraAnglePerSecond;
  CVector3f mBallCameraOffset;
  float mBallCameraMinSpeedDistance;
  float mBallCameraMaxSpeedDistance;
  float mBallCameraBackwardsDistance;
  float x90_;
  float mBallCameraSpringConstant;
  float mBallCameraSpringMax;
  float mBallCameraSpringTardis;
  float mBallCameraCentroidSpringConstant;
  float mBallCameraCentroidSpringMax;
  float mBallCameraCentroidSpringTardis;
  float mBallCameraCentroidDistanceSpringConstant;
  float mBallCameraCentroidDistanceSpringMax;
  float mBallCameraCentroidDistanceSpringTardis;
  float mBallCameraLookAtSpringConstant;
  float mBallCameraLookAtSpringMax;
  float mBallCameraLookAtSpringTardis;
  float mBallForwardBrakingAcceleration[8];
  float mBallGravity;
  float mBallWaterGravity;
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
  float mBallSlipFactor[8];
  float x14c_;
  float x150_;
  float x154_;
  float x158_;
  float x15c_;
  float x160_;
  float x164_;
  float x168_;
  float x16c_;
  float mConservativeDoorCamDistance;
  float x174_;
  float mBallCameraChaseElevation;
  float mBallCameraChaseDampenAngle;
  float mBallCameraChaseDistance;
  float mBallCameraChaseYawSpeed;
  float mBallCameraChaseAnglePerSecond;
  CVector3f mBallCameraChaseLookAtOffset;
  float mBallCameraChaseSpringConstant;
  float mBallCameraChaseSpringMax;
  float mBallCameraChaseSpringTardis;
  float mBallCameraBoostElevation;
  float mBallCameraBoostDampenAngle;
  float mBallCameraBoostDistance;
  float mBallCameraBoostYawSpeed;
  float mBallCameraBoostAnglePerSecond;
  CVector3f mBallCameraBoostLookAtOffset;
  float mBallCameraBoostSpringConstant;
  float mBallCameraBoostSpringMax;
  float mBallCameraBoostSpringTardis;
  float mBallCameraControlDistance;
  float x1d4_;
  float x1d8_;
  float mMinimumAlignmentSpeed;
  float mTireness;
  float mLeftStickDivisor;
  float mRightStickDivisor;
  float mMaxLeanAngle;
  float mTireToMarbleThresholdSpeed;
  float mMarbleToTireThresholdSpeed;
  float mForceToLeanGain;
  float mLeanTrackingGain;
  float x200_;
  float mBallTouchRadius;
  float x208_;
  float mBoostBallDrainTime;
  float mBoostBallMaxChargeTime;
  float x214_;
  float mBoostBallMinChargeTime;
  float mBoostBallMinRelativeSpeedForDamage;
  float mBoostBallChargeTime0;
  float mBoostBallChargeTime1;
  float mBoostBallChargeTime2;
  float mBoostBallIncrementalSpeed0;
  float mBoostBallIncrementalSpeed1;
  float mBoostBallIncrementalSpeed2;
};
CHECK_SIZEOF(CTweakBall, 0x238)

extern CTweakBall* gpTweakBall;

#endif // _CTWEAKBALL
