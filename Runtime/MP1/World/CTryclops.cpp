#include "Runtime/MP1/World/CTryclops.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Weapon/CBomb.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <cmath>

namespace urde::MP1 {
const CDamageVulnerability CTryclops::skVulnerabilities = CDamageVulnerability(
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Normal, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::Two);

CTryclops::CTryclops(urde::TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     urde::CModelData&& mData, const urde::CPatternedInfo& pInfo,
                     const urde::CActorParameters& actParms, float f1, float f2, float f3, float f4)
: CPatterned(ECharacter::Tryclops, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Small)
, x568_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x67c_(f1)
, x680_(std::cos(zeus::degToRad(0.5f * f2)))
, x684_(f3)
, x688_launchSpeed(f4) {
  CreateShadow(false);
  MakeThermalColdAndHot();
  x460_knockBackController.SetAutoResetImpulse(false);
  x328_30_lookAtDeathDir = false;
}

void CTryclops::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
  } else if (msg == EScriptObjectMessage::InitializedInArea) {
    x568_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
  }
}

void CTryclops::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);

  if (x400_25_alive && x68c_ > 0.f)
    x68c_ -= dt;

  if (mgr.GetPlayer().GetAttachedActor() != GetUniqueId() || x698_27_dizzy)
    return;

  x698_27_dizzy = (mgr.GetPlayer().GetAttachedActorStruggle() == 1.f && sub8025dbd0(mgr));
}

void CTryclops::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& poi, EUserEventType type, float dt) {

  switch (type) {
  case EUserEventType::Projectile:
    if (x694_bombId == kInvalidUniqueId) {
      LaunchPlayer(mgr, GetLctrTransform(poi.GetLocatorName()), (x698_27_dizzy ? 5.f : x688_launchSpeed));
    } else {
      DragBomb(mgr, GetLctrTransform(poi.GetLocatorName()));
    }
    return;
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, poi, type, dt);
}

void CTryclops::Death(CStateManager& mgr, const zeus::CVector3f& vec, EScriptObjectState state) {
  if (x400_25_alive) {
    CPlayer& player = mgr.GetPlayer();
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.SetLeaveMorphBallAllowed(true);
      player.AddMaterial(EMaterialTypes::Solid, mgr);
      player.DetachActorFromPlayer();
    } else if (x694_bombId != kInvalidUniqueId) {
      if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
        bomb->SetFuseDisabled(false);
        bomb->SetIsBeingDragged(false);
      }
      x694_bombId = kInvalidUniqueId;
    }
  }

  CPatterned::Death(mgr, vec, state);
}

void CTryclops::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::Patrol(mgr, msg, arg);

  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
  } else if (msg == EStateMsg::Update) {
    ApplySeparation(mgr);
  }
}

void CTryclops::PathFind(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::PathFind(mgr, msg, arg);

  zeus::CVector3f front = GetTransform().basis[1];
  if (front.dot(x450_bodyController->GetCommandMgr().GetMoveVector()) < 0.f && front.canBeNormalized()) {
    x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(front.normalized(), {}, 1.f));
  }
  ApplySeparation(mgr);
}

void CTryclops::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
    mgr.GetPlayer().AddMaterial(EMaterialTypes::Solid, mgr);
    x698_25_ = true;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal6);
  } else if (msg == EStateMsg::Update) {
    AttractBomb(mgr, arg);
  }
}

void CTryclops::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate)
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);

  CPatterned::TargetPatrol(mgr, msg, arg);
}

void CTryclops::TargetPlayer(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::TargetPlayer(mgr, msg, arg);

  if (msg == EStateMsg::Update && x694_bombId != kInvalidUniqueId) {
    if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
      bomb->SetFuseDisabled(false);
      bomb->SetIsBeingDragged(false);
      x694_bombId = kInvalidUniqueId;
    }
  }
}

void CTryclops::TargetCover(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Update) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    if (x694_bombId == kInvalidUniqueId)
      return;

    if (TCastToConstPtr<CBomb> bomb = mgr.GetObjectById(x694_bombId))
      SetDestPos(bomb->GetTranslation());
    else
      x694_bombId = kInvalidUniqueId;
  }
}

