#include "Runtime/MP1/World/CTryclops.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Weapon/CBomb.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <cmath>

namespace metaforce::MP1 {
const CDamageVulnerability CTryclops::skVulnerabilities = CDamageVulnerability(
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Normal, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::Two);

CTryclops::CTryclops(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, float f1,
                     float f2, float f3, float launchSpeed)
: CPatterned(ECharacter::Tryclops, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Small)
, x568_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x67c_(f1)
, x680_(std::cos(zeus::degToRad(0.5f * f2)))
, x684_(f3)
, x688_launchSpeed(launchSpeed) {
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
  if (x400_25_alive && x68c_ > 0.f) {
    x68c_ -= dt;
  }
  if (mgr.GetPlayer().GetAttachedActor() != GetUniqueId() || x698_27_dizzy) {
    return;
  }
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
  const auto moveVec = x450_bodyController->GetCommandMgr().GetMoveVector();
  if (GetTransform().frontVector().dot(moveVec) < 0.f && moveVec.canBeNormalized()) {
    x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, moveVec.normalized(), 1.f));
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
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
  }
  CPatterned::TargetPatrol(mgr, msg, arg);
}

void CTryclops::TargetPlayer(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::TargetPlayer(mgr, msg, arg);
  if (msg == EStateMsg::Activate && x694_bombId != kInvalidUniqueId) {
    if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
      bomb->SetFuseDisabled(false);
      bomb->SetIsBeingDragged(false);
      x694_bombId = kInvalidUniqueId;
    }
  }
}

void CTryclops::TargetCover(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    if (x694_bombId == kInvalidUniqueId) {
      return;
    }
    if (TCastToConstPtr<CBomb> bomb = mgr.GetObjectById(x694_bombId)) {
      SetDestPos(bomb->GetTranslation());
    } else {
      x694_bombId = kInvalidUniqueId;
    }
  }
}

void CTryclops::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  auto& player = mgr.GetPlayer();
  if (msg == EStateMsg::Activate) {
    player.Stop();
    player.RemoveMaterial(EMaterialTypes::Solid, mgr);
    player.SetLeaveMorphBallAllowed(false);
    x32c_animState = EAnimState::Ready;
    x698_24_ = false;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 1);
    if (!x698_24_) {
      DragPlayer(mgr, GetLctrTransform("bombGrab_locator"sv).origin);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.DetachActorFromPlayer();
    }

    player.SetLeaveMorphBallAllowed(true);
    player.AddMaterial(EMaterialTypes::Solid, mgr);
  }
}

void CTryclops::JumpBack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    if (TCastToConstPtr<CActor> wp =
            mgr.GetObjectById(GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow))) {
      SetDestPos(wp->GetTranslation());
    }

    if (x694_bombId != kInvalidUniqueId) {
      if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
        bomb->SetFuseDisabled(false);
      } else {
        x694_bombId = kInvalidUniqueId;
      }
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
    bool retreat = uid == kInvalidUniqueId;
    if (retreat) {
      uid = GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow);
    }

    if (TCastToConstPtr<CActor> wp = mgr.GetObjectById(uid)) {
      zeus::CVector3f destVec =
          (retreat ? wp->GetTransform().frontVector() : wp->GetTranslation() - GetTranslation()).normalized();
      destVec.z() = 0.f;
      SetDestPos(GetTranslation() + destVec);

      if (std::fabs(
              zeus::CVector3f(GetTransform().frontVector().x(), GetTransform().frontVector().y(), 0.f).dot(destVec)) <
          0.9998) {
        x32c_animState = EAnimState::Ready;
      }
    }
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Turn, &CPatterned::TryTurn, 0);

    if (x694_bombId == kInvalidUniqueId) {
      DragPlayer(mgr, GetLctrTransform("ballGrab_locator"sv).origin);
    } else {
      GrabBomb(mgr);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    player.SetLeaveMorphBallAllowed(true);
    player.AddMaterial(EMaterialTypes::Solid, mgr);
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.DetachActorFromPlayer();
    }
  }
}

void CTryclops::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  auto& player = mgr.GetPlayer();
  if (msg == EStateMsg::Activate) {
    if (TCastToConstPtr<CActor> wp =
            mgr.GetObjectById(GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow))) {
      SetDestPos(wp->GetTranslation());
    }

    player.Stop();
    player.RemoveMaterial(EMaterialTypes::Solid, mgr);
    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
    player.AttachActorToPlayer(GetUniqueId(), true);
    player.SetLeaveMorphBallAllowed(false);
    player.GetMorphBall()->DisableHalfPipeStatus();
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
  } else if (msg == EStateMsg::Update) {
    DragPlayer(mgr, GetLctrTransform("ballGrab_locator"sv).origin);
  } else if (msg == EStateMsg::Deactivate) {
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.DetachActorFromPlayer();
    }
    player.SetLeaveMorphBallAllowed(true);
    player.AddMaterial(EMaterialTypes::Solid, mgr);
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
  auto& player = mgr.GetPlayer();
  if (msg == EStateMsg::Activate) {
    player.SetLeaveMorphBallAllowed(false);
    player.GetMorphBall()->DisableHalfPipeStatus();
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal6);
  } else if (msg == EStateMsg::Update) {
    SuckPlayer(mgr, arg);
  } else if (msg == EStateMsg::Deactivate) {
    player.SetLeaveMorphBallAllowed(true);
    player.AddMaterial(EMaterialTypes::Solid, mgr);
  }
}

