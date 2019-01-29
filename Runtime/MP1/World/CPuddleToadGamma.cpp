#include "CPuddleToadGamma.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "Collision/CGameCollision.hpp"
#include "TCastTo.hpp"

namespace urde::MP1 {

const zeus::CVector3f CPuddleToadGamma::skBellyOffset(0.f, 0.1f, -.3f);

CPuddleToadGamma::CPuddleToadGamma(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                                   const CActorParameters& aParms, float f1, float f2, float f3,
                                   const zeus::CVector3f& v1, float f4, float f5, float f6, const CDamageInfo& dInfo1,
                                   const CDamageInfo& dInfo2, CAssetId dcln)
: CPatterned(ECharacter::PuddleToad, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::Zero, EBodyType::Restricted, aParms, EKnockBackVariant::Large)
, x570_(dInfo1)
, x58c_(dInfo2)
, x5a8_(f1)
, x5ac_(std::cos(zeus::degToRad(f2 * 0.5f)))
, x5b0_(f3)
, x5b4_(v1)
, x5c0_(f4)
, x5c4_(f5)
, x5c8_(f6)
, x5e8_24_(false)
, x5e8_25_(false)
, x5e8_26_(false) {
  x401_26_disableMove = true;
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x460_knockBackController.SetEnableShock(false);
  x460_knockBackController.SetX81_31(false);
  SetMovable(false);
  if (dcln.IsValid() && g_ResFactory->GetResourceTypeById(dcln) != 0) {
    TLockedToken<CCollidableOBBTreeGroupContainer> container = g_SimplePool->GetObj({FOURCC('DCLN'), dcln});
    x5e4_collisionTreePrim.reset(new CCollidableOBBTreeGroup(container.GetObj(), GetMaterialList()));
  }
}

void CPuddleToadGamma::SetSolid(CStateManager& mgr, bool solid) {
  if (solid) {
    AddMaterial(EMaterialTypes::Solid, mgr);
    RemoveMaterial(EMaterialTypes::NonSolidDamageable, mgr);
  } else {
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    AddMaterial(EMaterialTypes::NonSolidDamageable, mgr);
  }
}

const CCollisionPrimitive* CPuddleToadGamma::GetCollisionPrimitive() const {
  if (!x5e4_collisionTreePrim)
    return CPhysicsActor::GetCollisionPrimitive();
  return x5e4_collisionTreePrim.get();
}

zeus::CTransform CPuddleToadGamma::GetPrimitiveTransform() const {
  zeus::CTransform xf = GetTransform();
  xf.origin += GetPrimitiveOffset();
  return xf;
}

void CPuddleToadGamma::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
    zeus::CTransform bellyXf = GetLctrTransform(mBellyLocatorName);
    zeus::CVector3f bellyOffset = GetTransform().rotate(skBellyOffset);
    x5d8_ = x5cc_ = bellyXf.origin + bellyOffset;
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    AddMaterial(EMaterialTypes::Immovable, mgr);
    AddMaterial(EMaterialTypes::SolidCharacter);
  }
}

void CPuddleToadGamma::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (x5e8_25_)
    x56c_ += dt;
}

rstl::optional<zeus::CAABox> CPuddleToadGamma::GetTouchBounds() const {
  if (!GetActive())
    return {};

  return (x5e4_collisionTreePrim ? x5e4_collisionTreePrim->CalculateAABox(GetTransform()) : GetBoundingBox());
}

void CPuddleToadGamma::CenterPlayer(CStateManager& mgr, const zeus::CVector3f& pos, float dt) {
  zeus::CVector3f dir = (mgr.GetPlayer().GetTranslation() - pos).normalized();
  mgr.GetPlayer().SetVelocityWR((1.f / (2.f * dt)) * dir);
}

const CDamageVulnerability* CPuddleToadGamma::GetDamageVulnerability(const zeus::CVector3f& pos,
                                                                     const zeus::CVector3f& dir,
                                                                     const CDamageInfo& dInfo) const {
  if (x5e8_24_ && (x5d8_ - pos).magSquared() < 4.f)
    return CAi::GetDamageVulnerability();

  return &CDamageVulnerability::ImmuneVulnerabilty();
}

