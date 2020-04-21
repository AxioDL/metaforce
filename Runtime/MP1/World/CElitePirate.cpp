#include "Runtime/MP1/World/CElitePirate.hpp"

#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/MP1/World/CGrenadeLauncher.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/ScriptLoader.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
constexpr std::array<SJointInfo, 3> skLeftArmJointList{{
    {"L_shoulder", "L_elbow", 1.f, 1.5f},
    {"L_wrist", "L_elbow", 0.9f, 1.3f},
    {"L_knee", "L_ankle", 0.9f, 1.3f},
}};

constexpr std::array<SJointInfo, 3> skRightArmJointList{{
    {"R_shoulder", "R_elbow", 1.f, 1.5f},
    {"R_wrist", "R_elbow", 0.9f, 1.3f},
    {"R_knee", "R_ankle", 0.9f, 1.3f},
}};

constexpr std::array<SSphereJointInfo, 7> skSphereJointList{{
    {"Head_1", 1.2f},
    {"L_Palm_LCTR", 1.5f},
    {"R_Palm_LCTR", 1.5f},
    {"Spine_1", 1.5f},
    {"Collar", 1.2f},
    {"L_Ball", 0.8f},
    {"R_Ball", 0.8f},
}};
} // namespace

CElitePirateData::CElitePirateData(CInputStream& in, u32 propCount)
: x0_tauntInterval(in.readFloatBig())
, x4_tauntVariance(in.readFloatBig())
, x8_(in.readFloatBig())
, xc_(in.readFloatBig())
, x10_attackChance(in.readFloatBig())
, x14_shotAtTime(in.readFloatBig())
, x18_shotAtTimeVariance(in.readFloatBig())
, x1c_(in.readFloatBig())
, x20_(in)
, x24_sfxAbsorb(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x28_launcherActParams(ScriptLoader::LoadActorParameters(in))
, x90_launcherAnimParams(ScriptLoader::LoadAnimationParameters(in))
, x9c_(in)
, xa0_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xa4_(in)
, xa8_(in)
, xc4_launcherHp(in.readFloatBig())
, xc8_(in)
, xcc_(in)
, xd0_(in)
, xd4_(in)
, xd8_(in)
, xe0_trajectoryInfo(in)
, xf0_grenadeNumBounces(in.readUint32Big())
, xf4_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xf6_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xf8_(in)
, xfc_(in)
, x118_(in)
, x11c_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x11e_(in.readBool())
, x11f_(propCount < 42 ? true : in.readBool()) {}

CElitePirate::CElitePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           CElitePirateData data)
: CPatterned(ECharacter::ElitePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x56c_vulnerability(pInfo.GetDamageVulnerability())
, x5d8_data(std::move(data))
, x6f8_boneTracking(*GetModelData()->GetAnimationData(), "Head_1", zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x738_collisionAabb(GetBoundingBox(), GetMaterialList())
, x7a0_initialSpeed(x3b4_speed)
, x7d0_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f) {
  if (x5d8_data.GetX20().IsValid()) {
    x760_energyAbsorbDesc = g_SimplePool->GetObj({SBIG('PART'), x5d8_data.GetX20()});
  }

  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableExplodeDeath(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  SetupPathFindSearch();
}

void CElitePirate::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CElitePirate::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    x6f8_boneTracking.Update(dt);
    if (HasWeakPointHead()) {
      x730_collisionActorMgrHead->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
    }
    x5d4_collisionActorMgr->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
    if (IsAttractingEnergy() && x5d8_data.GetX11F()) {
      x3b4_speed = 2.f * x7a0_initialSpeed;
    } else {
      x3b4_speed = x7a0_initialSpeed;
    }
    UpdateTimers(dt);
    UpdatePositionHistory();
    UpdateActorTransform(mgr, x772_launcherId, "grenadeLauncher_LCTR"sv);
    UpdateHealthInfo(mgr);
    x328_31_energyAttractor = IsAttractingEnergy();
  }
}

void CElitePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool shouldPass = true;

  switch (msg) {
  case EScriptObjectMessage::Activate: {
    if (HasWeakPointHead()) {
      x730_collisionActorMgrHead->SetActive(mgr, true);
    }
    if (CEntity* ent = mgr.ObjectById(x772_launcherId)) {
      ent->SetActive(true);
    }
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    if (HasWeakPointHead()) {
      x730_collisionActorMgrHead->SetActive(mgr, false);
    }
    x5d4_collisionActorMgr->SetActive(mgr, false);
    if (CEntity* ent = mgr.ObjectById(x772_launcherId)) {
      ent->SetActive(false);
    }
    break;
  }
  case EScriptObjectMessage::Alert:
    x988_28_alert = true;
    break;
  case EScriptObjectMessage::Touched: {
    if (HealthInfo(mgr)->GetHP() <= 0.f) {
      break;
    }
    const TCastToConstPtr<CCollisionActor> actor = mgr.ObjectById(uid);
    if (!actor) {
      if (uid == x772_launcherId && x772_launcherId != kInvalidUniqueId) {
        SetShotAt(true, mgr);
      }
      break;
    }
    const TUniqueId touchedUid = actor->GetLastTouchedObject();
    if (touchedUid != mgr.GetPlayer().GetUniqueId()) {
      if (TCastToConstPtr<CGameProjectile>(mgr.ObjectById(touchedUid))) {
        SetShotAt(true, mgr);
      }
      break;
    }
    if (!x988_24_damageOn) {
      if (x420_curDamageRemTime <= 0.f) {
        CDamageInfo info = GetContactDamage();
        info.SetDamage(0.5f * info.GetDamage());
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), info,
                        CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      }
      break;
    }
    if ((!x988_25_attackingRightClaw || !IsArmClawCollider(uid, x774_collisionRJointIds)) &&
        (!x988_26_attackingLeftClaw || !IsArmClawCollider(uid, x788_collisionLJointIds))) {
      break;
    }
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                    CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
    x420_curDamageRemTime = x424_damageWaitTime;
    x988_24_damageOn = false;
    break;
  }
  case EScriptObjectMessage::Registered: {
    x450_bodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    x772_launcherId = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, x772_launcherId);
    const auto& bodyStateInfo = x450_bodyController->GetBodyStateInfo();
    if (bodyStateInfo.GetMaxSpeed() > 0.f) {
      x7a4_steeringSpeed = (0.99f * bodyStateInfo.GetLocomotionSpeed(pas::ELocomotionAnim::Walk)) / bodyStateInfo.GetMaxSpeed();
    }
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(x7a4_steeringSpeed, x7a4_steeringSpeed);
    break;
  }
  case EScriptObjectMessage::Deleted:
    if (HasWeakPointHead()) {
      x730_collisionActorMgrHead->Destroy(mgr);
    }
    x5d4_collisionActorMgr->Destroy(mgr);
    mgr.FreeScriptObject(x772_launcherId);
    break;
  case EScriptObjectMessage::InitializedInArea:
    x7d0_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  case EScriptObjectMessage::Damage:
    shouldPass = false;
    if (const TCastToConstPtr<CCollisionActor> actor = mgr.ObjectById(uid)) {
      if (const TCastToConstPtr<CGameProjectile> projectile = mgr.ObjectById(actor->GetLastTouchedObject())) {
        if (uid == x770_collisionHeadId) {
          x428_damageCooldownTimer = 0.33f;
          const auto& damageInfo = projectile->GetDamageInfo();
          KnockBack(projectile->GetTranslation() - projectile->GetPreviousPos(), mgr, damageInfo,
                    EKnockBackType::Radius, false, damageInfo.GetKnockBackPower());
          CPatterned::AcceptScriptMsg(msg, uid, mgr);
        } else if (uid == x79c_ && x760_energyAbsorbDesc->IsLoaded()) {
          CreateEnergyAbsorb(mgr, projectile->GetTransform());
        }
        SetShotAt(true, mgr);
      }
    } else if (uid == x772_launcherId && x772_launcherId != kInvalidUniqueId) {
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().frontVector(), pas::ESeverity::Eight));
    } else {
      ApplyDamageToHead(mgr, uid);
    }
    break;
  case EScriptObjectMessage::InvulnDamage: {
    SetShotAt(true, mgr);
    if (!TCastToConstPtr<CCollisionActor>(mgr.ObjectById(uid))) {
      ApplyDamageToHead(mgr, uid);
    }
    break;
  }
  default:
    break;
  }

  if (shouldPass) {
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
  }
}

void CElitePirate::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
  auto modelData = GetModelData();
  x6f8_boneTracking.PreRender(mgr, *modelData->GetAnimationData(), GetTransform(), modelData->GetScale(),
                              *x450_bodyController);
  auto numMaterialSets = modelData->GetNumMaterialSets();
  xb4_drawFlags.x1_matSetIdx =
      numMaterialSets - 1 < x7cc_activeMaterialSet ? numMaterialSets - 1 : x7cc_activeMaterialSet;
}

const CDamageVulnerability* CElitePirate::GetDamageVulnerability() const {
  return &CDamageVulnerability::PassThroughVulnerabilty();
}

const CDamageVulnerability* CElitePirate::GetDamageVulnerability(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                                 const CDamageInfo& dInfo) const {
  return &CDamageVulnerability::PassThroughVulnerabilty();
}

zeus::CVector3f CElitePirate::GetOrbitPosition(const CStateManager& mgr) const {
  if (x772_launcherId != kInvalidUniqueId &&
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
    if (const auto* actor = static_cast<const CActor*>(mgr.GetObjectById(x772_launcherId))) {
      return GetLockOnPosition(actor);
    }
  }
  if (HasWeakPointHead()) {
    if (const TCastToConstPtr<CCollisionActor> actor = mgr.GetObjectById(x770_collisionHeadId)) {
      return actor->GetTranslation();
    }
  }
  return GetLctrTransform("lockon_target_LCTR").origin;
}

zeus::CVector3f CElitePirate::GetAimPosition(const CStateManager& mgr, float) const {
  const std::shared_ptr<CPlayerState>& playerState = mgr.GetPlayerState();
  if (x5d4_collisionActorMgr->GetActive() && playerState->IsFiringComboBeam() &&
      playerState->GetCurrentBeam() == CPlayerState::EBeamId::Wave) {
    if (const TCastToConstPtr<CCollisionActor> actor = mgr.GetObjectById(x79c_)) {
      return actor->GetTranslation();
    }
  }
  return GetOrbitPosition(mgr);
}

void CElitePirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case EUserEventType::Projectile:
    if (x772_launcherId != kInvalidUniqueId) {
      CEntity* launcher = mgr.ObjectById(x772_launcherId);
      mgr.SendScriptMsg(launcher, GetUniqueId(), EScriptObjectMessage::Action);
    }
    handled = true;
    break;
  case EUserEventType::DamageOn:
    handled = true;
    x988_24_damageOn = true;
    break;
  case EUserEventType::DamageOff:
    handled = true;
    x988_24_damageOn = false;
    break;
  case EUserEventType::ScreenShake:
    HasWeakPointHead();
    handled = true;
    break;
  case EUserEventType::BeginAction: {
    const zeus::CVector3f origin = GetTranslation();
    const zeus::CVector3f front = GetTransform().frontVector();
    const float dot = (GetLctrTransform(node.GetLocatorName()).origin - origin).dot(front);
    const zeus::CTransform xf = zeus::CTransform::Translate({
        origin.x() + dot * front.x(),
        origin.y() + dot * front.y(),
        origin.z(),
    });
    mgr.AddObject(new CShockWave(mgr.AllocateUniqueId(), "Shock Wave", {GetAreaIdAlways(), CEntity::NullConnectionList},
                                 xf, GetUniqueId(), GetShockWaveData(), IsElitePirate() ? 2.f : 1.3f,
                                 IsElitePirate() ? 0.4f : 0.5f));
    handled = true;
    break;
  }
  case EUserEventType::BecomeShootThrough:
    if (HasWeakPointHead()) {
      const u32 numCollisionActors = x730_collisionActorMgrHead->GetNumCollisionActors();
      for (u32 i = 0; i < numCollisionActors; ++i) {
        const auto& description = x730_collisionActorMgrHead->GetCollisionDescFromIndex(i);
        if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(description.GetCollisionActorId())) {
          actor->AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
        }
      }
    }
    handled = true;
    break;
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

