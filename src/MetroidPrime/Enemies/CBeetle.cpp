#include "MetroidPrime/Enemies/CBeetle.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/math.hpp"

#include "float.h"

static const char* const kBiteLctrName = "LCTR_GARMOUTH";
static const char* const kDamageLctrName = "Target_Tail";
static const char* const kBetaLctrName = "Target_Tail";

CBeetle::CBeetle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                 CPatterned::EFlavorType flavor, CBeetle::EEntranceType entranceType,
                 const CDamageInfo& touchDamage, const CDamageVulnerability& platingVuln,
                 const CVector3f& tailAimReference, float initialAttackDelay, float retreatTime,
                 float f3, const CDamageVulnerability& tailVuln,
                 const CActorParameters& actorParams,
                 const rstl::optional_object< CStaticRes >& tailModel)
: CPatterned(kC_Beetle, uid, name, flavor, info, xf, mData, pInfo, kMT_Ground, kCT_One, kBT_BiPedal,
             actorParams, flavor != kFT_Zero ? kCS_Medium : kCS_Small)
, x568_stateProg(-1)
, x56c_entranceType(entranceType)
, x570_aiMgr(kInvalidUniqueId)
, x574_tailAimReference(tailAimReference)
, x580_f3(f3)
, x584_touchDamage(touchDamage)
, x5a0_headbuttDist(FLT_MAX)
, x5a4_jumpBackwardDist(FLT_MAX)
, x5a8_animTimeRem(0.f)
, x5ac_tailModel(tailModel.valid() ? rstl::optional_object< CModelData >(*tailModel)
                                   : rstl::optional_object_null())
, x5fc_pathFindSearch(NULL, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x6e0_retreatPoints()
, x744_platingVuln(platingVuln)
, x7ac_tailVuln(tailVuln)
, x814_attackDelayTimer(initialAttackDelay)
, x818_stateFinishTimer(FLT_MAX)
, x81c_speedBackup(x3b4_speed)
, x820_posDeviationCounter(0)
, x824_predictPos(CVector3f::Zero())
, x830_intoGroundFactor(1.f)
, x834_retreatTime(retreatTime)
, x838_24_hitSomething(false)
, x838_25_burrowing(false)
, x838_26_canSkid(false) {
  CPASAnimParmData headbuttParms(pas::kAS_MeleeAttack, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(1));
  x5a0_headbuttDist = GetAnimationDistance(headbuttParms);

  CPASAnimParmData jumpBackParms(pas::kAS_Step, CPASAnimParm::FromEnum(1),
                                 CPASAnimParm::FromEnum(0));
  const CVector3f& scale = ModelData()->ScaleCopy();
  const float jumpBackDist = GetAnimationDistance(jumpBackParms);
  const float scaleY = scale.GetY();
  x5a4_jumpBackwardDist = scaleY * jumpBackDist;

  MakeThermalColdAndHot();
  if (x3fc_flavor == kFT_One) {
    x460_knockBackController.SetLocomotionDuringElectrocution(true);
  }
}

CBeetle::~CBeetle() {}

ENTITY_ACCEPT_IMPL(CBeetle)

void CBeetle::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool forward = true;

  switch (msg) {
  case kSM_Activate:
    SquadAdd(mgr);
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    SquadRemove(mgr);
    break;
  case kSM_OnFloor:
    forward = false;
    SetMomentumWR(CVector3f::Zero());
    x328_27_onGround = true;
    break;
  case kSM_Falling:
    if (!x450_bodyController->IsFrozen()) {
      const float momentum = GetGravityConstant() * GetMass();
      SetMomentumWR(CVector3f(0.f, 0.f, -momentum));
      x328_27_onGround = false;
    }
    forward = false;
    break;
  case kSM_InitializedInArea: {
    if (x570_aiMgr == kInvalidUniqueId) {
      x570_aiMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
      if (GetActive()) {
        SquadAdd(mgr);
      }
    }

    SetupRetreatPoints(mgr);

    const TAreaId aid = GetCurrentAreaId();
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(aid);
    x5fc_pathFindSearch.SetArea(area.GetPostConstructed()->x10bc_pathArea);
    break;
  }
  default:
    break;
  }

  if (forward) {
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
  }
}

CVector3f CBeetle::GetOrbitPosition(const CStateManager& mgr) const {
  CVector3f ret = CPatterned::GetOrbitPosition(mgr);
  ret.SetZ(GetBoundingBox().GetCenterPoint().GetZ());
  return ret;
}

