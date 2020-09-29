#include "Runtime/MP1/World/CWarWasp.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CWarWasp::CWarWasp(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor,
                   CPatterned::EColliderType collider, const CDamageInfo& dInfo1, const CActorParameters& actorParms,
                   CAssetId projectileWeapon, const CDamageInfo& projectileDamage, CAssetId projectileVisorParticle,
                   u32 projecileVisorSfx)
: CPatterned(ECharacter::WarWasp, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer, collider,
             EBodyType::Flyer, actorParms, EKnockBackVariant::Small)
, x570_cSphere(zeus::CSphere({0.f, 0.f, 1.8f}, 1.f), x68_material)
, x590_pfSearch(nullptr, 0x3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x684_(dInfo1)
, x6d4_projectileInfo(projectileWeapon, projectileDamage)
, x72c_projectileVisorSfx(CSfxManager::TranslateSFXID(projecileVisorSfx))
, x72e_26_initiallyInactive(!pInfo.GetActive()) {
  x6d4_projectileInfo.Token().Lock();
  UpdateTouchBounds();
  SetCoefficientOfRestitutionModifier(0.1f);
  if (projectileVisorParticle.IsValid())
    x71c_projectileVisorParticle = g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), projectileVisorParticle});
  x328_29_noPatternShagging = true;
  x460_knockBackController.SetAnimationStateRange(EKnockBackAnimationState::KnockBack,
                                                  EKnockBackAnimationState::KnockBack);
}

void CWarWasp::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CWarWasp::SwarmAdd(CStateManager& mgr) {
  if (x674_aiMgr != kInvalidUniqueId) {
    if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x674_aiMgr)) {
      CTeamAiRole::ETeamAiRole role = x3fc_flavor == EFlavorType::Two ?
        CTeamAiRole::ETeamAiRole::Ranged : CTeamAiRole::ETeamAiRole::Melee;
      if (!aimgr->IsPartOfTeam(GetUniqueId())) {
        aimgr->AssignTeamAiRole(*this, role, CTeamAiRole::ETeamAiRole::Invalid, CTeamAiRole::ETeamAiRole::Invalid);
      }
    }
  }
}

void CWarWasp::SwarmRemove(CStateManager& mgr) {
  if (x674_aiMgr != kInvalidUniqueId) {
    if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x674_aiMgr)) {
      if (aimgr->IsPartOfTeam(GetUniqueId())) {
        aimgr->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

void CWarWasp::ApplyDamage(CStateManager& mgr) {
  if (x72e_25_canApplyDamage && x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
    if (mgr.GetPlayer().GetBoundingBox().pointInside(
            GetTransform() * (GetLocatorTransform("LCTR_WARTAIL"sv).origin * x64_modelData->GetScale()))) {
      mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
      x72e_25_canApplyDamage = false;
    }
  }
}

void CWarWasp::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (x700_attackRemTime > 0.f) {
    float rate = 1.f;
    if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed)
      rate =
          1.f - zeus::CVector2f::getAngleDiff(mgr.GetPlayer().GetTransform().basis[1].toVec2f(),
                                              GetTranslation().toVec2f() - mgr.GetPlayer().GetTranslation().toVec2f()) /
                    M_PIF * 0.666f;
    x700_attackRemTime -= rate * dt;
  }

  ApplyDamage(mgr);
  CPatterned::Think(dt, mgr);
}

void CWarWasp::SetUpCircleBurstWaypoint(CStateManager& mgr) {
  for (const auto& conn : GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::CloseIn && conn.x4_msg == EScriptObjectMessage::Follow) {
      if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId))) {
        x6b0_circleBurstPos = wp->GetTranslation();
        x6bc_circleBurstDir = wp->GetTransform().basis[1];
        x6c8_circleBurstRight = wp->GetTransform().basis[0];
        break;
      }
    }
  }
}

void CWarWasp::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case EScriptObjectMessage::Deleted:
  case EScriptObjectMessage::Deactivate:
    SwarmRemove(mgr);
    break;
  case EScriptObjectMessage::InitializedInArea:
    if (x674_aiMgr == kInvalidUniqueId)
      x674_aiMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    x590_pfSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    if (x6b0_circleBurstPos.isZero())
      SetUpCircleBurstWaypoint(mgr);
    break;
  default:
    break;
  }
}

std::optional<zeus::CAABox> CWarWasp::GetTouchBounds() const {
  return {x570_cSphere.CalculateAABox(GetTransform())};
}

zeus::CVector3f CWarWasp::GetProjectileAimPos(const CStateManager& mgr, float zBias) const {
  zeus::CVector3f ret = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    ret += zeus::CVector3f(0.f, 0.f, zBias);
  return ret;
}

void CWarWasp::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case EUserEventType::Projectile: {
    zeus::CTransform xf = GetLctrTransform(node.GetLocatorName());
    zeus::CVector3f aimPos = GetProjectileAimPos(mgr, -0.07f);
    if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
      zeus::CVector3f delta = aimPos - xf.origin;
      if (delta.canBeNormalized()) {
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        TUniqueId bestId = kInvalidUniqueId;
        CRayCastResult res = mgr.RayWorldIntersection(bestId, xf.origin, delta.normalized(), delta.magnitude(),
                                                      CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), nearList);
        if (res.IsValid())
          aimPos = res.GetPoint();
      }
    }
    LaunchProjectile(
        zeus::lookAt(xf.origin, GetProjectileInfo()->PredictInterceptPos(xf.origin, aimPos, mgr.GetPlayer(), true, dt)),
        mgr, 4, EProjectileAttrib::None, false, {x71c_projectileVisorParticle}, x72c_projectileVisorSfx, true,
        zeus::skOne3f);
    handled = true;
    break;
  }
  case EUserEventType::DeGenerate:
    SendScriptMsgs(EScriptObjectState::DeactivateState, mgr, EScriptObjectMessage::None);
    mgr.FreeScriptObject(GetUniqueId());
    handled = true;
    break;
  case EUserEventType::GenerateEnd:
    AddMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, mgr);
    handled = true;
    break;
  case EUserEventType::DamageOn:
    x72e_25_canApplyDamage = true;
    break;
  case EUserEventType::DamageOff:
    x72e_25_canApplyDamage = false;
    break;
  default:
    break;
  }
  if (!handled)
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

