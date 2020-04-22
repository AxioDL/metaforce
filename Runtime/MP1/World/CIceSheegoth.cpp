#include "Runtime/MP1/World/CIceSheegoth.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Weapon/CFlameInfo.hpp"
#include "Runtime/Weapon/CFlameThrower.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
constexpr std::array<SSphereJointInfo, 7> skSphereJointList = {{
    {"Jaw_end_LCTR", 0.55f},
    {"Pelvis", 1.1f},
    {"LCTR_SHEMOUTH", 1.1f},
    {"butt_LCTR", 0.7f},
    {"Ice_Shards_LCTR", 1.2f},
    {"GillL_LCTR", 0.6f},
    {"GillR_LCTR", 0.6f},
}};

constexpr std::array<SJointInfo, 2> skLeftLegJointList = {{
    {"L_hip", "L_knee", 0.4f, 0.75f},
    {"L_ankle", "L_Toe_3", 0.4f, 0.75f},
}};

constexpr std::array<SJointInfo, 2> skRightLegJointList = {{
    {"R_hip", "R_knee", 0.4f, 0.75f},
    {"R_ankle", "R_Toe_3", 0.4f, 0.75f},
}};
} // namespace

CIceSheegothData::CIceSheegothData(CInputStream& in, [[maybe_unused]] s32 propertyCount)
: x0_(zeus::degToRad(in.readFloatBig()))
, x4_(zeus::degToRad(in.readFloatBig()))
, x8_(zeus::CVector3f::ReadBig(in))
, x14_(in.readFloatBig())
, x18_(in)
, x80_(in)
, xe8_(in)
, x150_(in)
, x154_(in)
, x170_(in.readFloatBig())
, x174_(in.readFloatBig())
, x178_(in)
, x17c_fireBreathResId(in)
, x180_(in)
, x19c_(in)
, x1a0_(in)
, x1a4_(in)
, x1a8_(in)
, x1ac_(in)
, x1b0_(in.readFloatBig())
, x1b4_(in.readFloatBig())
, x1b8_(in)
, x1d4_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x1d8_(in.readFloatBig())
, x1dc_(in.readFloatBig())
, x1e0_(in.readFloatBig())
, x1e4_(in)
, x1e8_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x1ec_(in)
, x1f0_24_(in.readBool())
, x1f0_25_(in.readBool()) {}