void CTryclops::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    mgr.GetPlayer().Stop();
    mgr.GetPlayer().RemoveMaterial(EMaterialTypes::Solid, mgr);
    mgr.GetPlayer().SetLeaveMorphBallAllowed(false);
    x32c_animState = EAnimState::Ready;
    x698_24_ = false;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 1);
    if (!x698_24_)
      DragPlayer(mgr, GetLctrTransform("bombGrab_locator"sv).origin);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    if (mgr.GetPlayer().GetAttachedActor() == GetUniqueId())
      mgr.GetPlayer().DetachActorFromPlayer();

    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
    mgr.GetPlayer().AddMaterial(EMaterialTypes::Solid, mgr);
  }
}

void CTryclops::JumpBack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    if (TCastToConstPtr<CActor> wp = mgr.GetObjectById(GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow))) {
      SetDestPos(wp->GetTranslation());
    }

    if (x694_bombId != kInvalidUniqueId) {
      if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId))
        bomb->SetFuseDisabled(false);
      else
        x694_bombId = kInvalidUniqueId;
    }

    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
  } else if (msg == EStateMsg::Update) {
    GrabBomb(mgr);
  }
}

void CTryclops::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) { PathFind(mgr, msg, arg); }

void CTryclops::TurnAround(CStateManager& mgr, EStateMsg msg, float) {
  CPlayer& player = mgr.GetPlayer();
  if (msg == EStateMsg::Activate) {
    if (x694_bombId == kInvalidUniqueId) {
      player.Stop();
      player.RemoveMaterial(EMaterialTypes::Solid, mgr);
      player.SetLeaveMorphBallAllowed(true);
    }

    TUniqueId uid = GetWaypointForState(mgr, EScriptObjectState::Modify, EScriptObjectMessage::Follow);
    bool retreat = false;
    if (uid == kInvalidUniqueId) {
      uid = GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow);
      retreat = true;
    }

    if (TCastToConstPtr<CActor> wp = mgr.GetObjectById(uid)) {
      zeus::CVector3f destVec =
          (retreat ? wp->GetTransform().basis[1] : GetTranslation() - wp->GetTranslation()).normalized();
      destVec.z() = 0.f;
      SetDestPos(GetTranslation() + destVec);

      if (std::fabs(zeus::CVector3f(GetTransform().basis[1].x(), GetTransform().basis[1].y(), 0.f).dot(destVec)) < 0.9998)
        x32c_animState = EAnimState::Ready;
    }
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Turn, &CPatterned::TryTurn, 0);

    if (x694_bombId == kInvalidUniqueId)
      DragPlayer(mgr, GetLctrTransform("ballGrab_locator"sv).origin);
    else
      GrabBomb(mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
    mgr.GetPlayer().AddMaterial(EMaterialTypes::Solid, mgr);
    if (mgr.GetPlayer().GetAttachedActor() == GetUniqueId())
      mgr.GetPlayer().DetachActorFromPlayer();
  }
}

void CTryclops::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    if (TCastToConstPtr<CActor> wp =
            mgr.GetObjectById(GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow))) {
      SetDestPos(wp->GetTranslation());
    }

    mgr.GetPlayer().Stop();
    mgr.GetPlayer().RemoveMaterial(EMaterialTypes::Solid, mgr);
    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
    mgr.GetPlayer().AttachActorToPlayer(GetUniqueId(), true);
    mgr.GetPlayer().SetLeaveMorphBallAllowed(false);
    mgr.GetPlayer().GetMorphBall()->DisableHalfPipeStatus();
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
  } else if (msg == EStateMsg::Update) {
    DragPlayer(mgr, GetLctrTransform("ballGrab_locator"sv).origin);
  } else if (msg == EStateMsg::Deactivate) {
    if (mgr.GetPlayer().GetAttachedActor() == GetUniqueId())
      mgr.GetPlayer().DetachActorFromPlayer();
    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
    mgr.GetPlayer().AddMaterial(EMaterialTypes::Solid, mgr);
  }
}

void CTryclops::GetUp(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x698_24_ = false;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 1);
    if (!x698_24_) {
      GrabBomb(mgr);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CTryclops::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    mgr.GetPlayer().SetLeaveMorphBallAllowed(false);
    mgr.GetPlayer().GetMorphBall()->DisableHalfPipeStatus();
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal6);
  } else if (msg == EStateMsg::Update) {
    SuckPlayer(mgr, arg);
  } else if (msg == EStateMsg::Deactivate) {
    mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
    mgr.GetPlayer().AddMaterial(EMaterialTypes::Solid, mgr);
  }
}