const CCollisionPrimitive* CWarWasp::GetCollisionPrimitive() const { return &x570_cSphere; }

void CWarWasp::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  CPatterned::Death(mgr, direction, state);
  x328_25_verticalMovement = false;
  AddMaterial(EMaterialTypes::GroundCollider, mgr);
  SwarmRemove(mgr);
}

bool CWarWasp::IsListening() const { return true; }

bool CWarWasp::Listen(const zeus::CVector3f& pos, EListenNoiseType type) {
  switch (type) {
  case EListenNoiseType::PlayerFire:
  case EListenNoiseType::BombExplode:
  case EListenNoiseType::ProjectileExplode:
    if ((GetTranslation() - pos).magSquared() < x3bc_detectionRange * x3bc_detectionRange) {
      x72e_31_heardNoise = true;
      return true;
    }
    break;
  default:
    break;
  }
  return false;
}

float CWarWasp::GetCloseInZBasis(const CStateManager& mgr) const {
  return mgr.GetPlayer().GetTranslation().z() + mgr.GetPlayer().GetEyeHeight() - 0.5f;
}

zeus::CVector3f CWarWasp::GetCloseInPos(const CStateManager& mgr, const zeus::CVector3f& aimPos) const {
  float midRange = (x2fc_minAttackRange + x300_maxAttackRange) * 0.5f;
  zeus::CVector3f ret;
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    ret = mgr.GetPlayer().GetTransform().basis[1] * midRange + aimPos;
  } else {
    zeus::CVector3f delta = GetTranslation() - aimPos;
    if (delta.canBeNormalized())
      ret = delta.normalized() * midRange + aimPos;
    else
      ret = GetTransform().basis[1] * midRange + aimPos;
  }
  ret.z() = 0.5f + GetCloseInZBasis(mgr);
  return ret;
}

zeus::CVector3f CWarWasp::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                    const zeus::CVector3f& aimPos) const {
  if (x6b0_circleBurstPos.isZero())
    return GetCloseInPos(mgr, aimPos);
  else
    return GetTranslation();
}

void CWarWasp::UpdateTouchBounds() {
  zeus::CAABox aabb = x64_modelData->GetAnimationData()->GetBoundingBox();
  x570_cSphere.SetSphereCenter(aabb.center());
  SetBoundingBox(aabb.getTransformedAABox(zeus::CTransform(GetTransform().basis)));
}

void CWarWasp::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate: {
    float maxSpeed = x450_bodyController->GetBodyStateInfo().GetMaxSpeed();
    if (maxSpeed > 0.f) {
      x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
      float speedFactor =
          x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::ELocomotionAnim::Walk) * 0.9f / maxSpeed;
      x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(speedFactor, speedFactor);
    }
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x72e_31_heardNoise = false;
    break;
  }
  case EStateMsg::Deactivate:
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    break;
  default:
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CWarWasp::SetUpPathFindBehavior(CStateManager& mgr) {
  x72e_29_pathObstructed = false;
  if (GetSearchPath()) {
    SwarmAdd(mgr);
    zeus::CVector3f pos;
    if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, GetUniqueId()))
      pos = role->GetTeamPosition();
    else
      pos = GetCloseInPos(mgr, GetProjectileAimPos(mgr, -1.25f));
    SetDestPos(pos);
    if ((x2e0_destPos - GetTranslation()).magSquared() > 64.f ||
        IsPatternObstructed(mgr, GetTranslation(), x2e0_destPos)) {
      zeus::CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
      zeus::CVector3f delta = x2e0_destPos - aimPos;
      if (delta.canBeNormalized()) {
        zeus::CVector3f dir = delta.normalized();
        CRayCastResult res = mgr.RayStaticIntersection(
            aimPos, dir, delta.magnitude(),
            CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Player}));
        if (res.IsValid()) {
          SetDestPos(dir * res.GetT() * 0.5f + aimPos);
          x72e_29_pathObstructed = true;
        }
      }
      CPatterned::PathFind(mgr, EStateMsg::Activate, 0.f);
    }
  }
}

