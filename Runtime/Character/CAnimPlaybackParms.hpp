#ifndef __CANIMPLAYBACKPARMS_HPP__
#define __CANIMPLAYBACKPARMS_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CAnimPlaybackParms
{
    friend class CAnimData;
    s32   x0_animA = -1;
    s32   x4_animB = -1;
    float x8_blendWeight = 1.f;
    bool  xc_animating = true;
    s32   x10_ = 0;
    s32   x14_ = 0;
    bool  x18_ = false;
    s32   x1c_ = 0;
    s32   x20_ = 0;
    s32   x24_ = 0;
public:
    CAnimPlaybackParms() = default;
    CAnimPlaybackParms(s32 animA, s32 animB, float blendWeight, bool animating)
    : x0_animA(animA), x4_animB(animB), x8_blendWeight(blendWeight), xc_animating(animating)
    {}
};
}

#endif // CANIMPLAYBACKPARMS_HPP
