#include "CJellyZap.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "TCastTo.hpp"

namespace urde::MP1 {
const CMaterialFilter CJellyZap::kPlayerFilter = CMaterialFilter::MakeInclude({EMaterialTypes::Player});

CJellyZap::CJellyZap(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CDamageInfo& dInfo, bool b1, float f1, float f2, float f3, float f4,
                     float f5, float f6, float f7, float f8, float f9, float f10, float f11, float f12,
                     const CPatternedInfo& pInfo, const CActorParameters& actParms)
: CPatterned(ECharacter::JellyZap, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Medium)
, x56c_attackDamage(dInfo)
, x588_attackRadius(f1)
, x58c_(f2)
, x590_(f4)
, x594_(f3)
, x598_(f8)
, x59c_(f9)
, x5a0_(f10)
, x5a4_(f11)
, x5a8_attackDelay(f5)
, x5ac_(f6)
, x5b0_(f7)
, x5b4_(f12)
, x5b8_24_(false)
, x5b8_25_(false)
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
  if (!GetActive())
    return;
  if (x5b8_24_)
    x450_bodyController->FaceDirection(GetTranslation() - mgr.GetPlayer().GetTranslation(), dt);

  float fv = (x5b8_25_ && x450_bodyController->GetPercentageFrozen() == 0.f ? x50c_baseDamageMag + (dt / 0.3f)
                                                                            : x50c_baseDamageMag - (dt / 0.75f));
  x50c_baseDamageMag = zeus::clamp(0.f, fv, 1.f);
}

void CJellyZap::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::DamageOn) {
    mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), x56c_attackDamage, kPlayerFilter);
    return;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CJellyZap::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    AddRepulsor(mgr);
    x5b8_25_ = true;
    float dist = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared();
    if (dist < x56c_attackDamage.GetRadius()) {
      float staticTimer = 3.f * (1.f - (dist / x56c_attackDamage.GetRadius())) + 2.f;
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
  } else if (msg == EStateMsg::Update) {

  } else if (msg == EStateMsg::Deactivate) {

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
    x450_bodyController->GetCommandMgr().SetTargetVector(targetVector);
    if (x5b8_26_) {
      zeus::CVector3f moveToImpulse =
          GetMoveToORImpulseWR(GetTransform().transposeRotate(arg * (zeus::CVector3f(0.f, 1.f, 0.f) * x598_)), arg);
      ApplyImpulseOR(moveToImpulse, zeus::CAxisAngle::sIdentity);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x5b8_24_ = false;
  }
}

void CJellyZap::InActive(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x400_24_hitByPlayerProjectile = false;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    AddAttractor(mgr);
    x568_ = 0;
  }
}

void CJellyZap::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x400_24_hitByPlayerProjectile = false;
    x32c_animState = EAnimState::NotReady;
    x330_stateMachineState.SetDelay(x5b0_);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::KnockBack, &CPatterned::TryKnockBack, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

bool CJellyZap::InAttackPosition(CStateManager& mgr, float) {
  if (mgr.GetPlayer().GetFluidCounter() == 0)
    return false;

  return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magnitude() < x588_attackRadius * x588_attackRadius;
}

bool CJellyZap::InDetectionRange(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetFluidCounter() != 0 ? CPatterned::InDetectionRange(mgr, arg) : false);
}

void CJellyZap::AddSelfToFishCloud(CStateManager, float, bool b) {}

void CJellyZap::AddRepulsor(CStateManager&) {}

void CJellyZap::AddAttractor(CStateManager&) {}

void CJellyZap::RemoveSelfFromFishCloud(CStateManager&) {}

void CJellyZap::RemoveAllAttractors(CStateManager& mgr) { RemoveSelfFromFishCloud(mgr); }

bool CJellyZap::ClosestToPlayer(CStateManager& mgr) {
  zeus::CVector3f playerPos = mgr.GetPlayer().GetTranslation();
  float ourDistance = (playerPos - GetTranslation()).magnitude();
  float closestDistance = ourDistance;
  for (CEntity* ent : mgr.GetPhysicsActorObjectList()) {
    if (CJellyZap* zap = CPatterned::CastTo<CJellyZap>(ent)) {
      if (zap->GetAreaIdAlways() != GetAreaIdAlways())
        continue;

      float tmpDist = (playerPos - zap->GetTranslation()).magnitude();
      if (tmpDist < closestDistance)
        closestDistance = tmpDist;

      if (zap->x5b8_25_)
        return false;
    }
  }
  return zeus::close_enough(closestDistance, ourDistance);
}
} // namespace urde::MP1