void CTryclops::Cover(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    if (!x698_25_) {
      x68c_ = 1.5f;
    }
    auto& player = mgr.GetPlayer();
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.SetLeaveMorphBallAllowed(true);
      player.AddMaterial(EMaterialTypes::Solid, mgr);
      player.DetachActorFromPlayer();
    }
  }
}

void CTryclops::Approach(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::PathFind(mgr, msg, arg);
  ApplySeparation(mgr);
  if (msg == EStateMsg::Update) {
    GrabBomb(mgr);
  }
}

void CTryclops::PathFindEx(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::PathFind(mgr, msg, arg);
  ApplySeparation(mgr);

  auto& player = mgr.GetPlayer();
  if (msg == EStateMsg::Activate) {
    player.Stop();
    player.RemoveMaterial(EMaterialTypes::Solid, mgr);
    player.SetLeaveMorphBallAllowed(false);
    player.GetMorphBall()->DisableHalfPipeStatus();
    player.AttachActorToPlayer(GetUniqueId(), true);
  } else if (msg == EStateMsg::Update) {
    DragPlayer(mgr, GetLctrTransform("ballGrab_locator"sv).origin);
  }
}

void CTryclops::Dizzy(CStateManager& mgr, EStateMsg msg, float) {
  auto& player = mgr.GetPlayer();
  if (msg == EStateMsg::Activate) {
    player.Stop();
    player.RemoveMaterial(EMaterialTypes::Solid, mgr);
    player.SetLeaveMorphBallAllowed(false);
    x32c_animState = EAnimState::Ready;
    x698_24_ = false;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 0);
    if (!x698_24_) {
      DragPlayer(mgr, GetLctrTransform("ballGrab_locator"sv).origin);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.SetLeaveMorphBallAllowed(true);
      player.AddMaterial(EMaterialTypes::Solid, mgr);
      player.DetachActorFromPlayer();
      x698_27_dizzy = false;
    }
  }
}

