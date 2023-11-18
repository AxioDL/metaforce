#include "Runtime/MP1/World/CIceSheegoth.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CCameraShakeData.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
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

namespace metaforce::MP1 {
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
: x0_(zeus::degToRad(in.ReadFloat()))
, x4_(zeus::degToRad(in.ReadFloat()))
, x8_(in.Get<zeus::CVector3f>())
, x14_(in.ReadFloat())
, x18_(in)
, x80_(in)
, xe8_(in)
, x150_(in)
, x154_(in)
, x170_(in.ReadFloat())
, x174_(in.ReadFloat())
, x178_(in)
, x17c_fireBreathResId(in)
, x180_fireBreathDamage(in)
, x19c_(in)
, x1a0_(in)
, x1a4_(in)
, x1a8_(in)
, x1ac_(in)
, x1b0_(in.ReadFloat())
, x1b4_(in.ReadFloat())
, x1b8_(in)
, x1d4_(CSfxManager::TranslateSFXID(in.ReadLong()))
, x1d8_(in.ReadFloat())
, x1dc_(in.ReadFloat())
, x1e0_maxInterestTime(in.ReadFloat())
, x1e4_(in)
, x1e8_(CSfxManager::TranslateSFXID(in.ReadLong()))
, x1ec_(in)
, x1f0_24_(in.ReadBool())
, x1f0_25_(in.ReadBool()) {}

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
, xadc_(g_SimplePool->GetObj({SBIG('PART'), sheegothData.Get_x19c()})) {
  UpdateTouchBounds();
  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetX82_24(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x460_knockBackController.SetEnableExplodeDeath(false);
  x950_ = GetAnimationDistance(
              CPASAnimParmData(pas::EAnimationState::Step, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(0))) *
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
  UpdateTimers(dt);
  UpdateScanState(mgr);
  if (!IsAlive()) {
    x974_ = std::max(0.f, x974_ - (dt * x56c_sheegothData.Get_x170()));
    if (GetBodyController()->GetBodyStateInfo().GetCurrentState()->IsDying()) {
      SetPassthroughVulnerability(mgr);
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
  UpdateHealthInfo(mgr);
  SetSteeringSpeed(dt, mgr);
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
    if (const TCastToConstPtr<CCollisionActor> colAct = mgr.ObjectById(sender)) {
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
    if (xaf0_crackleSfx) {
      CSfxManager::RemoveEmitter(xaf0_crackleSfx);
      xaf0_crackleSfx.reset();
    }
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    x760_pathSearch.SetArea(mgr.GetWorld()->GetArea(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    x844_approachSearch.SetArea(mgr.GetWorld()->GetArea(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (const TCastToConstPtr<CCollisionActor> colAct = mgr.ObjectById(sender)) {
      if (const TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (sender == xaf6_iceShardsCollider && !xb28_27_) {
          sub_8019ebf0(mgr, wp->GetDamageInfo().GetDamage());
          if (!xaec_ || xaec_->IsSystemDeletable()) {
            xaec_ = std::make_unique<CElementGen>(xadc_, CElementGen::EModelOrientationType::Normal,
                                                  CElementGen::EOptionalSystemFlags::One);
          }
        } else {
          TakeDamage(zeus::skZero3f, 0.f);
          if (IsGillCollider(colAct.GetPtr())) {
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
    if (sender == xaf6_iceShardsCollider && !xb28_27_) {
      if (const TCastToConstPtr<CCollisionActor> colAct = mgr.ObjectById(sender)) {
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

  if (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot) {
    if (x428_damageCooldownTimer == 0.f && xb29_25_ &&
        (HasModelData() && (GetModelData()->HasAnimData() || GetModelData()->HasNormalModel()))) {
      GetModelData()->RenderParticles(frustum);
    }
  } else {
    std::optional<zeus::CAABox> bounds1;
    if (xaec_) {
      bounds1 = xaec_->GetBounds();
    }
    std::optional<zeus::CAABox> bounds2;
    if (xa9c_) {
      bounds2 = xa9c_->GetBounds();
    }

    std::optional<zeus::CAABox> bounds3;
    if (xab0_) {
      bounds3 = xab0_->GetBounds();
    }

    std::optional<zeus::CAABox> bounds4;
    if (xac4_) {
      bounds4 = xac4_->GetBounds();
    }

    std::optional<zeus::CAABox> bounds5;
    if (xad8_) {
      bounds5 = xad8_->GetBounds();
    }

    /* Lawl retro
    std::optional<zeus::CAABox> bounds6;
    if (xad8_) {
      bounds6 = xad8_->GetBounds();
    }
   */

    zeus::CAABox accumulatedBounds = zeus::skInvertedBox;
    if (bounds1) {
      accumulatedBounds.accumulateBounds(*bounds1);
    }
    /* Lawl retro
    if (bounds6) {
      accumulatedBounds.accumulateBounds(*bounds6);
    }
    */
    if (bounds2) {
      accumulatedBounds.accumulateBounds(*bounds2);
    }
    if (bounds3) {
      accumulatedBounds.accumulateBounds(*bounds3);
    }
    if (bounds4) {
      accumulatedBounds.accumulateBounds(*bounds4);
    }
    if (bounds5) {
      accumulatedBounds.accumulateBounds(*bounds5);
    }

    if (frustum.aabbFrustumTest(accumulatedBounds)) {
      if (xaec_) {
        g_Renderer->AddParticleGen(*xaec_);
      }

      g_Renderer->AddParticleGen(*xa9c_);
      g_Renderer->AddParticleGen(*xab0_);
      g_Renderer->AddParticleGen(*xac4_);
      g_Renderer->AddParticleGen(*xad8_);
    }
  }
}

zeus::CVector3f CIceSheegoth::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (GetBodyController()->GetLocomotionType() != pas::ELocomotionType::Crouch) {
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal) {
      zeus::CVector3f pos = zeus::skZero3f;
      for (TUniqueId uid : xafc_gillColliders) {
        if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
          pos += colAct->GetTranslation();
        }
      }
      if (!pos.isZero()) {
        return pos * (1.f / static_cast<float>(xafc_gillColliders.size()));
      }
    } else if (!xb29_29_scanned && mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Scan) {
      if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(xaf6_iceShardsCollider)) {
        return colAct->GetTranslation();
      }
    } else if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(xaf8_mouthCollider)) {
      return colAct->GetTranslation();
    }
  }

  return CPatterned::GetAimPosition(mgr, dt);
}

EWeaponCollisionResponseTypes CIceSheegoth::GetCollisionResponseType(const zeus::CVector3f& v1,
                                                                     const zeus::CVector3f& v2, const CWeaponMode& mode,
                                                                     EProjectileAttrib attrib) const {
  return mode.GetType() == EWeaponType::Ice ? EWeaponCollisionResponseTypes::None
                                            : CPatterned::GetCollisionResponseType(v1, v2, mode, attrib);
}

zeus::CAABox CIceSheegoth::GetSortingBounds(const CStateManager& mgr) const {
  const zeus::CAABox box = CActor::GetSortingBounds(mgr);
  const zeus::CVector3f offset = 0.125f * (box.max - box.min);
  const zeus::CVector3f center = box.center();
  return {center - offset, center + offset};
}

void CIceSheegoth::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch (type) {
  case EUserEventType::Projectile: {
    zeus::CTransform gunPos = GetLctrTransform(node.GetLocatorName());
    zeus::CVector3f interceptPos = GetProjectileInfo()->PredictInterceptPos(
        gunPos.origin, mgr.GetPlayer().GetAimPosition(mgr, 0.f), mgr.GetPlayer(), true, dt);
    zeus::CVector3f predictOffset = interceptPos - gunPos.origin;
    zeus::CVector3f lookPos;
    if (zeus::CVector3f::getAngleDiff(GetTransform().frontVector(), predictOffset) > zeus::degToRad(60.f)) {
      if (!predictOffset.canBeNormalized()) {
        lookPos = gunPos.origin + predictOffset.magnitude() * gunPos.frontVector();
      } else {
        lookPos = gunPos.origin + (predictOffset.magnitude() *
                                   zeus::CVector3f::slerp(GetTransform().frontVector(), predictOffset.normalized(),
                                                          zeus::CRelAngle::FromDegrees(60.f)));
      }
    }

    LaunchProjectile(zeus::lookAt(gunPos.origin, lookPos), mgr, 4, EProjectileAttrib::None, false, std::nullopt, -1,
                     false, zeus::skOne3f);
    x974_ = std::max(0.f, x974_ - 0.3333f * x56c_sheegothData.Get_x170());
    if (xb28_27_ && !ShouldSpecialAttack(mgr, 0.f)) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd{EBodyStateCmd::ExitState});
    }
    return;
  }
  case EUserEventType::DamageOn:
    if (!xb28_26_) {
      break;
    }

    if (CFlameThrower* fl = static_cast<CFlameThrower*>(mgr.ObjectById(xa80_flameThrowerId))) {
      fl->Fire(GetTransform(), mgr, false);
    }
    break;
  case EUserEventType::DamageOff:
    if (!xb28_26_) {
      break;
    }

    if (CFlameThrower* fl = static_cast<CFlameThrower*>(mgr.ObjectById(xa80_flameThrowerId))) {
      fl->Reset(mgr, false);
    }
    break;
  case EUserEventType::ScreenShake:
    ShakePlayer(mgr);
    return;
  case EUserEventType::BecomeShootThrough:
    SetPassthroughVulnerability(mgr);
    return;
  default:
    break;
  }
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

void CIceSheegoth::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);

    if (HasPatrolPath(mgr, 0.f)) {
      Patrol(mgr, msg, dt);
      UpdateDest(mgr);
    } else {
      SetDestPos(x3a0_latestLeashPosition);
    }

    const zeus::CVector3f arrivalPos = x45c_steeringBehaviors.Arrival(*this, x92c_lastDest, 15.f);
    x92c_lastDest = x2e0_destPos;

    if (GetSearchPath() != nullptr) {
      SetPathFindMode(EPathFindMode::Normal);
      CPatterned::PathFind(mgr, msg, dt);
      const zeus::CVector3f moveVec = GetBodyController()->GetCommandMgr().GetMoveVector();
      if (moveVec.canBeNormalized()) {
        GetBodyController()->GetCommandMgr().ClearLocomotionCmds();
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCLocomotionCmd{arrivalPos.magnitude() * moveVec.normalized(), zeus::skZero3f, 1.f});
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{arrivalPos, zeus::skZero3f, 1.f});
      }
    }
  } else if (msg == EStateMsg::Update) {
    const zeus::CVector3f arrivalPos = x45c_steeringBehaviors.Arrival(*this, x92c_lastDest, 15.f);

    if (GetSearchPath() && PathShagged(mgr, dt)) {
      SetPathFindMode(EPathFindMode::Normal);
      CPatterned::PathFind(mgr, EStateMsg::Update, dt);
      const zeus::CVector3f moveVec = GetBodyController()->GetCommandMgr().GetMoveVector();
      if (moveVec.canBeNormalized()) {
        GetBodyController()->GetCommandMgr().ClearLocomotionCmds();
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCLocomotionCmd{arrivalPos.magnitude() * moveVec.normalized(), zeus::skZero3f, 1.f});
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{arrivalPos, zeus::skZero3f, 1.f});
      }
    }
  }
}

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

void CIceSheegoth::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 1;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_ = 3;
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCGenerateCmd{pas::EGenerateType::One});
      }
    } else if (x568_ == 1) {
      if ((x3a0_latestLeashPosition - GetTranslation()).magSquared() > 1.f) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{
            x45c_steeringBehaviors.Arrival(*this, x3a0_latestLeashPosition, 15.f), zeus::skZero3f, 1.f});
      } else {
        x568_ = 2;
      }
    } else if (x568_ == 2) {
      const float angleDiff = zeus::CVector3f::getAngleDiff(GetTransform().frontVector(), x938_);
      if (angleDiff <= zeus::degToRad(5.f)) {
        x568_ = 0;
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{zeus::skZero3f, x938_, 1.f});
      }
    } else if (x568_ == 3) {
      x568_ = 4;
    }
  }
}

