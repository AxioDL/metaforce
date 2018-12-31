#include "CBabygoth.hpp"
#include "Character/CPASAnimParmData.hpp"
#include "Collision/CCollisionActorManager.hpp"
#include "Collision/CCollisionActor.hpp"
#include "World/CWorld.hpp"
#include "World/CGameArea.hpp"
#include "World/CPatternedInfo.hpp"
#include "World/CPlayer.hpp"
#include "World/CTeamAiMgr.hpp"
#include "Weapon/CWeapon.hpp"
#include "Weapon/CFlameThrower.hpp"
#include "Weapon/CFlameInfo.hpp"
#include "Particle/CWeaponDescription.hpp"
#include "CStateManager.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CSkinnedModel.hpp"
#include "TCastTo.hpp"

namespace urde::MP1 {
const std::string_view CBabygoth::skpMouthDamageJoint = "LCTR_SHEMOUTH"sv;
CBabygothData::CBabygothData(CInputStream& in)
  : x0_(in.readFloatBig())
  , x4_(in.readFloatBig())
  , x8_(in)
  , xc_(in)
  , x28_(in)
  , x44_(in)
  , x48_fireBreathRes(in)
  , x4c_fireBreathDamage(in)
  , x68_(in)
  , xd0_shellVulnerabilities(in)
  , x138_(in)
  , x13c_(in)
  , x140_shellHitPoints(in.readFloatBig())
  , x144_shellCrackSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
  , x148_(in)
  , x14c_(in)
  , x150_(in)
  , x154_(in)
  , x158_(CSfxManager::TranslateSFXID(in.readUint32Big()))
  , x15a_(CSfxManager::TranslateSFXID(in.readUint32Big()))
  , x15c_(CSfxManager::TranslateSFXID(in.readUint32Big()))
  , x160_(in.readFloatBig())
  , x164_(in.readFloatBig())
  , x168_(in.readFloatBig())
  , x16c_(in)
  , x170_(CSfxManager::TranslateSFXID(in.readUint32Big()))
  , x174_(in) {}

CBabygoth::CBabygoth(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                     const CBabygothData& babyData)
  : CPatterned(ECharacter::Babygoth, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
               EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Medium)
  , x570_babyData(babyData)
  , x6ec_(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
  , x7d0_(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
  , x8d0_(x3b4_speed)
  , x8f0_boneTracking(*ModelData()->AnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f), false)
  , x930_aabox(GetBoundingBox(), GetMaterialList())
  , x958_(babyData.x8_, babyData.xc_)
  , x984_flameThrowerDesc(babyData.x44_.IsValid() ? g_SimplePool->GetObj({SBIG('WPSC'), babyData.x44_})
                                  : g_SimplePool->GetObj("FlameThrower"sv))
  , x98c_(pInfo.GetDamageVulnerability())
  , xa00_shellHitPoints(babyData.GetShellHitPoints()) {
  TLockedToken<CModel> model = g_SimplePool->GetObj({SBIG('CMDL'), babyData.x138_});
  TLockedToken<CSkinRules> skin = g_SimplePool->GetObj({SBIG('CSKR'), babyData.x13c_});
  xa08_noShellModel = CToken(
    TObjOwnerDerivedFromIObj<CSkinnedModel>::GetNewDerivedObject(std::make_unique<CSkinnedModel>(
      model, skin, x64_modelData->AnimationData()->GetModelData()->GetLayoutInfo(), 1, 1)));
  xa14_ = g_SimplePool->GetObj({SBIG('PART'), babyData.x14c_});
  xa20_ = g_SimplePool->GetObj({SBIG('PART'), babyData.x150_});
  xa2c_ = g_SimplePool->GetObj({SBIG('PART'), babyData.x154_});
  if (x570_babyData.x148_.IsValid())
    xa38_ = g_SimplePool->GetObj({SBIG('PART'), babyData.x148_});
  xa48_31_ = true;
  const_cast<TToken<CWeaponDescription>*>(&x958_.Token())->Lock();
  UpdateTouchBounds();
  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetAutoResetImpulse(true);
  x460_knockBackController.SetEnableShock(true);
  x460_knockBackController.SetEnableExplodeDeath(true);
  x8d4_stepBackwardDist =
    GetAnimationDistance(CPASAnimParmData(3, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(0))) *
    GetModelData()->GetScale().y(); // B_backward_sheegoth
  xa08_noShellModel->SetLayoutInfo(GetModelData()->GetAnimationData()->GetModelData()->GetLayoutInfo());
  MakeThermalColdAndHot();
}

void CBabygoth::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Registered: {
    if (!HasPatrolPath(mgr, 0.f))
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    x450_bodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    CreateFlameThrower(mgr);
    const CBodyStateInfo& bStateInfo = x450_bodyController->GetBodyStateInfo();
    float maxSpeed = bStateInfo.GetMaxSpeed();
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(
      0.f, maxSpeed <= 0.f ? 1.f : bStateInfo.GetLocomotionSpeed(pas::ELocomotionAnim::Walk) / maxSpeed);
    x9f4_mouthLocator = GetModelData()->GetAnimationData()->GetLocatorSegId(skpMouthDamageJoint);
    break;
  }
  case EScriptObjectMessage::Activate:
    x928_colActMgr->SetActive(mgr, true);
    break;
  case EScriptObjectMessage::Deactivate: {
    x928_colActMgr->SetActive(mgr, false);
    xa49_29_ = false;
    RemoveFromTeam(mgr);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x928_colActMgr->Destroy(mgr);
    if (x980_flameThrower != kInvalidUniqueId) {
      mgr.FreeScriptObject(x980_flameThrower);
      x980_flameThrower = kInvalidUniqueId;
    }
    RemoveFromTeam(mgr);
    break;
  }
  case EScriptObjectMessage::Falling: {
    if (!x450_bodyController->IsFrozen()) {
      x150_momentum = {0.f, 0.f, -(GetGravityConstant() * GetMass())};
      x328_27_onGround = false;
      RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
    }
    return;
  }
  case EScriptObjectMessage::OnFloor: {
    x328_27_onGround = true;
    x150_momentum.zeroOut();
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    return;
  }
  case EScriptObjectMessage::Alert: {
    xa48_24_isAlert = true;
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    x6ec_.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    x7d0_.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  }
  case EScriptObjectMessage::Touched: {
    ApplyContactDamage(uid, mgr);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid))
      if (TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(colAct->GetLastTouchedObject()))
        if (wp->GetOwnerId() == mgr.GetPlayer().GetUniqueId())
          xa48_24_isAlert = true;
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      if (TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (wp->GetOwnerId() == mgr.GetPlayer().GetUniqueId()) {
          if (IsMouthCollisionActor(uid)) {
            TakeDamage({}, 0.f);
          } else if (IsShell(uid)) {
            TakeDamage({}, 0.f);
            if (x56c_ != 3 && x56c_ != 0) {
              zeus::CTransform xf = wp->GetTransform();
              xf.rotateLocalZ(zeus::degToRad(180.f));
              CrackShell(mgr, xa38_, xf, x570_babyData.GetShellCrackSfx(), false);
            }
          }
          KnockBack(GetTransform().frontVector(), mgr, wp->GetDamageInfo(), EKnockBackType::Radius, false,
                    wp->GetDamageInfo().GetKnockBackPower());
        }
      }
      xa48_24_isAlert = true;
      x8e8_ = 0.f;
      mgr.InformListeners(GetTranslation(), EListenNoiseType::PlayerFire);
    } else
      sub8021d478(mgr, uid);
    x400_24_hitByPlayerProjectile = true;
    break;
  }
  case EScriptObjectMessage::SuspendedMove: {
    if (x928_colActMgr)
      x928_colActMgr->SetMovable(mgr, false);
    break;
  }
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CBabygoth::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  sub8021ec58(dt, mgr);
  if (xa49_26_) {
    if (sub8023a180(x6e8_teamMgr, mgr) == 0)
      sub8021d6e8(mgr);
  }

  CPatterned::Think(dt, mgr);
  if (x450_bodyController->IsElectrocuting())
    x8f0_boneTracking.SetActive(false);
  sub8021e2c4(dt);
  ModelData()->AnimationData()->PreRender();
  x8f0_boneTracking.Update(dt);
  x8f0_boneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(), GetModelData()->GetScale(),
                              *x450_bodyController);
  x928_colActMgr->Update(dt, mgr, CCollisionActorManager::EUpdateOptions(xa49_29_));
  xa49_29_ = true;
  sub8021e708(mgr);
  UpdateParticleEffects(dt, mgr);
  TryToGetUp(mgr);
  CheckShouldWakeUp(mgr, dt);
  if (!x400_25_alive && x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsDying())
    ResetProjectileCollision(mgr);
}

