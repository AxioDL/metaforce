#ifndef __PSHAG_CPASANIMINFO_HPP__
#define __PSHAG_CPASANIMINFO_HPP__

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
    u32 GetId() const {return x0_id;}
};

}

#endif // __PSHAG_CPASANIMINFO_HPP__
