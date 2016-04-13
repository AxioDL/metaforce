#ifndef __URDE_CPRIMITIVE_HPP__
#define __URDE_CPRIMITIVE_HPP__

#include "IOStreams.hpp"
#include "RetroTypes.hpp"

namespace urde
{

class CPrimitive
{
    TResId x0_animId;
    u32 x4_animIdx;
    std::string x8_animName;
public:
    CPrimitive(CInputStream& in);
    TResId GetAnimResId() const {return x0_animId;}
};

}

#endif // __URDE_CPRIMITIVE_HPP__
