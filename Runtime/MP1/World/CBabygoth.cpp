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
#include "World/CExplosion.hpp"
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
, x8f0_boneTracking(*ModelData()->AnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x930_aabox(GetBoundingBox(), GetMaterialList())
, x958_(babyData.x8_, babyData.xc_)
, x984_flameThrowerDesc(babyData.x44_.IsValid() ? g_SimplePool->GetObj({SBIG('WPSC'), babyData.x44_})
                                                : g_SimplePool->GetObj("FlameThrower"sv))
, x98c_(pInfo.GetDamageVulnerability())
, xa00_shellHitPoints(babyData.GetShellHitPoints()) {
  TLockedToken<CModel> model = g_SimplePool->GetObj({SBIG('CMDL'), babyData.x138_});
  TLockedToken<CSkinRules> skin = g_SimplePool->GetObj({SBIG('CSKR'), babyData.x13c_});
  xa08_noShellModel =
      CToken(TObjOwnerDerivedFromIObj<CSkinnedModel>::GetNewDerivedObject(std::make_unique<CSkinnedModel>(
          model, skin, x64_modelData->AnimationData()->GetModelData()->GetLayoutInfo(), 1, 1)));
  xa14_ = g_SimplePool->GetObj({SBIG('PART'), babyData.x14c_});
  xa20_ = g_SimplePool->GetObj({SBIG('PART'), babyData.x150_});
  xa2c_ = g_SimplePool->GetObj({SBIG('PART'), babyData.x154_});
  if (x570_babyData.x148_.IsValid())
    xa38_ = g_SimplePool->GetObj({SBIG('PART'), babyData.x148_});
  xa48_31_ = true;
  x958_.Token().Lock();
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
      ApplyDamage(mgr, uid);
    x400_24_hitByPlayerProjectile = true;
    break;
  }
  case EScriptObjectMessage::InvulnDamage: {
    mgr.InformListeners(GetTranslation(), EListenNoiseType::PlayerFire);
    x400_24_hitByPlayerProjectile = true;
    xa48_24_isAlert = true;
    x8e8_ = 0.f;
    if (!TCastToPtr<CCollisionActor>(mgr.ObjectById(uid)))
      ApplyDamage(mgr, uid);
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

  AvoidPlayerCollision(dt, mgr);
  if (xa49_26_) {
    if (!CTeamAiMgr::GetTeamAiRole(mgr, x6e8_teamMgr, GetUniqueId()))
      AddToTeam(mgr);
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
    SetProjectilePasshtrough(mgr);
}

void CBabygoth::DoUserAnimEvent(urde::CStateManager& mgr, const urde::CInt32POINode& node, EUserEventType type,
                                float dt) {
  switch (type) {
  case EUserEventType::Projectile: {
    zeus::CTransform xf = GetLctrTransform(node.GetLocatorName());
    zeus::CVector3f plAimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    return;
  }
  case EUserEventType::DamageOn: {
    if (xa48_26_) {
      if (CFlameThrower* flame = static_cast<CFlameThrower*>(mgr.ObjectById(x980_flameThrower)))
        flame->Fire(GetTransform(), mgr, false);
    }
    break;
  }
  case EUserEventType::DamageOff: {
    if (xa48_26_) {
      if (CFlameThrower* flame = static_cast<CFlameThrower*>(mgr.ObjectById(x980_flameThrower)))
        flame->Reset(mgr, false);
    }
    break;
  }
  case EUserEventType::BeginAction: {
    if (xa48_26_) {
      x8f0_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
      x8f0_boneTracking.SetActive(true);
    }
    break;
  }
  case EUserEventType::ScreenShake:
    return;
  case EUserEventType::BecomeShootThrough:
    SetProjectilePasshtrough(mgr);
    return;
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

const SSphereJointInfo CBabygoth::skSphereJointList[skSphereJointCount] = {
    {"L_knee", 1.2f}, {"R_knee", 1.2f}, {"LCTR_SHEMOUTH", 1.7f}, {"Pelvis", 1.2f}, {"butt_LCTR", 0.9f}};

void CBabygoth::AddSphereCollisionList(const SSphereJointInfo* sphereJointInfo, s32 jointCount,
                                       std::vector<CJointCollisionDescription>& jointList) {
  for (s32 i = 0; i < jointCount; ++i) {
    CSegId seg = GetModelData()->GetAnimationData()->GetLocatorSegId(sphereJointInfo[i].name);
    jointList.push_back(
        CJointCollisionDescription::SphereCollision(seg, sphereJointInfo[i].radius, sphereJointInfo[i].name, 1000.f));
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
                                  EMaterialTypes::CollisionActor, x570_babyData.GetFireBreathDamage(),
                                  x980_flameThrower, GetAreaIdAlways(), GetUniqueId(), EProjectileAttrib::None,
                                  x570_babyData.x16c_, x570_babyData.x170_, x570_babyData.x174_));
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

void CBabygoth::ApplySeparationBehavior(CStateManager& mgr) {}

void CBabygoth::ApplyDamage(CStateManager& mgr, TUniqueId uid) {
  if (TCastToConstPtr<CWeapon> weap = mgr.GetObjectById(uid)) {
    if (x9f8_shellIds.empty())
      return;

    mgr.ApplyDamage(uid, x9f8_shellIds[0], weap->GetOwnerId(), weap->GetDamageInfo(),
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
  }
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

void CBabygoth::UpdateAttackPosition(CStateManager& mgr, zeus::CVector3f& attackPos) {
  attackPos = GetTranslation();
  if (x8d8_ > 0.f)
    return;
  attackPos = mgr.GetPlayer().GetTranslation();
  zeus::CVector3f distVec = GetTranslation() - attackPos;
  if (distVec.canBeNormalized())
    attackPos += x2fc_minAttackRange * distVec.normalized();
}

void CBabygoth::sub8021e3f4(urde::CStateManager& mgr) {
  if (xa00_shellHitPoints <= 0.f)
    return;

  float hp = 0.f;
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_))
    hp = zeus::max(hp, colAct->GetHealthInfo(mgr)->GetHP() - x570_babyData.GetShellHitPoints());

  for (TUniqueId uid : x9f8_shellIds) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      hp = zeus::max(hp, colAct->GetHealthInfo(mgr)->GetHP() - x570_babyData.GetShellHitPoints());
    }
  }

  xa00_shellHitPoints -= hp;
  if (xa00_shellHitPoints <= 0.f) {
    x56c_ = 3;
    sub8021d9d0(mgr);
    CrackShell(mgr, xa2c_, x34_transform, x570_babyData.x15c_, false);
    UpdateHealthInfo(mgr);
  } else {
    if (xa00_shellHitPoints < CalculateShellCrackHP(2)) {
      if (x56c_ != 2) {
        CrackShell(mgr, xa20_, x34_transform, x570_babyData.x15a_, false);
        x56c_ = 2;
        xa04_ = 2;
      }
    } else if (xa00_shellHitPoints < CalculateShellCrackHP(1)) {
      if (x56c_ != 1) {
        CrackShell(mgr, xa14_, x34_transform, x570_babyData.x158_, false);
        x56c_ = 1;
        xa04_ = 1;
      }
    }
  }

  hp = (x56c_ == 3 ? x8ec_ : x570_babyData.GetShellHitPoints());

  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_)) {
    colAct->HealthInfo(mgr)->SetHP(hp);
  }

  for (TUniqueId uid : x9f8_shellIds) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      colAct->HealthInfo(mgr)->SetHP(hp);
    }
  }
}