void CWarWasp::ApplyNormalSteering(CStateManager& mgr) {
  zeus::CVector3f delta = mgr.GetPlayer().GetTranslation() - GetTranslation();
  zeus::CVector3f teamPos;
  if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, GetUniqueId()))
    teamPos = role->GetTeamPosition();
  else
    teamPos = GetProjectileAimPos(mgr, -1.25f);
  zeus::CVector2f toTeamPos2d = (teamPos - GetTranslation()).toVec2f();
  float toTeamPosH = teamPos.z() - GetTranslation().z();
  if (toTeamPos2d.magSquared() > 1.f || std::fabs(toTeamPosH) > 2.5f) {
    pas::EStepDirection stepDir = GetStepDirection(toTeamPos2d);
    if (stepDir != pas::EStepDirection::Forward) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(stepDir, pas::EStepType::Normal));
    } else {
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCLocomotionCmd(x45c_steeringBehaviors.Arrival(*this, teamPos, 3.f), zeus::skZero3f, 1.f));
      zeus::CVector3f target = GetTranslation();
      target.z() = float(teamPos.z());
      zeus::CVector3f moveVec = x45c_steeringBehaviors.Arrival(*this, target, 2.5f);
      if (moveVec.magSquared() > 0.01f) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(moveVec, zeus::skZero3f, 3.f));
      }
    }
  } else {
    switch (mgr.GetActiveRandom()->Range(0, 2)) {
    case 0:
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Normal));
      break;
    case 1:
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Right, pas::EStepType::Normal));
      break;
    case 2:
      if (ShouldTurn(mgr, 30.f) && delta.canBeNormalized()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCLocomotionCmd(zeus::skZero3f, delta.normalized(), 1.f));
      }
      break;
    default:
      break;
    }
  }
  x450_bodyController->GetCommandMgr().DeliverTargetVector(delta);
}

void CWarWasp::ApplySeparationBehavior(CStateManager& mgr, float sep) {
  for (CEntity* ent : mgr.GetListeningAiObjectList()) {
    if (TCastToPtr<CPatterned> ai = ent) {
      if (ai.GetPtr() != this && ai->GetAreaIdAlways() == GetAreaIdAlways()) {
        float useSep = sep;
        if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, ai->GetUniqueId())) {
          if (role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Melee ||
              role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Ranged)
            useSep *= 2.f;
        }
        zeus::CVector3f separation = x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), useSep);
        if (!separation.isZero()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(separation, zeus::skZero3f, 1.f));
        }
      }
    }
  }
}

bool CWarWasp::PathToHiveIsClear(CStateManager& mgr) const {
  zeus::CVector3f delta = x3a0_latestLeashPosition - GetTranslation();
  if (GetTransform().basis[1].dot(delta) > 0.f) {
    zeus::CAABox aabb(GetTranslation() - 10.f, GetTranslation() + 10.f);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, aabb, CMaterialFilter::MakeInclude({EMaterialTypes::Character}), nullptr);
    float deltaMagSq = delta.magSquared();
    for (TUniqueId id : nearList) {
      if (const CWarWasp* other = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(id))) {
        if (other->GetUniqueId() != GetUniqueId() && other->x72e_30_isRetreating &&
            zeus::close_enough(other->x3a0_latestLeashPosition, x3a0_latestLeashPosition, 3.f)) {
          zeus::CVector3f waspDelta = other->GetTranslation() - GetTranslation();
          if (GetTransform().basis[1].dot(waspDelta) > 0.f && waspDelta.magSquared() < 3.f &&
              (other->GetTranslation() - x3a0_latestLeashPosition).magSquared() < deltaMagSq) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool CWarWasp::SteerToDeactivatePos(CStateManager& mgr, EStateMsg msg, float dt) {
  float distSq = (x3a0_latestLeashPosition - GetTranslation()).magSquared();
  if (distSq > 1.f + x570_cSphere.GetSphere().radius) {
    if (PathToHiveIsClear(mgr)) {
      zeus::CVector3f arrival1 = x45c_steeringBehaviors.Arrival(*this, x3a0_latestLeashPosition, 15.f);
      float maxSpeed = x450_bodyController->GetBodyStateInfo().GetMaxSpeed();
      float minMoveFactor;
      if (maxSpeed > 0.f)
        minMoveFactor =
            x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::ELocomotionAnim::Walk) * 0.5f / maxSpeed;
      else
        minMoveFactor = 1.f;
      float moveFactor = zeus::clamp(minMoveFactor, arrival1.magnitude(), 1.f);
      zeus::CVector3f moveVec;
      if (arrival1.canBeNormalized())
        moveVec = arrival1.normalized() * moveFactor;
      else
        moveVec = GetTransform().basis[1] * moveFactor;
      x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
      x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(moveFactor, moveFactor);
      if (distSq > 64.f + x570_cSphere.GetSphere().radius) {
        if (GetSearchPath() && !PathShagged(mgr, 0.f) && !GetSearchPath()->IsOver()) {
          CPatterned::PathFind(mgr, msg, dt);
        } else {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(moveVec, zeus::skZero3f, 1.f));
        }
      } else {
        RemoveMaterial(EMaterialTypes::Solid, mgr);
        zeus::CVector3f target = GetTranslation();
        target.z() = float(x3a0_latestLeashPosition.z());
        zeus::CVector3f arrival2 = x45c_steeringBehaviors.Arrival(*this, target, 2.5f);
        if (arrival2.magSquared() > 0.01f) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(arrival2, zeus::skZero3f, 3.f));
        }
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(moveVec, zeus::skZero3f, 1.f));
      }
    }
    return false;
  } else if (distSq > 0.01f) {
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    zeus::CQuaternion q;
    q.rotateZ(zeus::degToRad(180.f));
    SetTranslation(GetTranslation() * 0.9f + x3a0_latestLeashPosition * 0.1f);
    SetTransform(zeus::CQuaternion::slerpShort(zeus::CQuaternion(GetTransform().basis), x6a0_initialRot * q, 0.1f)
                     .normalized()
                     .toTransform(GetTranslation()));
    return false;
  }
  return true;
}

