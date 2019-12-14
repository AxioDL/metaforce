#include "Runtime/MP1/World/CThardus.hpp"

#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/MP1/World/CThardusRockProjectile.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CDestroyableRock.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

namespace {
constexpr SSphereJointInfo skDamageableSphereJointInfoList1[7] = {
    {"R_Knee", 1.f},
    {"R_Elbow_Collision_LCTR", 1.5f},
    {"L_Elbow_Collision_LCTR", 1.5f},
    {"L_Knee_Collision_LCTR", 1.f},
    {"R_Back_Rock_Collision_LCTR", 2.5f},
    {"L_Back_Rock_Collision_LCTR", 1.5f},
    {"Head_Collision_LCTR", 1.5f},
};

constexpr SSphereJointInfo skDamageableSphereJointInfoList2[5] = {
    {"R_Shoulder_Collision_LCTR", 0.75f}, {"L_Shoulder_Collision_LCTR", 0.75f}, {"Spine_Collision_LCTR", 0.75f},
    {"R_Hand_Collision_LCTR", 2.25f},     {"L_Hand_Collision_LCTR", 2.f},
};

constexpr SAABoxJointInfo skFootCollision[2] = {
    {"R_Foot_Collision_LCTR", zeus::CVector3f(3.f, 3.f, 1.f)},
    {"L_Foot_Collision_LCTR", zeus::CVector3f(3.f, 2.f, 3.f)},
};

constexpr std::array<std::string_view, 7> skSearchJointNames = {
    "R_Knee"sv,
    "R_Elbow_Collision_LCTR"sv,
    "L_Elbow_Collision_LCTR"sv,
    "L_Knee_Collision_LCTR"sv,
    "R_Back_Rock_Collision_LCTR"sv,
    "L_Back_Rock_Collision_LCTR"sv,
    "Head_Collision_LCTR"sv,
};
} // namespace
CThardus::CThardus(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                   const std::vector<CStaticRes>& mData1, const std::vector<CStaticRes>& mData2, CAssetId particle1,
                   CAssetId particle2, CAssetId particle3, float f1, float f2, float f3, float f4, float f5, float f6,
                   CAssetId stateMachine, CAssetId particle4, CAssetId particle5, CAssetId particle6,
                   CAssetId particle7, CAssetId particle8, CAssetId particle9, CAssetId texture, u32 sfxId1,
                   CAssetId particle10, u32 sfxId2, u32 sfxId3, u32 sfxId4)
