#include "CEffectComponent.hpp"

namespace urde
{

SObjectTag CEffectComponent::GetSObjectTagFromStream(CInputStream& in) { return SObjectTag(in); }

CEffectComponent::CEffectComponent(CInputStream& in)
{
    x0_name = in.readString();
    x10_tag = GetSObjectTagFromStream(in);
    x18_boneName = in.readString();
    x28_scale = in.readFloatBig();
    x2c_parentedMode = in.readUint32Big();
    x30_flags = in.readUint32Big();
}

const std::string& CEffectComponent::GetComponentName() const { return x0_name; }

const SObjectTag& CEffectComponent::GetParticleTag() const { return x10_tag; }

const std::string& CEffectComponent::GetSegmentName() const { return x18_boneName; }

float CEffectComponent::GetScale() const { return x28_scale; }

u32 CEffectComponent::GetParentedMode() const { return x2c_parentedMode; }

u32 CEffectComponent::GetFlags() const { return x30_flags; }
}
