#include "Runtime/MP1/World/CBabygoth.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/Weapon/CFlameInfo.hpp"
#include "Runtime/Weapon/CFlameThrower.hpp"
#include "Runtime/Weapon/CWeapon.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
constexpr std::string_view skpMouthDamageJoint = "LCTR_SHEMOUTH"sv;

constexpr std::array<SSphereJointInfo, 5> skSphereJointList{{
    {"L_knee", 1.2f},
    {"R_knee", 1.2f},
    {"LCTR_SHEMOUTH", 1.7f},
    {"Pelvis", 1.2f},
    {"butt_LCTR", 0.9f},
}};

CBabygothData::CBabygothData(CInputStream& in)
: x0_fireballAttackTime(in.readFloatBig())
, x4_fireballAttackTimeVariance(in.readFloatBig())
, x8_fireballWeapon(in)
, xc_fireballDamage(in)
, x28_attackContactDamage(in)
, x44_fireBreathWeapon(in)
, x48_fireBreathRes(in)
, x4c_fireBreathDamage(in)
, x68_mouthVulnerabilities(in)
, xd0_shellVulnerabilities(in)
, x138_noShellModel(in)
, x13c_noShellSkin(in)
, x140_shellHitPoints(in.readFloatBig())
, x144_shellCrackSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x148_intermediateCrackParticle(in)
, x14c_crackOneParticle(in)
, x150_crackTwoParticle(in)
, x154_destroyShellParticle(in)
, x158_crackOneSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x15a_crackTwoSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x15c_destroyShellSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x160_timeUntilAttack(in.readFloatBig())
, x164_attackCooldownTime(in.readFloatBig())
, x168_interestTime(in.readFloatBig())
, x16c_flamePlayerSteamTxtr(in)
, x170_flamePlayerHitSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x174_flamePlayerIceTxtr(in) {}

CBabygoth::CBabygoth(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                     const CBabygothData& babyData)
: CPatterned(ECharacter::Babygoth, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Medium)
, x570_babyData(babyData)
, x6ec_pathSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x7d0_approachPathSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x8d0_initialSpeed(x3b4_speed)
, x8f0_boneTracking(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x930_aabox(GetBoundingBox(), GetMaterialList())
, x958_iceProjectile(babyData.x8_fireballWeapon, babyData.xc_fireballDamage)
, x984_flameThrowerDesc(babyData.x44_fireBreathWeapon.IsValid() ?
                        g_SimplePool->GetObj({SBIG('WPSC'), babyData.x44_fireBreathWeapon})
                        : g_SimplePool->GetObj("FlameThrower"sv))
, x98c_dVuln(pInfo.GetDamageVulnerability())
, xa00_shellHitPoints(babyData.GetShellHitPoints()) {
  TLockedToken<CModel> model = g_SimplePool->GetObj({SBIG('CMDL'), babyData.x138_noShellModel});
  TLockedToken<CSkinRules> skin = g_SimplePool->GetObj({SBIG('CSKR'), babyData.x13c_noShellSkin});
  xa08_noShellModel =
      CToken(TObjOwnerDerivedFromIObj<CSkinnedModel>::GetNewDerivedObject(std::make_unique<CSkinnedModel>(
          model, skin, x64_modelData->GetAnimationData()->GetModelData()->GetLayoutInfo(), 1, 1)));
  xa14_crackOneParticle = g_SimplePool->GetObj({SBIG('PART'), babyData.x14c_crackOneParticle});
  xa20_crackTwoParticle = g_SimplePool->GetObj({SBIG('PART'), babyData.x150_crackTwoParticle});
  xa2c_destroyShellParticle = g_SimplePool->GetObj({SBIG('PART'), babyData.x154_destroyShellParticle});
  if (x570_babyData.x148_intermediateCrackParticle.IsValid())
    xa38_intermediateCrackParticle = g_SimplePool->GetObj({SBIG('PART'), babyData.x148_intermediateCrackParticle});
  x958_iceProjectile.Token().Lock();
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
    xa49_29_objectSpaceCollision = false;
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
    x6ec_pathSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    x7d0_approachPathSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    if (x6e8_teamMgr == kInvalidUniqueId)
      x6e8_teamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
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
            if (x56c_shellState != EShellState::Destroyed && x56c_shellState != EShellState::Default) {
              zeus::CTransform xf = wp->GetTransform();
              xf.rotateLocalZ(zeus::degToRad(180.f));
              CrackShell(mgr, xa38_intermediateCrackParticle, xf, x570_babyData.GetShellCrackSfx(), false);
            }
          }
          KnockBack(GetTransform().frontVector(), mgr, wp->GetDamageInfo(), EKnockBackType::Radius, false,
                    wp->GetDamageInfo().GetKnockBackPower());
        }
      }
      xa48_24_isAlert = true;
      x8e8_interestTimer = 0.f;
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
    x8e8_interestTimer = 0.f;
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
  if (xa49_26_readyForTeam) {
    if (!CTeamAiMgr::GetTeamAiRole(mgr, x6e8_teamMgr, GetUniqueId()))
      AddToTeam(mgr);
  }

  CPatterned::Think(dt, mgr);
  if (x450_bodyController->IsElectrocuting())
    x8f0_boneTracking.SetActive(false);
  UpdateTimers(dt);
  GetModelData()->GetAnimationData()->PreRender();
  x8f0_boneTracking.Update(dt);
  x8f0_boneTracking.PreRender(mgr, *GetModelData()->GetAnimationData(), GetTransform(), GetModelData()->GetScale(),
                              *x450_bodyController);
  x928_colActMgr->Update(dt, mgr, CCollisionActorManager::EUpdateOptions(!xa49_29_objectSpaceCollision));
  xa49_29_objectSpaceCollision = true;
  UpdateHealth(mgr);
  UpdateParticleEffects(dt, mgr);
  TryToGetUp(mgr);
  CheckShouldWakeUp(mgr, dt);
  if (!x400_25_alive && x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsDying())
    SetProjectilePasshtrough(mgr);
}