: CPatterned(ECharacter::Thardus, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x5cc_(std::move(mData1))
, x5dc_(std::move(mData2))
, x600_(particle1)
, x604_(particle2)
, x608_(particle3)
, x630_(stateMachine)
, x694_(f1)
, x698_(f2)
, x69c_(f3)
, x6a0_(f4)
, x6a4_(f5)
, x6a8_(f6)
, x6d0_(particle4)
, x6d4_(particle5)
, x6d8_(particle6)
, x6dc_(particle7)
, x6e0_(particle8)
, x6e4_(particle9)
, x6e8_(texture)
, x6ec_(CSfxManager::TranslateSFXID(sfxId1))
, x6f0_(particle10)
, x758_(sfxId2)
, x75c_(sfxId3)
, x760_(sfxId4)
, x7f0_(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  exclude.Add(EMaterialTypes::Player);
  exclude.Add(EMaterialTypes::Character);
  exclude.Add(EMaterialTypes::CollisionActor);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  std::vector<CAssetId> gens;
  gens.reserve(6);
  gens.push_back(particle4);
  gens.push_back(particle5);
  gens.push_back(particle6);
  gens.push_back(particle7);
  gens.push_back(particle8);
  gens.push_back(particle9);
  GetModelData()->GetAnimationData()->GetParticleDB().CacheParticleDesc(
      CCharacterInfo::CParticleResData(gens, {}, {}, {}));
  x798_.reserve(6);
  x7a8_.reserve(16);
  UpdateThermalFrozenState(true);
  xd0_damageMag = 0.f;
  x50c_baseDamageMag = 0.f;
  x8f4_.reserve(16);
  x91c_.Lock();
  x403_26_stateControlledMassiveDeath = false;
  x460_knockBackController.SetAutoResetImpulse(false);
  SetMass(100000.f);
}

void CThardus::Think(float dt, CStateManager& mgr) {
//    if (!x7c8_) {
//    float fVar2 = 10.f * GetModelData()->GetScale().x();
//    zeus::CVector3f diff = mgr.GetPlayer().GetTranslation() - GetTranslation();
//    if (diff.magSquared() < fVar2 * fVar2) {
//      mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(),
//                      CDamageInfo(CWeaponMode(EWeaponType::AI), 0.f, 0.f, 10.f), CMaterialFilter::skPassEverything,
//                      diff.normalized());
//      x688_ = true;
//      x7c8_ = false;
//      x7cc_.zeroOut();
//    }
//  }

  //sub801dbf34(dt, mgr);
  /*
  if (!sub801dc2c8()) {
    if (x648_ < x610_destroyableRocks.size() - 2)
      x690_ = 1.f;
    else
      x690_ = 1.f;
  } else {
    x690_ = 1.f;
    SendScriptMsgs(EScriptObjectState::DeactivateState, mgr, EScriptObjectMessage::None);
  }
*/
  /*
  if (!x93c_) {
    x3b4_speed = x690_;
    x402_28_isMakingBigStrike = false;
    x504_damageDur = 0.f;
  } else {
    x3b4_speed = x690_;
    x402_28_isMakingBigStrike = true;
    x504_damageDur = 1.f;
  } */

  CPatterned::Think(dt, mgr);

  if (x648_ > 2 && !x689_) {
    //sub801dc878(mgr);
  }

  x5f0_->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x5f4_->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x5f8_->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);

  //sub801dd608(mgr);
  //sub801dcfa4(mgr);

}
void CThardus::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Reset: {
    x95c_doCodeTrigger = true;
    x450_bodyController->SetFallState(pas::EFallState::Zero);
    x450_bodyController->SetState(pas::EAnimationState::Locomotion);
    x93d_ = false;
    break;
  }
  case EScriptObjectMessage::SetToMax: {
    for (size_t i = x648_; i < x610_destroyableRocks.size() - 1; ++i) {
      if (CEntity* ent = mgr.ObjectById(x610_destroyableRocks[i])) {
        ent->SetActive(false);
      }
      ++x648_;
    }
    break;
  }
  case EScriptObjectMessage::Stop: {
    Death(mgr, {}, EScriptObjectState::DeathRattle);
    break;
  }
  case EScriptObjectMessage::Action: {
    if (!x5c8_heardPlayer)
      x5c8_heardPlayer = true;
    break;
  }
  case EScriptObjectMessage::Touched: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      if (TCastToPtr<CPlayer> pl = mgr.ObjectById(colAct->GetLastTouchedObject())) {
        if (x420_curDamageRemTime > 0.f)
          break;
        u32 rand = mgr.GetActiveRandom()->Next();
        float damageMult = 1.f;
        zeus::CVector3f knockBack = zeus::skForward;
        if (x644_ == 1) {
          damageMult = 2.f;
          knockBack = (rand % 2) ? zeus::skRight : zeus::skLeft;
        }

        if (mgr.GetPlayer().GetFrozenState())
          mgr.GetPlayer().UnFreeze(mgr);

        knockBack = GetTransform().buildMatrix3f() * knockBack;
        CDamageInfo dInfo = GetContactDamage();
        dInfo.SetDamage(damageMult * dInfo.GetDamage());
        mgr.ApplyDamage(GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), dInfo,
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}),
                        x644_ == 1 ? knockBack : zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      } else if (TCastToConstPtr<CBomb>(mgr.GetObjectById(colAct->GetLastTouchedObject()))) {
#if 0
        if (x644_ == 1 && x93c_)
          sub801dae2c(mgr, x648_);
#endif
      }
    }

    break;
  }
  case EScriptObjectMessage::Registered: {
    x610_destroyableRocks.reserve(x5cc_.size());
    x6b0_.reserve(x5cc_.size());
    x6c0_rockLights.reserve(x5cc_.size());
    x90c_.reserve(x5cc_.size());
    for (size_t i = 0; i < x5cc_.size(); ++i) {
      float health = (i == x5cc_.size() - 1) ? 2.f * x6a8_ : x6a8_;
      TUniqueId rockId = mgr.AllocateUniqueId();
      CModelData mData1(x5cc_[i]);
      CModelData mData2(x5dc_[i]);
      mgr.AddObject(new CDestroyableRock(
          rockId, true, "", CEntityInfo(GetAreaIdAlways(), NullConnectionList), {}, std::move(mData1), 0.f,
          CHealthInfo(health, 0.f),
          CDamageVulnerability(
              EVulnerability::Normal, EVulnerability::Deflect, EVulnerability::Normal, EVulnerability::Normal,
              EVulnerability::Normal, EVulnerability::Normal, EVulnerability::Normal, EVulnerability::Deflect,
              EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
              EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::One),
          GetMaterialList(), x630_,
          CActorParameters(CLightParameters(false, 0.f, CLightParameters::EShadowTesselation::Invalid, 0.f, 0.f,
                                            zeus::skWhite, false, CLightParameters::EWorldLightingOptions::NoShadowCast,
                                            CLightParameters::ELightRecalculationOptions::LargeFrameCount,
                                            zeus::skZero3f, -1, -1, 0, 0),
                           {}, {}, {}, {}, true, true, false, false, 0.f, 0.f, 1.f),
          std::move(mData2), 0));
      x610_destroyableRocks.push_back(rockId);
      x6b0_.push_back(false);
      TUniqueId lightId = mgr.AllocateUniqueId();
      mgr.AddObject(new CGameLight(lightId, GetAreaIdAlways(), false, ""sv, {}, GetUniqueId(),
                                   CLight::BuildPoint({}, zeus::skBlue), 0, 0, 0.f));
      x6c0_rockLights.push_back(lightId);
      x90c_.push_back(health);
    }

    AddMaterial(EMaterialTypes::ScanPassthrough, mgr);
    AddMaterial(EMaterialTypes::CameraPassthrough, mgr);
    RemoveMaterial(EMaterialTypes::Orbit, mgr);
    RemoveMaterial(EMaterialTypes::Target, mgr);
    _SetupCollisionManagers(mgr);
    x450_bodyController->SetFallState(pas::EFallState::Two);
    x450_bodyController->Activate(mgr);
    x450_bodyController->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
    SetState(0, mgr);
    sub801dec80();
    AddMaterial(EMaterialTypes::RadarObject, mgr);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x5f0_->Destroy(mgr);
    x5f4_->Destroy(mgr);
    x5f8_->Destroy(mgr);
    mgr.FreeScriptObject(x64c_);
    for (const auto& id : x610_destroyableRocks) {
      mgr.FreeScriptObject(id);
    }

    for (const auto& id : x6c0_rockLights) {
      mgr.FreeScriptObject(id);
    }
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    if (x94c_)
      break;
    x94c_ = true;
    x764_ = GetTransform();

    for (const SConnection& conn : GetConnectionList()) {
      TUniqueId connId = mgr.GetIdForScript(conn.x8_objId);
      if (connId == kInvalidUniqueId)
        continue;

      if (conn.x0_state == EScriptObjectState::Patrol) {
        if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(connId)) {
          rstl::reserved_vector<TUniqueId, 16> wpIds;
          GatherWaypoints(wp, mgr, wpIds);
          for (const auto& id : wpIds)
            x570_.push_back(id);
        } else if (CPatterned* p = CPatterned::CastTo<CThardusRockProjectile>(mgr.ObjectById(connId))) {
          x5fc_ = connId;
          x60c_ = conn.x8_objId;
          p->SetActive(false);
        } else if (TCastToConstPtr<CRepulsor>(mgr.GetObjectById(connId))) {
          x660_.push_back(connId);
        } else if (TCastToConstPtr<CScriptDistanceFog>(mgr.GetObjectById(connId))) {
          x64c_ = connId;
        }
      } else if (conn.x0_state == EScriptObjectState::Zero) {
        if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(connId)) {
          x8f4_.push_back(connId);
          wp->SetActive(false);
        }
      } else if (conn.x0_state == EScriptObjectState::Dead) {
        if (TCastToConstPtr<CScriptTimer>(mgr.GetObjectById(connId)))
          x7a8_.push_back(connId);
      }
    }
    x7f0_.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  }
  case EScriptObjectMessage::Damage: {
    break;
  }
  default:
    break;
  }
}