CIceSheegoth::CIceSheegoth(TUniqueId uid, std::string_view name, const CEntityInfo& info, zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           const CIceSheegothData& sheegothData)
: CPatterned(ECharacter::IceSheeegoth, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x56c_sheegothData(sheegothData)
, x760_pathSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x844_approachSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x94c_(x3b4_speed)
, x974_(sheegothData.Get_x174())
, x98c_mouthVulnerability(pInfo.GetDamageVulnerability())
, x9f4_boneTracking(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, xa30_(GetBoundingBox(), GetMaterialList())
, xa58_(sheegothData.Get_x150(), sheegothData.Get_x154())
, xa84_(sheegothData.Get_x178().IsValid() ? g_SimplePool->GetObj({SBIG('WPSC'), sheegothData.Get_x178()})
                                          : g_SimplePool->GetObj("FlameThrower"sv))
, xa8c_(g_SimplePool->GetObj({SBIG('PART'), sheegothData.Get_x1a0()}))
, xa9c_(std::make_unique<CElementGen>(xa8c_, CElementGen::EModelOrientationType::Normal,
                                      CElementGen::EOptionalSystemFlags::One))
, xaa0_(g_SimplePool->GetObj({SBIG('PART'), sheegothData.Get_x1a4()}))
, xab0_(std::make_unique<CElementGen>(xaa0_, CElementGen::EModelOrientationType::Normal,
                                      CElementGen::EOptionalSystemFlags::One))
, xab4_(g_SimplePool->GetObj({SBIG('PART'), sheegothData.Get_x1a4()}))
, xac4_(std::make_unique<CElementGen>(xaa0_, CElementGen::EModelOrientationType::Normal,
                                      CElementGen::EOptionalSystemFlags::One))
, xac8_(g_SimplePool->GetObj({SBIG('ELSC'), sheegothData.Get_x1ac()}))
, xad8_(std::make_unique<CParticleElectric>(xac8_))
, xb28_24_shotAt(false)
, xb28_25_(false)
, xb28_26_(false)
, xb28_27_(false)
, xb28_28_(false)
, xb28_29_(false)
, xb28_30_(false)
, xb28_31_(false)
, xb29_24_(false)
, xb29_25_(false)
, xb29_26_(false)
, xb29_27_(false)
, xb29_28_(false)
, xb29_29_(false) {

  UpdateTouchBounds();
  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetX82_24(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x460_knockBackController.SetEnableExplodeDeath(false);
  x950_ = GetAnimationDistance(CPASAnimParmData(3, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(0))) *
          GetModelData()->GetScale().y();
  xa9c_->SetGlobalScale(GetModelData()->GetScale());
  xab0_->SetGlobalScale(GetModelData()->GetScale());
  xac4_->SetGlobalScale(GetModelData()->GetScale());
  xad8_->SetGlobalScale(GetModelData()->GetScale());
  x450_bodyController->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
  MakeThermalColdAndHot();
  x328_31_energyAttractor = true;
}

void CIceSheegoth::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CIceSheegoth::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
  AttractProjectiles(mgr);
  sub_801a0b8c(dt);
  sub_8019e6f0(mgr);
  if (!IsAlive()) {
    x974_ = std::max(0.f, x974_ - (dt * x56c_sheegothData.Get_x170()));
    if (GetBodyController()->GetBodyStateInfo().GetCurrentState()->IsDying()) {
      sub_8019eb50(mgr);
    }
  }
  x96c_ -= dt;
  if (x96c_ < 0.f) {
    GetBodyController()->GetCommandMgr().DeliverCmd(
        CBCAdditiveReactionCmd(pas::EAdditiveReactionType::Four, 1.f, false));
    x96c_ = 3.f * mgr.GetActiveRandom()->Float() + 2.f;
  }
  GetModelData()->GetAnimationData()->PreRender();
  x9f4_boneTracking.Update(dt);
  x9f4_boneTracking.PreRender(mgr, *GetModelData()->GetAnimationData(), GetTransform(), GetModelData()->GetScale(),
                              *GetBodyController());
  xa2c_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  PreventWorldCollisions(dt, mgr);
  sub_8019f680(mgr);
  sub_8019f5cc(dt, mgr);
  UpdateParticleEffects(dt, mgr);
  UpdateThermalFrozenState(x428_damageCooldownTimer > 0.f);
}

void CIceSheegoth::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate: {
    xa2c_collisionManager->SetActive(mgr, true);
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    xa2c_collisionManager->SetActive(mgr, false);
    break;
  }
  case EScriptObjectMessage::Start: {
    xb28_24_shotAt = true;
    break;
  }
  case EScriptObjectMessage::Touched: {
    ApplyContactDamage(sender, mgr);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(sender)) {
      if (const TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (wp->GetOwnerId() == mgr.GetPlayer().GetUniqueId()) {
          xb28_24_shotAt = true;
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::Registered: {
    if (!HasPatrolPath(mgr, 0.f)) {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    }
    x450_bodyController->Activate(mgr);
    SetupCollisionActorManager(mgr);
    CreateFlameThrower(mgr);
    if (x450_bodyController->GetBodyStateInfo().GetMaxSpeed() > 0.f) {
      x944_ = x948_ = (0.9f * x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::ELocomotionAnim::Walk)) /
                      x450_bodyController->GetBodyStateInfo().GetMaxSpeed();
    }
    GetBodyController()->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
    xaf4_mouthLocator = GetModelData()->GetAnimationData()->GetLocatorSegId("LCTR_SHEMOUTH"sv);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    xa2c_collisionManager->Destroy(mgr);
    if (xa80_flameThrowerId != kInvalidUniqueId) {
      mgr.FreeScriptObject(xa80_flameThrowerId);
      xa80_flameThrowerId = kInvalidUniqueId;
    }
    if (xaf0_) {
      CSfxManager::RemoveEmitter(xaf0_);
      xaf0_.reset();
    }
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    x760_pathSearch.SetArea(mgr.GetWorld()->GetArea(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    x844_approachSearch.SetArea(mgr.GetWorld()->GetArea(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    // TODO: Remove this
    mgr.SetBossParams(GetUniqueId(), GetHealthInfo(mgr)->GetHP(), 0);
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(sender)) {
      if (const TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (sender == xaf6_ && !xb28_27_) {
          sub_8019ebf0(mgr, wp->GetDamageInfo().GetDamage());
          if (!xaec_ || xaec_->IsSystemDeletable()) {
            xaec_ = std::make_unique<CElementGen>(xadc_, CElementGen::EModelOrientationType::Normal,
                                                  CElementGen::EOptionalSystemFlags::One);
          }
        } else {
          TakeDamage(zeus::skZero3f, 0.f);
          if (sub_8019fc40(colAct)) {
            x97c_ = 0.2f;
            x980_ = GetTransform().basis[1];
          }
        }
      }
    } else {
      ApplyWeaponDamage(mgr, sender);
    }
    xb28_24_shotAt = true;
    x968_interestTimer = 0.f;
    mgr.InformListeners(GetTranslation(), EListenNoiseType::PlayerFire);
    break;
  }
  case EScriptObjectMessage::InvulnDamage: {
    if (sender == xaf6_ && !xb28_27_) {
      if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(sender)) {
        if (const TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
          sub_8019ebf0(mgr, wp->GetDamageInfo().GetDamage());
          if (!xaec_ || xaec_->IsSystemDeletable()) {
            xaec_ = std::make_unique<CElementGen>(xadc_, CElementGen::EModelOrientationType::Normal,
                                                  CElementGen::EOptionalSystemFlags::One);
          }
        }
      }
    }
    mgr.InformListeners(GetTranslation(), EListenNoiseType::PlayerFire);
    xb28_24_shotAt = true;
    x968_interestTimer = 0.f;
    break;
  }
  case EScriptObjectMessage::SuspendedMove: {
    if (xa2c_collisionManager != nullptr) {
      xa2c_collisionManager->SetMovable(mgr, false);
    }
    break;
  }
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, sender, mgr);
}

void CIceSheegoth::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CPatterned::AddToRenderer(frustum, mgr);
}

void CIceSheegoth::Render(CStateManager& mgr) { CPatterned::Render(mgr); }
zeus::CVector3f CIceSheegoth::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPatterned::GetAimPosition(mgr, dt);
}

EWeaponCollisionResponseTypes CIceSheegoth::GetCollisionResponseType(const zeus::CVector3f& v1,
                                                                     const zeus::CVector3f& v2, const CWeaponMode& mode,
                                                                     EProjectileAttrib attrib) const {
  return mode.GetType() == EWeaponType::Ice ? EWeaponCollisionResponseTypes::None
                                            : CPatterned::GetCollisionResponseType(v1, v2, mode, attrib);
}

zeus::CAABox CIceSheegoth::GetSortingBounds(const CStateManager& mgr) const { return CActor::GetSortingBounds(mgr); }
void CIceSheegoth::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CIceSheegoth::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xb28_24_shotAt = false;
    xb29_28_ = false;
    x968_interestTimer = 0.f;
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x9f4_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x9f4_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(EPathFindMode::Normal);
    if (!x56c_sheegothData.Get_x1f0_24()) {
      CPatterned::PathFind(mgr, msg, dt);
    }
  } else if (msg == EStateMsg::Update) {
    SetPathFindMode(EPathFindMode::Normal);
    if (x56c_sheegothData.Get_x1f0_24() && GetSearchPath() != nullptr && !PathShagged(mgr, 0.f) &&
        x760_pathSearch.GetCurrentWaypoint() < x760_pathSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, EStateMsg::Update, dt);
      x968_interestTimer = 0.f;
      zeus::CVector3f moveVec = GetBodyController()->GetCommandMgr().GetMoveVector();
      if (GetTransform().basis[1].dot(moveVec) < 0.f && moveVec.canBeNormalized()) {
        GetBodyController()->GetCommandMgr().ClearLocomotionCmds();
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, moveVec.normalized(), 1.f));
      }
    } else {
      const zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - GetTranslation();
      if (sub_8019ecdc(mgr, zeus::degToRad(15.f)) && posDiff.canBeNormalized()) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, posDiff.normalized(), 1.f));
      }
    }

    x3b4_speed =
        (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Turn ? 2.f : 1.f) * x94c_;
  } else if (msg == EStateMsg::Deactivate) {
    x9f4_boneTracking.SetActive(false);
    x3b4_speed = x94c_;
  }
}
void CIceSheegoth::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {}
void CIceSheegoth::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x938_ = GetTransform().basis[1];
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
        GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
        x568_ = 3;
        SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCGenerateCmd{x56c_sheegothData.Get_x1f0_25() ? pas::EGenerateType::Eight : pas::EGenerateType::Zero});
      }
    } else if (x568_ == 3 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate) {
      x568_ = 4;
    }
  }
}
void CIceSheegoth::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {}
void CIceSheegoth::Attack(CStateManager& mgr, EStateMsg msg, float dt) {}
void CIceSheegoth::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {}
void CIceSheegoth::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x9f4_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x9f4_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(EPathFindMode::Normal);
    CPatterned::PathFind(mgr, EStateMsg::Activate, dt);
    GetBodyController()->GetCommandMgr().ClearLocomotionCmds();
  } else if (msg == EStateMsg::Update) {
    if (!sub_8019ecdc(mgr, zeus::degToRad(15.f))) {
      const float speedScale = GetModelData()->GetAnimationData()->GetSpeedScale();
      const zeus::CVector3f aimPos =
          (mgr.GetPlayer().GetAimPosition(mgr, speedScale > 0.f ? 1.25f / speedScale : 0.f).toVec2f() -
           GetTranslation().toVec2f());
      if (aimPos.canBeNormalized()) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{zeus::skZero3f, aimPos.normalized(), 1.f});
      }
    }
    x3b4_speed =
        (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Turn ? 2.f : 1.f) * x94c_;
  } else if (msg == EStateMsg::Deactivate) {
    x9f4_boneTracking.SetActive(false);
    x3b4_speed = x94c_;
  }
}

