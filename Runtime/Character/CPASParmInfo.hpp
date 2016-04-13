#ifndef __URDE_CPASPARMINFO_HPP__
#define __URDE_CPASPARMINFO_HPP__

#include "IOStreams.hpp"
#include "CPASAnimParm.hpp"

namespace urde
{

class CPASParmInfo
{
    CPASAnimParm::EParmType x0_type;
    u32 x4_unk1;
    float x8_unk2;
    CPASAnimParm::UParmValue xc_val1;
    CPASAnimParm::UParmValue x10_val2;
public:
    CPASParmInfo(CInputStream& in);
    CPASAnimParm::EParmType GetType() const {return x0_type;}
};

}

#endif // __URDE_CPASPARMINFO_HPP__
