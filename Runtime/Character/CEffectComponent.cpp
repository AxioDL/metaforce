#include "Runtime/Character/CEffectComponent.hpp"

namespace urde {

SObjectTag CEffectComponent::GetSObjectTagFromStream(CInputStream& in) { return SObjectTag(in); }

CEffectComponent::CEffectComponent(CInputStream& in) {
  x0_name = in.readString();
  x10_tag = GetSObjectTagFromStream(in);
  x18_boneName = in.readString();
  x28_scale = in.readFloatBig();
  x2c_parentedMode = CParticleData::EParentedMode(in.readUint32Big());
  x30_flags = in.readUint32Big();
}

} // namespace urde
