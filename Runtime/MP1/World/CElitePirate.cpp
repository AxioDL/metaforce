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
static constexpr std::array<SJointInfo, 3> skLeftArmJointList{{
    {"L_shoulder", "L_elbow", 1.f, 1.5f},
    {"L_wrist", "L_elbow", 0.9f, 1.3f},
    {"L_knee", "L_ankle", 0.9f, 1.3f},
}};

static constexpr std::array<SJointInfo, 3> skRightArmJointList{{
    {"R_shoulder", "R_elbow", 1.f, 1.5f},
    {"R_wrist", "R_elbow", 0.9f, 1.3f},
    {"R_knee", "R_ankle", 0.9f, 1.3f},
}};

static constexpr std::array<SSphereJointInfo, 7> skSphereJointList{{
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
, x10_(in.readFloatBig())
, x14_(in.readFloatBig())
, x18_(in.readFloatBig())
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
, xf0_(in)
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
                           const CElitePirateData& data)
: CPatterned(ECharacter::ElitePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x56c_vulnerability(pInfo.GetDamageVulnerability())
, x5d8_data(data)
, x6f8_boneTracking(*GetModelData()->GetAnimationData(), "Head_1", zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x738_(GetBoundingBox(), GetMaterialList())
, x7d0_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x8c0_(5.f)
, x988_24_(false)
, x988_25_(false)
, x988_26_(false)
, x988_27_(false)
, x988_28_alert(false)
, x988_29_(false)
, x988_30_(false)
, x988_31_(false)
, x989_24_(false) {
  if (x5d8_data.GetX20().IsValid()) {
    x760_energyAbsorbDesc = g_SimplePool->GetObj({SBIG('PART'), x5d8_data.GetX20()});
  }

  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableExplodeDeath(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  sub_80229248();
}

void CElitePirate::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CElitePirate::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    x6f8_boneTracking.Update(dt);
    if (sub_802273a8()) {
      x730_collisionActorMgr2->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
    }
    x5d4_collisionActorMgr1->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
    if (sub_80229208() && x5d8_data.x11f_) {
      x3b4_speed = 2.f * x7a0_;
    } else {
      x3b4_speed = x7a0_;
    }
    sub_80228e50(dt);
    sub_80228798();
    sub_802289dc(mgr, x772_launcherId, "grenadeLauncher_LCTR"sv);
    sub_80228e84(mgr);
    x328_31_ = sub_80229208();
  }
}

void CElitePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool shouldPass = true;

  switch (msg) {
  case EScriptObjectMessage::Activate: {
    if (sub_802273a8()) {
      x730_collisionActorMgr2->SetActive(mgr, true);
    }
    if (CEntity* ent = mgr.ObjectById(x772_launcherId)) {
      ent->SetActive(true);
    }
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    if (sub_802273a8()) {
      x730_collisionActorMgr2->SetActive(mgr, false);
    }
    x5d4_collisionActorMgr1->SetActive(mgr, false);
    if (CEntity* ent = mgr.ObjectById(x772_launcherId)) {
      ent->SetActive(false);
    }
    break;
  }
  case EScriptObjectMessage::Alert:
    x988_28_alert = true;
    break;
  case EScriptObjectMessage::Touched: {
    if (HealthInfo(mgr)->GetHP() <= 0.f)
      break;
    TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid);
    if (!actor) {
      if (uid == x772_launcherId && x772_launcherId != kInvalidUniqueId) {
        sub_8022759c(true, mgr);
      }
      break;
    }
    const TUniqueId& touchedUid = actor->GetLastTouchedObject();
    if (touchedUid != GetUniqueId()) {
      if (TCastToPtr<CGameProjectile>(mgr.ObjectById(touchedUid))) {
        sub_8022759c(true, mgr);
      }
      break;
    }
    if (!x988_31_) { // TODO is this right?
      if (x420_curDamageRemTime <= 0.f) {
        CDamageInfo info = GetContactDamage();
        info.SetDamage(0.5f * info.GetDamage());
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), info,
                        CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      }
      break;
    }
    if ((!x988_25_ || !IsArmClawCollider(uid, x774_collisionRJointIds)) &&
        (!x988_26_ || !IsArmClawCollider(uid, x788_collisionLJointIds))) {
      break;
    }
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                    CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
    x420_curDamageRemTime = x424_damageWaitTime;
    x988_24_ = false;
    break;
  }
  case EScriptObjectMessage::Registered: {
    x450_bodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    x772_launcherId = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, x772_launcherId);
    const auto& bodyStateInfo = x450_bodyController->GetBodyStateInfo();
    if (bodyStateInfo.GetMaxSpeed() > 0.f) {
      x7a4_ = (0.99f * bodyStateInfo.GetLocomotionSpeed(pas::ELocomotionAnim::Walk)) / bodyStateInfo.GetMaxSpeed();
    }
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(x7a4_, x7a4_);
    break;
  }
  case EScriptObjectMessage::Deleted:
    if (sub_802273a8()) {
      x730_collisionActorMgr2->Destroy(mgr);
    }
    x5d4_collisionActorMgr1->Destroy(mgr);
    mgr.FreeScriptObject(x772_launcherId);
    break;
  case EScriptObjectMessage::InitializedInArea:
    x7d0_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  case EScriptObjectMessage::Damage:
    shouldPass = false;
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid)) {
      if (TCastToPtr<CGameProjectile> projectile = mgr.ObjectById(actor->GetLastTouchedObject())) {
        if (uid == x770_collisionHeadId) {
          x428_damageCooldownTimer = 0.33f;
          const auto& damageInfo = projectile->GetDamageInfo();
          KnockBack(projectile->GetTranslation() - projectile->GetPreviousPos(), mgr, damageInfo,
                    EKnockBackType::Radius, false, damageInfo.GetKnockBackPower());
          CPatterned::AcceptScriptMsg(msg, uid, mgr);
        } else if (uid == x79c_ && x760_energyAbsorbDesc->IsLoaded()) {
          CreateEnergyAbsorb(mgr, projectile->GetTransform());
        }
        sub_8022759c(true, mgr);
      }
    } else if (uid == x772_launcherId && x772_launcherId != kInvalidUniqueId) {
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().frontVector(), pas::ESeverity::Eight));
    } else {
      ApplyDamageToHead(mgr, uid);
    }
    break;
  case EScriptObjectMessage::InvulnDamage: {
    sub_8022759c(true, mgr);
    if (!TCastToPtr<CCollisionActor>(mgr.ObjectById(uid))) {
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
  xb4_drawFlags.x1_matSetIdx = numMaterialSets - 1 < x7cc_ ? numMaterialSets - 1 : x7cc_;
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
    if (const CActor* actor = static_cast<const CActor*>(mgr.GetObjectById(x772_launcherId))) {
      return sub_80228864(actor);
    }
  }
  if (sub_802273a8()) {
    if (TCastToConstPtr<CCollisionActor> actor = mgr.GetObjectById(x770_collisionHeadId)) {
      return actor->GetTranslation();
    }
  }
  return GetLctrTransform("lockon_target_LCTR").origin;
}