void CWarWasp::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    SetUpPathFindBehavior(mgr);
    break;
  case EStateMsg::Update: {
    if (mgr.GetPlayer().GetOrbitState() != CPlayer::EPlayerOrbitState::NoOrbit &&
        mgr.GetPlayer().GetOrbitTargetId() == GetUniqueId())
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    else
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    if (GetSearchPath() && !PathShagged(mgr, 0.f) && !GetSearchPath()->IsOver()) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      ApplyNormalSteering(mgr);
    }
    ApplySeparationBehavior(mgr, 9.f);
    float distTest = 2.f * x300_maxAttackRange;
    if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() > distTest * distTest) {
      x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
      x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    } else {
      x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    }
    break;
  }
  case EStateMsg::Deactivate:
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    break;
  }
}

void CWarWasp::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    SwarmRemove(mgr);
    CPatterned::Patrol(mgr, msg, dt);
    CPatterned::UpdateDest(mgr);
    x678_targetPos = x2e0_destPos;
    if (GetSearchPath())
      CPatterned::PathFind(mgr, msg, dt);
    break;
  case EStateMsg::Update:
    if (GetSearchPath() && !PathShagged(mgr, 0.f))
      CPatterned::PathFind(mgr, msg, dt);
    else
      CPatterned::Patrol(mgr, msg, dt);
    ApplySeparationBehavior(mgr, 9.f);
    break;
  default:
    break;
  }
}

void CWarWasp::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->Activate(mgr);
    if (x72e_26_initiallyInactive) {
      RemoveMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, mgr);
      x6a0_initialRot = zeus::CQuaternion(GetTransform().basis);
      zeus::CQuaternion q;
      q.rotateZ(mgr.GetActiveRandom()->Float() * zeus::degToRad(45.f) - zeus::degToRad(22.5f));
      SetTransform((x6a0_initialRot * q).normalized().toTransform(GetTranslation()));
      x568_stateProg = 0;
    } else {
      x568_stateProg = 3;
    }
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_stateProg = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero, x388_anim));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate) {
        x568_stateProg = 3;
      } else {
        zeus::CVector3f moveVec;
        for (CEntity* ent : mgr.GetListeningAiObjectList())
          if (TCastToPtr<CPatterned> act = ent)
            if (act.GetPtr() != this && act->GetAreaIdAlways() == GetAreaIdAlways())
              moveVec += x45c_steeringBehaviors.Separation(*this, act->GetTranslation(), 5.f) * (5.f * dt);
        if (!moveVec.isZero())
          ApplyImpulseWR(GetMoveToORImpulseWR(moveVec, dt), {});
      }
      break;
    default:
      break;
    }
    break;
  case EStateMsg::Deactivate:
    if (x72e_26_initiallyInactive) {
      AddMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, mgr);
      if (x328_26_solidCollision)
        x401_30_pendingDeath = true;
    }
    break;
  }
}

void CWarWasp::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x568_stateProg = 1;
    x72e_30_isRetreating = true;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    SwarmRemove(mgr);
    x674_aiMgr = kInvalidUniqueId;
    x678_targetPos = x3a0_latestLeashPosition;
    SetDestPos(x678_targetPos);
    if (GetSearchPath())
      CPatterned::PathFind(mgr, msg, dt);
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 1:
      if (SteerToDeactivatePos(mgr, msg, dt)) {
        RemoveMaterial(EMaterialTypes::Solid, mgr);
        x568_stateProg = 0;
      }
      break;
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        RemoveMaterial(EMaterialTypes::Character, EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
        mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
        x568_stateProg = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::One));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate)
        x568_stateProg = 3;
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void CWarWasp::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x72e_27_teamMatesMelee = true;
    x72e_25_canApplyDamage = false;
    if (x674_aiMgr != kInvalidUniqueId)
      x568_stateProg = CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Melee, mgr, x674_aiMgr, GetUniqueId()) ? 0 : 3;
    else
      x568_stateProg = 0;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_stateProg = 2;
      } else {
        zeus::CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
        zeus::CVector3f aimDelta = aimPos - GetTranslation();
        if (aimDelta.canBeNormalized())
          aimPos += aimDelta.normalized() * 7.5f;
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One, aimPos));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
        x568_stateProg = 3;
      } else {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
      }
      break;
    default:
      break;
    }
    break;
  case EStateMsg::Deactivate:
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Melee, mgr, x674_aiMgr, GetUniqueId(), false);
    x700_attackRemTime = CalcTimeToNextAttack(mgr);
    x72e_27_teamMatesMelee = false;
    break;
  }
}