void CBabygoth::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch (type) {
  case EUserEventType::Projectile: {
    zeus::CTransform xf = GetLctrTransform(node.GetLocatorName());
    zeus::CVector3f plAimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    zeus::CVector3f pos = GetProjectileInfo()->PredictInterceptPos(xf.origin, plAimPos, mgr.GetPlayer(), false, dt);
    zeus::CVector3f delta = pos - xf.origin;
    if (zeus::CVector3f::getAngleDiff(GetTransform().basis[1], delta) > zeus::degToRad(30.f)) {
      if (delta.canBeNormalized()) {
        pos = zeus::CVector3f::slerp(GetTransform().basis[1], delta.normalized(), zeus::degToRad(30.f)) *
            delta.magnitude() + xf.origin;
      } else {
        pos = xf.basis[1] * delta.magnitude() + xf.origin;
      }
    }
    LaunchProjectile(zeus::lookAt(xf.origin, pos), mgr, 4, EProjectileAttrib::None,
                     false, {}, 0xffff, false, zeus::skOne3f);
    return;
  }
  case EUserEventType::DamageOn: {
    if (xa48_26_inProjectileAttack) {
      if (CFlameThrower* flame = static_cast<CFlameThrower*>(mgr.ObjectById(x980_flameThrower)))
        flame->Fire(GetTransform(), mgr, false);
    }
    break;
  }
  case EUserEventType::DamageOff: {
    if (xa48_26_inProjectileAttack) {
      if (CFlameThrower* flame = static_cast<CFlameThrower*>(mgr.ObjectById(x980_flameThrower)))
        flame->Reset(mgr, false);
    }
    break;
  }
  case EUserEventType::BeginAction: {
    if (xa48_26_inProjectileAttack) {
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

void CBabygoth::AddSphereCollisionList(const SSphereJointInfo* sphereJointInfo, size_t jointCount,
                                       std::vector<CJointCollisionDescription>& jointList) {
  for (size_t i = 0; i < jointCount; ++i) {
    const CSegId seg = GetModelData()->GetAnimationData()->GetLocatorSegId(sphereJointInfo[i].name);
    jointList.push_back(
        CJointCollisionDescription::SphereCollision(seg, sphereJointInfo[i].radius, sphereJointInfo[i].name, 1000.f));
  }
}

void CBabygoth::SetupCollisionManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> joints;
  AddSphereCollisionList(skSphereJointList.data(), skSphereJointList.size(), joints);
  x928_colActMgr = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);
  x928_colActMgr->SetActive(mgr, GetActive());

  for (u32 i = 0; i < x928_colActMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x928_colActMgr->GetCollisionDescFromIndex(i);
    TUniqueId id = desc.GetCollisionActorId();
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(id)) {
      colAct->SetDamageVulnerability(x570_babyData.x68_mouthVulnerabilities);
      if (desc.GetName().find(skpMouthDamageJoint) == 0)
        x9f6_mouthCollisionActor = id;
      else if (desc.GetName().find("Pelvis"sv) == 0 || desc.GetName().find("butt_LCTR"sv) == 0) {
        x9f8_shellIds.push_back(id);
        x300_maxAttackRange = 66;
      }
    }
  }

  SetupHealthInfo(mgr);

  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
      {EMaterialTypes::CollisionActor, EMaterialTypes::AIPassthrough, EMaterialTypes::Player}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
  x928_colActMgr->AddMaterial(mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough});
}

void CBabygoth::SetupHealthInfo(CStateManager& mgr) {
  CHealthInfo* thisHealth = HealthInfo(mgr);
  x8ec_bodyHP = thisHealth->GetHP();

  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_mouthCollisionActor))
    colAct->HealthInfo(mgr)->SetHP(x570_babyData.GetShellHitPoints());

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
                                  x570_babyData.x16c_flamePlayerSteamTxtr, x570_babyData.x170_flamePlayerHitSfx,
                                  x570_babyData.x174_flamePlayerIceTxtr));
}

