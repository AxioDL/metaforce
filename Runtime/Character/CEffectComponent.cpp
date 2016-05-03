#include "CEffectComponent.hpp"

namespace urde
{

SObjectTag CEffectComponent::GetSObjectTagFromStream(CInputStream& in)
{
    return SObjectTag(in);
}

CEffectComponent::CEffectComponent(CInputStream& in)
{
    x0_name = in.readString();
    x10_tag = GetSObjectTagFromStream(in);
    x18_boneName = in.readString();
    x28_scale = in.readFloatBig();
    x2c_ = in.readUint32Big();
    x30_ = in.readUint32Big();
}

const std::string& CEffectComponent::GetComponentName() const
{
    return x0_name;
}

const SObjectTag& CEffectComponent::GetParticleTag() const
{
    return x10_tag;
}

}