void CBabygoth::AvoidPlayerCollision(float dt, CStateManager& mgr) {
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
             posDiff.magSquared()) *
            posDiff);

        ApplyImpulseWR(GetMoveToORImpulseWR(xpos, dt), zeus::CAxisAngle());
      }
    }
  }
}

void CBabygoth::AddToTeam(CStateManager& mgr) {
  if (x6e8_teamMgr == kInvalidUniqueId)
    return;
  if (TCastToPtr<CTeamAiMgr> aiMgr = mgr.ObjectById(x6e8_teamMgr)) {
    if (!aiMgr->IsPartOfTeam(GetUniqueId()))
      aiMgr->AssignTeamAiRole(*this, CTeamAiRole::ETeamAiRole::Melee, CTeamAiRole::ETeamAiRole::Ranged,
                              CTeamAiRole::ETeamAiRole::Invalid);
  }
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

void CBabygoth::sub8021e708(CStateManager& mgr) {
  if (!x400_25_alive)
    return;

  if (x56c_ == 3) {
    float hp = 0.f;
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_)) {
      hp = zeus::max(hp, colAct->GetHealthInfo(mgr)->GetHP() - x8ec_);
    }

    for (TUniqueId uid : x9f8_shellIds) {
      if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
        hp = zeus::max(hp, colAct->GetHealthInfo(mgr)->GetHP() - x8ec_);
      }
    }

    HealthInfo(mgr)->SetHP(hp - HealthInfo(mgr)->GetHP());
    if (HealthInfo(mgr)->GetHP() <= 0.f) {
      Death(mgr, {}, EScriptObjectState::DeathRattle);
      xa48_26_ = true;
      xa49_26_ = false;
      RemoveFromTeam(mgr);
      RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    } else {
      if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_))
        colAct->HealthInfo(mgr)->SetHP(x8ec_);

      for (TUniqueId uid : x9f8_shellIds) {
        if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
          colAct->HealthInfo(mgr)->SetHP(x8ec_);
        }
      }
    }
  } else {
    sub8021e3f4(mgr);
  }
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