zeus::CVector3f CElitePirate::GetAimPosition(const CStateManager& mgr, float) const {
  std::shared_ptr<CPlayerState> playerState = mgr.GetPlayerState();
  if (x5d4_collisionActorMgr1->GetActive() && playerState->IsFiringComboBeam() &&
      playerState->GetCurrentBeam() == CPlayerState::EBeamId::Wave) {
    if (TCastToConstPtr<CCollisionActor> actor = mgr.GetObjectById(x79c_)) {
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
    x988_24_ = true;
    break;
  case EUserEventType::DamageOff:
    handled = true;
    x988_24_ = false;
    break;
  case EUserEventType::ScreenShake:
    sub_802273a8();
    handled = true;
    break;
  case EUserEventType::BeginAction: {
    const zeus::CVector3f& origin = GetTranslation();
    const zeus::CVector3f& front = GetTransform().frontVector();
    float dot = (GetLctrTransform(node.GetLocatorName()).origin - origin).dot(front);
    const zeus::CTransform& xf = zeus::CTransform::Translate({
        origin.x() + dot * front.x(),
        origin.y() + dot * front.y(),
        origin.z(),
    });
    mgr.AddObject(new CShockWave(mgr.AllocateUniqueId(), "Shock Wave", {GetAreaIdAlways(), CEntity::NullConnectionList},
                                 xf, GetUniqueId(), GetShockWaveData(), sub_802273b0() ? 2.f : 1.3f,
                                 sub_802273b0() ? 0.4f : 0.5f));
    handled = true;
    break;
  }
  case EUserEventType::BecomeShootThrough:
    if (sub_802273a8()) {
      u32 numCollisionActors = x730_collisionActorMgr2->GetNumCollisionActors();
      for (u32 i = 0; i < numCollisionActors; i++) {
        const auto& description = x730_collisionActorMgr2->GetCollisionDescFromIndex(i);
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

const CCollisionPrimitive* CElitePirate::GetCollisionPrimitive() const { return &x738_; }

void CElitePirate::KnockBack(const zeus::CVector3f& pos, CStateManager& mgr, const CDamageInfo& info,
                             EKnockBackType type, bool inDeferred, float magnitude) {
  if (!sub_80227430(info)) {
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
    x989_24_ = false;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CElitePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x989_24_ = true;
    x988_28_alert = false;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    sub_80228634(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    x7bc_tauntTimer = x5d8_data.x4_tauntVariance * mgr.GetActiveRandom()->Float() + x5d8_data.x0_tauntInterval;
    if (TooClose(mgr, 0.f)) {
      x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
    }
  } else if (msg == EStateMsg::Update) {
    if (x7bc_tauntTimer > 0.f) {
      x7bc_tauntTimer -= dt;
    }
    if (!TooClose(mgr, 0.f) && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      if (PathShagged(mgr, 0.f)) {
        const zeus::CVector3f& move = x8c0_.GetValue(GetTranslation(), GetTransform().frontVector());
        if (move != zeus::skZero3f) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
        }
      } else if (ShouldTurn(mgr, 0.f)) {
        const zeus::CVector3f& aim =
            mgr.GetPlayer().GetAimPosition(mgr, 0.5f * GetModelData()->GetAnimationData()->GetSpeedScale());
        const zeus::CVector3f& face = aim - GetTranslation();
        if (face.canBeNormalized()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
        }
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
    x8b4_ = x2e0_destPos;
    if (GetSearchPath() != nullptr) {
      CPatterned::PathFind(mgr, msg, dt);
    }
  } else if (msg == EStateMsg::Update) {
    if (PathShagged(mgr, 0.f)) {
      const zeus::CVector3f& move = x45c_steeringBehaviors.Arrival(*this, x8b4_, 25.f);
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
    x989_24_ = false;
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
    x988_31_ = true;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    sub_80228634(mgr);
    CPatterned::PathFind(mgr, msg, dt);
  } else if (msg == EStateMsg::Update) {
    if (PathShagged(mgr, 0.f)) {
      auto move = x8c0_.GetValue(GetTranslation(), GetTransform().frontVector());
      if (move != zeus::skZero3f) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
      } else if (ShouldTurn(mgr, 0.f)) {
        const zeus::CVector3f& aim =
            mgr.GetPlayer().GetAimPosition(mgr, 0.5f * GetModelData()->GetAnimationData()->GetSpeedScale());
        auto face = aim - GetTranslation();
        if (face.canBeNormalized()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
        }
      }
    } else {
      CPatterned::PathFind(mgr, msg, dt);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x988_31_ = false;
    x6f8_boneTracking.SetActive(false);
    x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(x7a4_, x7a4_);
  }
}

void CElitePirate::Generate(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = EState::One;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_ = EState::Two;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
      }
    } else if (x568_ == EState::One) {
      if (ShouldTurn(mgr, 0.f)) {
        const auto& face = mgr.GetPlayer().GetTranslation() - GetTranslation();
        if (face.canBeNormalized()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
        }
      } else {
        x568_ = EState::Zero;
      }
    } else if (x568_ == EState::Two && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate) {
      x568_ = EState::Over;
    }
  } else if (msg == EStateMsg::Deactivate) {
    sub_8022759c(false, mgr);
    sub_802289b0(mgr, true);
  }
}

void CElitePirate::Attack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = EState::Zero;
    ExtendTouchBounds(mgr, x774_collisionRJointIds, zeus::CVector3f(2.f));
    if (x64_modelData->GetNumMaterialSets() > 1) {
      x7cc_ = 1;
    }
  } else if (msg == EStateMsg::Update) {
    if (x568_ == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_ = EState::One;
        x988_25_ = true;
        x7c8_currAnimId = x450_bodyController->GetCurrentAnimId();
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One));
      }
    } else if (x568_ == EState::One) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        if (x7c8_currAnimId == x450_bodyController->GetCurrentAnimId()) {
          x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
          if (ShouldAttack(mgr, 0.f)) {
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Two));
          }
        } else {
          x568_ = EState::Two;
          x988_25_ = false;
          x988_26_ = true;
          ExtendTouchBounds(mgr, x774_collisionRJointIds, zeus::skZero3f);
          ExtendTouchBounds(mgr, x788_collisionLJointIds, zeus::CVector3f(2.f));
        }
      } else {
        x568_ = EState::Over;
      }
    } else if (x568_ == EState::Two) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
      } else {
        x568_ = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    sub_802285c4(mgr);
    x988_24_ = false;
    x988_26_ = false;
    x988_25_ = false;
    x7c8_currAnimId = -1;
    ExtendTouchBounds(mgr, x774_collisionRJointIds, zeus::skZero3f);
    ExtendTouchBounds(mgr, x788_collisionLJointIds, zeus::skZero3f);
    x7cc_ = 0;
  }
}

void CElitePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Taunt(mgr, msg, dt); }

void CElitePirate::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = EState::Zero;
  } else if (msg == EStateMsg::Update) {
    const zeus::CVector3f& playerPos = mgr.GetPlayer().GetTranslation();
    if (x568_ == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x568_ = EState::Two;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::ESeverity::One, playerPos, false));
      }
    } else if (x568_ == EState::Two) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(playerPos - GetTranslation());
      } else {
        x568_ = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    sub_802285c4(mgr);
  }
}

void CElitePirate::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) { CAi::SpecialAttack(mgr, msg, dt); }

void CElitePirate::CallForBackup(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = EState::Zero;
    x988_30_ = true;
    sub_8022759c(false, mgr);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == EState::Zero) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_ = EState::Two;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Five, zeus::skZero3f));
      }
    } else if (x568_ == EState::Two) {
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate) {
        x568_ = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
  }
}

void CElitePirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    if (sub_802273a8()) {
      if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(x770_collisionHeadId)) {
        actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
      }
    }
    x5d4_collisionActorMgr1->SetActive(mgr, true);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    sub_80228634(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    if (TooClose(mgr, 0.f)) {
      x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
    }
  } else if (msg == EStateMsg::Update) {
    if (x988_27_) {
      x7c0_ -= dt;
      if (x7c0_ <= 0.f) {
        x988_27_ = false;
      }
    }
    x7a8_ = PathShagged(mgr, 0.f) ? x7a8_ + dt : 0.f;
    if (!TooClose(mgr, 0.f) && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else if (PathShagged(mgr, 0.f)) {
      const zeus::CVector3f& move = x8c0_.GetValue(GetTranslation(), GetTransform().frontVector());
      if (move != zeus::skZero3f) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
      }
    } else if (ShouldTurn(mgr, 0.f)) {
      const zeus::CVector3f& aim =
          mgr.GetPlayer().GetAimPosition(mgr, 0.5f * GetModelData()->GetAnimationData()->GetSpeedScale());
      const zeus::CVector3f& face = aim - GetTranslation();
      if (face.canBeNormalized()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
      }
    }
    sub_80227a90(mgr);
    sub_802277e0(mgr, dt);
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x6f8_boneTracking.SetActive(false);
    if (sub_802273a8()) {
      if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(x770_collisionHeadId)) {
        actor->SetDamageVulnerability(x56c_vulnerability);
      }
    }
    x5d4_collisionActorMgr1->SetActive(mgr, false);
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

