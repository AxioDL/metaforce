#include "Runtime/MP1/World/CPuddleToadGamma.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Weapon/CBomb.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
constexpr zeus::CVector3f skBellyOffset(0.f, 0.1f, -.3f);

constexpr std::string_view skMouthLocatorName = "MOUTH_LCTR_SDK"sv;
constexpr std::string_view skBellyLocatorName = "SAMUS_POS_LCTR_SDK"sv;

constexpr CMaterialFilter skSolidFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Character, EMaterialTypes::Player,
                                                                  EMaterialTypes::ProjectilePassthrough});
} // Anonymous namespace

CPuddleToadGamma::CPuddleToadGamma(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                                   const CActorParameters& aParms, float suckForceMultiplier, float suckAngle,
                                   float playerSuckRange, const zeus::CVector3f& localShootDir, float playerShootSpeed,
                                   float shouldAttackWaitTime, float spotPlayerWaitTime,
                                   const CDamageInfo& playerShootDamage, const CDamageInfo& dInfo2, CAssetId dcln)
: CPatterned(ECharacter::PuddleToad, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::Zero, EBodyType::Restricted, aParms, EKnockBackVariant::Large)
, x570_playerShootDamage(playerShootDamage)
, x58c_(dInfo2)
, x5a8_suckForceMultiplier(suckForceMultiplier)
, x5ac_minSuckAngleProj(std::cos(zeus::degToRad(suckAngle * 0.5f)))
, x5b0_playerSuckRange(playerSuckRange)
, x5b4_localShootDir(localShootDir)
, x5c0_playerShootSpeed(playerShootSpeed)
, x5c4_shouldAttackWaitTime(shouldAttackWaitTime)
, x5c8_spotPlayerWaitTime(spotPlayerWaitTime) {
  x401_26_disableMove = true;
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x460_knockBackController.SetEnableShock(false);
  x460_knockBackController.SetX81_31(false);
  SetMovable(false);
  if (dcln.IsValid() && g_ResFactory->GetResourceTypeById(dcln) != 0) {
    TLockedToken<CCollidableOBBTreeGroupContainer> container = g_SimplePool->GetObj({FOURCC('DCLN'), dcln});
    x5e4_collisionTreePrim = std::make_unique<CCollidableOBBTreeGroup>(container.GetObj(), GetMaterialList());
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
    const zeus::CTransform bellyXf = GetLctrTransform(skBellyLocatorName);
    const zeus::CVector3f bellyOffset = GetTransform().rotate(skBellyOffset);
    x5d8_damageablePoint = x5cc_suckPoint = bellyXf.origin + bellyOffset;
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    AddMaterial(EMaterialTypes::Immovable, mgr);
    AddMaterial(EMaterialTypes::SolidCharacter);
  }
}

void CPuddleToadGamma::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (x5e8_25_waitTimerActive)
    x56c_waitTimer += dt;
}

std::optional<zeus::CAABox> CPuddleToadGamma::GetTouchBounds() const {
  if (!GetActive())
    return {};

  return (x5e4_collisionTreePrim ? x5e4_collisionTreePrim->CalculateAABox(GetTransform()) : GetBoundingBox());
}

void CPuddleToadGamma::CenterPlayer(CStateManager& mgr, const zeus::CVector3f& pos, float dt) {
  zeus::CVector3f dir = (pos - mgr.GetPlayer().GetTranslation()).normalized();
  mgr.GetPlayer().SetVelocityWR((1.f / (2.f * dt)) * dir);
}

const CDamageVulnerability* CPuddleToadGamma::GetDamageVulnerability(const zeus::CVector3f& pos,
                                                                     const zeus::CVector3f& dir,
                                                                     const CDamageInfo& dInfo) const {
  if (x5e8_24_playerInside && (x5d8_damageablePoint - pos).magSquared() < 4.f)
    return CAi::GetDamageVulnerability();

  return &CDamageVulnerability::ImmuneVulnerabilty();
}

