#include "Runtime/MP1/World/CBeetle.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

CBeetle::CBeetle(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                 CModelData&& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor,
                 CBeetle::EEntranceType entranceType, const CDamageInfo& touchDamage,
                 const CDamageVulnerability& platingVuln, const zeus::CVector3f& tailAimReference,
                 float initialAttackDelay, float retreatTime, float f3, const CDamageVulnerability& tailVuln,
                 const CActorParameters& aParams, const std::optional<CStaticRes>& tailModel)
: CPatterned(ECharacter::Beetle, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Ground,
             EColliderType::One, EBodyType::BiPedal, aParams, EKnockBackVariant(flavor))
, x56c_entranceType(entranceType)
, x574_tailAimReference(tailAimReference)
, x580_f3(f3)
, x584_touchDamage(touchDamage)
, x5ac_tailModel(tailModel ? std::optional<CModelData>(CModelData(*tailModel)) : std::nullopt)
, x5fc_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x744_platingVuln(platingVuln)
, x7ac_tailVuln(tailVuln)
, x814_attackDelayTimer(initialAttackDelay)
, x834_retreatTime(retreatTime) {
  x5a0_headbuttDist = GetAnimationDistance(CPASAnimParmData(7, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(1)));
  x5a4_jumpBackwardDist =
      x64_modelData->GetScale().y() *
      GetAnimationDistance(CPASAnimParmData(3, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(0)));
  MakeThermalColdAndHot();
  if (x3fc_flavor == EFlavorType::One)
    x460_knockBackController.SetLocomotionDuringElectrocution(true);
}

void CBeetle::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CBeetle::SquadAdd(CStateManager& mgr) {
  if (x570_aiMgr != kInvalidUniqueId)
    if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x570_aiMgr))
      aimgr->AssignTeamAiRole(*this, CTeamAiRole::ETeamAiRole::Melee, CTeamAiRole::ETeamAiRole::Unknown,
                              CTeamAiRole::ETeamAiRole::Invalid);
}

void CBeetle::SquadRemove(CStateManager& mgr) {
  if (x570_aiMgr != kInvalidUniqueId)
    if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x570_aiMgr))
      if (aimgr->IsPartOfTeam(GetUniqueId()))
        aimgr->RemoveTeamAiRole(GetUniqueId());
}

void CBeetle::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;
  if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
    x450_bodyController->SetLocomotionType(role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Melee
                                               ? pas::ELocomotionType::Lurk
                                               : pas::ELocomotionType::Relaxed);
  } else {
    SquadAdd(mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
  }
  x460_knockBackController.SetAutoResetImpulse(IsOnGround());
  if (x814_attackDelayTimer > 0.f)
    x814_attackDelayTimer -= dt;
  if ((x824_predictPos - GetTranslation()).toVec2f().magSquared() > 0.1f * dt)
    x820_posDeviationCounter += 1;
  else
    x820_posDeviationCounter = 0;
  CPatterned::Think(dt, mgr);
  x824_predictPos = x138_velocity * dt + GetTranslation();
}

void CBeetle::SetupRetreatPoints(CStateManager& mgr) {
  for (const auto& conn : GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::Retreat && conn.x4_msg == EScriptObjectMessage::Follow) {
      if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId))) {
        x6e0_retreatPoints.push_back(wp->GetTranslation());
        if (x6e0_retreatPoints.size() == 8)
          break;
      }
    }
  }
}

void CBeetle::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  bool forward = true;
  switch (msg) {
  case EScriptObjectMessage::Activate:
    SquadAdd(mgr);
    break;
  case EScriptObjectMessage::Deactivate:
  case EScriptObjectMessage::Deleted:
    SquadRemove(mgr);
    break;
  case EScriptObjectMessage::OnFloor:
    forward = false;
    SetMomentumWR(zeus::skZero3f);
    x328_27_onGround = true;
    break;
  case EScriptObjectMessage::Falling:
    if (!x450_bodyController->IsFrozen()) {
      SetMomentumWR({0.f, 0.f, -(GetGravityConstant() * xe8_mass)});
      x328_27_onGround = false;
    }
    forward = false;
    break;
  case EScriptObjectMessage::InitializedInArea:
    if (x570_aiMgr == kInvalidUniqueId) {
      x570_aiMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
      if (GetActive())
        SquadAdd(mgr);
    }
    SetupRetreatPoints(mgr);
    x5fc_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  default:
    break;
  }
  if (forward)
    CPatterned::AcceptScriptMsg(msg, sender, mgr);
}