bool CElitePirate::AnimOver(CStateManager& mgr, float) { return x568_ == EState::Over; }

bool CElitePirate::ShouldAttack(CStateManager& mgr, float) {
  if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() > x2fc_minAttackRange * x2fc_minAttackRange) {
    return false;
  }
  return !ShouldTurn(mgr, 0.f);
}

bool CElitePirate::InPosition(CStateManager& mgr, float) { return (x8b4_ - GetTranslation()).magSquared() < 25.f; }

bool CElitePirate::ShouldTurn(CStateManager& mgr, float) {
  return zeus::CVector2f::getAngleDiff((mgr.GetPlayer().GetTranslation() - GetTranslation()).toVec2f(),
                                       GetTransform().frontVector().toVec2f()) > zeus::degToRad(15.f);
}

bool CElitePirate::AggressionCheck(CStateManager& mgr, float arg) {
  if (x772_launcherId == kInvalidUniqueId && !PathShagged(mgr, arg)) {
    if (!x988_31_) {
      return 4.f * x300_maxAttackRange * x300_maxAttackRange <
             (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared();
    }
    return true;
  }
  return false;
}

bool CElitePirate::ShouldTaunt(CStateManager& mgr, float) { return x7bc_tauntTimer <= 0.f; }

bool CElitePirate::ShouldFire(CStateManager& mgr, float) { return ShouldFireFromLauncher(mgr, x772_launcherId); }

bool CElitePirate::ShotAt(CStateManager& mgr, float) { return x988_27_; }

bool CElitePirate::ShouldSpecialAttack(CStateManager& mgr, float) {
  if (x7b8_attackTimer <= 0.f && GetAreaIdAlways() == mgr.GetPlayer().GetAreaIdAlways()) {
    const zeus::CVector3f& dist = mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation();
    float magSquared = dist.magSquared();
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
  if (sub_802273a8()) {
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(x770_collisionHeadId)) {
      auto actHealth = actor->HealthInfo(mgr);
      actHealth->SetHP(health->GetHP());
      actHealth->SetKnockbackResistance(health->GetKnockbackResistance());
      actor->SetDamageVulnerability(x56c_vulnerability);
    }
  }
  UpdateHealthInfo(mgr, x772_launcherId);
}

void CElitePirate::sub_802289b0(CStateManager& mgr, bool b) { sub_80228920(mgr, b, x772_launcherId); }

void CElitePirate::sub_80229248() {
  float scale = 1.5f * GetModelData()->GetScale().y();
  float fVar1 = sub_802273b0() ? 5.f : 1.f;
  zeus::CAABox box{{-scale, -scale, 0.f}, {scale, scale, fVar1 * scale}};
  SetBoundingBox(box);
  x738_.SetBox(box);
  x7d0_pathFindSearch.SetCharacterRadius(scale);
  x7d0_pathFindSearch.SetCharacterHeight(3.f * scale);
}

void CElitePirate::sub_8022759c(bool param_1, CStateManager& mgr) {
  if (!sub_802273b0() || x7b4_hp <= 0.f || !param_1) {
    x988_27_ = param_1;
  } else if (HealthInfo(mgr)->GetHP() / x7b4_hp <= x7b0_) {
    x7b0_ -= 0.2f;
    x988_27_ = true;
  }
  if (x988_27_) {
    x7c0_ = mgr.GetActiveRandom()->Float() * x5d8_data.x18_ + x5d8_data.x14_;
  } else {
    x7c0_ = 0.f;
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
  if (sub_802273a8()) {
    x730_collisionActorMgr2 =
        std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, true);
    x730_collisionActorMgr2->SetActive(mgr, GetActive());
  }
  x774_collisionRJointIds.clear();
  x788_collisionLJointIds.clear();

  const CAnimData* animData = GetModelData()->GetAnimationData();
  constexpr zeus::CVector3f bounds{4.f, 4.f, 2.f};
  joints.emplace_back(CJointCollisionDescription::OBBCollision(animData->GetLocatorSegId("L_Palm_LCTR"sv), bounds,
                                                               zeus::skZero3f, "Shield"sv, 10.f));
  x5d4_collisionActorMgr1 =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);

  SetupCollisionActorInfo(mgr);
  SetupHealthInfo(mgr);

  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid},
      {EMaterialTypes::CollisionActor, EMaterialTypes::AIPassthrough, EMaterialTypes::Player}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
}

