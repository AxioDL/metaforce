#ifndef __URDE_CANIMATIONPARAMETERS_HPP__
#define __URDE_CANIMATIONPARAMETERS_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CAnimationParameters
{
    ResId x0_ancs;
    s32 x4_charIdx;
    u32 x8_defaultAnim;
public:
    CAnimationParameters(ResId ancs, s32 charIdx, u32 defaultAnim)
    : x0_ancs(ancs), x4_charIdx(charIdx), x8_defaultAnim(defaultAnim) {}
};

}

#endif // __URDE_CANIMATIONPARAMETERS_HPP__