void CWarWasp::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x568_stateProg = 0;
    x72e_24_jumpBackRepeat = true;
    SwarmRemove(mgr);
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Step) {
        x568_stateProg = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Step) {
        x568_stateProg = x72e_24_jumpBackRepeat ? 0 : 3;
        x72e_24_jumpBackRepeat = false;
      } else {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

zeus::CVector3f CWarWasp::CalcShuffleDest(const CStateManager& mgr) const {
  zeus::CVector2f aimPos2d = GetProjectileAimPos(mgr, -1.25f).toVec2f();
  zeus::CVector3f playerDir2d = mgr.GetPlayer().GetTransform().basis[1];
  playerDir2d.z() = 0.f;
  zeus::CVector3f useDir;
  if (playerDir2d.canBeNormalized())
    useDir = playerDir2d.normalized();
  else
    useDir = zeus::skForward;
  aimPos2d += useDir.toVec2f() * (7.5f + x300_maxAttackRange);
  zeus::CVector3f ret(aimPos2d);
  ret.z() = GetCloseInZBasis(mgr);
  return ret;
}

void CWarWasp::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x568_stateProg = 2;
    break;
  case EStateMsg::Update: {
    s32 numRoles = 0;
    if (TCastToConstPtr<CTeamAiMgr> aiMgr = mgr.GetObjectById(x674_aiMgr))
      numRoles = aiMgr->GetNumAssignedAiRoles();
    if (numRoles && x700_attackRemTime > 0.f) {
      zeus::CVector3f shuffleDest = CalcShuffleDest(mgr);
      float zDelta = shuffleDest.z() - GetTranslation().z();
      if (zDelta * zDelta > 1.f) {
        zeus::CVector3f dest = GetTranslation();
        dest.z() = float(shuffleDest.z());
        zeus::CVector3f moveVec = x45c_steeringBehaviors.Arrival(*this, dest, 1.f);
        if (moveVec.magSquared() > 0.01f)
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(moveVec, zeus::skZero3f, 1.f));
      } else {
        zeus::CVector3f moveVec = shuffleDest - GetTranslation();
        moveVec.z() = zDelta;
        if (moveVec.magSquared() > 64.f) {
          pas::EStepDirection stepDir = CPatterned::GetStepDirection(moveVec);
          if (stepDir != pas::EStepDirection::Forward) {
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(stepDir, pas::EStepType::Normal));
          } else {
            x450_bodyController->GetCommandMgr().DeliverCmd(
                CBCLocomotionCmd(x45c_steeringBehaviors.Seek(*this, shuffleDest), zeus::skZero3f, 1.f));
            ApplySeparationBehavior(mgr, 15.f);
          }
        } else {
          x568_stateProg = 3;
        }
      }
      x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    } else {
      x568_stateProg = 3;
    }
    break;
  }
  case EStateMsg::Deactivate:
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    break;
  }
}

void CWarWasp::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x72e_28_inProjectileAttack = true;
    if (x674_aiMgr != kInvalidUniqueId) {
      x568_stateProg = CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Ranged,
                                               mgr, x674_aiMgr, GetUniqueId()) ? 0 : 3;
    } else {
      x568_stateProg = 0;
    }
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x568_stateProg = 2;
      } else {
        SetDestPos(GetProjectileAimPos(mgr, -0.07f));
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::ESeverity::One, x2e0_destPos, false));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(x2e0_destPos - GetTranslation());
        x568_stateProg = 3;
      }
      break;
    default:
      break;
    }
    break;
  case EStateMsg::Deactivate:
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Ranged, mgr, x674_aiMgr, GetUniqueId(), false);
    x700_attackRemTime = CalcTimeToNextAttack(mgr);
    x72e_28_inProjectileAttack = false;
    break;
  }
}

s32 CWarWasp::GetAttackTeamSize(const CStateManager& mgr, s32 team) const {
  s32 count = 0;
  if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr)) {
    if (aimgr->IsPartOfTeam(GetUniqueId())) {
      for (const CTeamAiRole& role : aimgr->GetRoles()) {
        if (const CWarWasp* other = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(role.GetOwnerId()))) {
          if (team == other->x708_circleAttackTeam)
            ++count;
        }
      }
    }
  }
  return count;
}

float CWarWasp::CalcTimeToNextAttack(CStateManager& mgr) const {
  float mul = 1.f;
  if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr)) {
    const s32 maxCount =
        (x3fc_flavor == EFlavorType::Two) ? aimgr->GetMaxRangedAttackerCount() : aimgr->GetMaxMeleeAttackerCount();
    const s32 count = (x3fc_flavor == EFlavorType::Two) ? aimgr->GetNumAssignedOfRole(CTeamAiRole::ETeamAiRole::Ranged)
                                                        : aimgr->GetNumAssignedOfRole(CTeamAiRole::ETeamAiRole::Melee);
    if (count <= maxCount)
      mul *= 0.5f;
  }
  return (mgr.GetActiveRandom()->Float() * x308_attackTimeVariation + x304_averageAttackTime) * mul;
}

float CWarWasp::CalcOffTotemAngle(CStateManager& mgr) const {
  return mgr.GetActiveRandom()->Float() * zeus::degToRad(80.f) + zeus::degToRad(10.f);
}

void CWarWasp::JoinCircleAttackTeam(s32 unit, CStateManager& mgr) {
  if (!x6b0_circleBurstPos.isZero()) {
    if (x70c_initialCircleAttackTeam == -1) {
      x710_initialCircleAttackTeamUnit = GetAttackTeamSize(mgr, unit);
      x70c_initialCircleAttackTeam = unit;
    }
    x708_circleAttackTeam = unit;
    x700_attackRemTime = CalcTimeToNextAttack(mgr);
    x718_circleBurstOffTotemAngle = CalcOffTotemAngle(mgr);
  }
}

