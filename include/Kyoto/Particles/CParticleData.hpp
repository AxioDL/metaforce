#ifndef _CPARTICLEDATA
#define _CPARTICLEDATA

#include "types.h"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/string.hpp"

class CInputStream;
class CParticleData {
public:
  enum EParentedMode {
    kPM_Initial,
    kPM_ContinuousEmitter,
    kPM_ContinuousSystem,
  };

  CParticleData(int duration = 0, const SObjectTag& tag = SObjectTag(0, 0),
                const rstl::string& boneName = rstl::string_l("root"), float scale = 1.f,
                EParentedMode mode = kPM_Initial)
  : mDuration(duration)
  , mParticle(tag)
  , mBoneName(boneName)
  , mScale(scale)
  , mParentMode(mode) {}

  CParticleData(CInputStream& in);

  int GetDuration() const { return mDuration; }
  SObjectTag GetParticleAssetInfo() const {
    return SObjectTag(mParticle.GetType(), mParticle.GetId());
  }
  const rstl::string& GetLocatorName() const { return mBoneName; }
  float GetScale() const { return mScale; }
  EParentedMode GetParentedMode() const { return static_cast< EParentedMode >(mParentMode); }

private:
  int mDuration;
  SObjectTag mParticle;
  rstl::string mBoneName;
  float mScale;
  int mParentMode;
};

class CAuxiliaryParticleData {
public:
  CAuxiliaryParticleData(uint duration, const SObjectTag& tag, const CVector3f& translation,
                         float scale)
  : mDuration(duration), mParticle(tag), mTranslation(translation), mScale(scale) {}

  uint GetDuration() const { return mDuration; }
  SObjectTag GetParticleAssetInfo() const {
    return SObjectTag(mParticle.GetType(), mParticle.GetId());
  }
  const CVector3f& GetTranslation() const { return mTranslation; }
  float GetScale() const { return mScale; }

private:
  uint mDuration;
  SObjectTag mParticle;
  CVector3f mTranslation;
  float mScale;
};

#endif // _CPARTICLEDATA