const CCollisionPrimitive* CElitePirate::GetCollisionPrimitive() const { return &x738_collisionAabb; }

void CElitePirate::KnockBack(const zeus::CVector3f& pos, CStateManager& mgr, const CDamageInfo& info,
                             EKnockBackType type, bool inDeferred, float magnitude) {
  if (!CanKnockBack(info)) {
    return;
  }
  CPatterned::KnockBack(pos, mgr, info, type, inDeferred, magnitude);
  if (info.GetWeaponMode().IsComboed() && info.GetWeaponMode().GetType() == EWeaponType::Ice) {
    Freeze(mgr, zeus::skZero3f, GetTransform().transposeRotate(pos), 1.5f);
  }
}

void CElitePirate::TakeDamage(const zeus::CVector3f& pos, float) {}

void CElitePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x400_24_hitByPlayerProjectile = false;
    x989_24_onPath = false;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CElitePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x989_24_onPath = true;
    x988_28_alert = false;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    UpdateDestPos(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    x7bc_tauntTimer = x5d8_data.GetTauntVariance() * mgr.GetActiveRandom()->Float() + x5d8_data.GetTauntInterval();
    if (TooClose(mgr, 0.f)) {
      x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
    }
  } else if (msg == EStateMsg::Update) {
    if (x7bc_tauntTimer > 0.f) {
      x7bc_tauntTimer -= dt;
    }
    if (!TooClose(mgr, 0.f) && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else if (PathShagged(mgr, 0.f)) {
      const zeus::CVector3f move = x8c0_.GetValue(GetTranslation(), GetTransform().frontVector());
      if (move != zeus::skZero3f) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
      }
    } else if (ShouldTurn(mgr, 0.f)) {
      const zeus::CVector3f aim =
          mgr.GetPlayer().GetAimPosition(mgr, 0.5f * GetModelData()->GetAnimationData()->GetSpeedScale());
      const zeus::CVector3f face = aim - GetTranslation();
      if (face.canBeNormalized()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x6f8_boneTracking.SetActive(false);
  }
}

void CElitePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    if (HasPatrolPath(mgr, 0.f)) {
      CPatterned::Patrol(mgr, msg, dt);
      UpdateDest(mgr);
    } else {
      SetDestPos(x3a0_latestLeashPosition);
    }
    x8b4_targetDestPos = x2e0_destPos;
    if (GetSearchPath() != nullptr) {
      CPatterned::PathFind(mgr, msg, dt);
    }
  } else if (msg == EStateMsg::Update) {
    if (PathShagged(mgr, 0.f)) {
      const zeus::CVector3f move = x45c_steeringBehaviors.Arrival(*this, x8b4_targetDestPos, 25.f);
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
    } else {
      CPatterned::PathFind(mgr, msg, dt);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x988_28_alert = false;
  }
}

void CElitePirate::Halt(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x989_24_onPath = false;
    CMaterialFilter filter = GetMaterialFilter();
    filter.ExcludeList().Add(
        {EMaterialTypes::Wall, EMaterialTypes::Ceiling, EMaterialTypes::AIBlock, EMaterialTypes::Character});
    SetMaterialFilter(filter);
  } else if (msg == EStateMsg::Deactivate) {
    CMaterialFilter filter = GetMaterialFilter();
    filter.ExcludeList().Remove(
        {EMaterialTypes::Wall, EMaterialTypes::Ceiling, EMaterialTypes::AIBlock, EMaterialTypes::Character});
    SetMaterialFilter(filter);
  }
}

void CElitePirate::Run(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x988_31_running = true;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    UpdateDestPos(mgr);
    CPatterned::PathFind(mgr, msg, dt);
  } else if (msg == EStateMsg::Update) {
    if (PathShagged(mgr, 0.f)) {
      const auto move = x8c0_.GetValue(GetTranslation(), GetTransform().frontVector());
      if (move != zeus::skZero3f) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
      } else if (ShouldTurn(mgr, 0.f)) {
        const zeus::CVector3f aim =
            mgr.GetPlayer().GetAimPosition(mgr, 0.5f * GetModelData()->GetAnimationData()->GetSpeedScale());
        const auto face = aim - GetTranslation();
        if (face.canBeNormalized()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
        }
      }
    } else {
      CPatterned::PathFind(mgr, msg, dt);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x988_31_running = false;
    x6f8_boneTracking.SetActive(false);
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(x7a4_steeringSpeed, x7a4_steeringSpeed);
  }
}

void CElitePirate::Generate(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_state = EState::One;
  } else if (msg == EStateMsg::Update) {
    if (x568_state == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_state = EState::Two;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
      }
    } else if (x568_state == EState::One) {
      if (ShouldTurn(mgr, 0.f)) {
        const auto face = mgr.GetPlayer().GetTranslation() - GetTranslation();
        if (face.canBeNormalized()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
        }
      } else {
        x568_state = EState::Zero;
      }
    } else if (x568_state == EState::Two && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate) {
      x568_state = EState::Over;
    }
  } else if (msg == EStateMsg::Deactivate) {
    SetShotAt(false, mgr);
    SetLaunchersActive(mgr, true);
  }
}