void CPuddleToadGamma::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::Projectile) {
    ShootPlayer(mgr, x5c0_);
    return;
  }

  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

bool CPuddleToadGamma::SpotPlayer(CStateManager&, float arg) { return x56c_ >= x5c8_; }

bool CPuddleToadGamma::ShouldAttack(CStateManager&, float) { return x56c_ >= x5c4_; }

bool CPuddleToadGamma::LostInterest(CStateManager& mgr, float) {
  zeus::CAABox box = *GetTouchBounds();
  zeus::CAABox plBox = mgr.GetPlayer().GetBoundingBox();
  return !box.intersects(plBox);
}

void CPuddleToadGamma::ShootPlayer(CStateManager&, float) {}

bool CPuddleToadGamma::InAttackPosition(CStateManager& mgr, float) {
  return mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed &&
         PlayerInVortexArea(mgr);
}

static CMaterialFilter kSolidFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Character, EMaterialTypes::Player,
                                                                  EMaterialTypes::ProjectilePassthrough});

bool CPuddleToadGamma::PlayerInVortexArea(const CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  zeus::CTransform xf = GetLctrTransform(mMouthLocatorName);

  zeus::CVector3f playerOffset =
      player.GetTranslation() + zeus::CVector3f{0.f, 0.f, player.GetMorphBall()->GetBallRadius()};
  zeus::CVector3f rotatedOffset = GetTransform().rotate(zeus::CVector3f::skForward);

  zeus::CVector3f vec1 = (playerOffset - (xf.origin - (1.f * rotatedOffset)));
  float f31 = vec1.normalized().dot(rotatedOffset);
  float f28 = vec1.magnitude();
  float f26 = (player.GetTranslation() - (xf.origin - (4.f * rotatedOffset))).normalized().dot(rotatedOffset);
  if (f28 > 2.f) {
    CRayCastResult result =
        mgr.RayStaticIntersection(vec1, 1.f / f28 * vec1, f28 - player.GetMorphBall()->GetBallRadius(), kSolidFilter);
    if (result.IsValid())
      return false;
  }

  return (f28 < x5b0_ && f31 > 0.f && f26 > x5ac_);
}

void CPuddleToadGamma::InActive(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    SetSolid(mgr, true);
    mgr.GetPlayer().Set_X590(true);
    x330_stateMachineState.SetDelay(2.f);
  }
}

void CPuddleToadGamma::Active(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    zeus::CTransform xf = GetLctrTransform(mBellyLocatorName);
    x5cc_ = xf.origin + GetTransform().rotate(skBellyOffset);
    x56c_ = 0.f;
    x5e8_25_ = true;
    SetSolid(mgr, true);
    mgr.GetPlayer().Set_X590(true);
  } else if (msg == EStateMsg::Deactivate) {
    x5e8_25_ = false;
  }
}

void CPuddleToadGamma::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  return;
  if (msg == EStateMsg::Activate) {
    SetSolid(mgr, false);
    mgr.GetPlayer().Set_X590(false);
    mgr.GetPlayer().GetMorphBall()->DisableHalfPipeStatus();
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LoopReaction) {
        x568_ = 1;
        return;
      }

      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::EReactionType::Zero));
    } else if (x568_ == 1)
      SuckPlayer(mgr, arg);
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
  }
}

void CPuddleToadGamma::SuckPlayer(CStateManager& mgr, float arg) {
  CPlayer& player = mgr.GetPlayer();
  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    return;

  zeus::CVector3f posDiff = player.GetTranslation() - x5cc_;
  if (posDiff.magnitude() < 3.f) {
    player.Stop();
    CenterPlayer(mgr, x5cc_, arg);
    return;
  }

  float d = x5a8_ * (x5b0_ / (posDiff.magnitude() * posDiff.magnitude()));
  zeus::CVector3f force = d * (player.GetMass() * -posDiff);
  player.ApplyForceWR(force, zeus::CAxisAngle::sIdentity);
}

