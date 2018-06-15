#ifndef __URDE_CVISORPARAMETERS_HPP__
#define __URDE_CVISORPARAMETERS_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CVisorParameters
{
public:
    u8 x0_mask : 4;
    bool x0_4_b1 : 1;
    bool x0_5_scanPassthrough : 1;
    CVisorParameters()
    : x0_mask(0xf), x0_4_b1(false), x0_5_scanPassthrough(false) {}
    CVisorParameters(u8 mask, bool b1, bool scanPassthrough)
    : x0_mask(mask), x0_4_b1(b1), x0_5_scanPassthrough(scanPassthrough) {}
    u8 GetMask() const { return x0_mask; }
};

}

#endif // __URDE_CVISORPARAMETERS_HPP__
