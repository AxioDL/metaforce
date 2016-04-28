#ifndef __URDE_CPASANIMINFO_HPP__
#define __URDE_CPASANIMINFO_HPP__

#include "IOStreams.hpp"
#include "rstl.hpp"
#include "CPASAnimParm.hpp"

namespace urde
{

class CPASAnimInfo
{
    u32 x0_id;
    rstl::reserved_vector<CPASAnimParm::UParmValue, 8> x4_parms;
public:
    CPASAnimInfo(u32 id, rstl::reserved_vector<CPASAnimParm::UParmValue, 8>&& parms);
    u32 GetAnimId() const {return x0_id;}
    CPASAnimParm::UParmValue GetAnimParmValue(u32 idx) const;
    CPASAnimParm GetAnimParmData(u32, CPASAnimParm::EParmType) const;
};

}

#endif // __URDE_CPASANIMINFO_HPP__