void CIceSheegoth::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    xb28_29_ = true;
    xb28_25_ = true;
    x2e0_destPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    SetPathFindMode(EPathFindMode::Normal);
    CPatterned::PathFind(mgr, msg, dt);
    SetCollisionActorExtendedTouchBounds(mgr, zeus::CVector3f{2.f});
    x95c_ = 0.f;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      x95c_ += dt;
      if (x95c_ >= x56c_sheegothData.Get_x1d8()) {
        x568_ = 4;
      } else if (!xb28_29_) {
        x568_ = 3;
        xb28_29_ = true;
        const bool isPlayerMorphed =
            mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed;
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCMeleeAttackCmd{isPlayerMorphed ? pas::ESeverity::Two : pas::ESeverity::One});
      } else if (xb28_29_) {
        const zeus::CTransform mouthXf = GetLctrTransform(xaf4_mouthLocator);
        if (GetTransform().frontVector().dot(mgr.GetPlayer().GetTranslation() - mouthXf.origin) > 0.f) {
          SetPathFindMode(EPathFindMode::Normal);
          if (GetSearchPath() != nullptr && !PathShagged(mgr, 0.f)) {
            CPatterned::PathFind(mgr, EStateMsg::Update, dt);
          } else {
            x568_ = 4;
          }
        } else {
          x568_ = 4;
        }
      }
    } else if (x568_ == 3 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
      x568_ = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (!sub_801a1794(mgr)) {
      x954_attackTimeLeft = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
    }

    SetCollisionActorExtendedTouchBounds(mgr, zeus::skZero3f);
    xb28_29_ = false;
    xb28_25_ = false;
    x960_ = x56c_sheegothData.Get_x1dc();
    x95c_ = 0.f;
  }
}

