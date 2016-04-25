#ifndef __URDE_CANIMATIONPARAMETERS_HPP__
#define __URDE_CANIMATIONPARAMETERS_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CAnimationParameters
{
public:
    ResId x0_ancs;
    s32 x4_charIdx;
    u32 x8_defaultAnim;
    CAnimationParameters(ResId ancs, s32 charIdx, u32 defaultAnim)
    : x0_ancs(ancs), x4_charIdx(charIdx), x8_defaultAnim(defaultAnim) {}
    CAnimationParameters(CInputStream& in)
    : x0_ancs(in.readUint32Big()),
      x4_charIdx(in.readUint32Big()),
      x8_defaultAnim(in.readUint32Big()) {}
};

}

#endif // __URDE_CANIMATIONPARAMETERS_HPP__
