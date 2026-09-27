#include "Kyoto/Particles/CEffectComponent.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CEffectComponent::CEffectComponent(CInputStream& in)
: mName(in)
, mTag(GetSObjectTagFromStream(in))
, mBoneName(in)
, mScale(in.Get<float>())
, mParentedMode(CParticleData::EParentedMode(in.Get<uint>()))
, mFlags(in.Get<uint>()) {}


SObjectTag CEffectComponent::GetSObjectTagFromStream(CInputStream& in) {
  const FourCC type = in.Get<CAssetId>();
  const CAssetId id = in.Get<CAssetId>();  
  return SObjectTag(type, id);
}