void CIceSheegoth::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    xb28_29_ = true;
    xb28_28_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_ = 3;
      } else if (IsOnGround()) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd{pas::ESeverity::Zero});
      }
    } else if (x568_ == 3 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
      x568_ = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (!sub_801a1794(mgr) || !TooClose(mgr, dt)) {
      x958_ = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
    }
    xb28_29_ = false;
    xb28_28_ = false;
  }
}

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
    SetMouthVulnerability(mgr, true);
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
    SetMouthVulnerability(mgr, false);
    xb29_25_ = false;
  }
}

void CIceSheegoth::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    xb28_26_ = true;
    xb29_25_ = true;
    xb29_27_ = false;
    x968_interestTimer = 0.f;
    SetGillVulnerability(mgr, true);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCProjectileAttackCmd{pas::ESeverity::Two, mgr.GetPlayer().GetTranslation(), false});
      } else {
        x568_ = 3;
        x3b4_speed = 2.f * x94c_;
      }
    } else if (x568_ == 3 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
      x568_ = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x3b4_speed = x94c_;
    if (!sub_801a1794(mgr)) {
      x954_attackTimeLeft = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
    }

    if (CFlameThrower* fl = static_cast<CFlameThrower*>(mgr.ObjectById(xa80_flameThrowerId))) {
      fl->Reset(mgr, false);
    }
    SetGillVulnerability(mgr, false);
    if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd{EBodyStateCmd::NextState});
    }
    xb29_25_ = false;
    xb28_26_ = false;
  }
}

