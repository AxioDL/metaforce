#include "MetroidPrime/Enemies/CScriptContraption.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "MetroidPrime/Weapons/CFlameThrower.hpp"
#include "rstl/algorithm.hpp"

CScriptContraption::CScriptContraption(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& mData, const CAABox& aabox, const CMaterialList& matList, const float mass,
    const float zMomentum, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
    const CActorParameters& aParams, const CAssetId part, CDamageInfo dInfo, const bool active)
: CScriptActor(uid, name, info, xf, mData, aabox, matList, mass, zMomentum, hInfo, dVuln, aParams,
               false, active, 0, 1.f, false, false, false, false)
, mFlameThrowerGenDesc(gpSimplePool->GetObj("FlameThrower"))
, mFlameFxId(part)
, mDInfo(dInfo) {}

CFlameThrower* CScriptContraption::CreateFlameThrower(const rstl::string& name,
                                                      CStateManager& mgr) {
  for (rstl::list< SFlameThrower >::const_iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    if (it->name == name) {
      return static_cast< CFlameThrower* >(mgr.ObjectById(it->id));
    }
  }

  const CFlameInfo flameInfo(6, 6, mFlameFxId, 20, 0.5f, 1.f, 1.f);
  const TUniqueId id = mgr.AllocateUniqueId();
  CEntity* ret =
      rs_new CFlameThrower(mFlameThrowerGenDesc, rstl::string_l("Contraption_Flame"),
                           kWT_Plasma, flameInfo, CTransform4f::Identity(), kMT_CollisionActor,
                           mDInfo, id, GetCurrentAreaId(), GetUniqueId(), CWeapon::kPA_None,
                           kInvalidAssetId, CSfxManager::kInternalInvalidSfxId, kInvalidAssetId);
  mgr.AddObject(*ret);
  mChildren.push_back(SFlameThrower(id, name));
  return static_cast< CFlameThrower* >(ret);
}

void CScriptContraption::Think(float dt, CStateManager& mgr) {
  CScriptActor::Think(dt, mgr);
  for (rstl::list< SFlameThrower >::iterator uid = mChildren.begin();
       uid != mChildren.end(); ++uid) {
    CFlameThrower* act = static_cast< CFlameThrower* >(mgr.ObjectById(uid->id));
    if (act && act->GetActive()) {
      CTransform4f xf = GetTransform() * GetScaledLocatorTransform(uid->name);
      act->SetTransform(xf, mgr, dt);
    }
  }
}

void CScriptContraption::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                         CStateManager& mgr) {
  const bool curActive = GetActive();
  switch (msg) {
  case kSM_Registered:
    AddMaterial(kMT_ScanPassthrough, mgr);
    break;
  case kSM_Deleted: {
    for (rstl::list< SFlameThrower >::iterator uid = mChildren.begin();
         uid != mChildren.end();) {
      mgr.DeleteObjectRequest(uid->id);
      uid = mChildren.erase(uid);
    }
  } break;
  case kSM_SetToZero: {
    for (rstl::list< SFlameThrower >::iterator uid = mChildren.begin();
         uid != mChildren.end(); ++uid) {
      CFlameThrower* act = static_cast< CFlameThrower* >(mgr.ObjectById(uid->id));
      if (act && act->GetParticlesActive()) {
        act->Reset(mgr, false);
      }
    }
  } break;
  default:
    break;
  }

  CScriptActor::AcceptScriptMsg(msg, uid, mgr);
  if (curActive != GetActive() && !GetActive()) {
    for (rstl::list< SFlameThrower >::iterator uid = mChildren.begin();
         uid != mChildren.end(); ++uid) {
      CFlameThrower* act = static_cast< CFlameThrower* >(mgr.ObjectById(uid->id));
      if (act && act->GetParticlesActive()) {
        act->Reset(mgr, false);
      }
    }
    RemoveEmitter();
  }
}

void CScriptContraption::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                         EUserEventType type, float dt) {
  switch (type) {
  case kUE_DamageOn: {
    CFlameThrower* flame = CreateFlameThrower(node.GetLocatorName(), mgr);
    if (flame && !flame->GetParticlesActive()) {
      flame->Fire(GetTransform(), mgr, false);
    }
  } break;
  case kUE_DamageOff:
    for (rstl::list< SFlameThrower >::iterator uid = mChildren.begin();
         uid != mChildren.end(); ++uid) {
      if (uid->name == node.GetLocatorName()) {
        CFlameThrower* act = static_cast< CFlameThrower* >(mgr.ObjectById(uid->id));
        if (act && act->GetParticlesActive()) {
          act->Reset(mgr, false);
        }
      }
    }
    break;
  default:
    CActor::DoUserAnimEvent(mgr, node, type, dt);
    break;
  }
}
ENTITY_ACCEPT_IMPL(CScriptContraption)