void CIceSheegoth::Crouch(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Crouch);
    x968_interestTimer = x56c_sheegothData.GetMaxInterestTime();
    x400_24_hitByPlayerProjectile = false;
  } else if (msg == EStateMsg::Deactivate) {
    AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
  }
}

void CIceSheegoth::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    xb29_25_ = true;
    sub_8019e894(mgr, true);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Taunt) {
        x568_ = 3;
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCTauntCmd{pas::ETauntType::Zero});
      }
    } else if (x568_ == 3 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x568_ = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    sub_8019e894(mgr, false);
    xb29_25_ = false;
  }
}
void CIceSheegoth::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {}
void CIceSheegoth::Flinch(CStateManager& mgr, EStateMsg msg, float dt) {}
void CIceSheegoth::Approach(CStateManager& mgr, EStateMsg msg, float dt) {}
void CIceSheegoth::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {}
void CIceSheegoth::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {}
bool CIceSheegoth::Leash(CStateManager& mgr, float arg) {
  const zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - GetTranslation();
  if ((x3a0_latestLeashPosition - posDiff).magSquared() <= x3c8_leashRadius * x3c8_leashRadius) {
    return false;
  }

  return posDiff.magSquared() > x3cc_playerLeashRadius && x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
}

bool CIceSheegoth::OffLine(CStateManager& mgr, float arg) {
  SetPathFindMode(EPathFindMode::Normal);
  return PathShagged(mgr, arg);
}

