#ifndef _CEFFECTCOMPONENT
#define _CEFFECTCOMPONENT

#include "Kyoto/Particles/CParticleData.hpp"

#include "rstl/string.hpp"

class CEffectComponent {
public:
  explicit CEffectComponent(CInputStream& in);

  const rstl::string& GetComponentName() const { return mName; }
  const SObjectTag& GetParticleTag() const { return mTag; }
  const rstl::string& GetSegmentName() const { return mBoneName; }
  float GetScale() const { return mScale; }
  CParticleData::EParentedMode GetParentedMode() const { return mParentedMode; }
  uint GetFlags() const {return mFlags; }

private:
  SObjectTag GetSObjectTagFromStream(CInputStream& in);

  rstl::string mName;
  SObjectTag mTag;
  rstl::string mBoneName;
  float mScale;
  CParticleData::EParentedMode mParentedMode;
  uint mFlags;
};

#endif // _CEFFECTCOMPONENT