void CBabygoth::ApplyContactDamage(TUniqueId uid, CStateManager& mgr) {
  if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
    if (colAct->GetHealthInfo(mgr)->GetHP() > 0.f && colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId()) {
      if (xa48_28_pendingAttackContactDamage) {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(),
                        x570_babyData.x28_attackContactDamage,
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        xa48_28_pendingAttackContactDamage = false;
        x420_curDamageRemTime = x424_damageWaitTime;
      } else if (x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
  }
}

void CBabygoth::RemoveFromTeam(CStateManager& mgr) {
  if (x6e8_teamMgr == kInvalidUniqueId)
    return;

  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x6e8_teamMgr)) {
    if (teamMgr->IsPartOfTeam(GetUniqueId()))
      teamMgr->RemoveTeamAiRole(GetUniqueId());
  }
}

void CBabygoth::ApplySeparationBehavior(CStateManager& mgr) {
  for (CEntity* ent : mgr.GetListeningAiObjectList()) {
    if (TCastToPtr<CAi> ai = ent) {
      if (ai.GetPtr() != this && GetAreaIdAlways() == ai->GetAreaIdAlways()) {
        zeus::CVector3f sep = x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), 15.f);
        if (!sep.isZero()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(sep, {}, 1.f));
        }
      }
    }
  }
}

void CBabygoth::ApplyDamage(CStateManager& mgr, TUniqueId uid) {
  if (TCastToConstPtr<CWeapon> weap = mgr.GetObjectById(uid)) {
    if (x9f8_shellIds.empty())
      return;

    mgr.ApplyDamage(uid, x9f8_shellIds[0], weap->GetOwnerId(), weap->GetDamageInfo(),
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
  }
}

zeus::CVector3f CBabygoth::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (x450_bodyController->GetLocomotionType() != pas::ELocomotionType::Crouch && !x9f8_shellIds.empty()) {
    zeus::CVector3f shellAvgPos;
    for (TUniqueId id : x9f8_shellIds) {
      if (TCastToConstPtr<CCollisionActor> cact = mgr.GetObjectById(id))
        shellAvgPos += cact->GetSphereRadius() * zeus::skUp + cact->GetTranslation();
    }
    if (!shellAvgPos.isZero())
      shellAvgPos = shellAvgPos / float(x9f8_shellIds.size());
    float mouthZ = shellAvgPos.z();
    if (TCastToConstPtr<CCollisionActor> cact = mgr.GetObjectById(x9f6_mouthCollisionActor))
      mouthZ = cact->GetTranslation().z();
    float t = zeus::CVector2f::getAngleDiff(GetTransform().basis[1].toVec2f(),
              (mgr.GetPlayer().GetTranslation() - GetTranslation()).toVec2f()) / M_PIF;
    return zeus::CVector3f::lerp(zeus::CVector3f(shellAvgPos.toVec2f(), mouthZ),
                                 zeus::CVector3f(shellAvgPos.toVec2f(), shellAvgPos.z()), t);
  } else {
    return CPatterned::GetAimPosition(mgr, 0.f);
  }
}

zeus::CVector3f CBabygoth::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                     const zeus::CVector3f& aimPos) const {
  zeus::CVector3f ret = GetTranslation() - aimPos;
  if (ret.canBeNormalized())
    ret = ret.normalized() * x2fc_minAttackRange + aimPos;
  return ret;
}

void CBabygoth::KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info,
                          EKnockBackType type, bool inDeferred, float magnitude) {
  x460_knockBackController.SetAvailableState(EKnockBackAnimationState::Hurled,
                                             x56c_shellState == EShellState::Destroyed);
  CPatterned::KnockBack(backVec, mgr, info, type, inDeferred, magnitude);
  if (x400_25_alive && x460_knockBackController.GetActiveParms().x0_animState == EKnockBackAnimationState::Hurled)
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "GetUp"sv);
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
  if (x8d8_attackTimeLeft > 0.f)
    return;
  attackPos = mgr.GetPlayer().GetTranslation();
  zeus::CVector3f distVec = GetTranslation() - attackPos;
  if (distVec.canBeNormalized())
    attackPos += x2fc_minAttackRange * distVec.normalized();
}