void CIceSheegoth::Flinch(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    xb29_25_ = true;
    SetGillVulnerability(mgr, true);
    SetMouthVulnerability(mgr, true);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::KnockBack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd{x980_, pas::ESeverity::One});
      } else {
        x568_ = 4;
      }
    } else if (x568_ == 3 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::KnockBack) {
      x568_ = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    SetGillVulnerability(mgr, true);
    SetMouthVulnerability(mgr, true);
    xb29_25_ = false;
  }
}

void CIceSheegoth::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x9f4_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(EPathFindMode::Normal);
    if (!xb29_27_) {
      GetBodyController()->GetCommandMgr().ClearLocomotionCmds();
    }
    xb29_24_ = !xb29_24_;
    xb29_28_ = true;
    x92c_lastDest = mgr.GetPlayer().GetTranslation();
  } else if (msg == EStateMsg::Update) {
    SetPathFindMode(EPathFindMode::Approach);
    if (xb29_24_) {
      x2e0_destPos = x92c_lastDest;
      if (x844_approachSearch.FindClosestReachablePoint(GetTranslation(), x2e0_destPos) ==
          CPathFindSearch::EResult::Success) {
        if ((GetTranslation() - x2e0_destPos).magSquared() < 81.f) {
          x2e0_destPos = GetTranslation();
        }
        x92c_lastDest = x2e0_destPos;
        CPatterned::PathFind(mgr, EStateMsg::Activate, dt);
        if (!xb29_27_) {
          GetBodyController()->GetCommandMgr().ClearLocomotionCmds();
        }
      }
      xb29_24_ = false;
    }

    if (x56c_sheegothData.Get_x1f0_24() && GetSearchPath() != nullptr && !PathShagged(mgr, 0.f) &&
        x760_pathSearch.GetCurrentWaypoint() < x760_pathSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, EStateMsg::Update, dt);
    } else {
      const zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - GetTranslation();
      if (sub_8019ecdc(mgr, zeus::degToRad(15.f)) && posDiff.canBeNormalized()) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{zeus::skZero3f, posDiff.normalized(), 1.f});
      }
    }

    xb29_27_ = true;
    x3b4_speed =
        (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Turn ? 2.f : 1.f) * x94c_;
  } else if (msg == EStateMsg::Deactivate) {
    x9f4_boneTracking.SetActive(false);
    SetPathFindMode(EPathFindMode::Normal);
    x3b4_speed = x94c_;
  }
}