void CElitePirate::Attack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_state = EState::Zero;
    ExtendTouchBounds(mgr, x774_collisionRJointIds, zeus::CVector3f(2.f));
    if (x64_modelData->GetNumMaterialSets() > 1) {
      x7cc_activeMaterialSet = 1;
    }
  } else if (msg == EStateMsg::Update) {
    if (x568_state == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_state = EState::One;
        x988_25_attackingRightClaw = true;
        x7c8_currAnimId = x450_bodyController->GetCurrentAnimId();
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One));
      }
    } else if (x568_state == EState::One) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        if (x7c8_currAnimId == x450_bodyController->GetCurrentAnimId()) {
          x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
          if (ShouldAttack(mgr, 0.f)) {
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Two));
          }
        } else {
          x568_state = EState::Two;
          x988_25_attackingRightClaw = false;
          x988_26_attackingLeftClaw = true;
          ExtendTouchBounds(mgr, x774_collisionRJointIds, zeus::skZero3f);
          ExtendTouchBounds(mgr, x788_collisionLJointIds, zeus::CVector3f(2.f));
        }
      } else {
        x568_state = EState::Over;
      }
    } else if (x568_state == EState::Two) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
      } else {
        x568_state = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    CheckAttackChance(mgr);
    x988_24_damageOn = false;
    x988_26_attackingLeftClaw = false;
    x988_25_attackingRightClaw = false;
    x7c8_currAnimId = -1;
    ExtendTouchBounds(mgr, x774_collisionRJointIds, zeus::skZero3f);
    ExtendTouchBounds(mgr, x788_collisionLJointIds, zeus::skZero3f);
    x7cc_activeMaterialSet = 0;
  }
}

void CElitePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Taunt(mgr, msg, dt); }

void CElitePirate::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_state = EState::Zero;
  } else if (msg == EStateMsg::Update) {
    const zeus::CVector3f& playerPos = mgr.GetPlayer().GetTranslation();
    if (x568_state == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x568_state = EState::Two;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::ESeverity::One, playerPos, false));
      }
    } else if (x568_state == EState::Two) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(playerPos - GetTranslation());
      } else {
        x568_state = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    CheckAttackChance(mgr);
  }
}

void CElitePirate::SpecialAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_state = EState::Zero;
  } else if (msg == EStateMsg::Update) {
    if (x568_state == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x568_state = EState::Two;
        x988_29_shockWaveAnim = true;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::ESeverity::Two, mgr.GetPlayer().GetTranslation(), false));
      }
    } else if (x568_state == EState::Two) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
      } else {
        x568_state = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    CheckAttackChance(mgr);
    x988_29_shockWaveAnim = false;
  }
}

void CElitePirate::CallForBackup(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_state = EState::Zero;
    x988_30_calledForBackup = true;
    SetShotAt(false, mgr);
  } else if (msg == EStateMsg::Update) {
    if (x568_state == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_state = EState::Two;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Five, zeus::skZero3f));
      }
    } else if (x568_state == EState::Two) {
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate) {
        x568_state = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
  }
}

void CElitePirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    if (HasWeakPointHead()) {
      if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(x770_collisionHeadId)) {
        actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
      }
    }
    x5d4_collisionActorMgr->SetActive(mgr, true);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    UpdateDestPos(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    if (TooClose(mgr, 0.f)) {
      x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
    }
  } else if (msg == EStateMsg::Update) {
    if (x988_27_shotAt) {
      x7c0_shotAtTimer -= dt;
      if (x7c0_shotAtTimer <= 0.f) {
        x988_27_shotAt = false;
      }
    }
    x7a8_pathShaggedTime = PathShagged(mgr, 0.f) ? x7a8_pathShaggedTime + dt : 0.f;
    if (!TooClose(mgr, 0.f) && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else if (PathShagged(mgr, 0.f)) {
      const zeus::CVector3f move = x8c0_.GetValue(GetTranslation(), GetTransform().frontVector());
      if (move != zeus::skZero3f) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
      }
    } else if (ShouldTurn(mgr, 0.f)) {
      const zeus::CVector3f aim =
          mgr.GetPlayer().GetAimPosition(mgr, 0.5f * GetModelData()->GetAnimationData()->GetSpeedScale());
      const zeus::CVector3f face = aim - GetTranslation();
      if (face.canBeNormalized()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
      }
    }
    AttractProjectiles(mgr);
    UpdateAbsorbBodyState(mgr, dt);
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x6f8_boneTracking.SetActive(false);
    if (HasWeakPointHead()) {
      if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(x770_collisionHeadId)) {
        actor->SetDamageVulnerability(x56c_vulnerability);
      }
    }
    x5d4_collisionActorMgr->SetActive(mgr, false);
  }
}

bool CElitePirate::TooClose(CStateManager& mgr, float) {
  return x2fc_minAttackRange * x2fc_minAttackRange > (GetTranslation() - mgr.GetPlayer().GetTranslation()).magSquared();
}

bool CElitePirate::InDetectionRange(CStateManager& mgr, float arg) {
  return x988_28_alert ? true : CPatterned::InDetectionRange(mgr, arg);
}

bool CElitePirate::SpotPlayer(CStateManager& mgr, float arg) {
  return x988_28_alert ? true : CPatterned::SpotPlayer(mgr, arg);
}

bool CElitePirate::AnimOver(CStateManager& mgr, float) { return x568_state == EState::Over; }