void CBabygoth::UpdateShellHealth(CStateManager& mgr) {
  if (xa00_shellHitPoints <= 0.f)
    return;

  float dam = 0.f;
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_mouthCollisionActor))
    dam = zeus::max(dam, x570_babyData.GetShellHitPoints() - colAct->GetHealthInfo(mgr)->GetHP());

  for (TUniqueId uid : x9f8_shellIds) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      dam = zeus::max(dam, x570_babyData.GetShellHitPoints() - colAct->GetHealthInfo(mgr)->GetHP());
    }
  }

  xa00_shellHitPoints -= dam;
  if (xa00_shellHitPoints <= 0.f) {
    x56c_shellState = EShellState::Destroyed;
    DestroyShell(mgr);
    CrackShell(mgr, xa2c_destroyShellParticle, x34_transform, x570_babyData.x15c_destroyShellSfx, false);
    UpdateHealthInfo(mgr);
  } else {
    if (xa00_shellHitPoints < CalculateShellCrackHP(EShellState::CrackTwo)) {
      if (x56c_shellState != EShellState::CrackTwo) {
        CrackShell(mgr, xa20_crackTwoParticle, x34_transform, x570_babyData.x15a_crackTwoSfx, false);
        x56c_shellState = EShellState::CrackTwo;
        xa04_drawMaterialIdx = 2;
      }
    } else if (xa00_shellHitPoints < CalculateShellCrackHP(EShellState::CrackOne)) {
      if (x56c_shellState != EShellState::CrackOne) {
        CrackShell(mgr, xa14_crackOneParticle, x34_transform, x570_babyData.x158_crackOneSfx, false);
        x56c_shellState = EShellState::CrackOne;
        xa04_drawMaterialIdx = 1;
      }
    }
  }

  float hp = (x56c_shellState == EShellState::Destroyed ? x8ec_bodyHP : x570_babyData.GetShellHitPoints());

  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_mouthCollisionActor)) {
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

void CBabygoth::UpdateTimers(float dt) {
  if (x8d8_attackTimeLeft > 0.f)
    x8d8_attackTimeLeft -= dt * (xa49_28_onApproachPath ? 2.f : 1.f);
  if (x8e4_fireballAttackTimeLeft > 0.f)
    x8e4_fireballAttackTimeLeft -= dt * (xa49_28_onApproachPath ? 2.f : 1.f);
  if (x8e0_attackCooldownTimeLeft > 0.f)
    x8e0_attackCooldownTimeLeft -= dt;
  if (x8e8_interestTimer < x570_babyData.x168_interestTime)
    x8e8_interestTimer += dt;
}

void CBabygoth::UpdateHealth(CStateManager& mgr) {
  if (!x400_25_alive)
    return;

  if (x56c_shellState == EShellState::Destroyed) {
    float dam = 0.f;
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_mouthCollisionActor)) {
      dam = zeus::max(dam, x8ec_bodyHP - colAct->GetHealthInfo(mgr)->GetHP());
    }

    for (TUniqueId uid : x9f8_shellIds) {
      if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
        dam = zeus::max(dam, x8ec_bodyHP - colAct->GetHealthInfo(mgr)->GetHP());
      }
    }

    HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - dam);
    if (HealthInfo(mgr)->GetHP() <= 0.f) {
      Death(mgr, {}, EScriptObjectState::DeathRattle);
      xa48_26_inProjectileAttack = true;
      xa49_26_readyForTeam = false;
      RemoveFromTeam(mgr);
      RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    } else {
      if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_mouthCollisionActor))
        colAct->HealthInfo(mgr)->SetHP(x8ec_bodyHP);

      for (TUniqueId uid : x9f8_shellIds) {
        if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
          colAct->HealthInfo(mgr)->SetHP(x8ec_bodyHP);
        }
      }
    }
  } else {
    UpdateShellHealth(mgr);
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
  if (!x400_25_alive || x450_bodyController->GetFallState() == pas::EFallState::Zero || xa49_24_gettingUp)
    return;

  x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "GetUp"sv);
}

bool CBabygoth::CheckShouldWakeUp(CStateManager& mgr, float dt) {
  if (!xa48_30_heardPlayerFire)
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
    SetPathFindMode(EPathFindMode::Normal);
    CPatterned::PathFind(mgr, EStateMsg::Activate, arg);
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
    x568_stateProg = 0;
    xa49_24_gettingUp = true;
  } else if (msg == EStateMsg::Update) {
    if (x568_stateProg == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Getup)
        x568_stateProg = 3;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
    } else if (x568_stateProg == 3 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Getup)
      x568_stateProg = 4;
  } else if (msg == EStateMsg::Deactivate) {
    xa49_24_gettingUp = false;
  }
}

void CBabygoth::Enraged(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    xa48_29_hasBeenEnraged = true;
    x568_stateProg = 0;
  } else if (msg == EStateMsg::Update) {
    if (x568_stateProg == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate)
        x568_stateProg = 3;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Three));
    } else if (x568_stateProg == 3 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate)
      x568_stateProg = 4;
  }
}

