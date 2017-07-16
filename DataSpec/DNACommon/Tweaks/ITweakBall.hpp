#ifndef __DNACOMMON_ITWEAKBALL_HPP__
#define __DNACOMMON_ITWEAKBALL_HPP__

#include "ITweak.hpp"

namespace DataSpec
{
struct ITweakBall : ITweak
{
    virtual float GetBallCameraControlDistance() const=0;
    virtual float GetLeftStickDivisor() const=0;
    virtual float GetRightStickDivisor() const=0;
};
}

#endif // __DNACOMMON_ITWEAKBALL_HPP__
