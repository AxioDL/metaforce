#include "Runtime/Character/CEffectComponent.hpp"

namespace metaforce {

SObjectTag CEffectComponent::GetSObjectTagFromStream(CInputStream& in) { return in.Get<SObjectTag>(); }

CEffectComponent::CEffectComponent(CInputStream& in) {
  x0_name = in.Get<std::string>();
  x10_tag = GetSObjectTagFromStream(in);
  x18_boneName = in.Get<std::string>();
  x28_scale = in.ReadFloat();
  x2c_parentedMode = CParticleData::EParentedMode(in.ReadLong());
  x30_flags = in.ReadLong();
}

} // namespace metaforce