void CIceSheegoth::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    xb28_30_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Three));
      } else {
        x568_ = 3;
      }
    } else if (x568_ == 3 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate) {
      x568_ = 4;
    }
  }
}

void CIceSheegoth::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x968_interestTimer = 0.f;
    xb28_27_ = true;
    xb29_27_ = false;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::LoopAttack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLoopAttackCmd{pas::ELoopAttackType::Zero, true});
      } else {
        x568_ = 4;
        x3b4_speed = 2.f * x94c_;
      }
    } else if (x568_ == 3 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::LoopAttack) {
      x568_ = 4;
    }
  }
}

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
  return xb28_24_shotAt || CPatterned::SpotPlayer(mgr, arg);
}

bool CIceSheegoth::AnimOver(CStateManager& mgr, float arg) { return x568_ == 4; }

bool CIceSheegoth::ShouldAttack(CStateManager& mgr, float arg) {
  if (GetAreaIdAlways() == mgr.GetPlayer().GetAreaId() && x954_attackTimeLeft <= 0.f &&
      x974_ >= 0.3333f * x56c_sheegothData.Get_x170() && sub_8019ecbc()) {
    const zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    const zeus::CTransform lctrXf = GetLctrTransform(xaf4_mouthLocator);
    if ((aimPos - lctrXf.origin).magSquared() > x2fc_minAttackRange * x2fc_minAttackRange &&
        !ShouldTurn(mgr, zeus::degToRad(15.f))) {
      return !IsPatternObstructed(mgr, lctrXf.origin, aimPos);
    }
  }

  return false;
}

bool CIceSheegoth::ShouldDoubleSnap(CStateManager& mgr, float arg) {
  if (GetAreaIdAlways() != mgr.GetPlayer().GetAreaIdAlways() || x958_ > 0.f) {
    return false;
  }
  SetPathFindMode(EPathFindMode::Approach);
  return (!PathShagged(mgr, 0.f) && x760_pathSearch.GetCurrentWaypoint() < x760_pathSearch.GetWaypoints().size() - 1) ||
         (x92c_lastDest - GetTranslation()).magSquared() < 81.f;
}

bool CIceSheegoth::InPosition(CStateManager& mgr, float arg) {
  return (x92c_lastDest - GetTranslation()).magSquared() < 81.f;
}