bool CIceSheegoth::TooClose(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
    const zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - GetTranslation();
    if (posDiff.z() < 0.f && posDiff.magSquared() < x978_) {
      return true;
    }
  }
  return false;
}

bool CIceSheegoth::InMaxRange(CStateManager& mgr, float arg) {
  if (x56c_sheegothData.Get_x1f0_24()) {
    return true;
  }

  return CPatterned::InMaxRange(mgr, arg);
}

bool CIceSheegoth::InDetectionRange(CStateManager& mgr, float arg) {
  if (xb28_24_shotAt) {
    return true;
  }

  zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - GetTranslation();
  float range = std::min(1.f, arg) * x3bc_detectionRange;
  if (posDiff.magSquared() < range * range) {
    if (x3c0_detectionHeightRange <= 0.f) {
      return true;
    }
    return (posDiff.z() * posDiff.z()) < x3c0_detectionHeightRange * x3c0_detectionHeightRange;
  }
  return false;
}

bool CIceSheegoth::SpotPlayer(CStateManager& mgr, float arg) {
  if (!xb28_24_shotAt) {
    return CPatterned::SpotPlayer(mgr, arg);
  }
  return true;
}
bool CIceSheegoth::AnimOver(CStateManager& mgr, float arg) { return x568_ == 4; }