void CTryclops::Cover(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    if (!x698_25_)
      x68c_ = 1.5f;

    if (mgr.GetPlayer().GetAttachedActor() == GetUniqueId()) {
      mgr.GetPlayer().SetLeaveMorphBallAllowed(true);
      mgr.GetPlayer().AddMaterial(EMaterialTypes::Solid, mgr);
      mgr.GetPlayer().DetachActorFromPlayer();
    }
  }
}

void CTryclops::Approach(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::PathFind(mgr, msg, arg);
  ApplySeparation(mgr);
  if (msg == EStateMsg::Update)
    GrabBomb(mgr);
}

void CTryclops::PathFindEx(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::PathFind(mgr, msg, arg);
  ApplySeparation(mgr);

  CPlayer& player = mgr.GetPlayer();
  if (msg == EStateMsg::Activate) {
    player.Stop();
    player.RemoveMaterial(EMaterialTypes::Solid, mgr);
    player.GetMorphBall()->DisableHalfPipeStatus();
    player.AttachActorToPlayer(GetUniqueId(), true);
  } else if (msg == EStateMsg::Update) {
    DragPlayer(mgr, GetLctrTransform("ballGrab_locator"sv).origin);
  }
}

void CTryclops::Dizzy(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    mgr.GetPlayer().Stop();
    mgr.GetPlayer().RemoveMaterial(EMaterialTypes::Solid, mgr);
    mgr.GetPlayer().SetLeaveMorphBallAllowed(false);
    x32c_animState = EAnimState::Ready;
    x698_24_ = false;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 0);
    if (!x698_24_)
      DragPlayer(mgr, GetLctrTransform("ballGrab_locator"sv).origin);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    if (mgr.GetPlayer().GetAttachedActor() == GetUniqueId()) {
      mgr.GetPlayer().DetachActorFromPlayer();
      x698_27_dizzy = false;
    }
  }
}

bool CTryclops::InAttackPosition(CStateManager& mgr, float) {
  if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    return false;

  CPlayer& player = mgr.GetPlayer();
  return sub80260180(player.GetTranslation(),
                     player.GetTranslation() + zeus::CVector3f(0.f, 0.f, player.GetMorphBall()->GetBallRadius()),
                     player.GetBoundingBox(), mgr);
}

bool CTryclops::InRange(CStateManager& mgr, float) {
  if (x694_bombId != kInvalidUniqueId) {
    if (TCastToConstPtr<CBomb> bomb = mgr.GetObjectById(x694_bombId)) {
      return sub80260180(bomb->GetTranslation(), bomb->GetTranslation(), *bomb->GetTouchBounds(), mgr);
    }
  }

  return false;
}

bool CTryclops::InMaxRange(CStateManager& mgr, float) {
  if (x694_bombId != kInvalidUniqueId)
    return true;

  rstl::reserved_vector<TUniqueId, 1024> nearList;

  float dectRange = x3bc_detectionRange * x3bc_detectionRange;
  float dectRangeHeight = x3c0_detectionHeightRange * x3c0_detectionHeightRange;
  mgr.BuildNearList(nearList, {GetTranslation() - x3bc_detectionRange, GetTranslation() + x3bc_detectionRange},
                    CMaterialFilter::MakeInclude({EMaterialTypes::Bomb}), this);

  x694_bombId = kInvalidUniqueId;

  for (TUniqueId uid : nearList) {
    if (TCastToConstPtr<CBomb> bomb = mgr.GetObjectById(uid)) {
      if (!bomb->IsBeingDragged()) {
        float dist = (bomb->GetTranslation() - GetTranslation()).magSquared();
        float height = (bomb->GetTranslation().z() - GetTranslation().z());
        if (dist < dectRange && (height * height) < dectRangeHeight) {
          if (x568_pathFindSearch.OnPath(bomb->GetTranslation()) == CPathFindSearch::EResult::Success) {
            dectRange = dist;
            x694_bombId = bomb->GetUniqueId();
          }
        }
      }
    }
  }

  if (x694_bombId != kInvalidUniqueId) {
    if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
      bomb->SetFuseDisabled(true);
      bomb->SetIsBeingDragged(true);
      return true;
    }
  }

  return false;
}

