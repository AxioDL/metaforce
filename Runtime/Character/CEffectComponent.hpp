#ifndef __URDE_CEFFECTCOMPONENT_HPP__
#define __URDE_CEFFECTCOMPONENT_HPP__

#include "IOStreams.hpp"
#include "RetroTypes.hpp"

namespace urde
{

class CEffectComponent
{
    std::string x0_name;
    SObjectTag x10_tag;
    std::string x18_boneName;
    float x28_scale;
    u32 x2c_parentedMode;
    u32 x30_flags;
    static SObjectTag GetSObjectTagFromStream(CInputStream& in);
public:
    CEffectComponent(CInputStream& in);

    const std::string& GetComponentName() const;
    const SObjectTag& GetParticleTag() const;
    const std::string& GetSegmentName() const;
    float GetScale() const;
    u32 GetParentedMode() const;
    u32 GetFlags() const;
};

}

#endif // __URDE_CEFFECTCOMPONENT_HPP__
