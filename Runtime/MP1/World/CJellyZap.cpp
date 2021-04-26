#include "Runtime/MP1/World/CJellyZap.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CFishCloud.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce::MP1 {
constexpr CMaterialFilter kPlayerFilter = CMaterialFilter::MakeInclude({EMaterialTypes::Player});

CJellyZap::CJellyZap(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CDamageInfo& attackDamage, bool b1, float attackRadius, float f2,
                     float f3, float f4, float attackDelay, float f6, float f7, float f8, float priority,
                     float repulseRadius, float attractRadius, float f12, const CPatternedInfo& pInfo,
                     const CActorParameters& actParms)
: CPatterned(ECharacter::JellyZap, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Medium)
, x56c_attackDamage(attackDamage)
, x588_attackRadius(attackRadius)
, x58c_(f2)
, x590_(f4)
, x594_(f3)
, x598_(f8)
, x59c_priority(priority)
, x5a0_repulseRadius(repulseRadius)
, x5a4_attractRadius(attractRadius)
, x5a8_attackDelay(attackDelay)
, x5ac_(f6)
, x5b0_(f7)
, x5b4_(f12)
, x5b8_26_(b1) {
  UpdateThermalFrozenState(true);
  x50c_baseDamageMag = 0.f;
}

void CJellyZap::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CJellyZap::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
  } else if (msg == EScriptObjectMessage::Activate) {
    AddAttractor(mgr);
  } else if (msg == EScriptObjectMessage::Deleted || msg == EScriptObjectMessage::Deactivate) {
    RemoveAllAttractors(mgr);
  }
}

void CJellyZap::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  if (x5b8_24_) {
    x450_bodyController->FaceDirection(mgr.GetPlayer().GetTranslation() - GetTranslation(), dt);
  }

  float damage = x50c_baseDamageMag;

  if (x5b8_25_ || GetBodyController()->GetPercentageFrozen() != 0.f) {
    damage -= dt / 0.75f;
  } else {
    damage += dt / 0.3f;
  }
  x50c_baseDamageMag = zeus::clamp(0.f, damage, 1.f);
}

void CJellyZap::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::DamageOn) {
    mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), x56c_attackDamage, kPlayerFilter);
    return;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CJellyZap::KnockBack(const zeus::CVector3f& pos, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                          bool inDeferred, float magnitude) {
  if (info.GetWeaponMode().GetType() == EWeaponType::Ice) {
    Freeze(mgr, {}, GetTransform().transposeRotate(pos), x4fc_freezeDur);
  }
}

void CJellyZap::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    AddRepulsor(mgr);
    x5b8_25_ = true;
    float dist = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magnitude();
    if (dist < x56c_attackDamage.GetRadius()) {
      float staticTimer = 3.f * (1.f - dist / x56c_attackDamage.GetRadius()) + 2.f;
      if (staticTimer > mgr.GetPlayer().GetStaticTimer()) {
        mgr.GetPlayer().SetHudDisable(staticTimer, 0.5f, 2.5f);
        mgr.GetPlayer().SetOrbitRequestForTarget(mgr.GetPlayer().GetOrbitTargetId(),
                                                 CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
      }

      mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), 0.5f, 0.5f);
    }
    x330_stateMachineState.SetDelay(x5ac_);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 1);
  } else if (msg == EStateMsg::Deactivate) {
    RemoveAllAttractors(mgr);
    x32c_animState = EAnimState::NotReady;
    x5b8_25_ = false;
  }
}

void CJellyZap::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    RemoveAllAttractors(mgr);
    x568_ = 1;
    x400_24_hitByPlayerProjectile = false;
    x5b8_24_ = true;
    x5b8_25_ = true;
  } else if (msg == EStateMsg::Update) {
    auto curSuit = mgr.GetPlayerState()->GetCurrentSuit();
    TryCommand(mgr, pas::EAnimationState::LoopReaction, &CPatterned::TryLoopReaction, 0);
    x450_bodyController->GetCommandMgr().DeliverTargetVector(
        (mgr.GetPlayer().GetTranslation() + zeus::CVector3f(0.f, 0.f, 1.f)) - GetTranslation());

    float intensity = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit) ? 0.1f : 1.f;
    zeus::CVector3f posDiff = (mgr.GetPlayer().GetTranslation() - GetTranslation());
    float mag = 1.f / posDiff.magnitude();
    float massScale = mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ? x594_
                      : curSuit == CPlayerState::EPlayerSuit::Gravity                                          ? x590_
                                                                                                               : x58c_;
    mgr.GetPlayer().ApplyImpulseWR(
        arg * ((5.f * massScale * mgr.GetPlayer().GetMass()) * (intensity * (mag * -posDiff))), {});
    mgr.GetPlayer().UseCollisionImpulses();
    mgr.GetPlayer().SetAccelerationChangeTimer(2.f * arg);
    mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), 0.1f, 0.1f);
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
    mgr.GetPlayerState()->GetStaticInterference().RemoveSource(GetUniqueId());
    x32c_animState = EAnimState::NotReady;
    x5b8_24_ = false;
    x5b8_25_ = false;
  }
}