void CBabygoth::FollowPattern(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_stateProg = (xa49_25_shouldStepBackwards ? 0 : 4);
    xa49_25_shouldStepBackwards = false;
    x8f0_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x8f0_boneTracking.SetActive(true);
  } else if (msg == EStateMsg::Update) {
    if (x568_stateProg == 0) {
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));
    } else if (x568_stateProg == 3) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step)
        x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
      else
        x568_stateProg = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x8f0_boneTracking.SetActive(false);
    SetPathFindMode(EPathFindMode::Normal);
  }
}

void CBabygoth::Taunt(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
  } else if (msg == EStateMsg::Update &&
             x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
    x568_stateProg = 4;
  }
}

void CBabygoth::Crouch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    xa48_30_heardPlayerFire = xa48_24_isAlert = false;
    x400_24_hitByPlayerProjectile = false;
    x8e8_interestTimer = 0.f;
  } else if (msg == EStateMsg::Deactivate) {
    AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
  }
}

void CBabygoth::Deactivate(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_stateProg = 1;
  } else if (msg == EStateMsg::Update) {
    if (x568_stateProg == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_stateProg = 3;
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
      }
    } else if (x568_stateProg == 1) {
      if ((x3a0_latestLeashPosition - GetTranslation()).magSquared() > 1.f)
        x568_stateProg = 2;
      else {
        zeus::CVector3f arrivalVec = x45c_steeringBehaviors.Arrival(*this, x3a0_latestLeashPosition, 15.f);
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(arrivalVec, {}, 1.f));
      }
    } else if (x568_stateProg == 2) {
      float angle = zeus::CVector3f::getAngleDiff(GetTranslation(), x8c4_initialFaceDir);
      if (angle > zeus::degToRad(5.f)) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd({}, x8c4_initialFaceDir, 1.f));
        return;
      }
      x568_stateProg = 0;
    } else if (x568_stateProg == 3 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
      x568_stateProg = 4;
    }
  }
}

void CBabygoth::Generate(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_stateProg = 0;
    x8c4_initialFaceDir = GetTransform().basis[1];
  } else if (msg == EStateMsg::Update) {
    if (x568_stateProg == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
        x568_stateProg = 3;
        SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
      }
    } else if (x568_stateProg == 3 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate)
      x568_stateProg = 4;
  }
}

void CBabygoth::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    xa49_26_readyForTeam = false;
    RemoveFromTeam(mgr);
    x400_24_hitByPlayerProjectile = false;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    if (HasPatrolPath(mgr, 0.f)) {
      Patrol(mgr, msg, arg);
      UpdateDest(mgr);
    } else {
      SetDestPos(x3a0_latestLeashPosition);
    }
    x8b8_backupDestPos = x2e0_destPos;
    if (GetSearchPath()) {
      SetPathFindMode(EPathFindMode::Normal);
      CPatterned::PathFind(mgr, msg, arg);
    }
  } else if (msg == EStateMsg::Update) {
    if (GetSearchPath() && !PathShagged(mgr, 0.f)) {
      SetPathFindMode(EPathFindMode::Normal);
      CPatterned::PathFind(mgr, msg, arg);
      ApplySeparationBehavior(mgr);
    } else {
      zeus::CVector3f arrivalVec = x45c_steeringBehaviors.Arrival(*this, x8b8_backupDestPos, 9.f);
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
    SetPathFindMode(EPathFindMode::Normal);
    CPatterned::PathFind(mgr, EStateMsg::Activate, arg);
    if (!xa49_27_locomotionValid)
      x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
    xa48_31_approachNeedsPathSearch = true;
    xa49_28_onApproachPath = true;
  } else if (msg == EStateMsg::Update) {
    SetPathFindMode(EPathFindMode::Approach);
    if (xa48_31_approachNeedsPathSearch) {
      x2e0_destPos = x8b8_backupDestPos;
      if (x7d0_approachPathSearch.FindClosestReachablePoint(GetTranslation(), x2e0_destPos) ==
          CPathFindSearch::EResult::Success) {
        if ((x2e0_destPos - GetTranslation()).magSquared() < 10.f)
          x2e0_destPos = GetTranslation();
        x8b8_backupDestPos = x2e0_destPos;
        CPatterned::PathFind(mgr, EStateMsg::Activate, arg);
        if (!xa49_27_locomotionValid)
          x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
      }
      xa48_31_approachNeedsPathSearch = false;
    }

    xa49_27_locomotionValid &= !IsDestinationObstructed(mgr);
    if (xa49_27_locomotionValid && GetSearchPath() && !PathShagged(mgr, 0.f) &&
        (x7d0_approachPathSearch.GetCurrentWaypoint() < x7d0_approachPathSearch.GetWaypoints().size() - 1)) {
      CPatterned::PathFind(mgr, msg, arg);
      ApplySeparationBehavior(mgr);
    } else if (ShouldTurn(mgr, 0.f)) {
      zeus::CVector3f direction = mgr.GetPlayer().GetTranslation() - GetTranslation();
      if (direction.canBeNormalized()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(direction.normalized(), {}, 1.f));
      }
    }
    xa49_27_locomotionValid = true;
  } else if (msg == EStateMsg::Deactivate) {
    x8f0_boneTracking.SetActive(false);
    SetPathFindMode(EPathFindMode::Normal);
  }
}