bool CTryclops::InDetectionRange(CStateManager& mgr, float arg) {
  CPlayer& player = mgr.GetPlayer();

  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed ||
      player.GetAttachedActor() != kInvalidUniqueId || x68c_ > 0.f || !CPatterned::InDetectionRange(mgr, arg))
    return false;

  return x568_pathFindSearch.OnPath(player.GetBallPosition()) == CPathFindSearch::EResult::Success;
}

bool CTryclops::SpotPlayer(CStateManager& mgr, float) {
  if (x694_bombId != kInvalidUniqueId) {

    CPlayer& player = mgr.GetPlayer();
    if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
      if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
        bool isPlayerCloser = (player.GetTranslation() - GetTranslation()).magSquared() <
                              (bomb->GetTranslation() - GetTranslation()).magSquared();
        if (isPlayerCloser) {
          bomb->SetFuseDisabled(false);
          bomb->SetIsBeingDragged(false);
          x694_bombId = kInvalidUniqueId;
        }
        return isPlayerCloser;
      }
    }
  }

  return true;
}
bool CTryclops::InPosition(CStateManager& mgr, float arg) {
  if (x694_bombId != kInvalidUniqueId) {
    if (TCastToConstPtr<CBomb> bomb = mgr.GetObjectById(x694_bombId)) {
      return InRangeToLocator(bomb->GetTranslation(), arg);
    }
  }

  return false;
}

bool CTryclops::HearShot(CStateManager& mgr, float) {
  x698_26_ = false;
  if (x694_bombId != kInvalidUniqueId) {
    if (TCastToConstPtr<CBomb>(mgr.GetObjectById(x694_bombId))) {
      x698_26_ = true;
      return false;
    } else
      x694_bombId = kInvalidUniqueId;
  }

  return true;
}

bool CTryclops::CoverBlown(CStateManager&, float) {
  return x568_pathFindSearch.OnPath(GetTranslation()) != CPathFindSearch::EResult::InvalidArea;
}

bool CTryclops::Inside(CStateManager& mgr, float arg) {
  const zeus::CTransform xf = mgr.GetPlayer().GetTransform();
  x64c_ = xf.getRotation();

  return InRangeToLocator(xf.origin + zeus::CVector3f(0.f, 0.f, mgr.GetPlayer().GetMorphBall()->GetBallRadius()), arg);
}

bool CTryclops::ShouldRetreat(CStateManager& mgr, float) {
  if (TCastToConstPtr<CActor> wp =
          mgr.GetObjectById(GetWaypointForState(mgr, EScriptObjectState::Modify, EScriptObjectMessage::Next))) {
    SetDestPos(wp->GetTranslation());
    return true;
  }

  return false;
}

bool CTryclops::IsDizzy(CStateManager&, float) { return x698_27_dizzy; }

void CTryclops::LaunchPlayer(CStateManager& mgr, const zeus::CTransform& xf, float f1) {
  CPlayer& player = mgr.GetPlayer();
  player.SetLeaveMorphBallAllowed(true);

  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    return;

  x698_24_ = true;
  x68c_ = 1.5f;
  player.Stop();
  zeus::CTransform tmpXf = (xf * x64c_);
  tmpXf.origin += zeus::CVector3f(0.f, 0.f, -0.5f);
  player.Teleport(tmpXf, mgr, false);
  player.ApplyImpulseWR(f1 * (player.GetMass() * xf.basis[1].normalized()), zeus::CAxisAngle());
  player.SetMoveState(CPlayer::EPlayerMovementState::ApplyJump, mgr);
  player.AddMaterial(EMaterialTypes::Solid, mgr);
  mgr.ApplyDamage(GetUniqueId(), player.GetUniqueId(), GetUniqueId(), GetContactDamage(),
                  CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
}

void CTryclops::DragBomb(CStateManager& mgr, const zeus::CTransform& xf) {
  if (x694_bombId != kInvalidUniqueId) {
    if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
      bomb->SetVelocityWR((5.f * mgr.GetActiveRandom()->Float() + 20.f) * xf.basis[1].normalized());
      bomb->SetConstantAccelerationWR({0.f, 0.f, -CPhysicsActor::GravityConstant()});
    }
  }

  x698_26_ = false;
  x698_24_ = true;
  x694_bombId = kInvalidUniqueId;
}

