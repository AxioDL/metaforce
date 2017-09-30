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
    virtual float GetBallForwardBrakingAcceleration(int s) const=0;
    virtual float GetBallGravity() const=0;
    virtual float GetBallWaterGravity() const=0;
    virtual float GetBallSlipFactor(int s) const=0;
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