void CBeetle::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (x400_25_alive) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::EPlayerVisor::XRay:
      x42c_color.a() = 76.5f / 255.f;
      break;
    case CPlayerState::EPlayerVisor::Thermal:
      if (x838_25_burrowing)
        x42c_color.a() = x830_intoGroundFactor;
      else
        x42c_color.a() = 1.f;
      break;
    default:
      x42c_color.a() = 1.f;
      break;
    }
  }
  CPatterned::PreRender(mgr, frustum);
}

void CBeetle::Render(CStateManager& mgr) {
  if (x3fc_flavor == EFlavorType::One && x400_25_alive) {
    zeus::CTransform tailXf = GetLctrTransform("Target_Tail"sv);
    if (x428_damageCooldownTimer >= 0.f && x42c_color.a() == 1.f) {
      if (x5ac_tailModel) {
        CModelFlags flags(2, 0, 3, zeus::skWhite);
        flags.addColor = x42c_color;
        x5ac_tailModel->Render(mgr, tailXf, x90_actorLights.get(), flags);
      }
    } else if (x5ac_tailModel) {
      constexpr CModelFlags flags(0, 0, 3, zeus::skWhite);
      x5ac_tailModel->Render(mgr, tailXf, x90_actorLights.get(), flags);
    }
  }
  CPatterned::Render(mgr);
}

const CDamageVulnerability* CBeetle::GetDamageVulnerability() const {
  if (x838_25_burrowing)
    return &CDamageVulnerability::PassThroughVulnerabilty();
  if (x3fc_flavor == EFlavorType::One)
    return x450_bodyController->IsOnFire() ? &x7ac_tailVuln : &x744_platingVuln;
  return CAi::GetDamageVulnerability();
}

const CDamageVulnerability* CBeetle::GetDamageVulnerability(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                            const CDamageInfo& dInfo) const {
  if (x838_25_burrowing)
    return &CDamageVulnerability::PassThroughVulnerabilty();
  if (x3fc_flavor == EFlavorType::One) {
    if (dInfo.GetWeaponMode().IsComboed() && dInfo.GetWeaponMode().GetType() == EWeaponType::Wave)
      return &x7ac_tailVuln;
    if (GetTransform().basis[1].dot(dir) > 0.f &&
        GetTransform().basis[1].dot(zeus::CUnitVector3f(pos - GetBoundingBox().center())) < -0.5f)
      return &x7ac_tailVuln;
    else
      return &x744_platingVuln;
  }
  return GetDamageVulnerability();
}

zeus::CVector3f CBeetle::GetOrbitPosition(const CStateManager& mgr) const {
  zeus::CVector3f ret = CPatterned::GetOrbitPosition(mgr);
  ret.z() = float(GetBoundingBox().center().z());
  return ret;
}

zeus::CVector3f CBeetle::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (x3fc_flavor == EFlavorType::One || x3fc_flavor == EFlavorType::Two) {
    zeus::CTransform tailXf = GetLctrTransform("Target_Tail"sv);
    zeus::CVector3f scaleRange = tailXf * x574_tailAimReference - GetTranslation();
    zeus::CAABox aabb = GetBoundingBox();
    float minFactor = 10.f;
    for (int i = 0; i < 3; ++i) {
      if (scaleRange[i] < 0.f) {
        float factor = (aabb.min[i] - GetTranslation()[i]) / scaleRange[i];
        if (factor < minFactor)
          minFactor = factor;
      } else if (scaleRange[i] > 0.f) {
        float factor = (aabb.max[i] - GetTranslation()[i]) / scaleRange[i];
        if (factor < minFactor)
          minFactor = factor;
      }
    }
    return scaleRange * minFactor + GetTranslation();
  } else {
    return CPhysicsActor::GetAimPosition(mgr, dt);
  }
}