const SSphereJointInfo CBabygoth::skSphereJointList[skSphereJointCount] = {
  {"L_knee",        1.2f},
  {"R_knee",        1.2f},
  {"LCTR_SHEMOUTH", 1.7f},
  {"Pelvis",        1.2f},
  {"butt_LCTR",     0.9f}
};

void CBabygoth::AddSphereCollisionList(const SSphereJointInfo* sphereJointInfo, s32 jointCount,
                                       std::vector<CJointCollisionDescription>& jointList) {
  for (s32 i = 0; i < jointCount; ++i) {
    CSegId seg = GetModelData()->GetAnimationData()->GetLocatorSegId(sphereJointInfo[i].name);
    jointList.push_back(CJointCollisionDescription::SphereCollision(seg, sphereJointInfo[i].radius,
      sphereJointInfo[i].name, 1000.f));
  }
}

void CBabygoth::SetupCollisionManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> joints;
  AddSphereCollisionList(skSphereJointList, skSphereJointCount, joints);
  x928_colActMgr.reset(new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false));
  x928_colActMgr->SetActive(mgr, GetActive());

  for (u32 i = 0; i < x928_colActMgr->GetNumCollisionActors(); ++i) {
    TUniqueId id = x928_colActMgr->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(id)) {
      colAct->SetDamageVulnerability(x570_babyData.x68_);
      if (colAct->GetName().find(skpMouthDamageJoint))
        x9f6_ = id;
      else if (colAct->GetName().find("Pelvis"sv)) {
        x9f8_shellIds.push_back(id);
        x300_maxAttackRange = 66;
      }
    }
  }
  SetupHealthInfo(mgr);
}
void CBabygoth::SetupHealthInfo(CStateManager& mgr) {
  CHealthInfo* thisHealth = HealthInfo(mgr);
  x8ec_ = thisHealth->GetHP();
  for (const TUniqueId& uid : x9f8_shellIds) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      CHealthInfo* colHealth = colAct->HealthInfo(mgr);
      colHealth->SetHP(x570_babyData.GetShellHitPoints());
      colHealth->SetKnockbackResistance(thisHealth->GetKnockbackResistance());
      colAct->SetDamageVulnerability(x570_babyData.GetShellDamageVulnerability());
    }
  }

  xa00_shellHitPoints = x570_babyData.GetShellHitPoints();
}