void CTryclops::ApplySeparation(CStateManager& mgr) {
  for (CEntity* ent : mgr.GetListeningAiObjectList()) {
    if (TCastToPtr<CAi> ai = ent) {
      if (ai == this || ai->GetAreaIdAlways() != GetAreaId())
        continue;

      zeus::CVector3f sep = x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), 8.f);
      if (sep.x() != 0.f && sep.y() != 0.f && sep.z() != 0.f)
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(sep, {}, 1.f));
    }
  }
}

void CTryclops::GrabBomb(CStateManager& mgr) {
  if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
    zeus::CTransform grabLctr = GetLctrTransform("ballGrab_locator"sv);
    grabLctr.origin += zeus::CVector3f(0.f, 0.f, -.3f);
    bomb->SetTransform(grabLctr);
  }
}

void CTryclops::DragPlayer(CStateManager& mgr, const zeus::CVector3f& locOrig) {
  CPlayer& player = mgr.GetPlayer();
  player.Stop();
  player.RemoveMaterial(EMaterialTypes::Solid, mgr);
  zeus::CTransform xf = GetLctrTransform("ballGrab_locator"sv) * x64c_;
  xf.origin += {0.f, 0.f, -.5f};
  player.SetTransform(xf);
}

bool CTryclops::InRangeToLocator(const zeus::CVector3f& vec, float arg) const {
  return (vec - GetLctrTransform("ballGrab_locator"sv).origin).magSquared() <= arg;
}

bool CTryclops::sub80260180(const zeus::CVector3f& vec1, const zeus::CVector3f& vec2, const zeus::CAABox& bounds,
                            CStateManager& mgr) {

  if (bounds.intersects(GetBoundingBox()))
    return true;

  zeus::CTransform xf = GetLctrTransform("ballGrab_locator"sv);
  zeus::CVector3f tmpVec2 = vec2 - (xf.origin - (1.f * GetTransform().basis[1]));
  float f28 = tmpVec2.normalized().dot(GetTransform().basis[1]);
  zeus::CVector3f tmpVec4 = (vec1 - (xf.origin - (4.f * GetTransform().basis[1]))).normalized();
  float f30 = tmpVec4.dot(GetTransform().basis[1]);
  float f29 = tmpVec2.magnitude();

  if (f29 > 2.f) {
    CRayCastResult res = mgr.RayStaticIntersection(
        xf.origin, (1.f / f29) * tmpVec2, f29 - mgr.GetPlayer().GetMorphBall()->GetBallRadius(),
        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Character, EMaterialTypes::Player,
                                                                      EMaterialTypes::ProjectilePassthrough}));
    if (res.IsValid())
      return false;
  }

  return !(f29 >= x684_ || f28 <= 0.f || f30 <= x680_);
}

void CTryclops::SuckPlayer(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    return;

  CPlayer& player = mgr.GetPlayer();
  zeus::CTransform xf = GetLctrTransform("ballGrab_locator"sv);
  zeus::CVector3f diff = (player.GetTranslation() - xf.origin);
  float diffMag = diff.magnitude();
  if (diffMag < 3.f) {
    player.Stop();
    AttractPlayer(mgr, xf.origin, arg);
  } else {
    player.ApplyForceWR(((x67c_ * (x684_ / (diffMag * diffMag))) * (player.GetMass() * -diff)), {});
  }
}

void CTryclops::AttractPlayer(CStateManager& mgr, const zeus::CVector3f& dest, float arg) {
  CPlayer& player = mgr.GetPlayer();
  const float ballRad = player.GetMorphBall()->GetBallRadius();
  player.SetVelocityWR(1.f / (2.f * arg) *
                       (dest - (player.GetTranslation() + zeus::CVector3f(0.f, 0.f, ballRad))).normalized());
}

void CTryclops::AttractBomb(CStateManager& mgr, float arg) {
  if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
    bomb->SetVelocityWR(
        1.f / (2.f * arg) *
        (GetLctrTransform("ballGrab_locator"sv).origin + zeus::CVector3f(0.f, 0.f, -.3f) - bomb->GetTranslation())
            .normalized());
  }
}

} // namespace urde::MP1