#ifndef __CANIMPLAYBACKPARMS_HPP__
#define __CANIMPLAYBACKPARMS_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CAnimPlaybackParms
{
    s32   x0_;
    s32   x4_;
    float x8_;
    bool  xc_;
    s32   x10_ = 0;
    s32   x14_ = 0;
    bool  x18_ = false;
    s32   x1c_ = 0;
    s32   x20_ = 0;
    s32   x24_ = 0;
public:
    CAnimPlaybackParms(s32 a, s32 b, float c, bool d)
        : x0_(a), x4_(b), x8_(c), xc_(d)
    {}
};
}

#endif // CANIMPLAYBACKPARMS_HPP