void CWarWasp::SetUpCircleTelegraphTeam(CStateManager& mgr) {
  if (x708_circleAttackTeam == -1) {
    if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr)) {
      if (aimgr->IsPartOfTeam(GetUniqueId()) && aimgr->GetMaxMeleeAttackerCount() > 0) {
        s32 teamUnit = 0;
        s32 targetUnitSize = 0;
        bool rejoinInitial = false;
        for (const CTeamAiRole& role : aimgr->GetRoles()) {
          if (const CWarWasp* other = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(role.GetOwnerId()))) {
            if (x70c_initialCircleAttackTeam != -1 &&
                other->x70c_initialCircleAttackTeam == x70c_initialCircleAttackTeam &&
                other->x708_circleAttackTeam >= 0) {
              teamUnit = other->x708_circleAttackTeam;
              rejoinInitial = true;
              break;
            }
            if (other->x708_circleAttackTeam > teamUnit) {
              teamUnit = other->x708_circleAttackTeam;
              targetUnitSize = 1;
            } else if (other->x708_circleAttackTeam == teamUnit) {
              ++targetUnitSize;
            }
          }
        }
        if (!rejoinInitial &&
            (x70c_initialCircleAttackTeam != -1 || targetUnitSize >= aimgr->GetMaxMeleeAttackerCount()))
          ++teamUnit;
        JoinCircleAttackTeam(teamUnit, mgr);
        x714_circleTelegraphSeekHeight = mgr.GetActiveRandom()->Float() * -0.5f;
      }
    }
  }
}

TUniqueId CWarWasp::GetAttackTeamLeader(const CStateManager& mgr, s32 team) const {
  if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr)) {
    if (aimgr->IsPartOfTeam(GetUniqueId())) {
      for (const CTeamAiRole& role : aimgr->GetRoles()) {
        if (const CWarWasp* other = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(role.GetOwnerId()))) {
          if (team == other->x708_circleAttackTeam)
            return role.GetOwnerId();
        }
      }
    }
  }
  return kInvalidUniqueId;
}

void CWarWasp::TryCircleTeamMerge(CStateManager& mgr) {
  if (x708_circleAttackTeam > 0) {
    if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr)) {
      if (aimgr->IsPartOfTeam(GetUniqueId())) {
        if (GetAttackTeamLeader(mgr, x708_circleAttackTeam) == GetUniqueId()) {
          if (GetAttackTeamSize(mgr, x708_circleAttackTeam - 1) == 0) {
            for (const CTeamAiRole& role : aimgr->GetRoles()) {
              if (const CWarWasp* other = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(role.GetOwnerId()))) {
                if (x708_circleAttackTeam == other->x708_circleAttackTeam)
                  JoinCircleAttackTeam(x708_circleAttackTeam - 1, mgr);
              }
            }
          }
        }
      }
    }
  }
}

float CWarWasp::GetTeamZStratum(s32 team) const {
  if (team > 0) {
    if ((team & 1) == 1)
      return -3.f - float(team / 2) * 3.f;
    else
      return float(team / 2) * 3.f;
  }
  return 0.f;
}

float CWarWasp::CalcSeekMagnitude(const CStateManager& mgr) const {
  static constexpr std::array Table{0.4f, 0.6f, 1.f};

  const float ret = ((x708_circleAttackTeam >= 0 && x708_circleAttackTeam < 3) ? Table[x708_circleAttackTeam] : 1.f) * 0.9f;
  if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr)) {
    if (aimgr->IsPartOfTeam(GetUniqueId())) {
      if (aimgr->GetMaxMeleeAttackerCount() > 1) {
        if (GetAttackTeamLeader(mgr, x708_circleAttackTeam) != GetUniqueId()) {
          const zeus::CVector3f fromPlatformCenter = GetTranslation() - x6b0_circleBurstPos;
          float minAngle = zeus::degToRad(360.f);
          for (const CTeamAiRole& role : aimgr->GetRoles()) {
            if (const CWarWasp* other = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(role.GetOwnerId()))) {
              if (x708_circleAttackTeam == other->x708_circleAttackTeam &&
                  GetTransform().basis[1].dot(other->GetTranslation() - GetTranslation()) > 0.f) {
                const float angle =
                    zeus::CVector3f::getAngleDiff(fromPlatformCenter, other->GetTranslation() - x6b0_circleBurstPos);
                if (angle < minAngle)
                  minAngle = angle;
              }
            }
          }
          if (minAngle < zeus::degToRad(30.f))
            return 0.8f;
          if (minAngle > zeus::degToRad(50.f))
            return 1.f;
        }
      }
    }
  }
  return ret;
}

void CWarWasp::UpdateTelegraphMoveSpeed(CStateManager& mgr) {
  TUniqueId leaderId = GetAttackTeamLeader(mgr, x708_circleAttackTeam);
  if (const CWarWasp* other = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(leaderId))) {
    if (leaderId == GetUniqueId()) {
      float cycleTime = x330_stateMachineState.GetTime() - std::trunc(x330_stateMachineState.GetTime() / 2.8f) * 2.8f;
      if (cycleTime < 2.f) {
        x3b4_speed = x6fc_initialSpeed;
      } else {
        float t = (cycleTime - 2.f) / 0.8f;
        x3b4_speed = ((1.f - t) * 0.7f + 2.f * t) * x6fc_initialSpeed;
      }
    } else {
      x3b4_speed = other->x3b4_speed;
    }
  } else {
    x3b4_speed = x6fc_initialSpeed;
  }
}