void CElitePirate::SetupCollisionActorInfo(CStateManager& mgr) {
  if (sub_802273a8()) {
    for (size_t i = 0; i < x730_collisionActorMgr2->GetNumCollisionActors(); ++i) {
      const auto& colDesc = x730_collisionActorMgr2->GetCollisionDescFromIndex(i);
      const TUniqueId& uid = colDesc.GetCollisionActorId();
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
    x730_collisionActorMgr2->AddMaterial(
        mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough, EMaterialTypes::Immovable});
  }

  const CJointCollisionDescription& description = x5d4_collisionActorMgr1->GetCollisionDescFromIndex(0);
  x79c_ = description.GetCollisionActorId();
  if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(x79c_)) {
    act->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::None);
  }
  x5d4_collisionActorMgr1->AddMaterial(mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough});
}

bool CElitePirate::IsArmClawCollider(std::string_view name, std::string_view locator, const SJointInfo* info,
                                     size_t infoCount) {
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
  CAnimationParameters& params = x5d8_data.x90_launcherAnimParams;
  if (!params.GetACSFile().IsValid()) {
    return;
  }
  CModelData mData(CAnimRes(params.GetACSFile(), params.GetCharacter(), GetModelData()->GetScale(),
                            params.GetInitialAnimation(), true));
  SBouncyGrenadeData grenadeData{x5d8_data.xd8_, x5d8_data.xa8_, x5d8_data.xc8_, x5d8_data.xcc_, x5d8_data.xd0_,
                                 x5d8_data.xd4_, x5d8_data.xf0_, x5d8_data.xf4_, x5d8_data.xf6_};
  CGrenadeLauncherData launcherData{grenadeData, x5d8_data.xa4_, x5d8_data.x9c_, x5d8_data.xa0_,
                                    x5d8_data.xe0_trajectoryInfo};
  mgr.AddObject(new CGrenadeLauncher(uid, "Grenade Launcher", {GetAreaIdAlways(), CEntity::NullConnectionList},
                                     GetTransform(), std::move(mData), mData.GetBounds(GetTransform().getRotation()),
                                     CHealthInfo(x5d8_data.xc4_launcherHp, 10.f), x56c_vulnerability,
                                     x5d8_data.x28_launcherActParams, GetUniqueId(), launcherData, 0.f));
}

