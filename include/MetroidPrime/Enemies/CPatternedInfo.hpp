#ifndef _CPATTERNEDINFO
#define _CPATTERNEDINFO

#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/pair.hpp"

class CPatternedInfo {
  friend class CPatterned;

public:
  CPatternedInfo(CInputStream& in, uint pcount);

  float GetTurnSpeed() const { return mTurnSpeed; }
  float GetDetectionHeightRange() const { return mDetectionHeightRange; }
  const CHealthInfo& GetHealthInfo() const { return mHealthInfo; }
  const CDamageVulnerability& GetDamageVulnerability() const { return mDamageVulnerability; }
  const float& GetHalfExtent() const { return mHalfExtent; }
  const float& GetHeight() const { return mHeight; }
  const CVector3f GetBodyOrigin() const { return mBodyOrigin; }
  CAnimationParameters& GetAnimationParameters() { return mAnimParams; }
  const CAnimationParameters& GetAnimationParameters() const { return mAnimParams; }
  uint GetPathfindingIndex() const { return mPathfindingIndex; }
  bool GetActive() const { return mActive; }
  void SetActive(bool active) { mActive = active; }

  static rstl::pair< bool, uint > HasCorrectParameterCount(CInputStream& in, const char* name);

public:
  float mMass;
  float mSpeed;
  float mTurnSpeed;
  float mDetectionRange;
  float mDetectionHeightRange;
  float mDectectionAngle;
  float mMinAttackRange;
  float mMaxAttackRange;
  float mAverageAttackTime;
  float mAttackTimeVariation;
  float mLeashRadius;
  float mPlayerLeashRadius;
  float mPlayerLeashTime;
  CDamageInfo mContactDamageInfo;
  float mDamageWaitTime;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDamageVulnerability;
  float mHalfExtent;
  float mHeight;
  CVector3f mBodyOrigin;
  float mStepUpHeight;
  float mXDamage;
  float mFrozenXDamage;
  float mXDamageDelay;
  uint mDeathSfx;
  CAnimationParameters mAnimParams;
  bool mActive;
  CAssetId mStateMachineId;
  float mIntoFreezeDur;
  float mOutofFreezeDur;
  float mFreezeDur;
  uint mPathfindingIndex;
  CVector3f mParticle1Scale;
  CAssetId mParticle1;
  CAssetId mElectric;
  CVector3f mParticle2Scale;
  CAssetId mParticle2;
  uint mIceShatterSfx;
};
CHECK_SIZEOF(CPatternedInfo, 0x138)

#endif // _CPATTERNEDINFO