bool CElitePirate::ShouldAttack(CStateManager& mgr, float) {
  if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() > x2fc_minAttackRange * x2fc_minAttackRange) {
    return false;
  }
  return !ShouldTurn(mgr, 0.f);
}

bool CElitePirate::InPosition(CStateManager& mgr, float) {
  return (x8b4_targetDestPos - GetTranslation()).magSquared() < 25.f;
}

bool CElitePirate::ShouldTurn(CStateManager& mgr, float) {
  return zeus::CVector2f::getAngleDiff((mgr.GetPlayer().GetTranslation() - GetTranslation()).toVec2f(),
                                       GetTransform().frontVector().toVec2f()) > zeus::degToRad(15.f);
}

bool CElitePirate::AggressionCheck(CStateManager& mgr, float arg) {
  if (x772_launcherId == kInvalidUniqueId && !PathShagged(mgr, arg)) {
    if (x988_31_running) {
      return true;
    }
    return 4.f * x300_maxAttackRange * x300_maxAttackRange <
           (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared();
  }
  return false;
}

bool CElitePirate::ShouldTaunt(CStateManager& mgr, float) { return x7bc_tauntTimer <= 0.f; }

bool CElitePirate::ShouldFire(CStateManager& mgr, float) { return ShouldFireFromLauncher(mgr, x772_launcherId); }

bool CElitePirate::ShotAt(CStateManager& mgr, float) { return x988_27_shotAt; }

bool CElitePirate::ShouldSpecialAttack(CStateManager& mgr, float) {
  if (x7b8_attackTimer <= 0.f && GetAreaIdAlways() == mgr.GetPlayer().GetAreaIdAlways()) {
    const zeus::CVector3f dist = mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation();
    const float magSquared = dist.magSquared();
    if (x2fc_minAttackRange * x2fc_minAttackRange <= magSquared &&
        magSquared <= x300_maxAttackRange * x300_maxAttackRange) {
      return std::abs(dist.z()) < 3.f;
    }
  }
  return false;
}

bool CElitePirate::ShouldCallForBackup(CStateManager& mgr, float) {
  return ShouldCallForBackupFromLauncher(mgr, x772_launcherId);
}

CPathFindSearch* CElitePirate::GetSearchPath() { return &x7d0_pathFindSearch; }

void CElitePirate::SetupHealthInfo(CStateManager& mgr) {
  const CHealthInfo* const health = HealthInfo(mgr);
  x7b4_hp = health->GetHP();
  if (HasWeakPointHead()) {
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(x770_collisionHeadId)) {
      auto actHealth = actor->HealthInfo(mgr);
      actHealth->SetHP(health->GetHP());
      actHealth->SetKnockbackResistance(health->GetKnockbackResistance());
      actor->SetDamageVulnerability(x56c_vulnerability);
    }
  }
  SetupLauncherHealthInfo(mgr, x772_launcherId);
}

void CElitePirate::SetLaunchersActive(CStateManager& mgr, bool val) { SetLauncherActive(mgr, val, x772_launcherId); }

void CElitePirate::SetupPathFindSearch() {
  const float scale = 1.5f * GetModelData()->GetScale().y();
  const float fVar1 = IsElitePirate() ? 5.f : 1.f;
  const zeus::CAABox box{{-scale, -scale, 0.f}, {scale, scale, fVar1 * scale}};
  SetBoundingBox(box);
  x738_collisionAabb.SetBox(box);
  x7d0_pathFindSearch.SetCharacterRadius(scale);
  x7d0_pathFindSearch.SetCharacterHeight(3.f * scale);
}

void CElitePirate::SetShotAt(bool val, CStateManager& mgr) {
  if (!IsElitePirate() || x7b4_hp <= 0.f || !val) {
    x988_27_shotAt = val;
  } else if (HealthInfo(mgr)->GetHP() / x7b4_hp <= x7b0_) {
    x7b0_ -= 0.2f;
    x988_27_shotAt = true;
  }
  if (x988_27_shotAt) {
    x7c0_shotAtTimer = mgr.GetActiveRandom()->Float() * x5d8_data.GetShotAtTimeVariance() + x5d8_data.GetShotAtTime();
  } else {
    x7c0_shotAtTimer = 0.f;
  }
}

bool CElitePirate::IsArmClawCollider(TUniqueId uid, const rstl::reserved_vector<TUniqueId, 7>& vec) const {
  return std::find(vec.begin(), vec.end(), uid) != vec.end();
}

void CElitePirate::AddCollisionList(const SJointInfo* joints, size_t count,
                                    std::vector<CJointCollisionDescription>& outJoints) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId from = animData->GetLocatorSegId(joint.from);
    const CSegId to = animData->GetLocatorSegId(joint.to);
    if (to.IsInvalid() || from.IsInvalid()) {
      continue;
    }
    outJoints.emplace_back(CJointCollisionDescription::SphereSubdivideCollision(
        to, from, joint.radius, joint.separation, CJointCollisionDescription::EOrientationType::One, joint.from, 10.f));
  }
}

void CElitePirate::AddSphereCollisionList(const SSphereJointInfo* joints, size_t count,
                                          std::vector<CJointCollisionDescription>& outJoints) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId seg = animData->GetLocatorSegId(joint.name);
    if (seg.IsInvalid()) {
      continue;
    }
    outJoints.emplace_back(CJointCollisionDescription::SphereCollision(seg, joint.radius, joint.name, 10.f));
  }
}