void CElitePirate::ApplyDamageToHead(CStateManager& mgr, TUniqueId uid) {
  if (!sub_802273a8()) {
    return;
  }
  if (TCastToPtr<CWeapon> weapon = mgr.ObjectById(uid)) {
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
  CSfxManager::AddEmitter(x5d8_data.x24_sfxAbsorb, GetTranslation(), zeus::skUp, false, false, 0x7f, GetAreaIdAlways());
  x7ac_energyAbsorbCooldown = 0.25f;
}

void CElitePirate::UpdateHealthInfo(CStateManager& mgr, TUniqueId uid) {
  const CHealthInfo* const health = HealthInfo(mgr);
  if (uid != kInvalidUniqueId) {
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid)) {
      auto actHealth = actor->HealthInfo(mgr);
      actHealth->SetHP(x5d8_data.xc4_launcherHp);
      actHealth->SetKnockbackResistance(health->GetKnockbackResistance());
      actor->SetDamageVulnerability(x56c_vulnerability);
    }
  }
}

void CElitePirate::sub_80228920(CStateManager& mgr, bool b, TUniqueId uid) {
  if (uid == kInvalidUniqueId) {
    return;
  }
  if (auto entity = mgr.ObjectById(uid)) {
    mgr.SendScriptMsg(entity, GetUniqueId(), b ? EScriptObjectMessage::Start : EScriptObjectMessage::Stop);
  }
}