void CBeetle::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  CBodyController* bodyCtrl = x450_bodyController.get();

  switch (msg) {
  case kStateMsg_Activate:
    if (x56c_entranceType == kET_FacePlayer || bodyCtrl->GetIsActive()) {
      bodyCtrl->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));

      CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
      playerPos.SetZ(GetTranslation().GetZ());
      CTransform4f lookAt = CTransform4f::LookAt(GetTranslation(), playerPos, CVector3f::Up());
      lookAt.SetTranslation(GetTranslation());
      SetTransform(lookAt);
    } else {
      bodyCtrl->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, x388_anim));
    }

    if (!bodyCtrl->GetIsActive()) {
      bodyCtrl->Activate(mgr);
    }

    RemoveMaterial(kMT_Character, kMT_Solid, mgr);
    x568_stateProg = 0;
    break;

  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (bodyCtrl->GetCurrentStateId() == pas::kAS_Generate) {
        x568_stateProg = 2;
        x5a8_animTimeRem = x450_bodyController->GetAnimTimeRemaining();
      } else {
        bodyCtrl->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
      }
      break;

    case 2:
      if (bodyCtrl->GetCurrentStateId() != pas::kAS_Generate) {
        x568_stateProg = 4;
      } else if (GetMaterialList().HasMaterial(kMT_Solid) && x5a8_animTimeRem > 0.f) {
        TEntityList nearList;

        const CVector3f pos = GetTranslation();
        CAABox box(pos - CVector3f(5.f, 5.f, 5.f), pos + CVector3f(5.f, 5.f, 5.f));
        CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(SolidMaterial));

        mgr.BuildNearList(nearList, box, solidFilter, this);

        if (nearList.size() != 0) {
          CVector3f total = CVector3f::Zero();
          const float sepFactor = 5.f * dt / x5a8_animTimeRem;

          for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
            if (CActor* act = static_cast< CActor* >(mgr.ObjectById(*it))) {
              CVector3f separation =
                  x45c_steeringBehaviors.Separation(*this, act->GetTranslation(), 5.f);
              separation[kDZ] = rstl::max_val(0.f, separation[kDZ]);
              total += separation * sepFactor;
            }
          }

          SetTranslation(GetTranslation() + total);
        }
      }
      break;

    default:
      break;
    }
    break;

  case kStateMsg_Deactivate:
    AddMaterial(kMT_Character, kMT_Solid, kMT_GroundCollider, mgr);
    x328_25_verticalMovement = false;
    x838_25_burrowing = false;
    if (x328_26_solidCollision) {
      DeathDelete(mgr);
    }
    break;

  default:
    break;
  }
}

void CBeetle::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  CBodyController* bodyCtrl = x450_bodyController.get();

  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    SquadRemove(mgr);
    x818_stateFinishTimer = 0.f;
    break;

  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (bodyCtrl->GetCurrentStateId() == pas::kAS_Generate) {
        RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
        mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
        x838_25_burrowing = true;
        x5a8_animTimeRem = x450_bodyController->GetAnimTimeRemaining();
        x568_stateProg = 2;
      } else if (IsOnGround()) {
        bodyCtrl->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
      } else {
        const CVector3f move =
            x45c_steeringBehaviors.Seek(*this, mgr.GetPlayer()->GetTranslation());
        bodyCtrl->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
      break;

    case 2:
      if (bodyCtrl->GetCurrentStateId() != pas::kAS_Generate) {
        x568_stateProg = 3;
        x830_intoGroundFactor = 0.f;

        CAABox aabb = GetBoundingBox();
        const CVector3f downMove =
            CVector3f::Down() * (3.f * (aabb.GetMaxPoint().GetZ() - aabb.GetMinPoint().GetZ()));
        SetTranslation(GetTranslation() + downMove);
      } else {
        const float remTime = bodyCtrl->GetAnimTimeRemaining();
        x830_intoGroundFactor = x5a8_animTimeRem > 0.f ? remTime / x5a8_animTimeRem : 0.f;
      }
      break;

    case 3:
      x818_stateFinishTimer += dt;
      if (x818_stateFinishTimer >= 0.75f) {
        SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
        mgr.DeleteObjectRequest(GetUniqueId());
        x568_stateProg = 4;
        x830_intoGroundFactor = 0.f;
      } else {
        CAABox aabb = GetBoundingBox();
        const CVector3f downMove =
            CVector3f::Down() * (4.f * (aabb.GetMaxPoint().GetZ() - aabb.GetMinPoint().GetZ()));
        SetTranslation(GetTranslation() + downMove * dt);
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

void CBeetle::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
      x2e0_destPos = role->GetTeamPosition();
    } else {
      x2e0_destPos = mgr.GetPlayer()->GetTranslation();
    }
    x2dc_destObj = mgr.GetPlayer()->GetUniqueId();
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
    break;

  default:
    break;
  }
}