EWeaponCollisionResponseTypes CBeetle::GetCollisionResponseType(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                                const CWeaponMode& wMode,
                                                                EProjectileAttrib attribs) const {
  if (x450_bodyController->IsFrozen() && wMode.GetType() == EWeaponType::Ice)
    return EWeaponCollisionResponseTypes::None;
  if (x838_25_burrowing)
    return EWeaponCollisionResponseTypes::Unknown69;
  if (x3fc_flavor == EFlavorType::One) {
    if (GetTransform().basis[1].dot(dir) > 0.f &&
        GetTransform().basis[1].dot(zeus::CUnitVector3f(pos - GetBoundingBox().center())) < -0.5f)
      return EWeaponCollisionResponseTypes::Unknown44;
    if (!x744_platingVuln.WeaponHurts(wMode, false))
      return EWeaponCollisionResponseTypes::Unknown69;
  }
  return EWeaponCollisionResponseTypes::Unknown19;
}

void CBeetle::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case EUserEventType::ChangeMaterial:
    AddMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, mgr);
    x328_25_verticalMovement = true;
    RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
    handled = true;
    break;
  case EUserEventType::GenerateEnd:
    x328_25_verticalMovement = false;
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    handled = true;
    break;
  case EUserEventType::DamageOn: {
    zeus::CVector3f center =
        GetTransform() * (x64_modelData->GetScale() * GetLocatorTransform("LCTR_GARMOUTH"sv).origin);
    zeus::CVector3f extent = x64_modelData->GetScale() * zeus::CVector3f(2.f, 2.f, 0.5f);
    if (zeus::CAABox(center - extent, center + extent).intersects(mgr.GetPlayer().GetBoundingBox())) {
      mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), x584_touchDamage,
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
    }
    handled = true;
    break;
  }
  case EUserEventType::Delete:
    handled = true;
    break;
  default:
    break;
  }
  if (!handled)
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CBeetle::CollidedWith(TUniqueId uid, const CCollisionInfoList& list, CStateManager& mgr) {
  static CMaterialList envList(EMaterialTypes::Ceiling, EMaterialTypes::Wall);
  for (const auto& c : list) {
    if (c.GetMaterialLeft().Intersection(envList) != 0 &&
        x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
      SetVelocityWR(zeus::skZero3f);
    }
  }
  CPatterned::CollidedWith(uid, list, mgr);
}

void CBeetle::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  if (x400_25_alive) {
    if (x3fc_flavor == EFlavorType::One) {
      zeus::CTransform backupXf = GetTransform();
      SetTransform(GetLctrTransform("Target_Tail"sv));
      SendScriptMsgs(EScriptObjectState::DeathRattle, mgr, EScriptObjectMessage::None);
      SetTransform(backupXf);
    }
    CPatterned::Death(mgr, direction, state);
  }
}

void CBeetle::TakeDamage(const zeus::CVector3f& direction, float magnitude) { x428_damageCooldownTimer = 0.33f; }

bool CBeetle::IsListening() const { return true; }

zeus::CVector3f CBeetle::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                   const zeus::CVector3f& aimPos) const {
  float midRange = (x2fc_minAttackRange + x300_maxAttackRange) * 0.5f;
  zeus::CVector3f playerToThis = GetTranslation() - aimPos;
  if (playerToThis.canBeNormalized())
    return aimPos + playerToThis.normalized() * midRange;
  else
    return aimPos + GetTransform().basis[1] * midRange;
}

void CBeetle::FollowPattern(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x568_stateProg = 1;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 1:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Step) {
        x568_stateProg = 3;
      } else if (IsOnGround()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Normal));
      }
      break;
    case 3:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Step && IsOnGround()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Right, pas::EStepType::Normal));
        x568_stateProg = 2;
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Step)
        x568_stateProg = x814_attackDelayTimer <= 0.f ? 4 : 1;
      break;
    default:
      break;
    }
    x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    break;
  default:
    break;
  }
}

