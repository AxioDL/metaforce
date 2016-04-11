#ifndef __PSHAG_CPRIMITIVE_HPP__
#define __PSHAG_CPRIMITIVE_HPP__

#include "IOStreams.hpp"

namespace urde
{

class CPrimitive
{
    TResId x0_animId;
    u32 x4_animIdx;
    std::string x8_animName;
public:
    CPrimitive(CInputStream& in);
};

}

#endif // __PSHAG_CPRIMITIVE_HPP__