void CPuddleToadGamma::Attack(CStateManager& mgr, EStateMsg msg, float) {
  return;
  if (msg == EStateMsg::Activate) {
    mgr.GetPlayer().Stop();
    mgr.GetPlayer().SetVelocityWR({});
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One));
    x5e8_26_ = false;
    mgr.GetPlayer().GetMorphBall()->SetBombJumpState(CMorphBall::EBombJumpState::BombJumpDisabled);
  } else if (msg == EStateMsg::Update) {
    if (!x5e8_26_) {
      zeus::CTransform xf = GetLctrTransform(mBellyLocatorName);
      x5cc_ = xf.origin + GetTransform().rotate(skBellyOffset);
      SetPlayerPosition(mgr, x5cc_);
    } else if (LostInterest(mgr, 0.f))
      SetSolid(mgr, true);
  } else if (msg == EStateMsg::Deactivate) {
    SetSolid(mgr, true);
    mgr.GetPlayer().Set_X590(true);
    mgr.GetPlayer().GetMorphBall()->SetBombJumpState(CMorphBall::EBombJumpState::BombJumpAvailable);
    x5e8_24_ = false;
  }
}

void CPuddleToadGamma::SetPlayerPosition(CStateManager& mgr, const zeus::CVector3f& targetPos) {
  float preThinkDt = x500_preThinkDt;
  CPlayer& player = mgr.GetPlayer();
  player.Stop();
  player.SetVelocityWR({});
  bool hadPlayerMaterial = player.GetMaterialList().HasMaterial(EMaterialTypes::Player);

  if (hadPlayerMaterial)
    player.RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
  player.RemoveMaterial(EMaterialTypes::Player, mgr);

  bool hadSolidMaterial = GetMaterialList().HasMaterial(EMaterialTypes::Solid);
  if (hadSolidMaterial)
    RemoveMaterial(EMaterialTypes::Solid, mgr);

  CPhysicsState physState = player.GetPhysicsState();
  player.Stop();
  player.MoveToWR(targetPos, preThinkDt);
  CGameCollision::Move(mgr, player, preThinkDt, nullptr);
  physState.SetTranslation(player.GetTranslation());
  player.SetPhysicsState(physState);
  if (hadPlayerMaterial)
    player.AddMaterial(EMaterialTypes::GroundCollider, mgr);
  player.AddMaterial(EMaterialTypes::Player, mgr);
  if (hadSolidMaterial)
    AddMaterial(EMaterialTypes::Solid, mgr);
}

bool CPuddleToadGamma::Inside(CStateManager& mgr, float) {
  if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    return false;

  zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - x5cc_;
  return posDiff.dot(GetTransform().frontVector()) <= 0.f && posDiff.magSquared() < 2.f;
}

void CPuddleToadGamma::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  return;
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x56c_ = 0.f;
    x5e8_25_ = true;
    x5e8_24_ = true;
    mgr.GetPlayer().Stop();
    mgr.GetPlayer().SetVelocityWR({});
    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
    if (!mgr.GetPlayer().AttachActorToPlayer(GetUniqueId(), false))
      x56c_ = 100.f;

    SetSolid(mgr, false);
    mgr.GetPlayer().Set_X590(false);
    mgr.GetPlayer().GetMorphBall()->DisableHalfPipeStatus();
    SetSolid(mgr, false);
  } else if (msg == EStateMsg::Update) {
    zeus::CTransform xf = GetLctrTransform(mBellyLocatorName);
    x5cc_ = xf.origin + GetTransform().rotate(skBellyOffset);
    SetPlayerPosition(mgr, x5cc_);
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Locomotion)
        x568_ = 1;
      else
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (mgr.GetPlayer().GetAttachedActor() == GetUniqueId())
      mgr.GetPlayer().DetachActorFromPlayer();
    mgr.GetPlayer().Set_X590(true);
    x5e8_25_ = false;
  }
}

} // namespace urde::MP1
