#ifndef __URDE_CPASANIMSTATE_HPP__
#define __URDE_CPASANIMSTATE_HPP__

#include "IOStreams.hpp"
#include "CPASParmInfo.hpp"
#include "CPASAnimInfo.hpp"

namespace urde
{

class CPASAnimState
{
    u32 x0_id;
    std::vector<CPASParmInfo> x4_parms;
    std::vector<CPASAnimInfo> x14_anims;
    std::vector<u32> x24_;
public:
    CPASAnimState(CInputStream& in);
    u32 GetId() const {return x0_id;}
};

}

#endif // __URDE_CPASANIMSTATE_HPP__