void CBabygoth::PathFind(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    xa49_28_onApproachPath = false;
    xa49_26_readyForTeam = true;
    xa48_24_isAlert = false;
    x8e8_interestTimer = 0.f;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x8f0_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x8f0_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    x8b8_backupDestPos = x2e0_destPos;
    SetPathFindMode(EPathFindMode::Normal);
    CPatterned::PathFind(mgr, msg, arg);
  } else if (msg == EStateMsg::Update) {
    SetPathFindMode(EPathFindMode::Normal);
    if (GetSearchPath() && !PathShagged(mgr, 0.f) && x6ec_pathSearch.GetCurrentWaypoint() < x6ec_pathSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, msg, arg);
      x8e8_interestTimer = 0.f;
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

void CBabygoth::UpdateAttackTimeLeft(CStateManager& mgr) {
  float mult = x56c_shellState == EShellState::Destroyed ? 0.6f : 1.f;
  x8d8_attackTimeLeft = (mgr.GetActiveRandom()->Float() * x308_attackTimeVariation + x304_averageAttackTime) * mult;
  x8e4_fireballAttackTimeLeft = (mgr.GetActiveRandom()->Float() * x570_babyData.GetFireballAttackVariance() +
                                 x570_babyData.GetFireballAttackTime()) * mult;
}

void CBabygoth::SpecialAttack(CStateManager& mgr, EStateMsg state, float) {
  if (state == EStateMsg::Activate) {
    xa48_27_ = true;
    x8e8_interestTimer = 0.f;
    xa49_27_locomotionValid = false;
    TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x6e8_teamMgr);
    if (aimgr && aimgr->HasTeamAiRole(GetUniqueId()))
      x568_stateProg = aimgr->AddRangedAttacker(GetUniqueId()) ? 0 : 4;
    else
      x568_stateProg = 0;
  } else if (state == EStateMsg::Update) {
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x568_stateProg = 3;
        x3b4_speed = 2.f * x8d0_initialSpeed;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::ESeverity::One, mgr.GetPlayer().GetTranslation(), false));
      }
      break;
    case 3:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::ProjectileAttack)
        x568_stateProg = 4;
      break;
    default:
      break;
    }
  } else if (state == EStateMsg::Deactivate) {
    UpdateAttackTimeLeft(mgr);
    xa48_27_ = false;
    x3b4_speed = x8d0_initialSpeed;
    if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack)
      x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Ranged, mgr, x6e8_teamMgr, GetUniqueId(), false);
  }
}

void CBabygoth::ExtendCollisionActorTouchBounds(CStateManager& mgr, const zeus::CVector3f& extents) {
  for (u32 i = 0; i < x928_colActMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x928_colActMgr->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> cact = mgr.ObjectById(desc.GetCollisionActorId()))
      cact->SetExtendedTouchBounds(extents);
  }
}

void CBabygoth::UpdateAttack(CStateManager& mgr, float dt) {
  switch (x568_stateProg) {
  case 0:
    x8dc_attackTimer += dt;
    if (x8dc_attackTimer < x570_babyData.x160_timeUntilAttack) {
      if (!xa48_28_pendingAttackContactDamage) {
        x568_stateProg = 3;
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One));
      } else {
        zeus::CTransform mouthXf = GetLctrTransform(x9f4_mouthLocator);
        if ((mgr.GetPlayer().GetTranslation() - mouthXf.origin).dot(GetTransform().basis[1]) > 0.f) {
          SetPathFindMode(EPathFindMode::Normal);
          auto* path = GetSearchPath();
          if (path && !PathShagged(mgr, 0.f)) {
            CPatterned::PathFind(mgr, EStateMsg::Update, dt);
            ApplySeparationBehavior(mgr);
          } else {
            x568_stateProg = 4;
          }
        } else {
          x568_stateProg = 4;
        }
      }
    } else {
      x568_stateProg = 4;
    }
    break;
  case 3:
    if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
      x568_stateProg = 4;
    } else {
      xa49_25_shouldStepBackwards = true;
    }
    break;
  default:
    break;
  }
}