bool CIceSheegoth::ShouldTurn(CStateManager& mgr, float arg) {
  if (arg == 0.f) {
    arg = zeus::degToRad(45.f);
  }
  return zeus::CVector2f::getAngleDiff(GetTransform().frontVector().toVec2f(),
                                       mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f()) > arg;
}

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
        !ShouldTurn(mgr, zeus::degToRad(15.f))) {
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
    bool bite = (xb28_29_ && !xb28_25_ && xb28_28_) ? true : IsMouthCollider(sender);

    if (colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId()) {
      if (!bite) {
        if (x420_curDamageRemTime <= 0.f) {
          mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
        }
      } else {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), x56c_sheegothData.Get_x1b8(),
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
  xafc_gillColliders.clear();

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
        xaf6_iceShardsCollider = desc.GetCollisionActorId();
        colAct->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::None);
      } else if (desc.GetName() == "GillL_LCTR"sv || desc.GetName() == "GillR_LCTR"sv) {
        xafc_gillColliders.emplace_back(desc.GetCollisionActorId());
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
  x970_maxHp = thisHealth->GetHP();
  for (auto& id : xafc_gillColliders) {
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
void CIceSheegoth::AttractProjectiles(CStateManager& mgr) {
  if (!IsAlive())
    return;

  EntityList nearProjectiles;
  zeus::CAABox attractionBounds =
      zeus::CAABox{GetTranslation() - x56c_sheegothData.Get_x14(), GetTranslation() + x56c_sheegothData.Get_x14()};
  mgr.BuildNearList(nearProjectiles, attractionBounds, CMaterialFilter::MakeInclude({EMaterialTypes::Projectile}),
                    nullptr);

  if (nearProjectiles.empty())
    return;

  zeus::CVector3f attractionPos = GetEnergyAttractionPos(mgr);
  EntityList nearCharacters;
  mgr.BuildNearList(nearCharacters, attractionBounds, CMaterialFilter::MakeInclude({EMaterialTypes::Character}),
                    nullptr);

  for (TUniqueId projectileId : nearProjectiles) {
    if (TCastToPtr<CGameProjectile> proj = mgr.ObjectById(projectileId)) {
      if (!ShouldAttractProjectile(*proj, mgr))
        continue;

      const zeus::CVector3f projPos = (attractionPos - proj->GetTranslation()) - proj->GetPreviousPos();
      if (!projPos.canBeNormalized() || !IsClosestSheegoth(mgr, nearCharacters, projPos))
        continue;

      const float mag = (attractionPos - proj->GetTranslation()).magnitude();
      const zeus::CVector3f b =
          proj->GetTranslation() + (0.5f * mag) * (proj->GetTranslation() - proj->GetPreviousPos()).normalized();
      const zeus::CVector3f c = attractionPos + zeus::CVector3f{0.f, 0.f, 0.4f * (0.4f * mag)};
      const zeus::CVector3f point1 = zeus::getBezierPoint(proj->GetTranslation(), b, c, attractionPos, 0.333f);
      const zeus::CVector3f point2 = zeus::getBezierPoint(proj->GetTranslation(), b, c, attractionPos, 0.666f);
      const float t = (point2 - point1).magnitude() + (point1 - proj->GetTranslation()).magnitude() +
                      (attractionPos - point2).magnitude();
      const zeus::CVector3f point3 =
          zeus::getBezierPoint(proj->GetTranslation(), b, c, attractionPos,
                               t / (proj->GetTranslation() - proj->GetPreviousPos()).magnitude());
      zeus::CVector3f lookPos = point3 - proj->GetTranslation();
      if (!lookPos.canBeNormalized()) {
        return;
      }

      zeus::CTransform xf = zeus::lookAt(zeus::skZero3f, lookPos);
      xf.orthonormalize();
      proj->ProjectileWeapon().SetWorldSpaceOrientation(xf);
      zeus::CVector3f velocity = 0.8f * proj->GetProjectileWeapon().GetVelocity().normalized();
      proj->ProjectileWeapon().SetVelocity(proj->ProjectileWeapon().GetVelocity() * (0.3999f + (velocity * 0.6f)));
    }
  }
}

void CIceSheegoth::UpdateTimers(float dt) {
  if (x954_attackTimeLeft > 0.f) {
    x954_attackTimeLeft -= (xb29_27_ ? 2.f : 1.f) * dt;
  }
  if (x960_ > 0.f) {
    x960_ -= dt;
  }
  if (x97c_ > 0.f) {
    x97c_ -= dt;
  }
  if (x958_ > 0.f) {
    x958_ -= dt;
  }
  if (x968_interestTimer < x56c_sheegothData.GetMaxInterestTime()) {
    x968_interestTimer += dt;
  }
}

void CIceSheegoth::UpdateScanState(CStateManager& mgr) {
  if (!xb29_29_scanned && GetScannableObjectInfo() != nullptr &&
      zeus::close_enough(1.f, mgr.GetPlayerState()->GetScanTime(GetScannableObjectInfo()->GetScannableObjectId()))) {
    xb29_29_scanned = true;
  }
}

void CIceSheegoth::SetPassthroughVulnerability(CStateManager& mgr) {
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

void CIceSheegoth::UpdateHealthInfo(CStateManager& mgr) {
  if (!IsAlive()) {
    return;
  }

  float hpDelta = 0.f;
  if (const TCastToConstPtr<CCollisionActor> colAct = mgr.ObjectById(xaf8_mouthCollider)) {
    hpDelta = std::max(hpDelta, x970_maxHp - colAct->GetHealthInfo(mgr)->GetHP());
  }

  for (const auto& uid : xafc_gillColliders) {
    if (const TCastToConstPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      hpDelta = std::max(hpDelta, x970_maxHp - colAct->GetHealthInfo(mgr)->GetHP());
    }
  }

  for (const auto& uid : xb04_) {
    if (const TCastToConstPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      hpDelta = std::max(hpDelta, x970_maxHp - colAct->GetHealthInfo(mgr)->GetHP());
    }
  }
  HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - hpDelta);
  if (GetHealthInfo(mgr)->GetHP() > 0.f) {
    if (const TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(xaf8_mouthCollider)) {
      colAct->HealthInfo(mgr)->SetHP(x970_maxHp);
    }
    for (const auto& uid : xafc_gillColliders) {
      if (const TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
        colAct->HealthInfo(mgr)->SetHP(x970_maxHp);
      }
    }

    for (const auto& uid : xb04_) {
      if (const TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
        colAct->HealthInfo(mgr)->SetHP(x970_maxHp);
      }
    }
  } else {
    Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
  }
}

void CIceSheegoth::SetSteeringSpeed(float dt, CStateManager& mgr) {
  const float steeringSpeedRange = x95c_ > 0.f ? 1.f : x944_;

  if (steeringSpeedRange > x948_) {
    x948_ = std::max(steeringSpeedRange, x948_ - (2.f * dt));
  } else if (steeringSpeedRange < x948_) {
    x948_ = std::min(steeringSpeedRange, x948_ + (2.f * dt));
  }

  GetBodyController()->GetCommandMgr().SetSteeringSpeedRange(x948_, x948_);
}

void CIceSheegoth::UpdateParticleEffects(float dt, CStateManager& mgr) {
  if (auto* fl = static_cast<CFlameThrower*>(mgr.ObjectById(xa80_flameThrowerId))) {
    if (fl->GetActive()) {
      fl->SetTransform(GetLctrTransform("LCTR_SHEMOUTH"sv), dt);
    }
  }

  if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(xaf6_iceShardsCollider)) {
    const float time = 0.333f * x56c_sheegothData.Get_x170();
    if (x974_ >= 3.f * time) {
      xa9c_->SetParticleEmission(false);
      xab0_->SetParticleEmission(false);
      xac4_->SetParticleEmission(true);
      xad8_->SetParticleEmission(true);
    } else if (x974_ >= 2.f * time) {
      xa9c_->SetParticleEmission(false);
      xab0_->SetParticleEmission(true);
      xac4_->SetParticleEmission(false);
      xad8_->SetParticleEmission(false);
    } else {
      xa9c_->SetParticleEmission(x974_ > 0.f);
      xab0_->SetParticleEmission(false);
      xac4_->SetParticleEmission(false);
      xad8_->SetParticleEmission(false);
    }

    const zeus::CTransform rotation = GetTransform().getRotation();
    const zeus::CVector3f pos = colAct->GetTranslation();
    xa9c_->SetOrientation(rotation);
    xa9c_->SetGlobalTranslation(pos);
    xab0_->SetOrientation(rotation);
    xab0_->SetGlobalTranslation(pos);
    xac4_->SetOrientation(rotation);
    xac4_->SetGlobalTranslation(pos);
    xad8_->SetOrientation(rotation);
    xad8_->SetGlobalTranslation(pos);
    if (xaec_) {
      xaec_->SetParticleEmission(true);
      xaec_->SetOrientation(rotation);
      xaec_->SetGlobalTranslation(pos);
      xaec_->SetGlobalScale(GetModelData()->GetScale());
      xaec_->Update(dt);
    }
    xa9c_->Update(dt);
    xab0_->Update(dt);
    xac4_->Update(dt);
    xad8_->Update(dt);
    if (x974_ < 2.f * time && xaf0_crackleSfx) {
      CSfxManager::RemoveEmitter(xaf0_crackleSfx);
    } else {
      xaf0_crackleSfx = CSfxManager::AddEmitter(x56c_sheegothData.Get_x1d4(), GetTranslation(), zeus::skZero3f, false,
                                                true, 127, kInvalidAreaId);
    }
  } else {
    xa9c_->SetParticleEmission(false);
    xab0_->SetParticleEmission(false);
    xac4_->SetParticleEmission(false);
    xad8_->SetParticleEmission(false);
  }
}

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

void CIceSheegoth::SetMouthVulnerability(CStateManager& mgr, bool isVulnerable) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(xaf8_mouthCollider)) {
    colAct->SetDamageVulnerability(isVulnerable ? x56c_sheegothData.Get_xe8() : x98c_mouthVulnerability);
  }
}