void CBeetle::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f target = mgr.GetPlayer()->GetTranslation();
    target[kDZ] = GetTranslation()[kDZ];
    x838_24_hitSomething = x838_26_canSkid = false;
    x2e0_destPos = target;
    x568_stateProg = 0;
    break;
  }

  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0: {
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_stateProg = 1;
        x838_26_canSkid = true;
      } else if (IsOnGround()) {
        CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
        aimPos[kDZ] = GetTranslation()[kDZ];

        CVector3f aimDelta = aimPos - GetTranslation();
        if (aimDelta.CanBeNormalized()) {
          aimPos += 5.f * aimDelta.AsNormalized();
        }

        x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One, aimPos));
      }
      break;
    }
    case 1: {
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_stateProg = 4;
      } else if (IsOnGround()) {
        x450_bodyController->CommandMgr().DeliverTargetVector(x2e0_destPos - GetTranslation());
      } else {
        x568_stateProg = 2;
      }
      break;
    }
    case 2: {
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_stateProg = 4;
      }
      break;
    }
    default:
      break;
    }

    if (x328_26_solidCollision) {
      x838_24_hitSomething = true;
    }
    break;

  case kStateMsg_Deactivate:
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, x570_aiMgr, GetUniqueId(), true);
    x814_attackDelayTimer =
        mgr.Random()->Float() * x308_attackTimeVariation + x304_averageAttackTime;
    break;

  default:
    break;
  }
}

void CBeetle::FollowPattern(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 1;
    break;

  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 1:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step) {
        x568_stateProg = 3;
      } else if (IsOnGround()) {
        x450_bodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Left, pas::kStep_Normal));
      }
      break;

    case 3:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step && IsOnGround()) {
        x450_bodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Right, pas::kStep_Normal));
        x568_stateProg = 2;
      }
      break;

    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step) {
        x568_stateProg = x814_attackDelayTimer <= 0.f ? 4 : 1;
      }
      break;

    default:
      break;
    }

    x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                          GetTranslation());
    break;

  default:
    break;
  }
}

bool CBeetle::HasAttackPattern(CStateManager& mgr, float arg) { return true; }

bool CBeetle::InRange(CStateManager& mgr, float arg) {
  CVector3f target = mgr.GetPlayer()->GetTranslation();

  if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
    if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee) {
      target = role->GetTeamPosition();
    }
  }

  const CVector3f delta = target - GetTranslation();
  return delta.MagSquared() < 100.f;
}

bool CBeetle::InAttackPosition(CStateManager& mgr, float arg) {
  bool inAttackPos = false;

  const CPlayer* player = mgr.GetPlayer();
  const CVector3f& delta = player->GetTranslation() - GetTranslation();
  float distSq = delta.MagSquared();

  const float minRange = x2fc_minAttackRange;
  if (distSq > minRange * minRange) {
    const float maxRange = x300_maxAttackRange;
    if (distSq < maxRange * maxRange && SpotPlayer(mgr, x3c8_leashRadius)) {
      inAttackPos = true;
    }
  }

  return inAttackPos;
}

bool CBeetle::ShouldAttack(CStateManager& mgr, float arg) {
  if (x814_attackDelayTimer <= 0.f) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x570_aiMgr))) {
      if (aiMgr->HasTeamAiRole(GetUniqueId())) {
        return aiMgr->AddMeleeAttacker(GetUniqueId());
      }
    }

    return true;
  }

  return false;
}

bool CBeetle::ShouldJumpBack(CStateManager& mgr, float arg) {
  const CVector3f backDir = -GetTransform().GetForward();
  const CAABox& aabb = GetBaseBoundingBox();

  const float minZ = aabb.GetMinPoint().GetZ();
  const float maxZ = aabb.GetMaxPoint().GetZ();
  const float halfHeight = 0.5f * (maxZ - minZ);
  const CVector3f pos = GetTranslation() + CVector3f(0.f, 0.f, halfHeight);

  bool doJumpBack = false;
  TEntityList nearList;
  CMaterialFilter charFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
  mgr.BuildNearList(nearList, pos, backDir, x5a4_jumpBackwardDist, charFilter, this);

  TUniqueId id = kInvalidUniqueId;
  CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  CRayCastResult result =
      mgr.RayWorldIntersection(id, pos, backDir, x5a4_jumpBackwardDist, solidFilter, nearList);
  if (!result.IsValid()) {
    doJumpBack = true;
  }

  return doJumpBack;
}

