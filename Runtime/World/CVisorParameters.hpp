#ifndef __URDE_CVISORPARAMETERS_HPP__
#define __URDE_CVISORPARAMETERS_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CVisorParameters
{
    u8 mask : 4;
    bool b1 : 1;
    bool b2 : 1;
public:
    CVisorParameters()
    : mask(0xf), b1(false), b2(false) {}
};

}

#endif // __URDE_CVISORPARAMETERS_HPP__