void CBabygoth::SetProjectilePasshtrough(CStateManager& mgr) {
  for (u32 i = 0; i < x928_colActMgr->GetNumCollisionActors(); ++i) {
    const auto& desc = x928_colActMgr->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(desc.GetCollisionActorId())) {
      act->AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
    }
  }
}

void CBabygoth::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x8f0_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x8f0_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(EPathFindMode::Zero);
    CPatterned::PathFind(mgr, msg, arg);
    x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
  } else if (msg == EStateMsg::Update) {
    if (ShouldTurn(mgr, arg)) {
      float speed = (GetModelData()->GetAnimationData()->GetSpeedScale() > 0.f
                         ? 1.f / GetModelData()->GetAnimationData()->GetSpeedScale()
                         : 0.f);
      zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, speed) - GetTranslation();
      if (aimPos.canBeNormalized())
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd({}, aimPos.normalized(), 1.f));
    }
  } else if (msg == EStateMsg::Deactivate) {
    x8f0_boneTracking.SetActive(false);
  }
}

void CBabygoth::GetUp(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    xa49_24_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Getup)
        x568_ = 3;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
    } else if (x568_ == 3 && x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Getup)
      x568_ = 4;
  } else if (msg == EStateMsg::Deactivate) {
    xa49_24_ = false;
  }
}

void CBabygoth::Enraged(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    xa48_29_ = true;
    x568_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate)
        x568_ = 3;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Three));
    } else if (x568_ == 3 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate)
      x568_ = 4;
  }
}

void CBabygoth::FollowPattern(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = (xa49_25_ ? 0 : 4);
    xa49_25_ = false;
    x8f0_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x8f0_boneTracking.SetActive(true);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));
    } else if (x568_ == 3) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step)
        x450_bodyController->GetCommandMgr().SetTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
      else
        x568_ = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x8f0_boneTracking.SetActive(false);
    SetPathFindMode(EPathFindMode::Zero);
  }
}

void CBabygoth::Taunt(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
  } else if (msg == EStateMsg::Update &&
             x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
    x568_ = 4;
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

void CBabygoth::Deactivate(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 1;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_ = 3;
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
      }
    } else if (x568_ == 1) {
      if ((x3a0_latestLeashPosition - GetTranslation()).magSquared() > 1.f)
        x568_ = 2;
      else {
        zeus::CVector3f arrivalVec = x45c_steeringBehaviors.Arrival(*this, x3a0_latestLeashPosition, 15.f);
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(arrivalVec, {}, 1.f));
      }
    } else if (x568_ == 2) {
      float angle = zeus::CVector3f::getAngleDiff(GetTranslation(), x8c4_);
      if (angle > zeus::degToRad(5.f)) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd({}, x8c4_, 1.f));
        return;
      }
      x568_ = 0;
    } else if (x568_ == 3 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
      x568_ = 4;
    }
  }
}