bool CIceSheegoth::ShouldAttack(CStateManager& mgr, float arg) { return CAi::ShouldAttack(mgr, arg); }
bool CIceSheegoth::ShouldDoubleSnap(CStateManager& mgr, float arg) { return CAi::ShouldDoubleSnap(mgr, arg); }
bool CIceSheegoth::InPosition(CStateManager& mgr, float arg) { return CPatterned::InPosition(mgr, arg); }
bool CIceSheegoth::ShouldTurn(CStateManager& mgr, float arg) { return CAi::ShouldTurn(mgr, arg); }

bool CIceSheegoth::AggressionCheck(CStateManager& mgr, float arg) {
  return !(!IsAlive() || xb28_30_ || !sub_801a1794(mgr));
}
bool CIceSheegoth::ShouldFire(CStateManager& mgr, float arg) {
  if (GetAreaIdAlways() != mgr.GetPlayer().GetAreaIdAlways()) {
    return false;
  }
  zeus::CVector3f pos = mgr.GetPlayer().GetTranslation();
  zeus::CTransform lctrXf = GetLctrTransform(xaf4_mouthLocator);
  if ((pos - lctrXf.origin).magSquared() <= x300_maxAttackRange * x300_maxAttackRange &&
      std::fabs(pos.z() - GetTranslation().z()) < (lctrXf.origin.z() - GetTranslation().z()) &&
      !ShouldTurn(mgr, 0.2617994f)) {
    return !IsPatternObstructed(mgr, lctrXf.origin, pos);
  }
  return false;
}
bool CIceSheegoth::ShouldFlinch(CStateManager& mgr, float arg) { return xb29_25_ && x97c_ > 0.f; }
bool CIceSheegoth::ShotAt(CStateManager& mgr, float arg) { return x400_24_hitByPlayerProjectile; }
bool CIceSheegoth::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  if (GetAreaIdAlways() == mgr.GetPlayer().GetAreaId() && x954_attackTimeLeft <= 0.f &&
      x974_ >= 0.3333f * x56c_sheegothData.Get_x170() && sub_8019ecbc()) {
    const zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    const zeus::CTransform lctrXf = GetLctrTransform(xaf4_mouthLocator);
    if ((aimPos - lctrXf.origin).magSquared() > x2fc_minAttackRange * x2fc_minAttackRange &&
        !ShouldTurn(mgr, 0.2617994f)) {
      return !IsPatternObstructed(mgr, lctrXf.origin, aimPos);
    }
  }

  return false;
}
bool CIceSheegoth::LostInterest(CStateManager& mgr, float arg) {
  return x968_interestTimer >= x56c_sheegothData.GetMaxInterestTime();
}
void CIceSheegoth::UpdateTouchBounds() {
  x978_ = 1.75f * GetModelData()->GetScale().y();
  zeus::CAABox box{-x978_, -x978_, 0.f, x978_, x978_, 2.f * x978_};
  SetBoundingBox(box);
  xa30_.SetBox(box);
  x760_pathSearch.SetCharacterRadius(x978_);
  x760_pathSearch.SetCharacterHeight(x978_);
  x760_pathSearch.SetPadding(20.f);
  x844_approachSearch.SetCharacterRadius(x978_);
  x844_approachSearch.SetCharacterHeight(x978_);
  x844_approachSearch.SetPadding(20.f);
}
void CIceSheegoth::ApplyContactDamage(TUniqueId sender, CStateManager& mgr) {
  if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(sender)) {
    if (colAct->GetHealthInfo(mgr)->GetHP() <= 0.f) {
      return;
    }
    bool bVar5 = (xb28_29_ && !xb28_25_ && xb28_28_) ? true : sub_8019fc84(sender);

    if (colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId()) {
      if (bVar5) {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), x56c_sheegothData.Get_x1b8(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
      } else {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
      }
    }
  }
}
void CIceSheegoth::AddSphereCollisionList(const SSphereJointInfo* info, size_t count,
                                          std::vector<CJointCollisionDescription>& vecOut) {
  for (size_t i = 0; i < count; ++i) {
    const auto& joint = info[i];
    CSegId id = GetModelData()->GetAnimationData()->GetLocatorSegId(joint.name);
    if (id.IsInvalid()) {
      continue;
    }
    vecOut.push_back(CJointCollisionDescription::SphereCollision(id, joint.radius, joint.name, 1000.f));
    xb1c_.push_back(id);
  }
}