void CElitePirate::SetupCollisionManager(CStateManager& mgr) {
  constexpr size_t jointInfoCount = skLeftArmJointList.size() + skRightArmJointList.size() + skSphereJointList.size();
  std::vector<CJointCollisionDescription> joints;
  joints.reserve(jointInfoCount);
  AddCollisionList(skLeftArmJointList.data(), skLeftArmJointList.size(), joints);
  AddCollisionList(skRightArmJointList.data(), skLeftArmJointList.size(), joints);
  AddSphereCollisionList(skSphereJointList.data(), skSphereJointList.size(), joints);
  if (HasWeakPointHead()) {
    x730_collisionActorMgrHead =
        std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, true);
    x730_collisionActorMgrHead->SetActive(mgr, GetActive());
  }
  x774_collisionRJointIds.clear();
  x788_collisionLJointIds.clear();

  const CAnimData* animData = GetModelData()->GetAnimationData();
  constexpr zeus::CVector3f bounds{4.f, 4.f, 2.f};
  joints.emplace_back(CJointCollisionDescription::OBBCollision(animData->GetLocatorSegId("L_Palm_LCTR"sv), bounds,
                                                               zeus::skZero3f, "Shield"sv, 10.f));
  x5d4_collisionActorMgr =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);

  SetupCollisionActorInfo(mgr);
  SetupHealthInfo(mgr);

  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid},
      {EMaterialTypes::CollisionActor, EMaterialTypes::AIPassthrough, EMaterialTypes::Player}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
}

void CElitePirate::SetupCollisionActorInfo(CStateManager& mgr) {
  if (HasWeakPointHead()) {
    for (size_t i = 0; i < x730_collisionActorMgrHead->GetNumCollisionActors(); ++i) {
      const auto& colDesc = x730_collisionActorMgrHead->GetCollisionDescFromIndex(i);
      const TUniqueId uid = colDesc.GetCollisionActorId();
      if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(uid)) {
        if (colDesc.GetName() == "Head_1"sv) {
          x770_collisionHeadId = uid;
        } else if (IsArmClawCollider(colDesc.GetName(), "R_Palm_LCTR"sv, skRightArmJointList.data(),
                                     skRightArmJointList.size())) {
          x774_collisionRJointIds.push_back(uid);
        } else if (IsArmClawCollider(colDesc.GetName(), "L_Palm_LCTR"sv, skLeftArmJointList.data(),
                                     skLeftArmJointList.size())) {
          x788_collisionLJointIds.push_back(uid);
        }
        if (uid != x770_collisionHeadId) {
          act->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
        }
      }
    }
    x730_collisionActorMgrHead->AddMaterial(
        mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough, EMaterialTypes::Immovable});
  }

  const CJointCollisionDescription& description = x5d4_collisionActorMgr->GetCollisionDescFromIndex(0);
  x79c_ = description.GetCollisionActorId();
  if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(x79c_)) {
    act->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::None);
  }
  x5d4_collisionActorMgr->AddMaterial(mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough});
}

bool CElitePirate::IsArmClawCollider(std::string_view name, std::string_view locator, const SJointInfo* info,
                                     size_t infoCount) const {
  if (name == locator) {
    return true;
  }
  for (size_t i = 0; i < infoCount; ++i) {
    if (name == info[i].from) {
      return true;
    }
  }
  return false;
}

void CElitePirate::CreateGrenadeLauncher(CStateManager& mgr, TUniqueId uid) {
  const CAnimationParameters& params = x5d8_data.GetLauncherAnimParams();
  if (!params.GetACSFile().IsValid()) {
    return;
  }
  CModelData mData(CAnimRes(params.GetACSFile(), params.GetCharacter(), GetModelData()->GetScale(),
                            params.GetInitialAnimation(), true));
  const zeus::CAABox bounds = mData.GetBounds(GetTransform().getRotation());
  mgr.AddObject(
      new CGrenadeLauncher(uid, "Grenade Launcher", {GetAreaIdAlways(), CEntity::NullConnectionList}, GetTransform(),
                           std::move(mData), bounds, CHealthInfo(x5d8_data.GetLauncherHP(), 10.f), x56c_vulnerability,
                           x5d8_data.GetLauncherActParams(), GetUniqueId(), x5d8_data.GetGrenadeLauncherData(), 0.f));
}

void CElitePirate::ApplyDamageToHead(CStateManager& mgr, TUniqueId uid) {
  if (!HasWeakPointHead()) {
    return;
  }
  if (const TCastToConstPtr<CWeapon> weapon = mgr.ObjectById(uid)) {
    CDamageInfo damageInfo = weapon->GetDamageInfo();
    damageInfo.SetRadius(0.f);
    mgr.ApplyDamage(uid, x770_collisionHeadId, weapon->GetOwnerId(), damageInfo,
                    CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
  }
}

void CElitePirate::CreateEnergyAbsorb(CStateManager& mgr, const zeus::CTransform& xf) {
  if (x7ac_energyAbsorbCooldown > 0.f) {
    return;
  }
  mgr.AddObject(new CExplosion(*x760_energyAbsorbDesc, mgr.AllocateUniqueId(), true,
                               {GetAreaIdAlways(), CEntity::NullConnectionList}, "Absorb energy Fx", xf, 0,
                               GetModelData()->GetScale(), zeus::skWhite));
  CSfxManager::AddEmitter(x5d8_data.GetSFXAbsorb(), GetTranslation(), zeus::skUp, false, false, 0x7f,
                          GetAreaIdAlways());
  x7ac_energyAbsorbCooldown = 0.25f;
}

void CElitePirate::SetupLauncherHealthInfo(CStateManager& mgr, TUniqueId uid) {
  const CHealthInfo* const health = HealthInfo(mgr);
  if (uid != kInvalidUniqueId) {
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid)) {
      auto actHealth = actor->HealthInfo(mgr);
      actHealth->SetHP(x5d8_data.GetLauncherHP());
      actHealth->SetKnockbackResistance(health->GetKnockbackResistance());
      actor->SetDamageVulnerability(x56c_vulnerability);
    }
  }
}