void CPuddleToadGamma::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::Projectile) {
    ShootPlayer(mgr, x5c0_playerShootSpeed);
    return;
  }

  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

bool CPuddleToadGamma::SpotPlayer(CStateManager&, float arg) { return x56c_waitTimer >= x5c8_spotPlayerWaitTime; }

bool CPuddleToadGamma::ShouldAttack(CStateManager&, float) { return x56c_waitTimer >= x5c4_shouldAttackWaitTime; }

bool CPuddleToadGamma::LostInterest(CStateManager& mgr, float) {
  zeus::CAABox box = *GetTouchBounds();
  zeus::CAABox plBox = mgr.GetPlayer().GetBoundingBox();
  return !box.intersects(plBox);
}

void CPuddleToadGamma::ShootPlayer(CStateManager& mgr, float speed) {
  zeus::CVector3f shootDir = x34_transform.rotate(x5b4_localShootDir.normalized());
  mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
    x5e8_26_shotPlayer = true;
    mgr.GetPlayer().Stop();
    mgr.GetPlayer().SetVelocityWR(zeus::skZero3f);
    mgr.GetPlayer().ApplyImpulseWR(mgr.GetPlayer().GetMass() * shootDir * speed, {});
    mgr.GetPlayer().SetMoveState(CPlayer::EPlayerMovementState::ApplyJump, mgr);
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), x570_playerShootDamage,
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
    mgr.GetPlayer().GetMorphBall()->SetAsProjectile();
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, GetBoundingBox(), CMaterialFilter::MakeInclude({EMaterialTypes::Bomb}), this);
    for (TUniqueId id : nearList) {
      if (TCastToPtr<CBomb> bomb = mgr.ObjectById(id)) {
        bomb->SetVelocityWR((mgr.GetActiveRandom()->Float() * 5.f + 20.f) * shootDir);
        bomb->SetConstantAccelerationWR({0.f, 0.f, -CPhysicsActor::GravityConstant()});
      }
    }
  }
}

bool CPuddleToadGamma::InAttackPosition(CStateManager& mgr, float) {
  return mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed &&
         PlayerInVortexArea(mgr);
}

bool CPuddleToadGamma::PlayerInVortexArea(const CStateManager& mgr) const {
  const CPlayer& player = mgr.GetPlayer();
  const zeus::CTransform xf = GetLctrTransform(skMouthLocatorName);

  const zeus::CVector3f playerOffset =
      player.GetTranslation() + zeus::CVector3f{0.f, 0.f, player.GetMorphBall()->GetBallRadius()};
  const zeus::CVector3f rotatedDir = GetTransform().rotate(zeus::skForward);

  const zeus::CVector3f suckPointToPlayer = (playerOffset - (xf.origin - (1.f * rotatedDir)));
  const float suckProj = suckPointToPlayer.normalized().dot(rotatedDir);
  const float playerDist = suckPointToPlayer.magnitude();
  const float suckAngleProj = (player.GetTranslation() - (xf.origin - (4.f * rotatedDir))).normalized().dot(rotatedDir);
  if (playerDist > 2.f) {
    const CRayCastResult result =
        mgr.RayStaticIntersection(suckPointToPlayer, 1.f / playerDist * suckPointToPlayer,
            playerDist - player.GetMorphBall()->GetBallRadius(), skSolidFilter);
    if (result.IsValid())
      return false;
  }

  return (playerDist < x5b0_playerSuckRange && suckProj > 0.f && suckAngleProj > x5ac_minSuckAngleProj);
}

void CPuddleToadGamma::InActive(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    SetSolid(mgr, true);
    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
    x330_stateMachineState.SetDelay(2.f);
  }
}

void CPuddleToadGamma::Active(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    const zeus::CTransform xf = GetLctrTransform(skBellyLocatorName);
    x5cc_suckPoint = xf.origin + GetTransform().rotate(skBellyOffset);
    x56c_waitTimer = 0.f;
    x5e8_25_waitTimerActive = true;
    SetSolid(mgr, true);
    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
  } else if (msg == EStateMsg::Deactivate) {
    x5e8_25_waitTimerActive = false;
  }
}