void CBeetle::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Step) {
        x568_stateProg = 2;
      } else if (IsOnGround()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Step) {
        x568_stateProg = 4;
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

bool CBeetle::ShouldDoubleSnap(CStateManager& mgr, float arg) {
  bool doDoubleSnap = false;
  if (!GetSearchPath() && IsOnGround()) {
    const CPlayer* player = mgr.GetPlayer();
    const float dist = x5a0_headbuttDist + x300_maxAttackRange;
    const float distSq = dist * dist;
    CVector3f targetPos = player->GetTranslation();
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
      targetPos = role->GetTeamPosition();
    }

    const CVector3f delta = targetPos - GetTranslation();
    if (delta.MagSquared() > distSq) {
      const CVector3f forward = GetTransform().GetForward();
      if (CVector3f::Dot(forward, delta.AsNormalized()) > 0.98f) {
        TEntityList nearList;
        CMaterialFilter charFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
        mgr.BuildNearList(nearList, GetTranslation(), forward, x5a0_headbuttDist, charFilter, this);

        TUniqueId id = kInvalidUniqueId;
        CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
        CRayCastResult result = mgr.RayWorldIntersection(id, GetTranslation(), forward,
                                                         x5a0_headbuttDist, solidFilter, nearList);
        if (!result.IsValid()) {
          doDoubleSnap = true;
        }
      }
    }
  }

  return doDoubleSnap;
}

void CBeetle::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_stateProg = 2;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_stateProg = 4;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(x2e0_destPos - GetTranslation());
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

void CBeetle::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x328_24_inPosition = false;
    break;
  case kStateMsg_Update: {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    const float midRange = (x2fc_minAttackRange + x300_maxAttackRange) * 0.5f;
    const CVector3f playerToThis = GetTranslation() - playerPos;
    const CVector3f& playerLimit = playerToThis.CanBeNormalized()
                                       ? playerPos + midRange * playerToThis.AsNormalized()
                                       : playerPos + midRange * GetTransform().GetForward();

    const CVector3f dist = GetTranslation() - playerLimit;
    if (dist.MagSquared() > 4.f) {
      const int stepDir = FindBestStepDirection(-playerToThis);
      switch (stepDir) {
      case pas::kSD_Forward:
      case pas::kSD_Backward: {
        const CVector3f move =
            x45c_steeringBehaviors.Arrival(*this, x2e0_destPos, x300_maxAttackRange);
        const CVector3f forward = GetTransform().GetForward();

        if (CVector3f::Dot(forward, move) >= 0.f || CVector3f::Dot(forward, playerToThis) >= 0.f) {
          x450_bodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
        } else {
          const CVector3f faceDir =
              playerToThis.CanBeNormalized() ? -playerToThis.AsNormalized() : forward;
          x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, faceDir, 1.f));
        }
        break;
      }
      case pas::kSD_Left:
      case pas::kSD_Right:
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(static_cast< pas::EStepDirection >(stepDir), pas::kStep_Normal));
        break;
      default:
        break;
      }

      x450_bodyController->CommandMgr().DeliverTargetVector(-playerToThis);
    } else {
      x328_24_inPosition = true;
    }
    break;
  }
  default:
    break;
  }
}

bool CBeetle::HitSomething(CStateManager& mgr, float arg) { return x838_24_hitSomething; }

bool CBeetle::Stuck(CStateManager& mgr, float arg) { return x820_posDeviationCounter > 30; }

bool CBeetle::ShouldTurn(CStateManager& mgr, float arg) {
  CVector2f delta2f = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
  CVector2f forward2f = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward2f, delta2f) > 0.5235988f;
}

void CBeetle::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    break;

  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Turn) {
        x568_stateProg = 2;
      } else {
        CVector3f thisToPlayer = mgr.GetPlayer()->GetTranslation() - GetTranslation();
        CVector3f faceVec = thisToPlayer.Magnitude() > FLT_EPSILON ? thisToPlayer.AsNormalized()
                                                                   : CVector3f::Zero();
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), faceVec, 1.f));
      }
      break;

    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Turn) {
        x568_stateProg = 4;
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

bool CBeetle::AnimOver(CStateManager& mgr, float arg) { return x568_stateProg == 4; }

bool CBeetle::ShouldTaunt(CStateManager& mgr, float arg) {
  if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
    if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Unknown ||
        role->GetTeamAiRole() == CTeamAiRole::kTAR_Unassigned) {
      const CVector3f delta = role->GetTeamPosition() - GetTranslation();
      return delta.MagSquared() < 100.f;
    }
  }

  return false;
}

bool CBeetle::ShotAt(CStateManager& mgr, float arg) {
  if (x3fc_flavor == kFT_Two && x6e0_retreatPoints.size() > 0) {
    return x400_24_hitByPlayerProjectile != 0;
  }

  return false;
}
bool CBeetle::PatternOver(CStateManager& mgr, float arg) { return AnimOver(mgr, arg); }