void CBeetle::RefinePathFindDest(CStateManager& mgr, zeus::CVector3f& dest) {
  dest = mgr.GetPlayer().GetTranslation();
  if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
    dest = role->GetTeamPosition();
  } else {
    zeus::CVector3f thisToDest = dest - GetTranslation();
    dest += (thisToDest.canBeNormalized() ? thisToDest.normalized() : GetTransform().basis[1]) *
            (0.5f * (x2fc_minAttackRange + x300_maxAttackRange));
  }
}

void CBeetle::SeparateFromMelees(CStateManager& mgr) {
  for (CEntity* ent : mgr.GetListeningAiObjectList()) {
    if (TCastToPtr<CPatterned> ai = ent) {
      if (ai.GetPtr() != this && ai->GetAreaIdAlways() == GetAreaIdAlways()) {
        float dist = 4.f;
        if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, ai->GetUniqueId()))
          if (role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Melee)
            dist *= 2.f;
        zeus::CVector3f move = x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), dist);
        if (!move.isZero())
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
      }
    }
  }
}

void CBeetle::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (GetSearchPath()) {
      RefinePathFindDest(mgr, x2e0_destPos);
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case EStateMsg::Update: {
    zeus::CVector3f dest = mgr.GetPlayer().GetTranslation();
    RefinePathFindDest(mgr, dest);
    zeus::CVector3f delta = dest - GetTranslation();
    zeus::CVector3f move;
    if (!PathShagged(mgr, 0.f) && !x5fc_pathFindSearch.IsOver()) {
      CPatterned::PathFind(mgr, msg, dt);
      move = x450_bodyController->GetCommandMgr().GetMoveVector();
    } else {
      move = x45c_steeringBehaviors.Arrival(*this, dest, 5.f);
    }
    if (GetTransform().basis[1].dot(move) >= 0.f || GetTransform().basis[1].dot(delta) <= 0.f) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
    } else {
      zeus::CVector3f face = delta.canBeNormalized() ? delta.normalized() : GetTransform().basis[1];
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, face, 1.f));
    }
    SeparateFromMelees(mgr);
    break;
  }
  default:
    break;
  }
}

void CBeetle::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId()))
      x2e0_destPos = role->GetTeamPosition();
    else
      x2e0_destPos = mgr.GetPlayer().GetTranslation();
    x2dc_destObj = mgr.GetPlayer().GetUniqueId();
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
  }
}

void CBeetle::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (x56c_entranceType == EEntranceType::FacePlayer || x450_bodyController->GetActive()) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
      SetTransform(zeus::lookAt(GetTranslation(), mgr.GetPlayer().GetTranslation()));
    } else {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero, x388_anim));
    }
    if (!x450_bodyController->GetActive())
      x450_bodyController->Activate(mgr);
    RemoveMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, mgr);
    x568_stateProg = 0;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_stateProg = 2;
        x5a8_animTimeRem = x450_bodyController->GetAnimTimeRemaining();
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate) {
        x568_stateProg = 4;
      } else if (x68_material.HasMaterial(EMaterialTypes::Solid) && x5a8_animTimeRem > 0.f) {
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        mgr.BuildNearList(nearList, zeus::CAABox(GetTranslation() - 5.f, GetTranslation() + 5.f),
                          CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), this);
        if (!nearList.empty()) {
          zeus::CVector3f totalSep;
          float sepFactor = 5.f * dt / x5a8_animTimeRem;
          for (TUniqueId id : nearList) {
            if (CActor* act = static_cast<CActor*>(mgr.ObjectById(id))) {
              zeus::CVector3f sep = x45c_steeringBehaviors.Separation(*this, act->GetTranslation(), 5.f);
              sep.z() = std::max(0.f, float(sep.z()));
              totalSep += sep * sepFactor;
            }
          }
          SetTranslation(GetTranslation() + totalSep);
        }
      }
      break;
    default:
      break;
    }
    break;
  case EStateMsg::Deactivate:
    AddMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, EMaterialTypes::GroundCollider, mgr);
    x328_25_verticalMovement = false;
    x838_25_burrowing = false;
    if (x328_26_solidCollision)
      DeathDelete(mgr);
    break;
  default:
    break;
  }
}

