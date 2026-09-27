#include "MetroidPrime/Enemies/CPatternedInfo.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CPatternedInfo::CPatternedInfo(CInputStream& in, uint pcount)
: mMass(in.ReadFloat())
, mSpeed(in.ReadFloat())
, mTurnSpeed(in.ReadFloat())
, mDetectionRange(in.ReadFloat())
, mDetectionHeightRange(in.ReadFloat())
, mDectectionAngle(in.ReadFloat())
, mMinAttackRange(in.ReadFloat())
, mMaxAttackRange(in.ReadFloat())
, mAverageAttackTime(in.ReadFloat())
, mAttackTimeVariation(in.ReadFloat())
, mLeashRadius(in.ReadFloat())
, mPlayerLeashRadius(in.ReadFloat())
, mPlayerLeashTime(in.ReadFloat())
, mContactDamageInfo(in)
, mDamageWaitTime(in.ReadFloat())
, mHealthInfo(in)
, mDamageVulnerability(in)
, mHalfExtent(in.ReadFloat())
, mHeight(in.ReadFloat())
, mBodyOrigin(in)
, mStepUpHeight(in.ReadFloat())
, mXDamage(in.ReadFloat())
, mFrozenXDamage(in.ReadFloat())
, mXDamageDelay(in.ReadFloat())
, mDeathSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, mAnimParams(in)
, mActive(in.ReadBool())
, mStateMachineId(in.ReadLong())
, mIntoFreezeDur(in.ReadFloat())
, mOutofFreezeDur(in.ReadFloat())
, mFreezeDur(in.ReadFloat())

, mPathfindingIndex(in.ReadLong())

, mParticle1Scale(in)
, mParticle1(in.ReadLong())
, mElectric(in.ReadLong())
, mParticle2Scale(pcount >= 0x24 ? CVector3f(in) : CVector3f::Zero())
, mParticle2(pcount >= 0x25 ? in.ReadLong() : 0xffffffff)

, mIceShatterSfx(pcount >= 0x26 ? CSfxManager::TranslateSFXID(in.ReadLong())
                                    : CSfxManager::kInternalInvalidSfxId)

{}

rstl::pair< bool, uint > CPatternedInfo::HasCorrectParameterCount(CInputStream& in, const char* name) {
  u32 pcount = in.ReadLong();
  return rstl::pair< bool, uint >((pcount >= 35 && pcount <= 38), pcount);
}
