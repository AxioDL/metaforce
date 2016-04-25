#ifndef __URDE_CHEALTHINFO_HPP__
#define __URDE_CHEALTHINFO_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CHealthInfo
{
    float x0_health;
    float x4_knockbackResistance;
public:
    CHealthInfo(CInputStream& in);
    float GetHealth() const {return x0_health;}
    float GetKnockbackResistance() const {return x4_knockbackResistance;}
};

}

#endif // __URDE_CHEALTHINFO_HPP__