void CIceSheegoth::AddCollisionList(const SJointInfo* info, size_t count,
                                    std::vector<CJointCollisionDescription>& vecOut) {
  for (size_t i = 0; i < count; ++i) {
    const auto& joint = info[i];
    CSegId fromId = GetModelData()->GetAnimationData()->GetLocatorSegId(joint.from);
    CSegId toId = GetModelData()->GetAnimationData()->GetLocatorSegId(joint.to);
    if (fromId.IsInvalid() || toId.IsInvalid()) {
      continue;
    }
    vecOut.push_back(CJointCollisionDescription::SphereSubdivideCollision(
        fromId, toId, joint.radius, joint.separation, CJointCollisionDescription::EOrientationType::One, joint.from,
        1000.f));
  }
}
void CIceSheegoth::SetupCollisionActorManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> joints;
  joints.reserve(7);
  AddSphereCollisionList(skSphereJointList.data(), skSphereJointList.size(), joints);
  AddCollisionList(skLeftLegJointList.data(), skLeftLegJointList.size(), joints);
  AddCollisionList(skRightLegJointList.data(), skRightLegJointList.size(), joints);
  xa2c_collisionManager = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, true);
  xa2c_collisionManager->SetActive(mgr, GetActive());
  xa2c_collisionManager->AddMaterial(mgr, EMaterialTypes::CameraPassthrough);
  xb04_.clear();
  xafc_.clear();

  for (size_t i = 0; i < xa2c_collisionManager->GetNumCollisionActors(); ++i) {
    const auto& desc = xa2c_collisionManager->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(desc.GetCollisionActorId())) {
      colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
      if (desc.GetName() == "LCTR_SHEMOUTH"sv) {
        xaf8_mouthCollider = desc.GetCollisionActorId();
        colAct->SetDamageVulnerability(x98c_mouthVulnerability);
      } else if (desc.GetName() == "Jaw_end_LCTR"sv) {
        colAct->SetDamageVulnerability(CDamageVulnerability::PassThroughVulnerabilty());
      } else if (desc.GetName() == "Ice_Shards_LCTR"sv) {
        colAct->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::None);
      } else if (desc.GetName() == "GillL_LCTR"sv || desc.GetName() == "GillR_LCTR"sv) {
        xafc_.emplace_back(desc.GetCollisionActorId());
      } else {
        xb04_.emplace_back(desc.GetCollisionActorId());
        colAct->SetDamageVulnerability(x56c_sheegothData.Get_x80());
      }
    }
  }

  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid},
      {EMaterialTypes::Player, EMaterialTypes::CollisionActor, EMaterialTypes::AIPassthrough}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
  xa2c_collisionManager->AddMaterial(mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough});
}
void CIceSheegoth::SetupHealthInfo(CStateManager& mgr) {
  CHealthInfo* thisHealth = HealthInfo(mgr);
  for (auto& id : xafc_) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(id)) {
      *colAct->HealthInfo(mgr) = *thisHealth;
    }
  }

  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(xaf8_mouthCollider)) {
    *colAct->HealthInfo(mgr) = *thisHealth;
  }

  for (auto& id : xb04_) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(id)) {
      *colAct->HealthInfo(mgr) = *thisHealth;
    }
  }
}
void CIceSheegoth::sub_8019ebf0(CStateManager& mgr, float damage) {
  x974_ = zeus::clamp(0.f, x974_ + damage, x56c_sheegothData.Get_x170());
  float fVar1 = (1.f / 3.f) * x56c_sheegothData.Get_x170();
  if (fVar1 <= 0.f) {
    xb29_26_ = true;
  } else {
    xb29_26_ = mgr.GetActiveRandom()->Float() <= (x974_ / (3.f * fVar1));
  }
}