void CBabygoth::CreateFlameThrower(CStateManager& mgr) {
  if (x980_flameThrower != kInvalidUniqueId)
    return;

  x980_flameThrower = mgr.AllocateUniqueId();
  mgr.AddObject(new CFlameThrower(x984_flameThrowerDesc, "IceSheegoth_Flame"sv, EWeaponType::Plasma,
                                  CFlameInfo(6, 4, x570_babyData.GetFireBreathResId(), 15, 0.0625f, 20.f, 1.f), {},
                                  EMaterialTypes::CollisionActor, x570_babyData.GetFireBreathDamage(), x980_flameThrower,
                                  GetAreaIdAlways(), GetUniqueId(), EProjectileAttrib::None, x570_babyData.x16c_,
                                  x570_babyData.x170_, x570_babyData.x174_));
}

void CBabygoth::ApplyContactDamage(TUniqueId uid, CStateManager& mgr) {
  if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
    if (colAct->GetHealthInfo(mgr)->GetHP() > 0.f && colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId()) {
      if (xa48_28_) {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), x570_babyData.x28_,
          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        xa48_28_ = false;
        x420_curDamageRemTime = x424_damageWaitTime;
      } else if (x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
  }
}

void CBabygoth::RemoveFromTeam(urde::CStateManager& mgr) {
  if (x6e8_teamMgr == kInvalidUniqueId)
    return;

  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x6e8_teamMgr)) {
    if (teamMgr->IsPartOfTeam(GetUniqueId()))
      teamMgr->RemoveTeamAiRole(GetUniqueId());
  }
}

void CBabygoth::CrackShell(CStateManager&, const TLockedToken<CGenDescription>&, const zeus::CTransform&, s16, bool) {

}

