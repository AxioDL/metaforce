#ifndef __URDE_CPRIMITIVE_HPP__
#define __URDE_CPRIMITIVE_HPP__

#include "IOStreams.hpp"
#include "RetroTypes.hpp"

namespace urde
{

class CPrimitive
{
    ResId x0_animId;
    u32 x4_animIdx;
    std::string x8_animName;
public:
    CPrimitive(CInputStream& in);
    ResId GetAnimResId() const { return x0_animId; }
    u32 GetAnimDbIdx() const { return x4_animIdx; }
    const std::string& GetName() const { return x8_animName; }
};

}

#endif // __URDE_CPRIMITIVE_HPP__