void CThardus::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) { CPatterned::PreRender(mgr, frustum); }
void CThardus::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal && x7c4_ != 0) {
    // sub801e32a0(mgr, x7c0_);
  }
}
void CThardus::Touch(CActor& act, CStateManager& mgr) { CPatterned::Touch(act, mgr); }
zeus::CVector3f CThardus::GetOrbitPosition(const CStateManager& mgr) const { return GetAimPosition(mgr, 0.f); }
zeus::CVector3f CThardus::GetAimPosition(const CStateManager& mgr, float dt) const {
  return GetLctrTransform(x93c_ ? "center_LCTR"sv : "Neck_1"sv).origin;
}
zeus::CAABox CThardus::GetSortingBounds(const CStateManager& mgr) const {
  zeus::CVector3f extents = 0.15f * (x9c_renderBounds.max - x9c_renderBounds.min);
  return zeus::CAABox(x9c_renderBounds.min - extents, x9c_renderBounds.max + extents);
}

void CThardus::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}
void CThardus::Patrol(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::Patrol(mgr, msg, arg); }
void CThardus::Dead(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::Dead(mgr, msg, arg); }
void CThardus::PathFind(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::PathFind(mgr, msg, arg); }
void CThardus::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
    if (x95e_)
      return;

    mgr.SetBossParams(GetUniqueId(), GetHealthInfo(mgr)->GetHP(), 88);
    x95e_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Taunt)
        x5ec_ = 2;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
    } else if (x5ec_ == 2 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x5ec_ = 3;
    }
  }
}
void CThardus::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Getup) {
        x5ec_ = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
      }
    } else if (x5ec_ == 2 && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Getup) {
      x5ec_ = 3;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x93d_ = false;
  }
}
void CThardus::Attack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Attack(mgr, msg, arg); }
void CThardus::LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::LoopedAttack(mgr, msg, arg); }
void CThardus::DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) { CAi::DoubleSnap(mgr, msg, arg); }
void CThardus::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Shuffle(mgr, msg, arg); }
void CThardus::GetUp(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate)
    return;
  RemoveMaterial(EMaterialTypes::RadarObject, mgr);
}