void CBabygoth::sub8021d478(CStateManager&, TUniqueId) {

}

void CBabygoth::Shock(CStateManager& mgr, float duration, float damage) {
  if (x9f8_shellIds.empty())
    return;
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f8_shellIds[0])) {
    EVulnerability vuln = colAct->GetDamageVulnerability()->GetVulnerability(CWeaponMode::Wave(), false);
    if (vuln == EVulnerability::Weak) {
      x450_bodyController->SetElectrocuting(1.5f * duration);
      x3f0_pendingShockDamage = 1.5f * damage;
    } else if (vuln == EVulnerability::Normal) {
      x450_bodyController->SetElectrocuting(duration);
      x3f0_pendingShockDamage = damage;
    }
  }
}

void CBabygoth::UpdateTouchBounds() {
  zeus::CAABox bounds({-1.5f, -1.5f, 0.f}, {1.5f, 1.5f, 2.f});
  SetBoundingBox(bounds);
  x930_aabox.Box() = bounds;
}

void CBabygoth::sub8021ec58(float dt, CStateManager& mgr) {
  if (x450_bodyController->GetLocomotionType() == pas::ELocomotionType::Crouch ||
      x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step)
    return;

  auto dev = x928_colActMgr->GetDeviation(mgr, x9f4_mouthLocator);

  if (dev) {
    const CPlayer& pl = mgr.GetPlayer();
    if (GetModelData()->GetBounds().intersects(pl.GetBoundingBox())) {
      if (dev->magnitude() > 0.9f * GetModelData()->GetScale().y()) {
        zeus::CVector3f posDiff = GetTranslation() - pl.GetTranslation();
        zeus::CVector3f xpos = GetTransform().transposeRotate(
          (posDiff.dot(dev->magnitude() - 0.9f * GetModelData()->GetScale().y() * (*dev).normalized()) /
           posDiff.magSquared()) * posDiff);

        ApplyImpulseWR(GetMoveToORImpulseWR(xpos, dt), zeus::CAxisAngle::sIdentity);
      }
    }
  }
}

s32 CBabygoth::sub8023a180(TUniqueId, CStateManager&) {
  return 0;
}

void CBabygoth::sub8021d6e8(CStateManager& mgr) {
}

void CBabygoth::sub8021e2c4(float dt) {
  if (x8d8_ > 0.f)
    x8d8_ -= dt * (xa49_28_ ? 2.f : 1.f);
  if (x8e4_ > 0.f)
    x8e4_ -= dt * (xa49_28_ ? 2.f : 1.f);
  if (x8e0_ > 0.f)
    x8e0_ -= dt;
  if (x8e8_ < x570_babyData.x168_)
    x8e8_ += dt;
}

void CBabygoth::sub8021e708(CStateManager&) {

}

void CBabygoth::UpdateParticleEffects(float dt, CStateManager& mgr) {
  if (CFlameThrower* flame = static_cast<CFlameThrower*>(mgr.ObjectById(x980_flameThrower))) {
    if (!flame->GetActive())
      return;
    flame->SetTransform(GetLctrTransform(skpMouthDamageJoint), dt);
  }
}

void CBabygoth::TryToGetUp(CStateManager& mgr) {
  if (!x400_25_alive || x450_bodyController->GetFallState() == pas::EFallState::Zero || xa49_24_)
    return;

  x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "GetUp"sv);
}

bool CBabygoth::CheckShouldWakeUp(CStateManager& mgr, float dt) {
  if (!xa48_30_)
    return false;

  xa48_24_isAlert = x400_24_hitByPlayerProjectile && mgr.GetActiveRandom()->Float() < (0.5f * dt);
  return xa48_24_isAlert;
}

void CBabygoth::ResetProjectileCollision(CStateManager& mgr) {
  for (u32 i = 0; i < x928_colActMgr->GetNumCollisionActors(); ++i) {
    const auto& desc = x928_colActMgr->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(desc.GetCollisionActorId())) {
      act->AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
    }
  }
}

void CBabygoth::Crouch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    xa48_30_ = xa48_24_isAlert = false;
    x400_24_hitByPlayerProjectile = false;
    x8e8_ = 0.f;
  } else if (msg == EStateMsg::Deactivate) {
    AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
  }
}
} // namespace urde::MP1
