#ifndef __DNACOMMON_ITWEAKBALL_HPP__
#define __DNACOMMON_ITWEAKBALL_HPP__

#include "ITweak.hpp"

namespace DataSpec
{
struct ITweakBall : ITweak
{
    virtual float GetMaxBallTranslationAcceleration(int s) const=0;
    virtual float GetBallTranslationFriction(int s) const=0;
    virtual float GetBallTranslationMaxSpeed(int s) const=0;
    virtual float GetBallCameraElevation() const=0;
    virtual float GetBallCameraAnglePerSecond() const=0;
    virtual const zeus::CVector3f& GetBallCameraOffset() const=0;
    virtual float GetBallCameraMinSpeedDistance() const=0;
    virtual float GetBallCameraMaxSpeedDistance() const=0;
    virtual float GetBallCameraBackwardsDistance() const=0;
    virtual float GetBallCameraSpringConstant() const=0;
    virtual float GetBallCameraSpringMax() const=0;
    virtual float GetBallCameraSpringTardis() const=0;
    virtual float GetBallCameraCentroidSpringConstant() const=0;
    virtual float GetBallCameraCentroidSpringMax() const=0;
    virtual float GetBallCameraCentroidSpringTardis() const=0;
    virtual float GetBallCameraCentroidDistanceSpringConstant() const=0;
    virtual float GetBallCameraCentroidDistanceSpringMax() const=0;
    virtual float GetBallCameraCentroidDistanceSpringTardis() const=0;
    virtual float GetBallCameraLookAtSpringConstant() const=0;
    virtual float GetBallCameraLookAtSpringMax() const=0;
    virtual float GetBallCameraLookAtSpringTardis() const=0;
    virtual float GetBallForwardBrakingAcceleration(int s) const=0;
    virtual float GetBallGravity() const=0;
    virtual float GetBallWaterGravity() const=0;
    virtual float GetBallSlipFactor(int s) const=0;
    virtual float GetConservativeDoorCameraDistance() const=0;
    virtual float GetBallCameraChaseElevation() const=0;
    virtual float GetBallCameraChaseDampenAngle() const=0;
    virtual float GetBallCameraChaseDistance() const=0;
    virtual float GetBallCameraChaseYawSpeed() const=0;
    virtual float GetBallCameraChaseAnglePerSecond() const=0;
    virtual const zeus::CVector3f& GetBallCameraChaseLookAtOffset() const=0;
    virtual float GetBallCameraChaseSpringConstant() const=0;
    virtual float GetBallCameraChaseSpringMax() const=0;
    virtual float GetBallCameraChaseSpringTardis() const=0;
    virtual float GetBallCameraBoostElevation() const=0;
    virtual float GetBallCameraBoostDampenAngle() const=0;
    virtual float GetBallCameraBoostDistance() const=0;
    virtual float GetBallCameraBoostYawSpeed() const=0;
    virtual float GetBallCameraBoostAnglePerSecond() const=0;
    virtual const zeus::CVector3f& GetBallCameraBoostLookAtOffset() const=0;
    virtual float GetBallCameraBoostSpringConstant() const=0;
    virtual float GetBallCameraBoostSpringMax() const=0;
    virtual float GetBallCameraBoostSpringTardis() const=0;
    virtual float GetMinimumAlignmentSpeed() const=0;
    virtual float GetTireness() const=0;
    virtual float GetMaxLeanAngle() const=0;
    virtual float GetTireToMarbleThresholdSpeed() const=0;
    virtual float GetMarbleToTireThresholdSpeed() const=0;
    virtual float GetForceToLeanGain() const=0;
    virtual float GetLeanTrackingGain() const=0;
    virtual float GetBallCameraControlDistance() const=0;
    virtual float GetLeftStickDivisor() const=0;
    virtual float GetRightStickDivisor() const=0;
    virtual float GetBallTouchRadius() const=0;
    virtual float GetBoostBallDrainTime() const=0;
    virtual float GetBoostBallMaxChargeTime() const=0;
    virtual float GetBoostBallMinChargeTime() const=0;
    virtual float GetBoostBallMinRelativeSpeedForDamage() const=0;
    virtual float GetBoostBallChargeTimeTable(int i) const=0;
    virtual float GetBoostBallIncrementalSpeedTable(int i) const=0;
};
}

#endif // __DNACOMMON_ITWEAKBALL_HPP__
