#include "MetroidPrime/Enemies/CMetroidPrimeRelay.hpp"

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CCameraShakeData LoadAndBuildPrimeCameraShakeData(CInputStream& in);

CMetroidPrimeIceAttack::CMetroidPrimeIceAttack(CInputStream& in)
: x0_propertyCount(in.ReadLong())
, x4_particle1(in.ReadLong())
, x8_particle2(in.ReadLong())
, xc_particle3(in.ReadLong())
, x10_dInfo(in)
, x2c_(in.ReadFloat())
, x30_(in.ReadFloat())
, x34_texture(in.ReadLong())
, x38_(CSfxManager::TranslateSFXID(in.ReadLong()))
, x3a_(CSfxManager::TranslateSFXID(in.ReadLong())) {}

uint CMetroidPrimeData::VerifyExportCount(CInputStream& in) { return in.ReadLong(); }

CMetroidPrimeData::CMetroidPrimeData(CInputStream& in)
: x0_propertyCount(VerifyExportCount(in))
, x4_patternedInfo(in, CPatternedInfo::HasCorrectParameterCount(in, "Metroid Prime").second)
, x13c_actorParms(LoadActorParameters(in))
, x1a4_(in.ReadLong() - 1)
, x1a8_(LoadAndBuildPrimeCameraShakeData(in))
, x27c_(LoadAndBuildPrimeCameraShakeData(in))
, x350_(LoadAndBuildPrimeCameraShakeData(in))
, x424_(in)
, x460_particle1(in.ReadLong())
, x464_(LoadParasiteQueenBeams(in))
, x708_wpsc1(in.ReadLong())
, x70c_dInfo1(in)
, x728_shakeData1(LoadAndBuildPrimeCameraShakeData(in))
, x7fc_wpsc2(in.ReadLong())
, x800_dInfo2(in)
, x81c_shakeData2(LoadAndBuildPrimeCameraShakeData(in))
, x8f0_(in)
, x92c_(in)
, x948_(LoadAndBuildPrimeCameraShakeData(in))
, xa1c_particle2(in.ReadLong())
, xa20_swoosh(in.ReadLong())
, xa24_particle3(in.ReadLong())
, xa28_particle4(in.ReadLong())
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
: x0_beamInfo(in)
, x44_(in.ReadLong())
, x48_dInfo1(in)
, x64_struct5(in)
, x88_(in.ReadFloat())
, x8c_dInfo2(in) {}

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
: x0_attackTime(in.ReadFloat())
, x4_sustainTime(in.ReadFloat())
, x8_duration(in.ReadFloat())
, xc_magnitude(in.ReadFloat()) {}

SPrimeCameraShakerComponent::SPrimeCameraShakerComponent(CInputStream& in)
: x0_useModulation(in.ReadBool()), x4_am(in), x14_fm(in) {}

SPrimeCameraShakeData::SPrimeCameraShakeData(CInputStream& in)
: x0_useSfx(in.ReadBool())
, x4_duration(in.ReadFloat())
, x8_sfxDist(in.ReadFloat())
, xc_shakerX(in)
, x30_shakerY(in)
, x54_shakerZ(in) {}

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
, x34_mpUid(kInvalidUniqueId)
, x38_xf(xf)
, x68_scale(scale)
, x74_parms(parms)
, xc84_f1(f1)
, xc88_f2(f2)
, xc8c_f3(f3)
, xc90_w1(w1)
, xc94_b1(b1)
, xc98_w2(w2)
, xc9c_hInfo1(hInfo1)
, xca4_hInfo2(hInfo2)
, xcac_w3(w3)
, xcb0_w4(w4)
, xcb4_w5(w5)
, xcb8_roomParms(roomParms) {}

ENTITY_ACCEPT_IMPL(CMetroidPrimeRelay)

void CMetroidPrimeRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                         CStateManager& mgr) {
  if (uid != x34_mpUid) {
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
  CMetroidPrime* pat = PATTERNED_CAST_TO(CMetroidPrime, mgr.ObjectById(x34_mpUid));
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

  const CAnimationParameters& animParms = x74_parms.x4_patternedInfo.GetAnimationParameters();
  CMetroidPrime* mp = rs_new CMetroidPrime(
      mgr.AllocateUniqueId(), rstl::string_l("Metroid Prime! (Stage 1)"),
      CEntityInfo(GetCurrentAreaId(), NullConnectionList), x38_xf,
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), x68_scale,
               animParms.GetInitialAnimation(), true),
      x74_parms.x4_patternedInfo, x74_parms.x13c_actorParms, x74_parms.x1a4_, x74_parms.x1a8_,
      x74_parms.x27c_, x74_parms.x350_, x74_parms.x424_, x74_parms.x460_particle1, x74_parms.x464_,
      x74_parms.x708_wpsc1, x74_parms.x70c_dInfo1, x74_parms.x728_shakeData1, x74_parms.x7fc_wpsc2,
      x74_parms.x800_dInfo2, x74_parms.x81c_shakeData2, x74_parms.x8f0_, x74_parms.x92c_,
      x74_parms.x948_, x74_parms.xa1c_particle2, x74_parms.xa20_swoosh, x74_parms.xa24_particle3,
      x74_parms.xa28_particle4, x74_parms.xa2c_);

  mgr.AddObject(*mp);
  mgr.DeliverScriptMsg(mp, kInvalidUniqueId, kSM_InitializedInArea);
}
