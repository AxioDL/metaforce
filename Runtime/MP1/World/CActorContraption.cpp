#include "Runtime/MP1/World/CActorContraption.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CInt32POINode.hpp"
#include "Runtime/Weapon/CFlameInfo.hpp"
#include "Runtime/Weapon/CFlameThrower.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

MP1::CActorContraption::CActorContraption(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                          const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabox,
                                          const CMaterialList& matList, float mass, float zMomentum,
                                          const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                          const CActorParameters& aParams, CAssetId part, const CDamageInfo& dInfo,
                                          bool active)
: CScriptActor(uid, name, info, xf, std::move(mData), aabox, mass, zMomentum, matList, hInfo, dVuln, aParams, false,
               active, 0, 1.f, false, false, false, false)
, x300_flameThrowerGen(g_SimplePool->GetObj("FlameThrower"))
, x308_flameFxId(part)
, x30c_dInfo(dInfo) {}

void MP1::CActorContraption::Accept(IVisitor& visitor) { visitor.Visit(this); }

void MP1::CActorContraption::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool curActive = GetActive();
  switch (msg) {
  case EScriptObjectMessage::Registered:
    AddMaterial(EMaterialTypes::ScanPassthrough, mgr);
    break;
  case EScriptObjectMessage::Deleted:
    for (const std::pair<TUniqueId, std::string>& p : x2e8_children)
      mgr.FreeScriptObject(p.first);
    x2e8_children.clear();
    break;
  case EScriptObjectMessage::SetToZero:
    ResetFlameThrowers(mgr);
    break;
  default:
    break;
  }

  CScriptActor::AcceptScriptMsg(msg, uid, mgr);
  if (curActive != GetActive() && !GetActive()) {
    ResetFlameThrowers(mgr);
    CActor::RemoveEmitter();
  }
}

void MP1::CActorContraption::Think(float dt, CStateManager& mgr) {
  CScriptActor::Think(dt, mgr);

  for (const auto& [uid, name] : x2e8_children) {
    auto* act = static_cast<CFlameThrower*>(mgr.ObjectById(uid));

    if (act && act->GetActive()) {
      act->SetTransform(x34_transform * GetScaledLocatorTransform(name), dt);
    }
  }
}

void MP1::CActorContraption::ResetFlameThrowers(CStateManager& mgr) {
  for (const std::pair<TUniqueId, std::string>& uid : x2e8_children) {
    CFlameThrower* act = static_cast<CFlameThrower*>(mgr.ObjectById(uid.first));
    if (act && act->GetParticlesActive())
      act->Reset(mgr, false);
  }
}

void MP1::CActorContraption::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType evType,
                                             float dt) {
  if (evType == EUserEventType::DamageOff) {
    ResetFlameThrowers(mgr);
  } else if (evType == EUserEventType::DamageOn) {
    CFlameThrower* fl = CreateFlameThrower(node.GetLocatorName(), mgr);
    if (fl && !fl->GetParticlesActive())
      fl->Fire(GetTransform(), mgr, false);
  } else
    CActor::DoUserAnimEvent(mgr, node, evType, dt);
}

CFlameThrower* MP1::CActorContraption::CreateFlameThrower(std::string_view name, CStateManager& mgr) {
  const auto it =
      std::find_if(x2e8_children.cbegin(), x2e8_children.cend(), [&name](const auto& p) { return p.second == name; });

  if (it != x2e8_children.cend()) {
    return static_cast<CFlameThrower*>(mgr.ObjectById(it->first));
  }

  const TUniqueId id = mgr.AllocateUniqueId();
  const CFlameInfo flameInfo(6, 6, x308_flameFxId, 20, 0.5f, 1.f, 1.f);
  auto* ret = new CFlameThrower(x300_flameThrowerGen, name, EWeaponType::Missile, flameInfo, zeus::CTransform(),
                                EMaterialTypes::CollisionActor, x30c_dInfo, id, GetAreaId(), GetUniqueId(),
                                EProjectileAttrib::None, CAssetId(), -1, CAssetId());

  x2e8_children.emplace_back(id, name);

  mgr.AddObject(ret);
  return ret;
}
} // namespace metaforce