void CThardus::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Taunt)
        x5ec_ = 2;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
    } else if (x5ec_ == 2 && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x5ec_ = 3;
    }
  }
}

void CThardus::Suck(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Suck(mgr, msg, arg); }
void CThardus::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::ProjectileAttack(mgr, msg, arg); }
void CThardus::Flinch(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Flinch(mgr, msg, arg); }
void CThardus::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::TelegraphAttack(mgr, msg, arg); }
void CThardus::Explode(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Explode(mgr, msg, arg); }
void CThardus::Cover(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Cover(mgr, msg, arg); }
void CThardus::Enraged(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Enraged(mgr, msg, arg); }
void CThardus::Growth(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Growth(mgr, msg, arg); }
void CThardus::Faint(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Faint(mgr, msg, arg); }
bool CThardus::PathFound(CStateManager& mgr, float arg) { return CPatterned::PathFound(mgr, arg); }
bool CThardus::InRange(CStateManager& mgr, float arg) { return CPatterned::InRange(mgr, arg); }
bool CThardus::PatternOver(CStateManager& mgr, float arg) { return CPatterned::PatternOver(mgr, arg); }
bool CThardus::AnimOver(CStateManager& mgr, float arg) { return CPatterned::AnimOver(mgr, arg); }
bool CThardus::InPosition(CStateManager& mgr, float arg) { return CPatterned::InPosition(mgr, arg); }
bool CThardus::ShouldTurn(CStateManager& mgr, float arg) { return CAi::ShouldTurn(mgr, arg); }
bool CThardus::HitSomething(CStateManager& mgr, float arg) { return CAi::HitSomething(mgr, arg); }

