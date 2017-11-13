#ifndef __URDE_CEFFECTCOMPONENT_HPP__
#define __URDE_CEFFECTCOMPONENT_HPP__

#include "IOStreams.hpp"
#include "RetroTypes.hpp"
#include "CParticleData.hpp"

namespace urde
{

class CEffectComponent
{
    std::string x0_name;
    SObjectTag x10_tag;
    std::string x18_boneName;
    float x28_scale;
    CParticleData::EParentedMode x2c_parentedMode;
    u32 x30_flags;
    static SObjectTag GetSObjectTagFromStream(CInputStream& in);
public:
    CEffectComponent(CInputStream& in);

    std::string_view GetComponentName() const { return x0_name; }
    const SObjectTag& GetParticleTag() const { return x10_tag; }
    std::string_view GetSegmentName() const { return x18_boneName; }
    float GetScale() const { return x28_scale; }
    CParticleData::EParentedMode GetParentedMode() const { return x2c_parentedMode; }
    u32 GetFlags() const { return x30_flags; }
};

}

#endif // __URDE_CEFFECTCOMPONENT_HPP__