void CPuddleToadGamma::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    SetSolid(mgr, false);
    mgr.GetPlayer().SetLeaveMorphBallAllowed(false);
    mgr.GetPlayer().GetMorphBall()->DisableHalfPipeStatus();
  } else if (msg == EStateMsg::Update) {
    if (x568_stateProg == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LoopReaction) {
        x568_stateProg = 1;
        return;
      }

      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::EReactionType::Zero));
    } else if (x568_stateProg == 1)
      SuckPlayer(mgr, arg);
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
  }
}

void CPuddleToadGamma::SuckPlayer(CStateManager& mgr, float arg) {
  CPlayer& player = mgr.GetPlayer();
  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    return;

  zeus::CVector3f posDiff = player.GetTranslation() - x5cc_suckPoint;
  float posMag = posDiff.magnitude();
  if (posMag < 3.f) {
    player.Stop();
    CenterPlayer(mgr, x5cc_suckPoint, arg);
    return;
  }

  float forceMag = x5a8_suckForceMultiplier * (x5b0_playerSuckRange / (posMag * posMag));
  zeus::CVector3f force = forceMag * (player.GetMass() * -posDiff);
  player.ApplyForceWR(force, zeus::CAxisAngle());
}

void CPuddleToadGamma::Attack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    mgr.GetPlayer().Stop();
    mgr.GetPlayer().SetVelocityWR({});
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One));
    x5e8_26_shotPlayer = false;
    mgr.GetPlayer().GetMorphBall()->SetBombJumpState(CMorphBall::EBombJumpState::BombJumpDisabled);
  } else if (msg == EStateMsg::Update) {
    if (!x5e8_26_shotPlayer) {
      const zeus::CTransform xf = GetLctrTransform(skBellyLocatorName);
      x5cc_suckPoint = xf.origin + GetTransform().rotate(skBellyOffset);
      SetPlayerPosition(mgr, x5cc_suckPoint);
    } else if (LostInterest(mgr, 0.f))
      SetSolid(mgr, true);
  } else if (msg == EStateMsg::Deactivate) {
    SetSolid(mgr, true);
    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
    mgr.GetPlayer().GetMorphBall()->SetBombJumpState(CMorphBall::EBombJumpState::BombJumpAvailable);
    x5e8_24_playerInside = false;
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

  zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - x5cc_suckPoint;
  return posDiff.dot(GetTransform().frontVector()) <= 0.f && posDiff.magSquared() < 2.f;
}

void CPuddleToadGamma::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_stateProg = 0;
    x56c_waitTimer = 0.f;
    x5e8_25_waitTimerActive = true;
    x5e8_24_playerInside = true;
    mgr.GetPlayer().Stop();
    mgr.GetPlayer().SetVelocityWR({});
    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
    if (!mgr.GetPlayer().AttachActorToPlayer(GetUniqueId(), false))
      x56c_waitTimer = 100.f;

    SetSolid(mgr, false);
    mgr.GetPlayer().SetLeaveMorphBallAllowed(false);
    mgr.GetPlayer().GetMorphBall()->DisableHalfPipeStatus();
    SetSolid(mgr, false);
  } else if (msg == EStateMsg::Update) {
    const zeus::CTransform xf = GetLctrTransform(skBellyLocatorName);
    x5cc_suckPoint = xf.origin + GetTransform().rotate(skBellyOffset);
    SetPlayerPosition(mgr, x5cc_suckPoint);
    if (x568_stateProg == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Locomotion)
        x568_stateProg = 1;
      else
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (mgr.GetPlayer().GetAttachedActor() == GetUniqueId())
      mgr.GetPlayer().DetachActorFromPlayer();
    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
    x5e8_25_waitTimerActive = false;
  }
}

} // namespace urde::MP1