void CBeetle::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x568_stateProg = 0;
    SquadRemove(mgr);
    x818_stateFinishTimer = 0.f;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        RemoveMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, EMaterialTypes::Target, EMaterialTypes::Orbit,
                       mgr);
        mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
        x838_25_burrowing = true;
        x5a8_animTimeRem = x450_bodyController->GetAnimTimeRemaining();
        x568_stateProg = 2;
      } else if (IsOnGround()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::One));
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
            x45c_steeringBehaviors.Seek(*this, mgr.GetPlayer().GetTranslation()), zeus::skZero3f, 1.f));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate) {
        x568_stateProg = 3;
        x830_intoGroundFactor = 0.f;
        auto aabb = GetBoundingBox();
        SetTranslation((aabb.max.z() - aabb.min.z()) * 3.f * zeus::skDown + GetTranslation());
      } else {
        float remTime = x450_bodyController->GetAnimTimeRemaining();
        x830_intoGroundFactor = x5a8_animTimeRem > 0.f ? remTime / x5a8_animTimeRem : 0.f;
      }
      break;
    case 3:
      x818_stateFinishTimer += dt;
      if (x818_stateFinishTimer >= 0.75f) {
        SendScriptMsgs(EScriptObjectState::DeactivateState, mgr, EScriptObjectMessage::None);
        mgr.FreeScriptObject(GetUniqueId());
        x568_stateProg = 4;
        x830_intoGroundFactor = 0.f;
      } else {
        auto aabb = GetBoundingBox();
        SetTranslation((aabb.max.z() - aabb.min.z()) * 4.f * zeus::skDown * dt + GetTranslation());
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

void CBeetle::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x838_26_canSkid = false;
    x838_24_hitSomething = false;
    x2e0_destPos = mgr.GetPlayer().GetTranslation();
    x2e0_destPos.z() = GetTranslation().z();
    x568_stateProg = 0;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_stateProg = 1;
        x838_26_canSkid = true;
      } else if (IsOnGround()) {
        zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
        aimPos.z() = GetTranslation().z();
        zeus::CVector3f aimDelta = aimPos - GetTranslation();
        aimDelta.z() = 0.f;
        if (aimDelta.canBeNormalized())
          aimPos += aimDelta.normalized() * 5.f;
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One, aimPos));
      }
      break;
    case 1:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
        x568_stateProg = 4;
      } else if (IsOnGround()) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(x2e0_destPos - GetTranslation());
      } else {
        x568_stateProg = 2;
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::MeleeAttack)
        x568_stateProg = 4;
      break;
    default:
      break;
    }
    if (x328_26_solidCollision)
      x838_24_hitSomething = true;
    break;
  case EStateMsg::Deactivate:
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Melee, mgr, x570_aiMgr, GetUniqueId(), true);
    x814_attackDelayTimer = mgr.GetActiveRandom()->Float() * x308_attackTimeVariation + x304_averageAttackTime;
    break;
  }
}