zeus::CVector3f CElitePirate::sub_80228864(const CActor* actor) const {
  const zeus::CTransform& targetTransform = actor->GetLocatorTransform("lockon_target_LCTR");
  return actor->GetTranslation() + actor->GetTransform().rotate(targetTransform.origin);
}

bool CElitePirate::sub_80227430(const CDamageInfo& info) const {
  return !x400_25_alive || info.GetWeaponMode().IsComboed() || info.GetWeaponMode().GetType() != EWeaponType::Plasma;
}

void CElitePirate::sub_80228634(CStateManager& mgr) {
  x8b4_ = GetTranslation();
  const zeus::CVector3f& dist = GetTranslation() - mgr.GetPlayer().GetTranslation();
  if (dist.canBeNormalized() && dist.magSquared() > x2fc_minAttackRange * x2fc_minAttackRange) {
    x2e0_destPos = GetTranslation() + (x2fc_minAttackRange * dist.normalized());
    x8b4_ = x2e0_destPos;
  }
}

void CElitePirate::sub_802285c4(CStateManager& mgr) {
  if (mgr.GetActiveRandom()->Float() > x5d8_data.x10_) {
    x7b8_attackTimer = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
  }
}

void CElitePirate::sub_80227a90(CStateManager& mgr) {
  // TODO
}

void CElitePirate::sub_802277e0(CStateManager& mgr, float dt) {
  if (!x988_27_ || x450_bodyController->IsFrozen()) {
    return;
  }
  x7c4_ += dt;
  if (x7c4_ < 3.f) {
    return;
  }
  if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Turn &&
      x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsMoving()) {
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCAdditiveReactionCmd(pas::EAdditiveReactionType::Six, 1.f, false));
  } else {
    bool b = false;
    if (sub_802273a8()) {
      if (TCastToConstPtr<CCollisionActor> actor = mgr.GetObjectById(x770_collisionHeadId)) {
        float z = actor->GetTranslation().z();
        b = z - 0.5f * (z - GetTranslation().z()) <= mgr.GetPlayer().GetTranslation().z();
      }
    }
    b = b || TooClose(mgr, 0.f);
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCAdditiveReactionCmd(b ? pas::EAdditiveReactionType::Seven : pas::EAdditiveReactionType::Five, 1.f, false));
  }
  x7c4_ = 0.f;
}

bool CElitePirate::sub_80229208() {
  if (x450_bodyController->GetLocomotionType() != pas::ELocomotionType::Crouch) {
    return false;
  }
  const pas::EAnimationState state = x450_bodyController->GetCurrentStateId();
  return state == pas::EAnimationState::Locomotion || state == pas::EAnimationState::Turn;
}