void CBeetle::Skid(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (IsOnGround() && x838_26_canSkid) {
      x450_bodyController->CommandMgr().DeliverCmd(CBCSlideCmd(pas::kSlide_Zero, GetTranslation()));
      x568_stateProg = 2;
    }
    break;

  case kStateMsg_Update:
    if (x450_bodyController->GetCurrentStateId() != pas::kAS_Slide) {
      x568_stateProg = 4;
    }
    break;

  case kStateMsg_Deactivate:
    x838_26_canSkid = false;
    break;

  default:
    break;
  }
}

void CBeetle::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->CommandMgr().DeliverCmd(
        CBCTauntCmd(mgr.Random()->Float() < 0.75f ? pas::kTT_One : pas::kTT_Zero));
    x568_stateProg = 2;
    break;

  case kStateMsg_Update:
    if (x450_bodyController->GetCurrentStateId() != pas::kAS_Taunt) {
      x568_stateProg = 4;
    } else {
      x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                            GetTranslation());
    }
    break;

  default:
    break;
  }
}

void CBeetle::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (GetSearchPath()) {
      RefinePathFindDest(mgr, x2e0_destPos);
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;

  case kStateMsg_Update: {
    CVector3f dest = mgr.GetPlayer()->GetTranslation();
    RefinePathFindDest(mgr, dest);

    const CVector3f delta = dest - GetTranslation();
    CVector3f move;
    if (!PathShagged(mgr, 0.f) &&
        x5fc_pathFindSearch.xc8_curWaypoint < x5fc_pathFindSearch.x4_waypoints.size() - 1) {
      CPatterned::PathFind(mgr, msg, dt);
      move = x450_bodyController->CommandMgr().GetMoveVector();
    } else {
      move = x45c_steeringBehaviors.Arrival(*this, dest, 5.f);
    }

    const CVector3f forward = GetTransform().GetForward();
    if (CVector3f::Dot(forward, move) >= 0.f || CVector3f::Dot(forward, delta) <= 0.f) {
      x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    } else {
      const CVector3f face = delta.CanBeNormalized() ? delta.AsNormalized() : forward;
      x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, face, 1.f));
    }

    SeparateFromMelees(mgr);
    break;
  }

  default:
    break;
  }
}

void CBeetle::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    SquadRemove(mgr);
    x818_stateFinishTimer = 0.f;
    x3b4_speed = 2.f * x81c_speedBackup;
    break;

  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
        mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
        x838_25_burrowing = true;
        x5a8_animTimeRem = x450_bodyController->GetAnimTimeRemaining();
        x568_stateProg = 2;
      } else if (IsOnGround()) {
        x450_bodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
      } else {
        const CVector3f move =
            x45c_steeringBehaviors.Seek(*this, mgr.GetPlayer()->GetTranslation());
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
      break;

    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_stateProg = 3;
        x830_intoGroundFactor = 0.f;

        CAABox aabb = GetBoundingBox();
        const float downScale = 3.f * (aabb.GetMaxPoint().GetZ() - aabb.GetMinPoint().GetZ());
        const CVector3f downMove = downScale * CVector3f::Down();
        SetTranslation(GetTranslation() + downMove);
      } else {
        const float remTime = x450_bodyController->GetAnimTimeRemaining();
        x830_intoGroundFactor = x5a8_animTimeRem > 0.f ? remTime / x5a8_animTimeRem : 0.f;
      }
      break;

    case 3:
      x818_stateFinishTimer += dt;
      if (x818_stateFinishTimer >= x834_retreatTime) {
        x568_stateProg = 4;
        x830_intoGroundFactor = 0.f;
      } else {
        CAABox aabb = GetBoundingBox();
        const float downScale = 3.f * (aabb.GetMaxPoint().GetZ() - aabb.GetMinPoint().GetZ());
        const CVector3f downMove = downScale * CVector3f::Down();
        const CVector3f retreatMove =
            x834_retreatTime > 0.f ? (1.f / x834_retreatTime) * downMove : downMove;
        SetTranslation(GetTranslation() + dt * retreatMove);
      }
      break;

    default:
      break;
    }
    break;

  case kStateMsg_Deactivate: {
    const s32 point = FindFurthestRetreatPoint(mgr);
    if (point != -1) {
      SetTranslation(x6e0_retreatPoints[point]);
      AddMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
    } else {
      SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
      mgr.DeleteObjectRequest(GetUniqueId());
    }

    x400_24_hitByPlayerProjectile = false;
    x3b4_speed = x81c_speedBackup;
    x838_25_burrowing = false;
    break;
  }

  default:
    break;
  }
}