void CBeetle::JumpBack(CStateManager&, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x568_stateProg = 0;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Step) {
        x568_stateProg = 2;
      } else if (IsOnGround()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Step)
        x568_stateProg = 4;
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void CBeetle::DoubleSnap(CStateManager&, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x568_stateProg = 0;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_stateProg = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Zero));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::MeleeAttack)
        x568_stateProg = 4;
      else
        x450_bodyController->GetCommandMgr().DeliverTargetVector(x2e0_destPos - GetTranslation());
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void CBeetle::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x328_24_inPosition = false;
    break;
  case EStateMsg::Update: {
    zeus::CVector3f playerPos = mgr.GetPlayer().GetTranslation();
    zeus::CVector3f playerToThis = GetTranslation() - playerPos;
    float midRange = 0.5f * (x2fc_minAttackRange + x300_maxAttackRange);
    zeus::CVector3f playerLimit;
    if (playerToThis.canBeNormalized())
      playerLimit = playerToThis.normalized() * midRange + playerPos;
    else
      playerLimit = GetTransform().basis[1] * midRange + playerPos;
    if ((GetTranslation() - playerLimit).magSquared() > 4.f) {
      pas::EStepDirection dir = GetStepDirection(-playerToThis);
      switch (dir) {
      case pas::EStepDirection::Forward:
      case pas::EStepDirection::Backward: {
        zeus::CVector3f move = x45c_steeringBehaviors.Arrival(*this, x2e0_destPos, x300_maxAttackRange);
        if (GetTransform().basis[1].dot(move) >= 0.f || GetTransform().basis[1].dot(playerToThis) >= 0.f) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
        } else {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
              move, playerToThis.canBeNormalized() ? -playerToThis.normalized() : GetTransform().basis[1], 1.f));
        }
        break;
      }
      case pas::EStepDirection::Left:
      case pas::EStepDirection::Right: {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(dir, pas::EStepType::Normal));
        break;
      }
      default:
        break;
      }
      x450_bodyController->GetCommandMgr().DeliverTargetVector(-playerToThis);
    } else {
      x328_24_inPosition = true;
    }
    break;
  }
  default:
    break;
  }
}

void CBeetle::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x568_stateProg = 0;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Turn) {
        x568_stateProg = 2;
      } else {
        zeus::CVector3f thisToPlayer = mgr.GetPlayer().GetTranslation() - GetTranslation();
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
            zeus::skZero3f,
            (thisToPlayer.magnitude() > FLT_EPSILON) ? thisToPlayer.normalized() : zeus::skZero3f, 1.f));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Turn)
        x568_stateProg = 4;
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void CBeetle::Skid(CStateManager&, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (IsOnGround() && x838_26_canSkid) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCSlideCmd(pas::ESlideType::Zero, GetTransform().basis[1]));
      x568_stateProg = 2;
    }
    break;
  case EStateMsg::Update:
    if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Slide)
      x568_stateProg = 4;
    break;
  case EStateMsg::Deactivate:
    x838_26_canSkid = false;
    break;
  default:
    break;
  }
}

void CBeetle::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCTauntCmd(mgr.GetActiveRandom()->Float() < 0.75f ? pas::ETauntType::One : pas::ETauntType::Zero));
    x568_stateProg = 2;
    break;
  case EStateMsg::Update:
    if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Taunt)
      x568_stateProg = 4;
    else
      x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    break;
  default:
    break;
  }
}

s32 CBeetle::FindFurthestRetreatPoint(CStateManager& mgr) const {
  s32 ret = -1;
  if (x6e0_retreatPoints.empty())
    return ret;
  zeus::CVector2f playerPos = mgr.GetPlayer().GetTranslation().toVec2f();
  ret = mgr.GetActiveRandom()->Range(0, s32(x6e0_retreatPoints.size() - 1));
  float maxDist = (playerPos - x6e0_retreatPoints[ret].toVec2f()).magSquared();
  if (maxDist < 100.f) {
    s32 i = 0;
    for (const auto& v : x6e0_retreatPoints) {
      float dist = (playerPos - v.toVec2f()).magSquared();
      if (dist > maxDist) {
        maxDist = dist;
        ret = i;
      }
      ++i;
    }
  }
  return ret;
}

