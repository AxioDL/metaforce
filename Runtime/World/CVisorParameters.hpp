#ifndef __URDE_CVISORPARAMETERS_HPP__
#define __URDE_CVISORPARAMETERS_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CVisorParameters
{
public:
    u32 x0_mask : 4;
    bool x0_4_b1 : 1;
    bool x0_5_b2 : 1;
    bool x0_28_b3 : 1;
    bool x0_29_b4 : 1;
    CVisorParameters()
    : x0_mask(0xf), x0_4_b1(false), x0_5_b2(false) {}
    CVisorParameters(u32 mask, bool b1, bool b2)
    : x0_mask(mask), x0_4_b1(b1), x0_5_b2(b2) {}
};

}

#endif // __URDE_CVISORPARAMETERS_HPP__
