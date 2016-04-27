#ifndef __URDE_CPASANIMSTATE_HPP__
#define __URDE_CPASANIMSTATE_HPP__

#include "IOStreams.hpp"
#include "CPASParmInfo.hpp"
#include "CPASAnimInfo.hpp"

namespace urde
{
class CRandom16;
class CPASAnimParmData;
class CPASAnimState
{
    s32 x0_id;
    std::vector<CPASParmInfo> x4_parms;
    std::vector<CPASAnimInfo> x14_anims;
    std::vector<u32> x24_;
public:
    CPASAnimState(CInputStream& in);
    s32 GetStateId() const {return x0_id;}
    s32 GetNumAnims() const { return x14_anims.size(); }
    const CPASAnimParmData& GetAnimParmData(s32, u32) const;
    std::pair<float,s32> FindBestAnimation(const rstl::reserved_vector<CPASAnimParm,8>&, CRandom16&, s32) const;
    float ComputeExactMatchWeight(u32, const CPASAnimParm&, CPASAnimParm::UParmValue) const;
    float ComputePercentErrorWeight(u32, const CPASAnimParm&, CPASAnimParm::UParmValue) const;
    float ComputeAngularPercentErrorWeight(u32, const CPASAnimParm&, CPASAnimParm::UParmValue) const;
};

}

#endif // __URDE_CPASANIMSTATE_HPP__