void CBeetle::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x568_stateProg = 0;
    SquadRemove(mgr);
    x818_stateFinishTimer = 0.f;
    x3b4_speed = 2.f * x81c_;
    break;
  case EStateMsg::Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        RemoveMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, EMaterialTypes::Target, EMaterialTypes::Orbit,
                       mgr);
        mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
        x838_25_burrowing = true;
        x5a8_animTimeRem = x450_bodyController->GetAnimTimeRemaining();
        x568_stateProg = 2;
      } else if (IsOnGround()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::One));
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
            x45c_steeringBehaviors.Seek(*this, mgr.GetPlayer().GetTranslation()), zeus::skZero3f, 1.f));
      }
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate) {
        x568_stateProg = 3;
        x830_intoGroundFactor = 0.f;
        auto aabb = GetBoundingBox();
        SetTranslation((aabb.max.z() - aabb.min.z()) * 3.f * zeus::skDown + GetTranslation());
      } else {
        float remTime = x450_bodyController->GetAnimTimeRemaining();
        x830_intoGroundFactor = (x5a8_animTimeRem > 0.f) ? remTime / x5a8_animTimeRem : 0.f;
      }
      break;
    case 3:
      x818_stateFinishTimer += dt;
      if (x818_stateFinishTimer >= x834_retreatTime) {
        x568_stateProg = 4;
        x830_intoGroundFactor = 0.f;
      } else {
        auto aabb = GetBoundingBox();
        zeus::CVector3f downVec = (aabb.max.z() - aabb.min.z()) * 3.f * zeus::skDown;
        SetTranslation(((x834_retreatTime > 0.f) ? (1.f / x834_retreatTime) * downVec : downVec) * dt +
                       GetTranslation());
      }
      break;
    default:
      break;
    }
    break;
  case EStateMsg::Deactivate: {
    s32 idx = FindFurthestRetreatPoint(mgr);
    if (idx != -1) {
      SetTranslation(x6e0_retreatPoints[idx]);
      AddMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    } else {
      SendScriptMsgs(EScriptObjectState::DeactivateState, mgr, EScriptObjectMessage::None);
      mgr.FreeScriptObject(GetUniqueId());
    }
    x400_24_hitByPlayerProjectile = false;
    x3b4_speed = x81c_;
    x838_25_burrowing = false;
    break;
  }
  }
}

bool CBeetle::InAttackPosition(CStateManager& mgr, float arg) {
  float distSq = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared();
  if (distSq > x2fc_minAttackRange * x2fc_minAttackRange && distSq < x300_maxAttackRange * x300_maxAttackRange)
    if (SpotPlayer(mgr, x3c8_leashRadius))
      return true;
  return false;
}

bool CBeetle::PathShagged(CStateManager&, float arg) { return false; }

bool CBeetle::InRange(CStateManager& mgr, float arg) {
  zeus::CVector3f targetPos = mgr.GetPlayer().GetTranslation();
  if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
    if (role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Melee)
      targetPos = role->GetTeamPosition();
  }
  return (targetPos - GetTranslation()).magSquared() < 100.f;
}

bool CBeetle::PatternOver(CStateManager& mgr, float arg) { return AnimOver(mgr, arg); }

bool CBeetle::HasAttackPattern(CStateManager&, float arg) { return true; }

bool CBeetle::AnimOver(CStateManager&, float arg) { return x568_stateProg == 4; }

bool CBeetle::ShouldAttack(CStateManager& mgr, float arg) {
  if (x814_attackDelayTimer <= 0.f) {
    if (TCastToPtr<CTeamAiMgr> tmgr = mgr.ObjectById(x570_aiMgr)) {
      if (tmgr->HasTeamAiRole(GetUniqueId()))
        return tmgr->AddMeleeAttacker(GetUniqueId());
    }
    return true;
  }
  return false;
}

bool CBeetle::ShouldDoubleSnap(CStateManager& mgr, float arg) {
  if (!GetSearchPath() && IsOnGround()) {
    zeus::CVector3f targetPos = mgr.GetPlayer().GetTranslation();
    float dist = x5a0_headbuttDist + x300_maxAttackRange;
    if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId()))
      targetPos = role->GetTeamPosition();
    zeus::CVector3f delta = targetPos - GetTranslation();
    if (delta.magSquared() > dist * dist && GetTransform().basis[1].dot(delta.normalized()) > 0.98f) {
      rstl::reserved_vector<TUniqueId, 1024> nearList;
      mgr.BuildNearList(nearList, GetTranslation(), GetTransform().basis[1], x5a0_headbuttDist,
                        CMaterialFilter::MakeInclude({EMaterialTypes::Character}), this);
      TUniqueId bestId = kInvalidUniqueId;
      CRayCastResult res =
          mgr.RayWorldIntersection(bestId, GetTranslation(), GetTransform().basis[1], x5a0_headbuttDist,
                                   CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), nearList);
      if (res.IsInvalid())
        return true;
    }
  }
  return false;
}