void CBeetle::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                              float dt) {
  bool handled = false;

  switch (type) {
  case kUE_ChangeMaterial:
    AddMaterial(kMT_Character, kMT_Solid, mgr);
    x328_25_verticalMovement = true;
    RemoveMaterial(kMT_GroundCollider, mgr);
    handled = true;
    break;

  case kUE_GenerateEnd:
    x328_25_verticalMovement = false;
    AddMaterial(kMT_GroundCollider, mgr);
    handled = true;
    break;

  case kUE_DamageOn: {
    CVector3f scale = ModelData()->GetScale();
    CVector3f bite = GetLocatorTransform(rstl::string_l(kBiteLctrName)).GetTranslation();

    const CVector3f bitePos = GetTransform() * CVector3f::ByElementMultiply(scale, bite);
    const CVector3f extent = CVector3f::ByElementMultiply(scale, CVector3f(2.f, 2.f, 0.5f));
    const CAABox biteBox(bitePos - extent, bitePos + extent);
    if (biteBox.DoBoundsOverlap(mgr.GetPlayer()->GetBoundingBox())) {
      CMaterialFilter filter =
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList());
      mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
                      x584_touchDamage, filter, CVector3f::Zero());
    }

    handled = true;
    break;
  }

  case kUE_Delete:
    handled = true;
    break;

  default:
    break;
  }

  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CBeetle::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
    CBodyController* bodyCtrl = x450_bodyController.get();
    pas::ELocomotionType locomotion = pas::kLT_Relaxed;
    if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee) {
      locomotion = pas::kLT_Lurk;
    }
    bodyCtrl->SetLocomotionType(locomotion);
  } else {
    SquadAdd(mgr);
    x450_bodyController->SetLocomotionType(pas::kLT_Lurk);
  }

  x460_knockBackController.SetAutoResetImpulse(IsOnGround());

  if (x814_attackDelayTimer > 0.f) {
    x814_attackDelayTimer -= dt;
  }

  CVector2f predictPos = (x824_predictPos - GetTranslation()).ToVec2f();
  if (CVector2f::Dot(predictPos, predictPos) > 0.1f * dt) {
    ++x820_posDeviationCounter;
  } else {
    x820_posDeviationCounter = 0;
  }

  CPatterned::Think(dt, mgr);

  x824_predictPos = GetTranslation() + dt * GetVelocityWR();
}

void CBeetle::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                           CStateManager& mgr) {
  static CMaterialList skEnvList(kMT_Ceiling, kMT_Wall);

  for (int i = 0; i < list.GetCount(); ++i) {
    if (list[i].GetMaterialLeft().SharesMaterials(skEnvList) &&
        x450_bodyController->GetCurrentStateId() == pas::kAS_MeleeAttack) {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
      SetVelocityWR(CVector3f::Zero());
    }
  }

  CPatterned::CollidedWith(id, list, mgr);
}

void CBeetle::Burn(float duration, float damage) {
  CDamageVulnerability dVuln = *GetDamageVulnerability();
  if (!x838_25_burrowing && x3fc_flavor == kFT_One) {
    dVuln = x7ac_tailVuln;
  }

  switch (dVuln.GetVulnerability(CWeaponMode(kWT_Wave), CDamageVulnerability::kRD_No)) {
  case kVN_Weak:
    x450_bodyController->SetOnFire(1.5f * duration);
    x3ec_pendingFireDamage = 1.5f * damage;
    break;
  case kVN_Normal:
    x450_bodyController->SetOnFire(duration);
    x3ec_pendingFireDamage = damage;
    break;
  default:
    break;
  }
}

void CBeetle::Shock(float duration, float damage) {
  CDamageVulnerability dVuln = *GetDamageVulnerability();
  if (!x838_25_burrowing && x3fc_flavor == kFT_One) {
    dVuln = x7ac_tailVuln;
  }

  switch (dVuln.GetVulnerability(CWeaponMode(kWT_Wave), CDamageVulnerability::kRD_No)) {
  case kVN_Weak:
    x450_bodyController->SetElectrocuting(1.5f * duration);
    x3f0_pendingShockDamage = 1.5f * damage;
    break;
  case kVN_Normal:
    x450_bodyController->SetElectrocuting(duration);
    x3f0_pendingShockDamage = damage;
    break;
  default:
    break;
  }
}

