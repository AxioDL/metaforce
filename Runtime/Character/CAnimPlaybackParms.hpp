#ifndef __CANIMPLAYBACKPARMS_HPP__
#define __CANIMPLAYBACKPARMS_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CAnimPlaybackParms
{
    s32   x0_defaultAnim;
    s32   x4_;
    float x8_scale;
    bool  xc_loop;
    s32   x10_ = 0;
    s32   x14_ = 0;
    bool  x18_ = false;
    s32   x1c_ = 0;
    s32   x20_ = 0;
    s32   x24_ = 0;
public:
    CAnimPlaybackParms(s32 defaultAnim, s32 b, float scale, bool loop)
        : x0_defaultAnim(defaultAnim), x4_(b), x8_scale(scale), xc_loop(loop)
    {}
};
}

#endif // CANIMPLAYBACKPARMS_HPP