bool CTryclops::InAttackPosition(CStateManager& mgr, float) {
  if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    return false;
  }
  auto& player = mgr.GetPlayer();
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
  if (x694_bombId != kInvalidUniqueId) {
    return true;
  }

  EntityList nearList;

  float dectRange = x3bc_detectionRange * x3bc_detectionRange;
  float dectRangeHeight = x3c0_detectionHeightRange * x3c0_detectionHeightRange;
  zeus::CAABox aabb{GetTranslation() + zeus::CVector3f{-x3bc_detectionRange, -x3bc_detectionRange, 0.f},
                    GetTranslation() +
                        zeus::CVector3f{x3bc_detectionRange, x3bc_detectionRange, x3c0_detectionHeightRange}};
  mgr.BuildNearList(nearList, aabb, CMaterialFilter::MakeInclude({EMaterialTypes::Bomb}), this);

  x694_bombId = kInvalidUniqueId;

  for (TUniqueId uid : nearList) {
    if (TCastToConstPtr<CBomb> bomb = mgr.GetObjectById(uid)) {
      if (!bomb->IsBeingDragged()) {
        const auto dist = bomb->GetTranslation() - GetTranslation();
        float distSq = dist.magSquared();
        if (distSq < dectRange && (dectRangeHeight <= 0.f || (dectRangeHeight > dist.z() * dist.z()))) {
          if (x568_pathFindSearch.OnPath(bomb->GetTranslation()) == CPathFindSearch::EResult::Success) {
            x694_bombId = bomb->GetUniqueId();
            dectRange = distSq;
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
  auto& player = mgr.GetPlayer();
  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed ||
      player.GetAttachedActor() != kInvalidUniqueId || x68c_ > 0.f || !CPatterned::InDetectionRange(mgr, arg)) {
    return false;
  }
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
    }
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

  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    return;
  }

  x698_24_ = true;
  x68c_ = 1.5f;
  player.Stop();
  zeus::CTransform tmpXf = (xf * x64c_);
  tmpXf.origin += zeus::CVector3f(0.f, 0.f, -0.5f);
  player.Teleport(tmpXf, mgr, false);
  player.ApplyImpulseWR(f1 * (player.GetMass() * xf.frontVector().normalized()), zeus::CAxisAngle());
  player.SetMoveState(CPlayer::EPlayerMovementState::ApplyJump, mgr);
  player.AddMaterial(EMaterialTypes::Solid, mgr);
  mgr.ApplyDamage(GetUniqueId(), player.GetUniqueId(), GetUniqueId(), GetContactDamage(),
                  CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
}

void CTryclops::DragBomb(CStateManager& mgr, const zeus::CTransform& xf) {
  if (x694_bombId != kInvalidUniqueId) {
    if (TCastToPtr<CBomb> bomb = mgr.ObjectById(x694_bombId)) {
      bomb->SetVelocityWR((5.f * mgr.GetActiveRandom()->Float() + 20.f) * xf.frontVector().normalized());
      bomb->SetConstantAccelerationWR({0.f, 0.f, -CPhysicsActor::GravityConstant()});
    }
  }

  x698_26_ = false;
  x698_24_ = true;
  x694_bombId = kInvalidUniqueId;
}

void CTryclops::ApplySeparation(CStateManager& mgr) {
  for (CEntity* ent : mgr.GetAiWaypointObjectList()) {
    if (TCastToPtr<CPatterned> ai = ent) {
      if (ai == this || ai->GetAreaIdAlways() != GetAreaId()) {
        continue;
      }

      zeus::CVector3f sep = x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), 8.f);
      if (sep.x() != 0.f || sep.y() != 0.f || sep.z() != 0.f) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(sep, zeus::skZero3f, 1.f));
      }
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

  if (bounds.intersects(GetBoundingBox())) {
    return true;
  }

  zeus::CTransform xf = GetLctrTransform("ballGrab_locator"sv);
  zeus::CVector3f tmpVec2 = vec2 - (xf.origin - (1.f * GetTransform().frontVector()));
  float tmpVec2Dot = tmpVec2.normalized().dot(GetTransform().frontVector());
  zeus::CVector3f tmpVec1 = vec1 - (xf.origin - (4.f * GetTransform().frontVector()));
  float tmpVec1Dot = tmpVec1.normalized().dot(GetTransform().frontVector());
  float tmpVec2Mag = tmpVec2.magnitude();

  if (tmpVec2Mag > 2.f) {
    CRayCastResult res = mgr.RayStaticIntersection(
        xf.origin, (1.f / tmpVec2Mag) * tmpVec2, tmpVec2Mag - mgr.GetPlayer().GetMorphBall()->GetBallRadius(),
        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Character, EMaterialTypes::Player,
                                                                      EMaterialTypes::ProjectilePassthrough}));
    if (res.IsValid()) {
      return false;
    }
  }

  return !(x684_ <= tmpVec2Mag || tmpVec2Dot <= 0.f || tmpVec1Dot <= x680_);
}

void CTryclops::SuckPlayer(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    return;
  }
  CPlayer& player = mgr.GetPlayer();
  zeus::CTransform xf = GetLctrTransform("ballGrab_locator"sv);
  zeus::CVector3f diff = (player.GetTranslation() - xf.origin);
  float diffMag = diff.magnitude();
  if (diffMag < 3.f) {
    player.Stop();
    AttractPlayer(mgr, xf.origin, arg);
  } else {
    player.ApplyForceWR(((x67c_ * (x684_ / (diffMag * diffMag))) * player.GetMass() * -diff), {});
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

bool CTryclops::sub8025dbd0(CStateManager& mgr) {
  const auto result = mgr.RayStaticIntersection(GetLctrTransform("Skeleton_Root").origin, GetTransform().frontVector(),
                                                3.f, CMaterialFilter::skPassEverything);
  if (result.IsValid()) {
    return true;
  }

  const auto& player = mgr.GetPlayer();
  const float ballRadius = player.GetMorphBall()->GetBallRadius();
  constexpr CMaterialList matList{EMaterialTypes::Player, EMaterialTypes::Solid};
  const CCollidableSphere colSphere{zeus::CSphere{GetTranslation() + zeus::CVector3f{0.f, 0.f, ballRadius}, ballRadius},
                                    matList};
  EntityList nearList;
  mgr.BuildColliderList(nearList, player, colSphere.CalculateLocalAABox());
  constexpr auto matFilter = CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Player});
  const zeus::CTransform skIdentity4f{}; // TODO move to zeus & make constexpr
  if (CGameCollision::DetectStaticCollisionBoolean(mgr, colSphere, skIdentity4f, matFilter)) {
    return true;
  }

  for (const auto& id : nearList) {
    if (id == GetUniqueId()) {
      continue;
    }
    if (TCastToConstPtr<CPhysicsActor> actor = mgr.GetObjectById(id)) {
      if (CCollisionPrimitive::CollideBoolean(
              {colSphere, matFilter, actor->GetPrimitiveTransform()},
              {*actor->GetCollisionPrimitive(), CMaterialFilter::skPassEverything, skIdentity4f})) {
        return true;
      }
    }
  }

  return false;
}

} // namespace metaforce::MP1