void CBabygoth::Generate(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x8c4_ = GetTransform().basis[1];
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
        SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
      }
    } else if (x568_ == 3 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate)
      x568_ = 4;
  }
}

void CBabygoth::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    xa49_29_ = false;
    RemoveFromTeam(mgr);
    x400_24_hitByPlayerProjectile = false;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    if (HasPatrolPath(mgr, 0.f)) {
      Patrol(mgr, msg, arg);
      UpdateDest(mgr);
    } else {
      SetDestPos(x3a0_latestLeashPosition);
    }
    x8b8_ = x2e0_destPos;
    if (GetSearchPath()) {
      SetPathFindMode(EPathFindMode::Zero);
      PathFind(mgr, msg, arg);
    }
  } else if (msg == EStateMsg::Update) {
    if (GetSearchPath() && !PathShagged(mgr, 0.f)) {
      SetPathFindMode(EPathFindMode::Zero);
      PathFind(mgr, msg, arg);
      ApplySeparationBehavior(mgr);
    } else {
      zeus::CVector3f arrivalVec = x45c_steeringBehaviors.Arrival(*this, x8b8_, 9.f);
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(arrivalVec, {}, 1.f));
    }
  }
}

void CBabygoth::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Update)
    ApplySeparationBehavior(mgr);

  CPatterned::Patrol(mgr, msg, arg);
}

void CBabygoth::Approach(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x8f0_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x8f0_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(EPathFindMode::Zero);
    CPatterned::PathFind(mgr, msg, arg);
    if (!xa49_27_)
      x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
    xa48_31_ = true;
    xa48_28_ = true;
  } else if (msg == EStateMsg::Update) {
    SetPathFindMode(EPathFindMode::One);
    if (xa48_31_) {
      x2e0_destPos = x8b8_;
      if (x7d0_.FindClosestReachablePoint(GetTranslation(), x2e0_destPos) == CPathFindSearch::EResult::Success &&
          (x2e0_destPos - GetTranslation()).magSquared() < 10.f) {
        x2e0_destPos = GetTranslation();
      }
      x8b8_ = x2e0_destPos;
      CPatterned::PathFind(mgr, msg, arg);

      if (!xa49_27_)
        x450_bodyController->GetCommandMgr().ClearLocomotionCmds();

      xa48_31_ = true;
    }

    xa49_27_ = IsDestinationObstructed(mgr);
    if (xa49_27_ && GetSearchPath() && !PathShagged(mgr, 0.f) &&
        (x7d0_.GetCurrentWaypoint() < x7d0_.GetWaypoints().size() - 1)) {
      CPatterned::PathFind(mgr, msg, arg);
      ApplySeparationBehavior(mgr);
    } else if (ShouldTurn(mgr, 0.f)) {
      zeus::CVector3f direction = mgr.GetPlayer().GetTranslation() - GetTranslation();
      if (direction.canBeNormalized()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(direction.normalized(), {}, 1.f));
      }
    }
    xa49_27_ = true;
  } else if (msg == EStateMsg::Deactivate) {
    x8f0_boneTracking.SetActive(false);
    SetPathFindMode(EPathFindMode::Zero);
  }
}

void CBabygoth::PathFind(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    xa49_28_ = false;
    xa49_26_ = true;
    xa48_24_isAlert = false;
    x8e8_ = 0.f;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x8f0_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x8f0_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    x8b8_ = x2e0_destPos;
    SetPathFindMode(EPathFindMode::Zero);
    CPatterned::PathFind(mgr, msg, arg);
  } else if (msg == EStateMsg::Update) {
    SetPathFindMode(EPathFindMode::Zero);
    if (GetSearchPath() && !PathShagged(mgr, 0.f) && x6ec_.GetCurrentWaypoint() < x6ec_.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, msg, arg);
      x8e8_ = 0.f;
      zeus::CVector3f move = x450_bodyController->GetCommandMgr().GetMoveVector();
      if (move.canBeNormalized()) {
        float mag = x45c_steeringBehaviors.Arrival(*this, mgr.GetPlayer().GetTranslation(), 15.f).magnitude();
        x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(mag * move.normalized(), {}, 10.f));
      }
      ApplySeparationBehavior(mgr);
      if (GetTransform().basis[1].magSquared() < 0.f && move.canBeNormalized()) {
        x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd({}, move.normalized(), 1.f));
      }
    } else {
      zeus::CVector3f diffPos = mgr.GetPlayer().GetTranslation() - GetTranslation();
      if (diffPos.canBeNormalized()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd({}, diffPos.normalized(), 1.f));
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x8f0_boneTracking.SetActive(false);
  }
}