void CElitePirate::SetLauncherActive(CStateManager& mgr, bool val, TUniqueId uid) {
  if (uid == kInvalidUniqueId) {
    return;
  }
  if (auto entity = mgr.ObjectById(uid)) {
    mgr.SendScriptMsg(entity, GetUniqueId(), val ? EScriptObjectMessage::Start : EScriptObjectMessage::Stop);
  }
}

zeus::CVector3f CElitePirate::GetLockOnPosition(const CActor* actor) const {
  const zeus::CTransform targetTransform = actor->GetLocatorTransform("lockon_target_LCTR"sv);
  return actor->GetTranslation() + actor->GetTransform().rotate(targetTransform.origin);
}

bool CElitePirate::CanKnockBack(const CDamageInfo& info) const {
  return !x400_25_alive || info.GetWeaponMode().IsComboed() || info.GetWeaponMode().GetType() != EWeaponType::Plasma;
}

void CElitePirate::UpdateDestPos(CStateManager& mgr) {
  x8b4_targetDestPos = GetTranslation();
  const zeus::CVector3f playerPos = mgr.GetPlayer().GetTranslation();
  const zeus::CVector3f dist = GetTranslation() - playerPos;
  if (dist.canBeNormalized() && dist.magSquared() > x2fc_minAttackRange * x2fc_minAttackRange) {
    x2e0_destPos = playerPos + (x2fc_minAttackRange * dist.normalized());
    x8b4_targetDestPos = x2e0_destPos;
  }
}

void CElitePirate::CheckAttackChance(CStateManager& mgr) {
  if (mgr.GetActiveRandom()->Float() > x5d8_data.GetAttackChance()) {
    x7b8_attackTimer = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
  }
}

void CElitePirate::AttractProjectiles(CStateManager& mgr) {
  if (!IsAlive()) {
    return;
  }
  const TCastToConstPtr<CCollisionActor> actor = mgr.GetObjectById(x79c_);
  if (!actor) {
    return;
  }

  float radius = x5d8_data.GetX1C();
  const zeus::CVector3f actorPos = actor->GetTranslation();
  const zeus::CVector3f pos = GetTranslation();
  rstl::reserved_vector<TUniqueId, 1024> projNearList;
  const zeus::CAABox aabb{pos - radius, pos + radius};
  mgr.BuildNearList(projNearList, aabb, CMaterialFilter::MakeInclude({EMaterialTypes::Projectile}), nullptr);
  if (projNearList.empty()) {
    return;
  }

  rstl::reserved_vector<TUniqueId, 1024> charNearList;
  mgr.BuildNearList(charNearList, aabb, CMaterialFilter::MakeInclude({EMaterialTypes::Character}), nullptr);
  for (const TUniqueId projId : projNearList) {
    TCastToPtr<CGameProjectile> projectile = mgr.ObjectById(projId);
    if (!projectile || projectile->GetType() == EWeaponType::Missile ||
        projectile->GetOwnerId() != mgr.GetPlayer().GetUniqueId() ||
        projectile->GetAreaIdAlways() != GetAreaIdAlways()) {
      continue;
    }

    const zeus::CVector3f projectilePos = projectile->GetTranslation();
    const zeus::CVector3f actorProjDist = actorPos - projectilePos;
    if (GetTransform().frontVector().dot(actorProjDist) < 0.f) {
      const zeus::CVector3f projectileDir = projectilePos - projectile->GetPreviousPos();
      if (projectileDir.canBeNormalized() && IsClosestEnergyAttractor(mgr, charNearList, projectilePos)) {
        const float actorProjMag = actorProjDist.magnitude();
        const zeus::CVector3f b = projectilePos + ((0.5f * actorProjMag) * projectileDir.normalized());
        const zeus::CVector3f c = actorPos + zeus::CVector3f{0.f, 0.f, 0.4f * 0.4f * actorProjMag};
        const zeus::CVector3f p1 = zeus::getBezierPoint(projectilePos, b, c, actorPos, 0.333f);
        const zeus::CVector3f p2 = zeus::getBezierPoint(projectilePos, b, c, actorPos, 0.666f);

        const float magAdd = (p2 - p1).magnitude() + (p1 - projectilePos).magnitude() + (actorPos - p2).magnitude();
        const zeus::CVector3f p3 =
            zeus::getBezierPoint(projectilePos, b, c, actorPos, projectileDir.magnitude() / magAdd);

        const zeus::CVector3f look = p3 - projectilePos;
        if (look.canBeNormalized()) {
          zeus::CTransform xf = zeus::lookAt(zeus::skZero3f, look);
          xf.orthonormalize();
          CProjectileWeapon& weapon = projectile->ProjectileWeapon();
          weapon.SetWorldSpaceOrientation(xf);
          const zeus::CVector3f scaledVelocity = 0.8f * weapon.GetVelocity().normalized();
          weapon.SetVelocity(weapon.GetVelocity() * 0.39999998f + (scaledVelocity * 0.6f));
        }
      }
    }
    SetShotAt(true, mgr);
  }
}

void CElitePirate::UpdateAbsorbBodyState(CStateManager& mgr, float dt) {
  if (!x988_27_shotAt || x450_bodyController->IsFrozen()) {
    return;
  }
  x7c4_absorbUpdateTimer += dt;
  if (x7c4_absorbUpdateTimer < 3.f) {
    return;
  }
  if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Turn &&
      x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsMoving()) {
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCAdditiveReactionCmd(pas::EAdditiveReactionType::Six, 1.f, false));
  } else {
    bool b = false;
    if (HasWeakPointHead()) {
      if (const TCastToConstPtr<CCollisionActor> actor = mgr.GetObjectById(x770_collisionHeadId)) {
        const float z = actor->GetTranslation().z();
        b = z - 0.5f * (z - GetTranslation().z()) <= mgr.GetPlayer().GetTranslation().z();
      }
    }
    b = b || TooClose(mgr, 0.f);
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCAdditiveReactionCmd(b ? pas::EAdditiveReactionType::Seven : pas::EAdditiveReactionType::Five, 1.f, false));
  }
  x7c4_absorbUpdateTimer = 0.f;
}

