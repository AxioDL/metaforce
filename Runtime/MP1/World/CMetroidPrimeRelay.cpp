#include "Runtime/MP1/World/CMetroidPrimeRelay.hpp"

#include "Runtime/MP1/World/CMetroidPrimeExo.hpp"
#include "Runtime/CStateManager.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce::MP1 {

CMetroidPrimeRelay::CMetroidPrimeRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active,
                                       const zeus::CTransform& xf, const zeus::CVector3f& scale,
                                       SPrimeExoParameters&& parms, float f1, float f2, float f3, u32 w1, bool b1,
                                       u32 w2, const CHealthInfo& hInfo1, const CHealthInfo& hInfo2, u32 w3, u32 w4,
                                       u32 w5, rstl::reserved_vector<CMetroidPrimeAttackWeights, 4>&& roomParms)
: CEntity(uid, info, active, name)
, x38_xf(xf)
, x68_scale(scale)
, x74_parms(std::move(parms))
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
, xcb8_roomParms(std::move(roomParms)) {}

void CMetroidPrimeRelay::Accept(IVisitor& visitor) { visitor.Visit(this); }
void CMetroidPrimeRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  if (x34_mpUid != objId) {
    ForwardMessageToMetroidPrimeExo(msg, mgr);
  }
  if (msg == EScriptObjectMessage::InitializedInArea) {
    GetOrBuildMetroidPrimeExo(mgr);
  }
}

void CMetroidPrimeRelay::ForwardMessageToMetroidPrimeExo(EScriptObjectMessage msg, CStateManager& mgr) {
  if (auto* exo = CPatterned::CastTo<CMetroidPrimeExo>(mgr.ObjectById(x34_mpUid))) {
    mgr.SendScriptMsg(exo, GetUniqueId(), msg);
  }
}

void CMetroidPrimeRelay::GetOrBuildMetroidPrimeExo(CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  for (const auto act : mgr.GetPhysicsActorObjectList()) {
    if (CPatterned::CastTo<CMetroidPrimeExo>(act) != nullptr) {
      return;
    }
  }

  const auto& animParms = x74_parms.x4_patternedInfo.GetAnimationParameters();
  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), x68_scale, animParms.GetInitialAnimation(), true));
  auto* exo = new CMetroidPrimeExo(
      mgr.AllocateUniqueId(), "Metroid Prime! (Stage 1)"sv, CEntityInfo(GetAreaId(), NullConnectionList), x38_xf,
      std::move(mData), x74_parms.x4_patternedInfo, x74_parms.x13c_actorParms, x74_parms.x1a4_, x74_parms.x1a8_,
      x74_parms.x27c_, x74_parms.x350_, x74_parms.x424_, x74_parms.x460_particle1, x74_parms.x464_,
      x74_parms.x708_wpsc1, x74_parms.x70c_dInfo1, x74_parms.x728_shakeData1, x74_parms.x7fc_wpsc2,
      x74_parms.x800_dInfo2, x74_parms.x81c_shakeData2, x74_parms.x8f0_, x74_parms.x92c_, x74_parms.x948_,
      x74_parms.xa1c_particle2, x74_parms.xa20_swoosh, x74_parms.xa24_particle3, x74_parms.xa28_particle4,
      x74_parms.xa2c_);
  mgr.AddObject(exo);
  mgr.SendScriptMsg(exo, kInvalidUniqueId, EScriptObjectMessage::InitializedInArea);
}

} // namespace metaforce::MP1