void CWarWasp::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    RemoveMaterial(EMaterialTypes::Orbit, mgr);
    mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
    SwarmAdd(mgr);
    SetUpCircleTelegraphTeam(mgr);
    break;
  case EStateMsg::Update:
    if (!x6b0_circleBurstPos.isZero()) {
      TryCircleTeamMerge(mgr);
      zeus::CVector3f closeInDelta = GetTranslation() - x6b0_circleBurstPos;
      closeInDelta.z() = 0.f;
      zeus::CVector3f moveVec = GetTransform().basis[1];
      if (closeInDelta.canBeNormalized()) {
        zeus::CVector3f closeInDeltaNorm = closeInDelta.normalized();
        moveVec = closeInDeltaNorm.cross(zeus::skUp);
        zeus::CVector3f seekOrigin = x6b0_circleBurstPos + closeInDeltaNorm * x2fc_minAttackRange;
        if (x708_circleAttackTeam > 0)
          moveVec = moveVec * -1.f;
        float seekHeight = x714_circleTelegraphSeekHeight + GetTeamZStratum(x708_circleAttackTeam);
        float seekMag = CalcSeekMagnitude(mgr);
        moveVec =
            x45c_steeringBehaviors.Seek(*this, seekOrigin + moveVec * 5.f + zeus::CVector3f(0.f, 0.f, seekHeight)) *
            seekMag;
      }
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(moveVec, zeus::skZero3f, 1.f));
      UpdateTelegraphMoveSpeed(mgr);
    }
    break;
  case EStateMsg::Deactivate:
    AddMaterial(EMaterialTypes::Orbit, mgr);
    break;
  }
}

void CWarWasp::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (x704_dodgeDir != pas::EStepDirection::Invalid) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(x704_dodgeDir, pas::EStepType::Dodge));
      x568_stateProg = 2;
    }
    break;
  case EStateMsg::Update:
    if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Step) {
      x568_stateProg = 3;
    } else {
      x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    }
    break;
  case EStateMsg::Deactivate:
    x704_dodgeDir = pas::EStepDirection::Invalid;
    break;
  }
}

void CWarWasp::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    SwarmRemove(mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal5);
    break;
  case EStateMsg::Update:
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(x45c_steeringBehaviors.Flee2D(*this, mgr.GetPlayer().GetTranslation().toVec2f()),
                         zeus::skZero3f, 1.f));
    break;
  case EStateMsg::Deactivate:
    x400_24_hitByPlayerProjectile = false;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    break;
  }
}

void CWarWasp::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x72e_27_teamMatesMelee = true;
    x72e_25_canApplyDamage = true;
    x568_stateProg = 0;
    x3b4_speed = x6fc_initialSpeed;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_stateProg = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Eight));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
        x568_stateProg = 3;
      } else if (GetTransform().basis[1].dot(x6b0_circleBurstPos - GetTranslation()) > 0.f) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(x6b0_circleBurstPos - GetTranslation());
      }
      break;
    default:
      break;
    }
    break;
  case EStateMsg::Deactivate:
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Melee, mgr, x674_aiMgr, GetUniqueId(), false);
    x72e_27_teamMatesMelee = false;
    x708_circleAttackTeam = -1;
    x718_circleBurstOffTotemAngle = CalcOffTotemAngle(mgr);
    break;
  }
}

bool CWarWasp::InAttackPosition(CStateManager& mgr, float arg) {
  zeus::CVector3f delta = GetProjectileAimPos(mgr, -1.25f) - GetTranslation();
  float negTest = x2fc_minAttackRange - 5.f;
  float posTest = 5.f + x300_maxAttackRange;
  float magSq = delta.magSquared();
  bool ret = magSq > negTest * negTest && magSq < posTest * posTest && !ShouldTurn(mgr, 45.f);
  if (ret && delta.canBeNormalized()) {
    float deltaMag = delta.magnitude();
    ret = mgr.RayStaticIntersection(
                 GetTranslation(), delta / deltaMag, deltaMag,
                 CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Player}))
              .IsInvalid();
  }
  return ret;
}

bool CWarWasp::Leash(CStateManager& mgr, float arg) {
  if ((x3a0_latestLeashPosition - GetTranslation()).magSquared() > x3c8_leashRadius * x3c8_leashRadius) {
    if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() >
            x3cc_playerLeashRadius * x3cc_playerLeashRadius &&
        x3d4_curPlayerLeashTime > x3d0_playerLeashTime) {
      return true;
    }
  }
  return false;
}

bool CWarWasp::PathShagged(CStateManager& mgr, float arg) {
  if (CPathFindSearch* pf = GetSearchPath())
    return pf->IsShagged();
  return false;
}

bool CWarWasp::AnimOver(CStateManager& mgr, float arg) { return x568_stateProg == 3; }

bool CWarWasp::ShouldAttack(CStateManager& mgr, float arg) {
  if (x700_attackRemTime <= 0.f) {
    if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, GetUniqueId())) {
      if (role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Melee && !mgr.GetPlayer().IsInWaterMovement()) {
        if (TCastToPtr<CTeamAiMgr> tmgr = mgr.ObjectById(x674_aiMgr)) {
          if (!tmgr->HasMeleeAttackers()) {
            return !IsPatternObstructed(mgr, GetTranslation(), GetProjectileAimPos(mgr, -1.25f));
          }
        }
      }
    }
    if (x3fc_flavor != EFlavorType::Two && !mgr.GetPlayer().IsInWaterMovement()) {
      return !IsPatternObstructed(mgr, GetTranslation(), GetProjectileAimPos(mgr, -1.25f));
    }
  }
  return false;
}

bool CWarWasp::InPosition(CStateManager& mgr, float arg) {
  if (CPathFindSearch* pf = GetSearchPath()) {
    return pf->IsOver();
  } else {
    return (x678_targetPos - GetTranslation()).magSquared() < 1.f;
  }
}

bool CWarWasp::ShouldTurn(CStateManager& mgr, float arg) {
  return zeus::CVector2f::getAngleDiff(GetTransform().basis[1].toVec2f(),
                                       (mgr.GetPlayer().GetTranslation() - GetTranslation()).toVec2f()) >
         zeus::degToRad(arg);
}