void CBabygoth::Attack(CStateManager& mgr, EStateMsg state, float dt) {
  if (state == EStateMsg::Activate) {
    x568_stateProg = 0;
    xa48_25_ = xa48_28_pendingAttackContactDamage = true;
    x8e8_interestTimer = 0.f;
    xa49_25_shouldStepBackwards = false;
    x2e0_destPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    SetPathFindMode(EPathFindMode::Normal);
    CPatterned::PathFind(mgr, state, dt);
    x8dc_attackTimer = 0.f;
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    ExtendCollisionActorTouchBounds(mgr, zeus::CVector3f(0.2f));
  } else if (state == EStateMsg::Update) {
    UpdateAttack(mgr, dt);
  } else if (state == EStateMsg::Deactivate) {
    UpdateAttackTimeLeft(mgr);
    xa48_25_ = xa48_28_pendingAttackContactDamage = false;
    x8e0_attackCooldownTimeLeft = x570_babyData.x164_attackCooldownTime;
    x8dc_attackTimer = 0.f;
    float maxSpeed = x450_bodyController->GetBodyStateInfo().GetMaxSpeed();
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Clamped);
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(0.f,
      (maxSpeed > 0.f) ? x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(
          pas::ELocomotionAnim::Walk) / maxSpeed : 1.f);
    ExtendCollisionActorTouchBounds(mgr, zeus::skZero3f);
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Melee, mgr, x6e8_teamMgr, GetUniqueId(), true);
  }
}

void CBabygoth::ProjectileAttack(CStateManager& mgr, EStateMsg state, float) {
  if (state == EStateMsg::Activate) {
    xa48_26_inProjectileAttack = true;
    x8e8_interestTimer = 0.f;
    xa49_27_locomotionValid = false;
    TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x6e8_teamMgr);
    if (aimgr && aimgr->HasTeamAiRole(GetUniqueId()))
      x568_stateProg = aimgr->AddRangedAttacker(GetUniqueId()) ? 0 : 4;
    else
      x568_stateProg = 0;
  } else if (state == EStateMsg::Update) {
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x568_stateProg = 3;
        x3b4_speed = 2.f * x8d0_initialSpeed;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::ESeverity::Two, aimPos, false));
      }
      break;
    case 3:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        x568_stateProg = 4;
      } else if (!mgr.ObjectById(x980_flameThrower)) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(aimPos - GetTranslation());
        x8f0_boneTracking.UnsetTarget();
        x8f0_boneTracking.SetTargetPosition(aimPos);
      }
    }
  } else if (state == EStateMsg::Deactivate) {
    UpdateAttackTimeLeft(mgr);
    if (auto* fthrower = static_cast<CFlameThrower*>(mgr.ObjectById(x980_flameThrower)))
      fthrower->Reset(mgr, false);
    xa48_26_inProjectileAttack = false;
    if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack)
      x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
    x8f0_boneTracking.SetActive(false);
    x3b4_speed = x8d0_initialSpeed;
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Ranged, mgr, x6e8_teamMgr, GetUniqueId(), false);
  }
}

bool CBabygoth::Leash(CStateManager& mgr, float) {
  if ((x3a0_latestLeashPosition - GetTranslation()).magSquared() > x3c8_leashRadius * x3c8_leashRadius)
    return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() >
           x3cc_playerLeashRadius * x3cc_playerLeashRadius && x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
  return false;
}

bool CBabygoth::IsDestinationObstructed(const CStateManager& mgr) const {
  for (const CEntity* obj : mgr.GetListeningAiObjectList()) {
    if (const TCastToConstPtr<CAi> ai = obj) {
      if (ai->GetAreaIdAlways() == GetAreaIdAlways()) {
        if ((x8b8_backupDestPos - ai->GetTranslation()).magSquared() <= 10.f) {
          return true;
        }
      }
    }
  }
  return false;
}

void CBabygoth::DestroyShell(CStateManager& mgr) {
  GetModelData()->GetAnimationData()->SubstituteModelData(xa08_noShellModel);

  for (TUniqueId uid : x9f8_shellIds) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      colAct->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::Unknown41);
    }
  }
  xa04_drawMaterialIdx = 0;
}

void CBabygoth::CrackShell(CStateManager& mgr, const TLockedToken<CGenDescription>& desc,
                           const zeus::CTransform& xf, u16 sfx, bool nonEmitterSfx) {
  mgr.AddObject(new CExplosion(desc, mgr.AllocateUniqueId(), true,
                               CEntityInfo(GetAreaIdAlways(), CEntity::NullConnectionList), "Babygoth Shell Crack Fx"sv,
                               xf, 0, GetModelData()->GetScale(), zeus::skWhite));

  if (nonEmitterSfx)
    CSfxManager::SfxStart(sfx, 0x7f, 64 / 127.f, false, 0x7f, false, -1);
  else
    CSfxManager::AddEmitter(sfx, GetTranslation(), zeus::skUp, false, false, 0x7f, GetAreaIdAlways());
}

void CBabygoth::UpdateHealthInfo(CStateManager& mgr) {
  CHealthInfo* hInfo = HealthInfo(mgr);
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x9f6_mouthCollisionActor)) {
    (*colAct->HealthInfo(mgr)) = *hInfo;
    colAct->SetDamageVulnerability(x98c_dVuln);
  }

  for (TUniqueId uid : x9f8_shellIds) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      (*colAct->HealthInfo(mgr)) = *hInfo;
      colAct->SetDamageVulnerability(x98c_dVuln);
    }
  }
}