void CJellyZap::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5b8_24_ = true;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x568_ = 0;
    x330_stateMachineState.SetDelay(x3d0_playerLeashTime);
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f targetVector =
        GetTranslation() - (zeus::CVector3f(0.f, 0.f, 1.f) + mgr.GetPlayer().GetTranslation());
    x450_bodyController->GetCommandMgr().DeliverTargetVector(targetVector);
    if (x5b8_26_) {
      zeus::CVector3f moveToImpulse =
          GetMoveToORImpulseWR(GetTransform().transposeRotate(arg * (zeus::CVector3f(0.f, 1.f, 0.f) * x598_)), arg);
      ApplyImpulseOR(moveToImpulse, zeus::CAxisAngle());
    }
  } else if (msg == EStateMsg::Deactivate) {
    x5b8_24_ = false;
  }
}

void CJellyZap::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate) {
    return;
  }
  x400_24_hitByPlayerProjectile = false;
  x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
  AddAttractor(mgr);
  x568_ = 0;
}

void CJellyZap::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x400_24_hitByPlayerProjectile = false;
    x32c_animState = EAnimState::Ready;
    x330_stateMachineState.SetDelay(x5b0_);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::KnockBack, &CPatterned::TryKnockBack, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

bool CJellyZap::InAttackPosition(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetFluidCounter() == 0) {
    return false;
  }

  return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() < x588_attackRadius * x588_attackRadius;
}

bool CJellyZap::InDetectionRange(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetFluidCounter() != 0 ? CPatterned::InDetectionRange(mgr, arg) : false);
}

void CJellyZap::AddSelfToFishCloud(CStateManager& mgr, float radius, float priority, bool repulsor) {
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::ScanStart || conn.x4_msg != EScriptObjectMessage::Follow) {
      continue;
    }
    if (TCastToPtr<CFishCloud> cloud = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
      if (repulsor) {
        cloud->AddRepulsor(GetUniqueId(), false, radius, priority);
      } else {
        cloud->AddAttractor(GetUniqueId(), false, radius, priority);
      }
    }
  }
}

void CJellyZap::AddRepulsor(CStateManager& mgr) { AddSelfToFishCloud(mgr, x5a0_repulseRadius, x59c_priority, true); }

void CJellyZap::AddAttractor(CStateManager& mgr) {
  AddSelfToFishCloud(mgr, x5a0_repulseRadius, x59c_priority, true);
  AddSelfToFishCloud(mgr, x5a4_attractRadius, x59c_priority, false);
}

void CJellyZap::RemoveSelfFromFishCloud(CStateManager& mgr) {
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::ScanStart || conn.x4_msg != EScriptObjectMessage::Follow) {
      continue;
    }

    if (TCastToPtr<CFishCloud> cloud = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
      cloud->RemoveAttractor(GetUniqueId());
      cloud->RemoveRepulsor(GetUniqueId());
    }
  }
}

void CJellyZap::RemoveAllAttractors(CStateManager& mgr) { RemoveSelfFromFishCloud(mgr); }

bool CJellyZap::ClosestToPlayer(const CStateManager& mgr) const {
  const zeus::CVector3f playerPos = mgr.GetPlayer().GetTranslation();
  const float ourDistance = (playerPos - GetTranslation()).magnitude();
  float closestDistance = ourDistance;
  for (CEntity* ent : mgr.GetPhysicsActorObjectList()) {
    if (auto* zap = CPatterned::CastTo<CJellyZap>(ent)) {
      if (zap->GetAreaIdAlways() != GetAreaIdAlways() || zap == this) {
        continue;
      }

      const float tmpDist = (playerPos - zap->GetTranslation()).magnitude();
      if (tmpDist < closestDistance) {
        closestDistance = tmpDist;
      }

      if (zap->x5b8_25_) {
        return false;
      }
    }
  }
  return zeus::close_enough(closestDistance, ourDistance);
}
const CDamageVulnerability* CJellyZap::GetDamageVulnerability(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                              const CDamageInfo& info) const {
  if (!sub801d8190()) {
    return GetDamageVulnerability();
  }

  return &CDamageVulnerability::ReflectVulnerabilty();
}
} // namespace metaforce::MP1