void CIceSheegoth::ApplyWeaponDamage(CStateManager& mgr, TUniqueId sender) {
  if (const TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(sender)) {
    mgr.ApplyDamage(sender, xaf8_mouthCollider, wp->GetOwnerId(), wp->GetDamageInfo(),
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
  }
}
void CIceSheegoth::CreateFlameThrower(CStateManager& mgr) {
  if (xa80_flameThrowerId != kInvalidUniqueId) {
    return;
  }

  xa80_flameThrowerId = mgr.AllocateUniqueId();
  mgr.AddObject(new CFlameThrower(xa84_, "IceSheegoth_Flame"sv, EWeaponType::Plasma,
                                  CFlameInfo{6, 4, x56c_sheegothData.GetFireBreathResId(), 15, 0.0625f, 20.f, 1.f}, {},
                                  EMaterialTypes::CollisionActor, x56c_sheegothData.GetFireBreathDamage(),
                                  xa80_flameThrowerId, GetAreaIdAlways(), GetUniqueId(), EProjectileAttrib::None,
                                  x56c_sheegothData.Get_x1e4(), x56c_sheegothData.Get_x1e8(),
                                  x56c_sheegothData.Get_x1ec()));
}
void CIceSheegoth::AttractProjectiles(CStateManager& mgr) {}
void CIceSheegoth::sub_801a0b8c(float dt) {}
void CIceSheegoth::sub_8019e6f0(CStateManager& mgr) {}
void CIceSheegoth::sub_8019eb50(CStateManager& mgr) {
  for (size_t i = 0; i < xa2c_collisionManager->GetNumCollisionActors(); ++i) {
    const auto& jInfo = xa2c_collisionManager->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(jInfo.GetCollisionActorId())) {
      colAct->AddMaterial(EMaterialTypes::ProjectilePassthrough);
    }
  }
}
void CIceSheegoth::PreventWorldCollisions(float dt, CStateManager& mgr) {
  if (GetBodyController()->GetLocomotionType() == pas::ELocomotionType::Crouch || !IsOnGround() ||
      mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
    return;
  }

  zeus::CAABox ourBox = GetModelData()->GetBounds(GetTransform());
  zeus::CAABox plBox = mgr.GetPlayer().GetBoundingBox();
  if (!ourBox.intersects(plBox)) {
    return;
  }

  zeus::CVector3f maxDeviation = zeus::skZero3f;
  float lastMag = 0.f;
  zeus::CVector3f predictedTrans = PredictMotion(dt).x0_translation;
  for (CSegId uid : xb1c_) {
    auto deviation = xa2c_collisionManager->GetDeviation(mgr, uid);
    if (!deviation) {
      continue;
    }
    const zeus::CVector3f curDeviation = ((*deviation) + predictedTrans);
    if (curDeviation.magnitude() > lastMag) {
      maxDeviation = curDeviation;
      lastMag = curDeviation.magnitude();
    }
  }

  if (lastMag <= (0.6f * GetModelData()->GetScale().y())) {
    return;
  }

  zeus::CVector3f posDiff = GetTranslation() - mgr.GetPlayer().GetTranslation();
  const zeus::CVector3f direction = GetTransform().transposeRotate(
      (posDiff.dot(posDiff * maxDeviation.normalized()) / posDiff.magSquared()) * posDiff);
  ApplyImpulseOR(GetMoveToORImpulseWR(direction, dt), zeus::CAxisAngle{});
}

void CIceSheegoth::sub_8019f680(CStateManager& mgr) {}
void CIceSheegoth::sub_8019f5cc(float dt, CStateManager& mgr) {}
void CIceSheegoth::UpdateParticleEffects(float dt, CStateManager& mgr) {}
void CIceSheegoth::UpdateAttackPosition(CStateManager& mgr, zeus::CVector3f& attackPos) {
  attackPos = GetTranslation();
  if (x954_attackTimeLeft > 0.f) {
    return;
  }

  attackPos = mgr.GetPlayer().GetTranslation();
  zeus::CVector3f distVec = GetTranslation() - attackPos;
  if (distVec.canBeNormalized()) {
    attackPos += x2fc_minAttackRange * distVec.normalized();
  }
}
bool CIceSheegoth::sub_8019ecdc(CStateManager& mgr, float minAngle) {
  const zeus::CVector3f plAimPos =
      mgr.GetPlayer().GetAimPosition(mgr, GetModelData()->GetAnimationData()->GetSpeedScale() > 0.f
                                              ? 1.25f / GetModelData()->GetAnimationData()->GetSpeedScale()
                                              : 0.f);
  return zeus::CVector2f::getAngleDiff(GetTransform().basis[1].toVec2f(),
                                       plAimPos.toVec2f() - GetTranslation().toVec2f()) > minAngle;
}
void CIceSheegoth::sub_8019e894(CStateManager& mgr, bool b1) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(xaf8_mouthCollider)) {
    colAct->SetDamageVulnerability(b1 ? x56c_sheegothData.Get_xe8() : x98c_mouthVulnerability);
  }
}
} // namespace urde::MP1