bool CElitePirate::IsAttractingEnergy() const {
  if (x450_bodyController->GetLocomotionType() == pas::ELocomotionType::Crouch) {
    const auto state = x450_bodyController->GetCurrentStateId();
    return state == pas::EAnimationState::Locomotion || state == pas::EAnimationState::Turn;
  }
  return false;
}

void CElitePirate::UpdateTimers(float dt) {
  if (x7b8_attackTimer > 0.f) {
    x7b8_attackTimer -= dt;
  }
  if (x7ac_energyAbsorbCooldown > 0.f) {
    x7ac_energyAbsorbCooldown -= dt;
  }
}

void CElitePirate::UpdatePositionHistory() {
  const zeus::CVector3f pos = GetTranslation();
  if (x7d0_pathFindSearch.OnPath(pos) == CPathFindSearch::EResult::Success) {
    x8c0_.Clear();
  }
  x8c0_.AddValue(pos);
}

void CElitePirate::UpdateActorTransform(CStateManager& mgr, TUniqueId& uid, std::string_view name) {
  if (uid == kInvalidUniqueId) {
    return;
  }
  auto* actor = static_cast<CActor*>(mgr.ObjectById(uid));
  if (actor == nullptr) {
    uid = kInvalidUniqueId;
    return;
  }
  actor->SetTransform(GetLctrTransform(name));
}

void CElitePirate::UpdateHealthInfo(CStateManager& mgr) {
  const float hp = HealthInfo(mgr)->GetHP();
  if (HasWeakPointHead()) {
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(x770_collisionHeadId)) {
      const float headHp = actor->HealthInfo(mgr)->GetHP();
      HealthInfo(mgr)->SetHP(hp - (hp - headHp));
      *actor->HealthInfo(mgr) = *HealthInfo(mgr); // TODO does this work?
    }
  }
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
    RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
  }
}

void CElitePirate::ExtendTouchBounds(const CStateManager& mgr, const rstl::reserved_vector<TUniqueId, 7>& uids,
                                     const zeus::CVector3f& vec) const {
  for (const TUniqueId uid : uids) {
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid)) {
      actor->SetExtendedTouchBounds(vec);
    }
  }
}

bool CElitePirate::ShouldFireFromLauncher(CStateManager& mgr, TUniqueId launcherId) {
  if (x7b8_attackTimer > 0.f || launcherId == kInvalidUniqueId) {
    return false;
  }
  const auto* launcher = static_cast<const CActor*>(mgr.GetObjectById(launcherId));
  if (launcher == nullptr) {
    return false;
  }
  const zeus::CVector3f aim = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  if (x300_maxAttackRange * x300_maxAttackRange > (aim - GetTranslation()).magSquared() || ShouldTurn(mgr, 0.f)) {
    return false;
  }
  const zeus::CVector3f origin = GetLockOnPosition(launcher);
  if (IsPatternObstructed(mgr, origin, aim)) {
    return false;
  }
  const zeus::CVector3f target = CGrenadeLauncher::GrenadeTarget(mgr);
  float angleOut = x5d8_data.GetGrenadeTrajectoryInfo().GetAngleMin();
  float velocityOut = x5d8_data.GetGrenadeTrajectoryInfo().GetVelocityMin();
  CGrenadeLauncher::CalculateGrenadeTrajectory(target, origin, x5d8_data.GetGrenadeTrajectoryInfo(), angleOut,
                                               velocityOut);
  const zeus::CVector3f rot = GetTransform().rotate({0.f, std::cos(angleOut), std::sin(angleOut)});
  return !CPatterned::IsPatternObstructed(mgr, target, target + (7.5f * rot));
}

bool CElitePirate::ShouldCallForBackupFromLauncher(const CStateManager& mgr, TUniqueId uid) const {
  if (x988_30_calledForBackup || uid != kInvalidUniqueId || !x5d8_data.GetX11E()) {
    return false;
  }
  return x7a8_pathShaggedTime >= 3.f;
}

bool CElitePirate::IsClosestEnergyAttractor(const CStateManager& mgr,
                                            const rstl::reserved_vector<TUniqueId, 1024>& charNearList,
                                            const zeus::CVector3f& projectilePos) const {
  const float distance = (projectilePos - GetTranslation()).magSquared();
  for (const auto id : charNearList) {
    if (const TCastToConstPtr<CPatterned> actor = mgr.GetObjectById(id)) {
      if (actor->GetUniqueId() != GetUniqueId() && actor->IsEnergyAttractor() &&
          (projectilePos - actor->GetTranslation()).magSquared() < distance) {
        return false;
      }
    }
  }
  return true;
}

zeus::CVector3f CElitePirate::SUnknownStruct::GetValue(const zeus::CVector3f& v1, const zeus::CVector3f& v2) {
  while (!x4_.empty()) {
    const zeus::CVector3f v = x4_.back() - v1;
    if (v.dot(v2) > 0.f && v.isMagnitudeSafe()) {
      return v.normalized();
    }
    x4_.pop_back();
  }
  return zeus::skZero3f;
}

void CElitePirate::SUnknownStruct::AddValue(const zeus::CVector3f& vec) {
  if (x4_.size() > 15) {
    return;
  }
  if (x4_.empty()) {
    x4_.emplace_back(vec);
    return;
  }
  if (x4_.back().magSquared() > x0_) {
    x4_.emplace_back(vec);
  }
}
} // namespace urde::MP1