void CElitePirate::sub_80228e50(float dt) {
  if (x7b8_attackTimer > 0.f) {
    x7b8_attackTimer -= dt;
  }
  if (x7ac_energyAbsorbCooldown > 0.f) {
    x7ac_energyAbsorbCooldown -= dt;
  }
}

void CElitePirate::sub_80228798() {
  const zeus::CVector3f& pos = GetTranslation();
  if (x7d0_pathFindSearch.OnPath(pos) == CPathFindSearch::EResult::Success) {
    x8c0_.Clear();
  }
  x8c0_.AddValue(pos);
}

void CElitePirate::sub_802289dc(CStateManager& mgr, TUniqueId& uid, std::string_view name) {
  if (uid == kInvalidUniqueId) {
    return;
  }
  CActor* actor = static_cast<CActor*>(mgr.ObjectById(uid));
  if (actor == nullptr) {
    uid = kInvalidUniqueId;
    return;
  }
  actor->SetTransform(GetLctrTransform(name));
}

void CElitePirate::sub_80228e84(CStateManager& mgr) {
  float hp = HealthInfo(mgr)->GetHP();
  if (sub_802273a8()) {
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(x770_collisionHeadId)) {
      float headHp = actor->HealthInfo(mgr)->GetHP();
      HealthInfo(mgr)->SetHP(hp - (hp - headHp));
      *actor->HealthInfo(mgr) = *HealthInfo(mgr); // TODO does this work?
    }
  }
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
    RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
  }
}

void CElitePirate::ExtendTouchBounds(CStateManager& mgr, const rstl::reserved_vector<TUniqueId, 7>& uids,
                                     const zeus::CVector3f& vec) {
  for (const TUniqueId uid : uids) {
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid)) {
      actor->SetExtendedTouchBounds(vec);
    }
  }
}

bool CElitePirate::ShouldFireFromLauncher(CStateManager& mgr, TUniqueId launcherId) {
  if (x7b8_attackTimer <= 0.f && launcherId != kInvalidUniqueId) {
    const CActor* launcher = static_cast<const CActor*>(mgr.GetObjectById(launcherId));
    if (launcher != nullptr) {
      const zeus::CVector3f& aim = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
      if (x300_maxAttackRange * x300_maxAttackRange <= (aim - GetTranslation()).magSquared() && !ShouldTurn(mgr, 0.f)) {
        const zeus::CVector3f& origin = sub_80228864(launcher);
        if (!IsPatternObstructed(mgr, origin, aim)) {
          const zeus::CVector3f& target = CGrenadeLauncher::GrenadeTarget(mgr);
          float angleOut = x5d8_data.xe0_trajectoryInfo.x8_angleMin, velocityOut = x5d8_data.xe0_trajectoryInfo.x0_;
          CGrenadeLauncher::CalculateGrenadeTrajectory(target, origin, x5d8_data.xe0_trajectoryInfo, angleOut,
                                                       velocityOut);
          const zeus::CVector3f& rot = GetTransform().rotate({0.f, std::cos(angleOut), std::sin(angleOut)});
          return !CPatterned::IsPatternObstructed(mgr, target, target + (7.5f * rot));
        }
      }
    }
  }
  return false;
}

bool CElitePirate::ShouldCallForBackupFromLauncher(CStateManager& mgr, TUniqueId uid) {
  if (!x988_30_ && uid == kInvalidUniqueId && x5d8_data.x11e_) {
    return x7a8_ >= 3.f;
  }
  return false;
}

zeus::CVector3f CElitePirate::SUnknownStruct::GetValue(const zeus::CVector3f& v1, const zeus::CVector3f& v2) {
  while (x4_.size()) {
    const zeus::CVector3f v = x4_[x4_.size() - 1] - v1;
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
  if (x4_[x4_.size() - 1].magSquared() > x0_) {
    x4_.emplace_back(vec);
  }
}
} // namespace urde::MP1