float CBabygoth::CalculateShellCrackHP(EShellState state) const {
  if (state == EShellState::Default)
    return x570_babyData.GetShellHitPoints();
  else if (state == EShellState::CrackOne)
    return (2.f / 3.f) * x570_babyData.GetShellHitPoints();
  else if (state == EShellState::CrackTwo)
    return (1.f / 3.f) * x570_babyData.GetShellHitPoints();

  return 0.f;
}

bool CBabygoth::ShouldTurn(CStateManager& mgr, float arg) {
  const float speedScale = GetModelData()->GetAnimationData()->GetSpeedScale();
  zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, (speedScale > 0.f ? 1.f / speedScale : 0.f));
  return zeus::CVector2f::getAngleDiff(GetTransform().basis[1].toVec2f(), (aimPos - GetTranslation()).toVec2f()) >
         (arg == 0.f ? zeus::degToRad(45.f) : arg);
}

bool CBabygoth::ShouldAttack(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetAreaIdAlways() == GetAreaIdAlways() && !mgr.GetPlayer().GetFrozenState() &&
      x8d8_attackTimeLeft <= 0.f && x8e0_attackCooldownTimeLeft <= 0.f) {
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    if (x450_bodyController->GetBodyStateInfo().GetMaxSpeed() * x570_babyData.x160_timeUntilAttack >
        (aimPos - GetTranslation()).magnitude() && !ShouldTurn(mgr, zeus::degToRad(25.f)) &&
        !IsPatternObstructed(mgr, GetLctrTransform(x9f4_mouthLocator).origin, aimPos)) {
      if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x6e8_teamMgr)) {
        if (aimgr->IsPartOfTeam(GetUniqueId()))
          return aimgr->AddMeleeAttacker(GetUniqueId());
      }
      return true;
    }
  }
  return false;
}

bool CBabygoth::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetAreaIdAlways() == GetAreaIdAlways() && x8e4_fireballAttackTimeLeft <= 0.f) {
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    zeus::CVector3f mouthPos = GetLctrTransform(x9f4_mouthLocator).origin;
    return (aimPos - mouthPos).magSquared() >= x300_maxAttackRange * x300_maxAttackRange &&
            !ShouldTurn(mgr, zeus::degToRad(25.f)) && !IsPatternObstructed(mgr, mouthPos, aimPos);
  }
  return false;
}

bool CBabygoth::ShouldFire(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetAreaIdAlways() == GetAreaIdAlways() && x8d8_attackTimeLeft <= 0.f) {
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    zeus::CVector3f mouthPos = GetLctrTransform(x9f4_mouthLocator).origin;
    return (aimPos - mouthPos).magSquared() <= x300_maxAttackRange * x300_maxAttackRange &&
           !ShouldTurn(mgr, zeus::degToRad(30.f)) && !IsPatternObstructed(mgr, mouthPos, aimPos);
  }
  return false;
}

bool CBabygoth::TooClose(CStateManager& mgr, float arg) {
  if (x930_aabox.CalculateAABox(GetTransform()).intersects(mgr.GetPlayer().GetBoundingBox())) {
    xa49_25_shouldStepBackwards = true;
    return true;
  }
  return false;
}

bool CBabygoth::LineOfSight(CStateManager& mgr, float arg) {
  zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  zeus::CVector3f mouthPos = GetLctrTransform(x9f4_mouthLocator).origin;
  return !IsPatternObstructed(mgr, mouthPos, aimPos);
}

bool CBabygoth::LostInterest(CStateManager& mgr, float arg) {
  return x8e8_interestTimer >= x570_babyData.x168_interestTime &&
         x6ec_pathSearch.OnPath(GetTranslation()) == CPathFindSearch::EResult::Success;
}

bool CBabygoth::InMaxRange(CStateManager& mgr, float) {
  return (GetTranslation() - mgr.GetPlayer().GetTranslation()).magSquared() < (1.5f * x300_maxAttackRange);
}

bool CBabygoth::InDetectionRange(CStateManager& mgr, float arg) {
  if (xa48_24_isAlert)
    return true;
  float range = (arg > 0.f ? arg : 1.f) * x3bc_detectionRange;
  zeus::CVector3f delta = mgr.GetPlayer().GetTranslation() - GetTranslation();
  if (delta.magSquared() < range * range) {
    if (x3c0_detectionHeightRange > 0.f)
      return delta.z() * delta.z() < x3c0_detectionHeightRange * x3c0_detectionHeightRange;
    return true;
  }
  return false;
}

bool CBabygoth::Listen(const zeus::CVector3f& origin, EListenNoiseType noiseType) {
  if (!x400_25_alive || noiseType != EListenNoiseType::PlayerFire || (origin - GetTranslation()).magSquared() >= 1600.f)
    return false;

  xa48_30_heardPlayerFire = true;
  return true;
}
} // namespace urde::MP1