const CDamageVulnerability* CBeetle::GetDamageVulnerability(const CVector3f& position,
                                                            const CVector3f& direction,
                                                            const CDamageInfo& damage) const {
  if (x838_25_burrowing) {
    return &CDamageVulnerability::PassThroughVulnerability();
  }

  if (x3fc_flavor == kFT_One) {
    const CWeaponMode& mode = damage.GetWeaponMode();
    if (mode.IsComboed() && mode.GetType() == kWT_Wave) {
      return &x7ac_tailVuln;
    }

    CUnitVector3f centerToPositionDir(position - GetBoundingBox().GetCenterPoint());
    const CVector3f forward = GetTransform().GetForward();
    float forwardDotDirection = CVector3f::Dot(forward, direction);
    float forwardDotCenterPos = CVector3f::Dot(forward, centerToPositionDir);

    if (forwardDotDirection > 0.f && forwardDotCenterPos < -0.5f) {
      return &x7ac_tailVuln;
    }

    return &x744_platingVuln;
  }

  return static_cast< const CActor* >(this)->GetDamageVulnerability();
}

const CDamageVulnerability* CBeetle::GetDamageVulnerability() const {
  if (x838_25_burrowing) {
    return &CDamageVulnerability::PassThroughVulnerability();
  }

  if (x3fc_flavor == kFT_One) {
    if (x450_bodyController->IsOnFire()) {
      return &x7ac_tailVuln;
    }
    return &x744_platingVuln;
  }

  return CAi::GetDamageVulnerability();
}

EWeaponCollisionResponseTypes CBeetle::GetCollisionResponseType(const CVector3f& position,
                                                                const CVector3f& direction,
                                                                const CWeaponMode& mode,
                                                                int attrib) const {
  EWeaponCollisionResponseTypes ret = kWCR_Unknown19;

  if (x450_bodyController->IsFrozen() && mode.GetType() == kWT_Ice) {
    ret = kWCR_None;
  } else if (x838_25_burrowing) {
    ret = kWCR_Unknown69;
  } else if (x3fc_flavor == kFT_One) {
    CUnitVector3f centerToPositionDir(position - GetBoundingBox().GetCenterPoint());
    const CVector3f forward = GetTransform().GetForward();
    float forwardDotDirection = CVector3f::Dot(GetTransform().GetForward(), direction);
    float forwardDotCenterPos = CVector3f::Dot(forward, centerToPositionDir);

    if (forwardDotDirection > 0.f && forwardDotCenterPos < -0.5f) {
      ret = kWCR_Unknown44;
    } else if (!x744_platingVuln.WeaponHurts(mode, CDamageVulnerability::kRD_No)) {
      ret = kWCR_Unknown69;
    }
  }

  return ret;
}

void CBeetle::TakeDamage(const CVector3f& direction, float magnitude) {
  x428_damageCooldownTimer = CPatterned::skDamageHitTime;
}

CVector3f CBeetle::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (x3fc_flavor == kFT_One || x3fc_flavor == kFT_Two) {
    CTransform4f tailXf = GetLctrTransform(rstl::string_l(kDamageLctrName));

    float minFactor = 10.f;
    CVector3f scaleRange = tailXf * x574_tailAimReference - GetTranslation();
    const CAABox& aabb = GetBoundingBox();
    for (int i = 0; i < 3; ++i) {
      if (scaleRange[i] < 0.f) {
        float factor = (aabb.GetMinPoint()[i] - GetTranslation()[i]) / scaleRange[i];
        if (factor < minFactor) {
          minFactor = factor;
        }
      } else if (scaleRange[i] > 0.f) {
        float factor = (aabb.GetMaxPoint()[i] - GetTranslation()[i]) / scaleRange[i];
        if (factor < minFactor) {
          minFactor = factor;
        }
      }
    }

    return GetTranslation() + minFactor * scaleRange;
  }

  return CPhysicsActor::GetAimPosition(mgr, dt);
}

void CBeetle::Render(const CStateManager& mgr) const {
  if (x3fc_flavor == kFT_One && x400_25_alive) {
    CTransform4f tailXf = GetLctrTransform(rstl::string_l(kBetaLctrName));

    if (x428_damageCooldownTimer >= 0.f && x42c_color.GetAlphau8() == 255) {
      if (x5ac_tailModel.valid()) {
        x5ac_tailModel->Render(mgr, tailXf, GetActorLights(),
                               CModelFlags(CModelFlags::kT_Two, x42c_color));
      }
    } else if (x5ac_tailModel.valid()) {
      x5ac_tailModel->Render(mgr, tailXf, GetActorLights(),
                             CModelFlags(CModelFlags::kT_Opaque, 1.f));
    }
  }

  CPatterned::Render(mgr);
}

