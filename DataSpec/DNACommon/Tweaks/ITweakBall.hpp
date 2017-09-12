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
    virtual float GetBallGravity() const=0;
    virtual float GetBallWaterGravity() const=0;
    virtual float GetBallCameraControlDistance() const=0;
    virtual float GetLeftStickDivisor() const=0;
    virtual float GetRightStickDivisor() const=0;
    virtual float GetBallTouchRadius() const=0;
    virtual float GetBoostBallMinRelativeSpeedForDamage() const=0;
};
}

#endif // __DNACOMMON_ITWEAKBALL_HPP__