bool CWarWasp::HearShot(CStateManager& mgr, float arg) {
  if (x72e_31_heardNoise || x400_24_hitByPlayerProjectile)
    return true;
  if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr))
    return aimgr->GetNumRoles() != 0;
  return false;
}

bool CWarWasp::ShouldFire(CStateManager& mgr, float arg) {
  if (x700_attackRemTime <= 0.f) {
    if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, GetUniqueId())) {
      if (role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Ranged) {
        zeus::CVector3f delta = GetProjectileAimPos(mgr, -1.25f) - GetTranslation();
        if (delta.canBeNormalized() && GetTransform().basis[1].dot(delta.normalized()) >= 0.906f) {
          if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x674_aiMgr)) {
            return !aimgr->HasRangedAttackers();
          }
        }
      }
    } else if (x3fc_flavor == EFlavorType::Two) {
      zeus::CVector3f delta = GetProjectileAimPos(mgr, -1.25f) - GetTranslation();
      if (delta.canBeNormalized()) {
        return GetTransform().basis[1].dot(delta.normalized()) >= 0.906f;
      }
    }
  }
  return false;
}

bool CWarWasp::ShouldDodge(CStateManager& mgr, float arg) {
  zeus::CAABox aabb(GetTranslation() - 7.5f, GetTranslation() + 7.5f);
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, aabb, CMaterialFilter::MakeInclude({EMaterialTypes::Projectile}), nullptr);
  for (TUniqueId id : nearList) {
    if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(id)) {
      if (mgr.GetPlayer().GetUniqueId() == proj->GetOwnerId()) {
        zeus::CVector3f delta = proj->GetTranslation() - GetTranslation();
        if (zeus::CVector3f::getAngleDiff(GetTransform().basis[1], delta) < zeus::degToRad(45.f)) {
          x704_dodgeDir =
              GetTransform().basis[0].dot(delta) > 0.f ? pas::EStepDirection::Right : pas::EStepDirection::Left;
          return true;
        }
      }
    }
  }
  return false;
}

bool CWarWasp::CheckCircleAttackSpread(const CStateManager& mgr, s32 team) const {
  if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr)) {
    const s32 teamSize = GetAttackTeamSize(mgr, team);
    if (teamSize == 1)
      return true;
    const TUniqueId leaderId = GetAttackTeamLeader(mgr, team);
    if (const CWarWasp* leaderWasp = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(leaderId))) {
      const zeus::CVector3f platformToLeaderWasp = leaderWasp->GetTranslation() - x6b0_circleBurstPos;
      float maxAng = 0.f;
      for (const CTeamAiRole& role : aimgr->GetRoles()) {
        if (role.GetOwnerId() == leaderId)
          continue;
        if (const CWarWasp* wasp2 = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(role.GetOwnerId()))) {
          if (wasp2->x708_circleAttackTeam == team) {
            if (leaderWasp->GetTransform().basis[1].dot(wasp2->GetTranslation() - leaderWasp->GetTranslation()) > 0.f)
              return false;
            const float angle =
                zeus::CVector3f::getAngleDiff(wasp2->GetTranslation() - x6b0_circleBurstPos, platformToLeaderWasp);
            if (angle > maxAng)
              maxAng = angle;
          }
        }
      }
      return maxAng < zeus::degToRad(40.f) * float(teamSize - 1) + zeus::degToRad(20.f);
    }
  }
  return false;
}

bool CWarWasp::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  if (x708_circleAttackTeam == 0 && !mgr.GetPlayer().IsInWaterMovement()) {
    if (TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(x674_aiMgr)) {
      if (CheckCircleAttackSpread(mgr, x708_circleAttackTeam)) {
        TUniqueId leaderId = GetAttackTeamLeader(mgr, x708_circleAttackTeam);
        if (leaderId == GetUniqueId()) {
          if (x700_attackRemTime <= 0.f &&
              (mgr.GetPlayer().GetTranslation().toVec2f() - x6b0_circleBurstPos.toVec2f()).magSquared() < 90.25f) {
            zeus::CVector3f fromPlatformCenter = GetTranslation() - x6b0_circleBurstPos;
            zeus::CVector3f thresholdVec = x6bc_circleBurstDir;
            if (x718_circleBurstOffTotemAngle <= zeus::degToRad(90.f)) {
              thresholdVec =
                  zeus::CVector3f::slerp(x6c8_circleBurstRight, x6bc_circleBurstDir, x718_circleBurstOffTotemAngle);
            } else {
              thresholdVec = zeus::CVector3f::slerp(x6bc_circleBurstDir, -x6c8_circleBurstRight,
                                                    x718_circleBurstOffTotemAngle - zeus::degToRad(90.f));
            }
            if (zeus::CVector3f::getAngleDiff(thresholdVec, fromPlatformCenter) < zeus::degToRad(10.f))
              return CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Melee, mgr, x674_aiMgr, GetUniqueId());
          }
        } else {
          if (const CWarWasp* leaderWasp = CPatterned::CastTo<CWarWasp>(mgr.GetObjectById(leaderId))) {
            if (leaderWasp->x72e_27_teamMatesMelee) {
              return CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Melee, mgr, x674_aiMgr, GetUniqueId());
            }
          }
        }
      }
    }
  }
  return false;
}

CPathFindSearch* CWarWasp::GetSearchPath() { return &x590_pfSearch; }

CProjectileInfo* CWarWasp::GetProjectileInfo() { return &x6d4_projectileInfo; }
} // namespace urde::MP1
