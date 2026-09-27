#include "MetroidPrime/Enemies/CMetroidPrimeRelay.hpp"

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CCameraShakeData LoadAndBuildPrimeCameraShakeData(CInputStream& in);

CMetroidPrimeIceAttack::CMetroidPrimeIceAttack(CInputStream& in)
: mPropertyCount(in.ReadLong())
, mParticle1(in.ReadLong())
, mParticle2(in.ReadLong())
, mParticle3(in.ReadLong())
, mDInfo(in)
, x2c_(in.ReadFloat())
, x30_(in.ReadFloat())
, mTexture(in.ReadLong())
, x38_(CSfxManager::TranslateSFXID(in.ReadLong()))
, x3a_(CSfxManager::TranslateSFXID(in.ReadLong())) {}

uint CMetroidPrimeData::VerifyExportCount(CInputStream& in) { return in.ReadLong(); }

CMetroidPrimeData::CMetroidPrimeData(CInputStream& in)
: mPropertyCount(VerifyExportCount(in))
, mPatternedInfo(in, CPatternedInfo::HasCorrectParameterCount(in, "Metroid Prime").second)
, mActorParms(LoadActorParameters(in))
, x1a4_(in.ReadLong() - 1)
, x1a8_(LoadAndBuildPrimeCameraShakeData(in))
, x27c_(LoadAndBuildPrimeCameraShakeData(in))
, x350_(LoadAndBuildPrimeCameraShakeData(in))
, x424_(in)
, mParticle1(in.ReadLong())
, x464_(LoadParasiteQueenBeams(in))
, mWpsc1(in.ReadLong())
, mDInfo1(in)
, mShakeData1(LoadAndBuildPrimeCameraShakeData(in))
, mWpsc2(in.ReadLong())
, mDInfo2(in)
, mShakeData2(LoadAndBuildPrimeCameraShakeData(in))
, x8f0_(in)
, x92c_(in)
, x948_(LoadAndBuildPrimeCameraShakeData(in))
, mParticle2(in.ReadLong())
, mSwoosh(in.ReadLong())
, mParticle3(in.ReadLong())
, mParticle4(in.ReadLong())
, xa2c_(LoadVulnerabilities(in)) {}

CWeaponAssetInfo::CWeaponAssetInfo(CInputStream& in) : count(in.ReadLong()) {
  data[0] = in.ReadLong();
  data[1] = in.ReadLong();
  data[2] = in.ReadLong();
  data[3] = in.ReadLong();
  data[4] = in.ReadLong();
  data[5] = in.ReadLong();
  data[6] = in.ReadLong();
  data[7] = in.ReadLong();
}

CMetroidPrimeParasiteQueenAttack::CMetroidPrimeParasiteQueenAttack(CInputStream& in)
: mBeamInfo(in)
, x44_(in.ReadLong())
, mDInfo1(in)
, mStruct5(in)
, x88_(in.ReadFloat())
, mDInfo2(in) {}

rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, 4 >
CMetroidPrimeData::LoadParasiteQueenBeams(CInputStream& in) {
  rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, 4 > result;
  for (int i = 0; i < 4; ++i) {
    result.push_back(CMetroidPrimeParasiteQueenAttack(in));
  }
  return result;
}

rstl::reserved_vector< CMetroidPrime::CVulnerabilityEntry, 4 >
CMetroidPrimeData::LoadVulnerabilities(CInputStream& in) {
  rstl::reserved_vector< CMetroidPrime::CVulnerabilityEntry, 4 > result;
  for (int i = 0; i < 4; ++i) {
    result.push_back(CMetroidPrime::CVulnerabilityEntry(in));
  }
  return result;
}

SPrimeCameraShakePoint::SPrimeCameraShakePoint(CInputStream& in)
: mAttackTime(in.ReadFloat())
, mSustainTime(in.ReadFloat())
, mDuration(in.ReadFloat())
, mMagnitude(in.ReadFloat()) {}