void CIceSheegoth::SetGillVulnerability(CStateManager& mgr, bool isVulnerable) {
  for (TUniqueId uid : xafc_gillColliders) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      colAct->SetDamageVulnerability(isVulnerable ? x56c_sheegothData.Get_x18()
                                                  : CDamageVulnerability::ImmuneVulnerabilty());
    }
  }
}

void CIceSheegoth::SetCollisionActorExtendedTouchBounds(CStateManager& mgr, const zeus::CVector3f& extents) {
  for (size_t i = 0; i < xa2c_collisionManager->GetNumCollisionActors(); ++i) {
    if (TCastToPtr<CCollisionActor> colAct =
            mgr.ObjectById(xa2c_collisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId())) {
      colAct->SetExtendedTouchBounds(extents);
    }
  }
}

void CIceSheegoth::ShakePlayer(CStateManager& mgr) {
  const zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - GetTranslation();
  const float magnitude = (0.5f - (0.01f * posDiff.magnitude()));
  if (magnitude > 0.f) {
    if (mgr.GetPlayer().GetSurfaceRestraint() != CPlayer::ESurfaceRestraints::Air &&
        !mgr.GetPlayer().IsInWaterMovement()) {
      bool applyImpulseToPlayer = true;
      if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
        if (mgr.GetCameraManager()->GetCurrentCameraId() ==
            mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()) {
          mgr.GetCameraManager()->AddCameraShaker(
              CCameraShakeData::BuildPatternedExplodeShakeData(GetTranslation(), 0.5f, magnitude, 50.f), true);
        }
        applyImpulseToPlayer = xb28_27_;
      }

      if (applyImpulseToPlayer) {
        const zeus::CVector3f direction = magnitude * ((xb28_27_ ? 40.f : 25.f) * zeus::skUp);
        zeus::CVector3f impulse = zeus::skZero3f;
        if (x978_ < posDiff.magnitude() && xb28_27_ && posDiff.toVec2f().canBeNormalized()) {
          impulse = magnitude * (12.5f * posDiff.toVec2f().normalized());
        }
        mgr.GetPlayer().ApplyImpulseWR(mgr.GetPlayer().GetMass() * (direction + impulse), zeus::CAxisAngle{});
        mgr.GetPlayer().SetMoveState(CPlayer::EPlayerMovementState::ApplyJump, mgr);
      }
    }
  }

  if (xb28_28_) {
    sub_8019ebf0(mgr, 0.25f * x56c_sheegothData.Get_x170());
  }
}