bool CBeetle::ShouldTurn(CStateManager& mgr, float arg) {
  return zeus::CVector2f::getAngleDiff(GetTransform().basis[1].toVec2f(),
                                       (mgr.GetPlayer().GetTranslation() - GetTranslation()).toVec2f()) >
         zeus::degToRad(30.f);
}

bool CBeetle::HitSomething(CStateManager&, float arg) { return x838_24_hitSomething; }

bool CBeetle::ShouldJumpBack(CStateManager& mgr, float arg) {
  zeus::CVector3f backDir = -GetTransform().basis[1];
  const auto& aabb = GetBaseBoundingBox();
  zeus::CVector3f pos = GetTranslation() + zeus::CVector3f(0.f, 0.f, (aabb.max.z() - aabb.min.z()) * 0.5f);
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, pos, backDir, x5a4_jumpBackwardDist,
                    CMaterialFilter::MakeInclude({EMaterialTypes::Character}), this);
  TUniqueId bestId = kInvalidUniqueId;
  CRayCastResult res = mgr.RayWorldIntersection(bestId, pos, backDir, x5a4_jumpBackwardDist,
                                                CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), nearList);
  return res.IsInvalid();
}

bool CBeetle::Stuck(CStateManager&, float arg) { return x820_posDeviationCounter > 30; }

bool CBeetle::NoPathNodes(CStateManager&, float arg) { return false; }

bool CBeetle::ShouldTaunt(CStateManager& mgr, float arg) {
  if (CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
    if (role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Unknown ||
        role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Unassigned) {
      return (role->GetTeamPosition() - GetTranslation()).magSquared() < 100.f;
    }
  }
  return false;
}

bool CBeetle::ShotAt(CStateManager&, float arg) {
  if (x3fc_flavor == EFlavorType::Two && x6e0_retreatPoints.size() > 0)
    return x400_24_hitByPlayerProjectile;
  return false;
}

void CBeetle::Burn(float duration, float damage) {
  CDamageVulnerability dVuln = *GetDamageVulnerability();
  if (!x838_25_burrowing && x3fc_flavor == EFlavorType::One)
    dVuln = x7ac_tailVuln;
  switch (dVuln.GetVulnerability(CWeaponMode(EWeaponType::Wave), false)) {
  case EVulnerability::Weak:
    x450_bodyController->SetOnFire(1.5f * duration);
    x3ec_pendingFireDamage = 1.5f * damage;
    break;
  case EVulnerability::Normal:
    x450_bodyController->SetOnFire(duration);
    x3ec_pendingFireDamage = damage;
    break;
  default:
    break;
  }
}

void CBeetle::Shock(CStateManager& mgr, float duration, float damage) {
  CDamageVulnerability dVuln = *GetDamageVulnerability();
  if (!x838_25_burrowing && x3fc_flavor == EFlavorType::One)
    dVuln = x7ac_tailVuln;
  switch (dVuln.GetVulnerability(CWeaponMode(EWeaponType::Wave), false)) {
  case EVulnerability::Weak:
    x450_bodyController->SetElectrocuting(1.5f * duration);
    x3f0_pendingShockDamage = 1.5f * damage;
    break;
  case EVulnerability::Normal:
    x450_bodyController->SetElectrocuting(duration);
    x3f0_pendingShockDamage = damage;
    break;
  default:
    break;
  }
}

CPathFindSearch* CBeetle::GetSearchPath() { return &x5fc_pathFindSearch; }

float CBeetle::GetGravityConstant() const { return 4.f * CPhysicsActor::GravityConstant(); }

} // namespace urde::MP1