void CBabygoth::SpecialAttack(CStateManager&, EStateMsg, float) {}

void CBabygoth::Attack(CStateManager&, EStateMsg, float) {}

void CBabygoth::ProjectileAttack(CStateManager&, EStateMsg, float) {}

bool CBabygoth::IsDestinationObstructed(CStateManager& mgr) {
  for (auto obj : mgr.GetListeningAiObjectList()) {
    if (TCastToPtr<CAi> ai = obj) {
      if (ai->GetAreaIdAlways() == GetAreaIdAlways()) {
        if ((x8b8_ - ai->GetTranslation()).magSquared() <= 10.f)
          return true;
      }
    }
  }
  return false;
}

void CBabygoth::sub8021d9d0(urde::CStateManager& mgr) {
  ModelData()->AnimationData()->SubstituteModelData(xa08_noShellModel);

  for (TUniqueId uid : x9f8_shellIds) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      colAct->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::Unknown41);
    }
  }
  xa04_ = 0;
}

void CBabygoth::CrackShell(CStateManager& mgr, const TLockedToken<urde::CGenDescription>& desc,
                           const zeus::CTransform& xf, u16 sfx, bool b1) {
  mgr.AddObject(new CExplosion(desc, mgr.AllocateUniqueId(), true,
                               CEntityInfo(GetAreaIdAlways(), CEntity::NullConnectionList), "Babygoth Shell Crack Fx"sv,
                               xf, 0, GetModelData()->GetScale(), zeus::skWhite));

  if (b1)
    CSfxManager::SfxStart(sfx, 0x7f, 64 / 127.f, false, 0x7f, false, -1);
  else
    CSfxManager::AddEmitter(sfx, GetTranslation(), zeus::skUp, false, false, 0x7f, GetAreaIdAlways());
}

void CBabygoth::UpdateHealthInfo(urde::CStateManager& mgr) {
  CHealthInfo* hInfo = HealthInfo(mgr);
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_)) {
    (*colAct->HealthInfo(mgr)) = *hInfo;
    colAct->SetDamageVulnerability(x98c_);
  }

  for (TUniqueId uid : x9f8_shellIds) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      (*colAct->HealthInfo(mgr)) = *hInfo;
      colAct->SetDamageVulnerability(x98c_);
    }
  }
}

float CBabygoth::CalculateShellCrackHP(u32 w1) {
  if (w1 == 0)
    return x570_babyData.GetShellHitPoints();
  else if (w1 == 1)
    return 0.66666669f * x570_babyData.GetShellHitPoints();
  else if (w1 == 2)
    return 0.33333334f * x570_babyData.GetShellHitPoints();

  return 0.f;
}

bool CBabygoth::ShouldTurn(urde::CStateManager& mgr, float arg) {
  const float speedScale = GetModelData()->GetAnimationData()->GetSpeedScale();
  zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, (speedScale > 0.f ? 1.f / speedScale : 0.f));
  return zeus::CVector2f::getAngleDiff(GetTransform().basis[1].toVec2f(), (aimPos - GetTranslation()).toVec2f()) >
         (arg == 0.f ? 0.78539819f : arg);
}

bool CBabygoth::InMaxRange(CStateManager& mgr, float) {
  return (GetTranslation() - mgr.GetPlayer().GetTranslation()).magSquared() < (1.5f * x300_maxAttackRange);
}

bool CBabygoth::Listen(const zeus::CVector3f& origin, EListenNoiseType noiseType) {
  if (!x400_25_alive || noiseType != EListenNoiseType::PlayerFire || (origin - GetTranslation()).magSquared() >= 1600.f)
    return false;

  xa48_30_ = true;
  return true;
}
} // namespace urde::MP1
