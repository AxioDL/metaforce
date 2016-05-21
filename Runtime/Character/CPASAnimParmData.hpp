#ifndef CPASANIMPARMDATA_HPP
#define CPASANIMPARMDATA_HPP
#include "RetroTypes.hpp"
#include "CPASAnimParm.hpp"

namespace urde
{
class CPASAnimParmData
{
    s32 x0_stateId;
    rstl::reserved_vector<CPASAnimParm,8> x4_parms;
public:
    CPASAnimParmData()
    {};

    CPASAnimParmData(s32 stateId, const CPASAnimParm& parm1, const CPASAnimParm& parm2, const CPASAnimParm& parm3,
                     const CPASAnimParm& parm4, const CPASAnimParm& parm5, const CPASAnimParm& parm6, const CPASAnimParm& parm7,
                     const CPASAnimParm& parm8);

    s32 GetStateId();
    const std::vector<CPASAnimParm>& GetAnimParmData() const;
};
}

#endif // CPASANIMPARMDATA_HPP