void CBeetle::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (x400_25_alive) {
    if (x3fc_flavor == kFT_One) {
      CTransform4f backupXf = GetTransform();
      SetTransform(GetLctrTransform(rstl::string_l(kBetaLctrName)));
      SendScriptMsgs(kSS_DeathRattle, mgr, kSM_None);
      SetTransform(backupXf);
      CPatterned::Death(mgr, direction, kSS_Any);
    } else {
      CPatterned::Death(mgr, direction, state);
    }
  }
}

void CBeetle::SquadAdd(CStateManager& mgr) {
  if (x570_aiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x570_aiMgr))) {
      aiMgr->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Unknown,
                              CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CBeetle::SquadRemove(CStateManager& mgr) {
  if (x570_aiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x570_aiMgr))) {
      if (aiMgr->IsPartOfTeam(GetUniqueId())) {
        aiMgr->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

CVector3f CBeetle::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                             const CVector3f& aimPos) const {
  const float midRange = 0.5f * (x2fc_minAttackRange + x300_maxAttackRange);
  const CVector3f playerToThis = GetTranslation() - aimPos;
  const CVector3f ret = playerToThis.CanBeNormalized()
                            ? aimPos + midRange * playerToThis.AsNormalized()
                            : aimPos + midRange * GetTransform().GetForward();
  return ret;
}

void CBeetle::SeparateFromMelees(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int idx = list.GetFirstObjectIndex(); idx != -1; idx = list.GetNextObjectIndex(idx)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[idx])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        float dist = 4.f;
        if (const CTeamAiRole* role =
                CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, ai->GetUniqueId())) {
          if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee) {
            dist *= 2.f;
          }
        }

        const CVector3f move = x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), dist);
        if (move.IsNonZero()) {
          x450_bodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CBeetle::SetupRetreatPoints(CStateManager& mgr) {
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->x0_state == kSS_Retreat && conn->x4_msg == kSM_Follow) {
      TUniqueId wpId = mgr.GetIdForScript(conn->x8_objId);
      if (const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wpId))) {
        x6e0_retreatPoints.push_back(wp->GetTranslation());
        if (x6e0_retreatPoints.size() == 8) {
          break;
        }
      }
    }
  }
}

s32 CBeetle::FindFurthestRetreatPoint(CStateManager& mgr) {
  s32 ret = -1;
  if (x6e0_retreatPoints.size() > 0) {
    CVector2f playerPos = mgr.GetPlayer()->GetTranslation().ToVec2f();

    ret = mgr.Random()->Range(0, x6e0_retreatPoints.size() - 1);

    CVector2f retreatPos(x6e0_retreatPoints[ret].GetX(), x6e0_retreatPoints[ret].GetY());
    CVector2f maxDelta = playerPos - retreatPos;
    float maxDist = maxDelta.MagSquared();
    if (maxDist < 100.f) {
      for (int i = 0; i < x6e0_retreatPoints.size(); ++i) {
        CVector2f pointPos(x6e0_retreatPoints[i].GetX(), x6e0_retreatPoints[i].GetY());
        CVector2f delta = playerPos - pointPos;
        const float dist = delta.MagSquared();
        if (dist > maxDist) {
          maxDist = dist;
          ret = i;
        }
      }
    }
  }

  return ret;
}

void CBeetle::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (x400_25_alive) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_XRay:
      x42c_color.SetAlpha(0.3f);
      break;
    case CPlayerState::kPV_Thermal:
      if (x838_25_burrowing) {
        x42c_color.SetAlpha(x830_intoGroundFactor);
      } else {
        x42c_color.SetAlpha(1.f);
      }
      break;
    default:
      x42c_color.SetAlpha(1.f);
      break;
    }
  }

  CPatterned::PreRender(mgr, frustum);
}

void CBeetle::RefinePathFindDest(CStateManager& mgr, CVector3f& dest) {
  dest = mgr.GetPlayer()->GetTranslation();

  if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x570_aiMgr, GetUniqueId())) {
    dest = role->GetTeamPosition();
  } else {
    CVector3f thisToDest = dest - GetTranslation();
    const CVector3f thisToDestNorm =
        thisToDest.CanBeNormalized() ? -thisToDest.AsNormalized() : GetTransform().GetForward();
    const float midRange = 0.5f * (x2fc_minAttackRange + x300_maxAttackRange);
    dest += midRange * thisToDestNorm;
  }
}

float CBeetle::GetGravityConstant() const { return 4.f * GravityConstant(); }

bool CBeetle::IsListening() const { return true; }

bool CBeetle::PathShagged(CStateManager& mgr, float arg) { return false; }

bool CBeetle::NoPathNodes(CStateManager& mgr, float arg) { return false; }

CPathFindSearch* CBeetle::GetSearchPath() { return &x5fc_pathFindSearch; }