zeus::CVector3f CIceSheegoth::GetEnergyAttractionPos(CStateManager& mgr) const {
  if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(xaf6_iceShardsCollider)) {
    return colAct->GetTranslation();
  }

  return GetTranslation();
}

bool CIceSheegoth::ShouldAttractProjectile(const CGameProjectile& proj, CStateManager& mgr) const {
  if (proj.GetType() != EWeaponType::Missile && proj.GetType() != EWeaponType::Plasma &&
      (!proj.GetDamageInfo().GetWeaponMode().IsComboed() || proj.GetType() != EWeaponType::Power)) {
    const CActor* ent = static_cast<const CActor*>(mgr.GetObjectById(proj.GetOwnerId()));
    if (ent != nullptr) {
      if (CPatterned::CastTo<CIceSheegoth>(ent) == nullptr && ent->GetAreaIdAlways() == GetAreaIdAlways()) {
        zeus::CVector3f r = GetTransform().rotate(x56c_sheegothData.Get_x8());
        zeus::CVector3f posDiff = proj.GetTranslation() - ent->GetTranslation();
        if (proj.GetType() == EWeaponType::Wave && !proj.GetDamageInfo().GetWeaponMode().IsCharged() &&
            !proj.GetDamageInfo().GetWeaponMode().IsComboed() && posDiff.magSquared() < 100.f) {
          return false;
        }

        zeus::CVector3f xyPos = posDiff.toVec2f() - GetTranslation().toVec2f() + r.toVec2f();
        if (zeus::CVector3f::getAngleDiff(GetTransform().frontVector(), xyPos) <= x56c_sheegothData.Get_x4()) {
          if (zeus::CVector3f::getAngleDiff(GetTransform().frontVector(), -xyPos) >= x56c_sheegothData.Get_x0()) {
            return false;
          }
        }
        return true;
      }
    }
  }

  return false;
}

bool CIceSheegoth::IsClosestSheegoth(CStateManager& mgr, const EntityList& nearList,
                                     const zeus::CVector3f& projectileOffset) const {
  zeus::CVector3f diff = projectileOffset - GetTranslation();
  const float diffMag = diff.magSquared();
  for (const auto& uid : nearList) {
    const CIceSheegoth* goth = CPatterned::CastTo<CIceSheegoth>(mgr.GetObjectById(uid));
    if (!goth || goth->GetUniqueId() == GetUniqueId())
      continue;

    diff = projectileOffset - goth->GetTranslation();
    if (diff.magSquared() < diffMag) {
      return false;
    }
  }
  return true;
}
} // namespace metaforce::MP1