void CThardus::GatherWaypoints(urde::CScriptWaypoint* wp, urde::CStateManager& mgr,
                               rstl::reserved_vector<TUniqueId, 16>& uids) {
  if (uids.size() < uids.capacity() && wp->GetActive()) {
    uids.push_back(wp->GetUniqueId());
    wp->SetActive(false);
    for (const SConnection& conn : wp->GetConnectionList()) {
      TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
      if (TCastToPtr<CScriptWaypoint> wp2 = mgr.ObjectById(uid))
        GatherWaypoints(wp2, mgr, uids);
    }
    wp->SetActive(true);
  }
}

void CThardus::_BuildSphereJointList(const SSphereJointInfo* arr, int count,
                                     std::vector<CJointCollisionDescription>& list) {
  for (size_t i = 0; i < count; ++i) {
    const auto& jInfo = arr[i];
    list.push_back(CJointCollisionDescription::SphereCollision(
        GetModelData()->GetAnimationData()->GetLocatorSegId(jInfo.name), jInfo.radius, jInfo.name, 0.001f));
  }
}

void CThardus::_BuildAABoxJointList(const SAABoxJointInfo* arr, int count,
                                    std::vector<CJointCollisionDescription>& list) {
  for (size_t i = 0; i < count; ++i) {
    const auto& jInfo = arr[i];
    list.push_back(CJointCollisionDescription::AABoxCollision(
        GetModelData()->GetAnimationData()->GetLocatorSegId(jInfo.name), jInfo.extents, jInfo.name, 0.001f));
  }
}

void CThardus::_SetupCollisionActorMaterials(const std::unique_ptr<CCollisionActorManager>& colMgr,
                                             CStateManager& mgr) {
  for (size_t i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    const auto& colDesc = colMgr->GetCollisionDescFromIndex(i);
    if (CActor* act = static_cast<CActor*>(mgr.ObjectById(colDesc.GetCollisionActorId()))) {
      act->AddMaterial(EMaterialTypes::ScanPassthrough, mgr);
      act->AddMaterial(EMaterialTypes::CameraPassthrough, mgr);
      act->AddMaterial(EMaterialTypes::Immovable, mgr);
      act->AddMaterial(EMaterialTypes::NoPlayerCollision, mgr);
      CMaterialList include = GetMaterialFilter().GetIncludeList();
      include.Add(act->GetMaterialFilter().GetIncludeList());
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      exclude.Add(act->GetMaterialFilter().GetExcludeList());
      act->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    }
  }
}

void CThardus::_SetupCollisionManagers(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> list;
  _BuildSphereJointList(skDamageableSphereJointInfoList1, 7, list);
  x5f0_.reset(new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), list, true));
  _SetupCollisionActorMaterials(x5f0_, mgr);
  list.clear();
  _BuildSphereJointList(skDamageableSphereJointInfoList2, 5, list);
  x5f4_.reset(new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), list, true));
  _SetupCollisionActorMaterials(x5f4_, mgr);
  list.clear();
  _BuildAABoxJointList(skFootCollision, 2, list);
  x5f8_.reset(new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), list, true));
  _SetupCollisionActorMaterials(x5f8_, mgr);
  list.clear();
  x634_.reserve(x5f4_->GetNumCollisionActors() + x5f0_->GetNumCollisionActors() + x5f8_->GetNumCollisionActors());
  sub801dd4fc(x5f4_);
  sub801dd4fc(x5f8_);
  for (size_t i = 0; i < x5f0_->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x5f0_->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      if (CDestroyableRock* rock = static_cast<CDestroyableRock*>(mgr.ObjectById(x610_destroyableRocks[i]))) {
        if (i == 0) {
          colAct->SetDamageVulnerability(*rock->GetDamageVulnerability());
          rock->Set_x32c(0.8f);
        } else {
          colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
          rock->Set_x32c(0.f);
        }

        *colAct->HealthInfo(mgr) = *rock->HealthInfo(mgr);
      }
    }
  }
}

void CThardus::sub801dd4fc(const std::unique_ptr<CCollisionActorManager>& colMgr) {
  for (size_t i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    const auto& colDesc = colMgr->GetCollisionDescFromIndex(i);
    TUniqueId uid = colDesc.GetCollisionActorId();
    bool foundBone = false;
    for (const auto& name : skSearchJointNames) {
      if (colDesc.GetName().find(name) != std::string::npos) {
        foundBone = true;
        break;
      }
    }

    if (!foundBone)
      x634_.push_back(uid);
  }
}

}