SPrimeCameraShakerComponent::SPrimeCameraShakerComponent(CInputStream& in)
: mUseModulation(in.ReadBool()), mAm(in), mFm(in) {}

SPrimeCameraShakeData::SPrimeCameraShakeData(CInputStream& in)
: mUseSfx(in.ReadBool())
, mDuration(in.ReadFloat())
, mSfxDist(in.ReadFloat())
, mShakerX(in)
, mShakerY(in)
, mShakerZ(in) {}

CCameraShakeData LoadAndBuildPrimeCameraShakeData(CInputStream& in) {
  return BuildCameraShakeData(SPrimeCameraShakeData(in));
}

CMetroidPrimeRelay::CMetroidPrimeRelay(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const bool active,
    const CTransform4f& xf, const CVector3f& scale, const CMetroidPrimeData& parms, const float f1,
    const float f2, const float f3, const int w1, const bool b1, const int w2,
    const CHealthInfo& hInfo1, const CHealthInfo& hInfo2, const int w3, const int w4, const int w5,
    const rstl::reserved_vector< CMetroidPrime::CMetroidPrimeAttackWeights, 4 >& roomParms)
: CEntity(uid, info, active, name)
, mMpUid(kInvalidUniqueId)
, mXf(xf)
, mScale(scale)
, mParms(parms)
, mF1(f1)
, mF2(f2)
, mF3(f3)
, mW1(w1)
, mB1(b1)
, mW2(w2)
, mHInfo1(hInfo1)
, mHInfo2(hInfo2)
, mW3(w3)
, mW4(w4)
, mW5(w5)
, mRoomParms(roomParms) {}

ENTITY_ACCEPT_IMPL(CMetroidPrimeRelay)

void CMetroidPrimeRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                         CStateManager& mgr) {
  if (uid != mMpUid) {
    ForwardScriptMessageToMP(msg, mgr);
  }
  switch (msg) {
  case kSM_InitializedInArea:
    CreateMetroidPrime(mgr);
    break;
  default:
    break;
  }
}

void CMetroidPrimeRelay::ForwardScriptMessageToMP(EScriptObjectMessage msg, CStateManager& mgr) {
  CMetroidPrime* pat = PATTERNED_CAST_TO(CMetroidPrime, mgr.ObjectById(mMpUid));
  if (!pat) {
    return;
  }
  if (msg < kSM_WorldInitialized && msg >= kSM_Registered) {
    return;
  }
  mgr.DeliverScriptMsg(pat, GetUniqueId(), msg);
}

void CMetroidPrimeRelay::CreateMetroidPrime(CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CObjectList& list = mgr.ObjectListById(kOL_PhysicsActor);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CEntity* ent = list[i];
    if (PATTERNED_CAST_TO(CMetroidPrime, ent) != nullptr) {
      return;
    }
  }

  const CAnimationParameters& animParms = mParms.mPatternedInfo.GetAnimationParameters();
  CMetroidPrime* mp = rs_new CMetroidPrime(
      mgr.AllocateUniqueId(), rstl::string_l("Metroid Prime! (Stage 1)"),
      CEntityInfo(GetCurrentAreaId(), NullConnectionList), mXf,
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), mScale,
               animParms.GetInitialAnimation(), true),
      mParms.mPatternedInfo, mParms.mActorParms, mParms.x1a4_, mParms.x1a8_,
      mParms.x27c_, mParms.x350_, mParms.x424_, mParms.mParticle1, mParms.x464_,
      mParms.mWpsc1, mParms.mDInfo1, mParms.mShakeData1, mParms.mWpsc2,
      mParms.mDInfo2, mParms.mShakeData2, mParms.x8f0_, mParms.x92c_,
      mParms.x948_, mParms.mParticle2, mParms.mSwoosh, mParms.mParticle3,
      mParms.mParticle4, mParms.xa2c_);

  mgr.AddObject(*mp);
  mgr.DeliverScriptMsg(mp, kInvalidUniqueId, kSM_InitializedInArea);
}
