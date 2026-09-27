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
, mStateProg(-1)
, mEntranceType(entranceType)
, mAiMgr(kInvalidUniqueId)
, mTailAimReference(tailAimReference)
, mF3(f3)
, mTouchDamage(touchDamage)
, mHeadbuttDist(FLT_MAX)
, mJumpBackwardDist(FLT_MAX)
, mAnimTimeRem(0.f)
, mTailModel(tailModel.valid() ? rstl::optional_object< CModelData >(*tailModel)
                                   : rstl::optional_object_null())
, mPathFindSearch(NULL, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mRetreatPoints()
, mPlatingVuln(platingVuln)
, mTailVuln(tailVuln)
, mAttackDelayTimer(initialAttackDelay)
, mStateFinishTimer(FLT_MAX)
, mSpeedBackup(mSpeed)
, mPosDeviationCounter(0)
, mPredictPos(CVector3f::Zero())
, mIntoGroundFactor(1.f)
, mRetreatTime(retreatTime)
, mHitSomething(false)
, mBurrowing(false)
, mCanSkid(false) {
  CPASAnimParmData headbuttParms(pas::kAS_MeleeAttack, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(1));
  mHeadbuttDist = GetAnimationDistance(headbuttParms);

  CPASAnimParmData jumpBackParms(pas::kAS_Step, CPASAnimParm::FromEnum(1),
                                 CPASAnimParm::FromEnum(0));
  const CVector3f& scale = ModelData()->ScaleCopy();
  const float jumpBackDist = GetAnimationDistance(jumpBackParms);
  const float scaleY = scale.GetY();
  mJumpBackwardDist = scaleY * jumpBackDist;

  MakeThermalColdAndHot();
  if (mFlavor == kFT_One) {
    mKnockBackController.SetLocomotionDuringElectrocution(true);
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
    mOnGround = true;
    break;
  case kSM_Falling:
    if (!mBodyController->IsFrozen()) {
      const float momentum = GetGravityConstant() * GetMass();
      SetMomentumWR(CVector3f(0.f, 0.f, -momentum));
      mOnGround = false;
    }
    forward = false;
    break;
  case kSM_InitializedInArea: {
    if (mAiMgr == kInvalidUniqueId) {
      mAiMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
      if (GetActive()) {
        SquadAdd(mgr);
      }
    }

    SetupRetreatPoints(mgr);

    const TAreaId aid = GetCurrentAreaId();
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(aid);
    mPathFindSearch.SetArea(area.GetPostConstructed()->mPathArea);
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
  CBodyController* bodyCtrl = mBodyController.get();

  switch (msg) {
  case kStateMsg_Activate:
    if (mEntranceType == kET_FacePlayer || bodyCtrl->GetIsActive()) {
      bodyCtrl->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));

      CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
      playerPos.SetZ(GetTranslation().GetZ());
      CTransform4f lookAt = CTransform4f::LookAt(GetTranslation(), playerPos, CVector3f::Up());
      lookAt.SetTranslation(GetTranslation());
      SetTransform(lookAt);
    } else {
      bodyCtrl->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, mAnim));
    }

    if (!bodyCtrl->GetIsActive()) {
      bodyCtrl->Activate(mgr);
    }

    RemoveMaterial(kMT_Character, kMT_Solid, mgr);
    mStateProg = 0;
    break;

  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (bodyCtrl->GetCurrentStateId() == pas::kAS_Generate) {
        mStateProg = 2;
        mAnimTimeRem = mBodyController->GetAnimTimeRemaining();
      } else {
        bodyCtrl->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
      }
      break;

    case 2:
      if (bodyCtrl->GetCurrentStateId() != pas::kAS_Generate) {
        mStateProg = 4;
      } else if (GetMaterialList().HasMaterial(kMT_Solid) && mAnimTimeRem > 0.f) {
        TEntityList nearList;

        const CVector3f pos = GetTranslation();
        CAABox box(pos - CVector3f(5.f, 5.f, 5.f), pos + CVector3f(5.f, 5.f, 5.f));
        CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(SolidMaterial));

        mgr.BuildNearList(nearList, box, solidFilter, this);

        if (nearList.size() != 0) {
          CVector3f total = CVector3f::Zero();
          const float sepFactor = 5.f * dt / mAnimTimeRem;

          for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
            if (CActor* act = static_cast< CActor* >(mgr.ObjectById(*it))) {
              CVector3f separation =
                  mSteeringBehaviors.Separation(*this, act->GetTranslation(), 5.f);
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
    mVerticalMovement = false;
    mBurrowing = false;
    if (mSolidCollision) {
      DeathDelete(mgr);
    }
    break;

  default:
    break;
  }
}

void CBeetle::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  CBodyController* bodyCtrl = mBodyController.get();

  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 0;
    SquadRemove(mgr);
    mStateFinishTimer = 0.f;
    break;

  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (bodyCtrl->GetCurrentStateId() == pas::kAS_Generate) {
        RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
        mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
        mBurrowing = true;
        mAnimTimeRem = mBodyController->GetAnimTimeRemaining();
        mStateProg = 2;
      } else if (IsOnGround()) {
        bodyCtrl->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
      } else {
        const CVector3f move =
            mSteeringBehaviors.Seek(*this, mgr.GetPlayer()->GetTranslation());
        bodyCtrl->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
      break;

    case 2:
      if (bodyCtrl->GetCurrentStateId() != pas::kAS_Generate) {
        mStateProg = 3;
        mIntoGroundFactor = 0.f;

        CAABox aabb = GetBoundingBox();
        const CVector3f downMove =
            CVector3f::Down() * (3.f * (aabb.GetMaxPoint().GetZ() - aabb.GetMinPoint().GetZ()));
        SetTranslation(GetTranslation() + downMove);
      } else {
        const float remTime = bodyCtrl->GetAnimTimeRemaining();
        mIntoGroundFactor = mAnimTimeRem > 0.f ? remTime / mAnimTimeRem : 0.f;
      }
      break;

    case 3:
      mStateFinishTimer += dt;
      if (mStateFinishTimer >= 0.75f) {
        SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
        mgr.DeleteObjectRequest(GetUniqueId());
        mStateProg = 4;
        mIntoGroundFactor = 0.f;
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
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId())) {
      mDestPos = role->GetTeamPosition();
    } else {
      mDestPos = mgr.GetPlayer()->GetTranslation();
    }
    mDestObj = mgr.GetPlayer()->GetUniqueId();
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
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
    mHitSomething = mCanSkid = false;
    mDestPos = target;
    mStateProg = 0;
    break;
  }

  case kStateMsg_Update:
    switch (mStateProg) {
    case 0: {
      if (mBodyController->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        mStateProg = 1;
        mCanSkid = true;
      } else if (IsOnGround()) {
        CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
        aimPos[kDZ] = GetTranslation()[kDZ];

        CVector3f aimDelta = aimPos - GetTranslation();
        if (aimDelta.CanBeNormalized()) {
          aimPos += 5.f * aimDelta.AsNormalized();
        }

        mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One, aimPos));
      }
      break;
    }
    case 1: {
      if (mBodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mStateProg = 4;
      } else if (IsOnGround()) {
        mBodyController->CommandMgr().DeliverTargetVector(mDestPos - GetTranslation());
      } else {
        mStateProg = 2;
      }
      break;
    }
    case 2: {
      if (mBodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mStateProg = 4;
      }
      break;
    }
    default:
      break;
    }

    if (mSolidCollision) {
      mHitSomething = true;
    }
    break;

  case kStateMsg_Deactivate:
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, mAiMgr, GetUniqueId(), true);
    mAttackDelayTimer =
        mgr.Random()->Float() * mAttackTimeVariation + mAverageAttackTime;
    break;

  default:
    break;
  }
}

void CBeetle::FollowPattern(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 1;
    break;

  case kStateMsg_Update:
    switch (mStateProg) {
    case 1:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mStateProg = 3;
      } else if (IsOnGround()) {
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Left, pas::kStep_Normal));
      }
      break;

    case 3:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step && IsOnGround()) {
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Right, pas::kStep_Normal));
        mStateProg = 2;
      }
      break;

    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mStateProg = mAttackDelayTimer <= 0.f ? 4 : 1;
      }
      break;

    default:
      break;
    }

    mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                          GetTranslation());
    break;

  default:
    break;
  }
}

bool CBeetle::HasAttackPattern(CStateManager& mgr, float arg) { return true; }

bool CBeetle::InRange(CStateManager& mgr, float arg) {
  CVector3f target = mgr.GetPlayer()->GetTranslation();

  if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId())) {
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

  const float minRange = mMinAttackRange;
  if (distSq > minRange * minRange) {
    const float maxRange = mMaxAttackRange;
    if (distSq < maxRange * maxRange && SpotPlayer(mgr, mLeashRadius)) {
      inAttackPos = true;
    }
  }

  return inAttackPos;
}

bool CBeetle::ShouldAttack(CStateManager& mgr, float arg) {
  if (mAttackDelayTimer <= 0.f) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mAiMgr))) {
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
  mgr.BuildNearList(nearList, pos, backDir, mJumpBackwardDist, charFilter, this);

  TUniqueId id = kInvalidUniqueId;
  CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  CRayCastResult result =
      mgr.RayWorldIntersection(id, pos, backDir, mJumpBackwardDist, solidFilter, nearList);
  if (!result.IsValid()) {
    doJumpBack = true;
  }

  return doJumpBack;
}

void CBeetle::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 0;
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Step) {
        mStateProg = 2;
      } else if (IsOnGround()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Step) {
        mStateProg = 4;
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
    const float dist = mHeadbuttDist + mMaxAttackRange;
    const float distSq = dist * dist;
    CVector3f targetPos = player->GetTranslation();
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId())) {
      targetPos = role->GetTeamPosition();
    }

    const CVector3f delta = targetPos - GetTranslation();
    if (delta.MagSquared() > distSq) {
      const CVector3f forward = GetTransform().GetForward();
      if (CVector3f::Dot(forward, delta.AsNormalized()) > 0.98f) {
        TEntityList nearList;
        CMaterialFilter charFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
        mgr.BuildNearList(nearList, GetTranslation(), forward, mHeadbuttDist, charFilter, this);

        TUniqueId id = kInvalidUniqueId;
        CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
        CRayCastResult result = mgr.RayWorldIntersection(id, GetTranslation(), forward,
                                                         mHeadbuttDist, solidFilter, nearList);
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
    mStateProg = 0;
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mStateProg = 4;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(mDestPos - GetTranslation());
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
    mInPosition = false;
    break;
  case kStateMsg_Update: {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    const float midRange = (mMinAttackRange + mMaxAttackRange) * 0.5f;
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
            mSteeringBehaviors.Arrival(*this, mDestPos, mMaxAttackRange);
        const CVector3f forward = GetTransform().GetForward();

        if (CVector3f::Dot(forward, move) >= 0.f || CVector3f::Dot(forward, playerToThis) >= 0.f) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
        } else {
          const CVector3f faceDir =
              playerToThis.CanBeNormalized() ? -playerToThis.AsNormalized() : forward;
          mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, faceDir, 1.f));
        }
        break;
      }
      case pas::kSD_Left:
      case pas::kSD_Right:
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(static_cast< pas::EStepDirection >(stepDir), pas::kStep_Normal));
        break;
      default:
        break;
      }

      mBodyController->CommandMgr().DeliverTargetVector(-playerToThis);
    } else {
      mInPosition = true;
    }
    break;
  }
  default:
    break;
  }
}

bool CBeetle::HitSomething(CStateManager& mgr, float arg) { return mHitSomething; }

bool CBeetle::Stuck(CStateManager& mgr, float arg) { return mPosDeviationCounter > 30; }

bool CBeetle::ShouldTurn(CStateManager& mgr, float arg) {
  CVector2f delta2f = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
  CVector2f forward2f = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward2f, delta2f) > 0.5235988f;
}

void CBeetle::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 0;
    break;

  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Turn) {
        mStateProg = 2;
      } else {
        CVector3f thisToPlayer = mgr.GetPlayer()->GetTranslation() - GetTranslation();
        CVector3f faceVec = thisToPlayer.Magnitude() > FLT_EPSILON ? thisToPlayer.AsNormalized()
                                                                   : CVector3f::Zero();
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), faceVec, 1.f));
      }
      break;

    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Turn) {
        mStateProg = 4;
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

bool CBeetle::AnimOver(CStateManager& mgr, float arg) { return mStateProg == 4; }

bool CBeetle::ShouldTaunt(CStateManager& mgr, float arg) {
  if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId())) {
    if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Unknown ||
        role->GetTeamAiRole() == CTeamAiRole::kTAR_Unassigned) {
      const CVector3f delta = role->GetTeamPosition() - GetTranslation();
      return delta.MagSquared() < 100.f;
    }
  }

  return false;
}

bool CBeetle::ShotAt(CStateManager& mgr, float arg) {
  if (mFlavor == kFT_Two && mRetreatPoints.size() > 0) {
    return mHitByPlayerProjectile != 0;
  }

  return false;
}
bool CBeetle::PatternOver(CStateManager& mgr, float arg) { return AnimOver(mgr, arg); }

void CBeetle::Skid(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (IsOnGround() && mCanSkid) {
      mBodyController->CommandMgr().DeliverCmd(CBCSlideCmd(pas::kSlide_Zero, GetTranslation()));
      mStateProg = 2;
    }
    break;

  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() != pas::kAS_Slide) {
      mStateProg = 4;
    }
    break;

  case kStateMsg_Deactivate:
    mCanSkid = false;
    break;

  default:
    break;
  }
}

void CBeetle::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->CommandMgr().DeliverCmd(
        CBCTauntCmd(mgr.Random()->Float() < 0.75f ? pas::kTT_One : pas::kTT_Zero));
    mStateProg = 2;
    break;

  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() != pas::kAS_Taunt) {
      mStateProg = 4;
    } else {
      mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
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
      RefinePathFindDest(mgr, mDestPos);
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;

  case kStateMsg_Update: {
    CVector3f dest = mgr.GetPlayer()->GetTranslation();
    RefinePathFindDest(mgr, dest);

    const CVector3f delta = dest - GetTranslation();
    CVector3f move;
    if (!PathShagged(mgr, 0.f) &&
        mPathFindSearch.mCurWaypoint < mPathFindSearch.mWaypoints.size() - 1) {
      CPatterned::PathFind(mgr, msg, dt);
      move = mBodyController->CommandMgr().GetMoveVector();
    } else {
      move = mSteeringBehaviors.Arrival(*this, dest, 5.f);
    }

    const CVector3f forward = GetTransform().GetForward();
    if (CVector3f::Dot(forward, move) >= 0.f || CVector3f::Dot(forward, delta) <= 0.f) {
      mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    } else {
      const CVector3f face = delta.CanBeNormalized() ? delta.AsNormalized() : forward;
      mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, face, 1.f));
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
    mStateProg = 0;
    SquadRemove(mgr);
    mStateFinishTimer = 0.f;
    mSpeed = 2.f * mSpeedBackup;
    break;

  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
        mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
        mBurrowing = true;
        mAnimTimeRem = mBodyController->GetAnimTimeRemaining();
        mStateProg = 2;
      } else if (IsOnGround()) {
        mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
      } else {
        const CVector3f move =
            mSteeringBehaviors.Seek(*this, mgr.GetPlayer()->GetTranslation());
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
      break;

    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        mStateProg = 3;
        mIntoGroundFactor = 0.f;

        CAABox aabb = GetBoundingBox();
        const float downScale = 3.f * (aabb.GetMaxPoint().GetZ() - aabb.GetMinPoint().GetZ());
        const CVector3f downMove = downScale * CVector3f::Down();
        SetTranslation(GetTranslation() + downMove);
      } else {
        const float remTime = mBodyController->GetAnimTimeRemaining();
        mIntoGroundFactor = mAnimTimeRem > 0.f ? remTime / mAnimTimeRem : 0.f;
      }
      break;

    case 3:
      mStateFinishTimer += dt;
      if (mStateFinishTimer >= mRetreatTime) {
        mStateProg = 4;
        mIntoGroundFactor = 0.f;
      } else {
        CAABox aabb = GetBoundingBox();
        const float downScale = 3.f * (aabb.GetMaxPoint().GetZ() - aabb.GetMinPoint().GetZ());
        const CVector3f downMove = downScale * CVector3f::Down();
        const CVector3f retreatMove =
            mRetreatTime > 0.f ? (1.f / mRetreatTime) * downMove : downMove;
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
      SetTranslation(mRetreatPoints[point]);
      AddMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
    } else {
      SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
      mgr.DeleteObjectRequest(GetUniqueId());
    }

    mHitByPlayerProjectile = false;
    mSpeed = mSpeedBackup;
    mBurrowing = false;
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
    mVerticalMovement = true;
    RemoveMaterial(kMT_GroundCollider, mgr);
    handled = true;
    break;

  case kUE_GenerateEnd:
    mVerticalMovement = false;
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
                      mTouchDamage, filter, CVector3f::Zero());
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

  if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId())) {
    CBodyController* bodyCtrl = mBodyController.get();
    pas::ELocomotionType locomotion = pas::kLT_Relaxed;
    if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee) {
      locomotion = pas::kLT_Lurk;
    }
    bodyCtrl->SetLocomotionType(locomotion);
  } else {
    SquadAdd(mgr);
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
  }

  mKnockBackController.SetAutoResetImpulse(IsOnGround());

  if (mAttackDelayTimer > 0.f) {
    mAttackDelayTimer -= dt;
  }

  CVector2f predictPos = (mPredictPos - GetTranslation()).ToVec2f();
  if (CVector2f::Dot(predictPos, predictPos) > 0.1f * dt) {
    ++mPosDeviationCounter;
  } else {
    mPosDeviationCounter = 0;
  }

  CPatterned::Think(dt, mgr);

  mPredictPos = GetTranslation() + dt * GetVelocityWR();
}

void CBeetle::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                           CStateManager& mgr) {
  static CMaterialList skEnvList(kMT_Ceiling, kMT_Wall);

  for (int i = 0; i < list.GetCount(); ++i) {
    if (list[i].GetMaterialLeft().SharesMaterials(skEnvList) &&
        mBodyController->GetCurrentStateId() == pas::kAS_MeleeAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
      SetVelocityWR(CVector3f::Zero());
    }
  }

  CPatterned::CollidedWith(id, list, mgr);
}

void CBeetle::Burn(float duration, float damage) {
  CDamageVulnerability dVuln = *GetDamageVulnerability();
  if (!mBurrowing && mFlavor == kFT_One) {
    dVuln = mTailVuln;
  }

  switch (dVuln.GetVulnerability(CWeaponMode(kWT_Wave), CDamageVulnerability::kRD_No)) {
  case kVN_Weak:
    mBodyController->SetOnFire(1.5f * duration);
    mPendingFireDamage = 1.5f * damage;
    break;
  case kVN_Normal:
    mBodyController->SetOnFire(duration);
    mPendingFireDamage = damage;
    break;
  default:
    break;
  }
}

void CBeetle::Shock(float duration, float damage) {
  CDamageVulnerability dVuln = *GetDamageVulnerability();
  if (!mBurrowing && mFlavor == kFT_One) {
    dVuln = mTailVuln;
  }

  switch (dVuln.GetVulnerability(CWeaponMode(kWT_Wave), CDamageVulnerability::kRD_No)) {
  case kVN_Weak:
    mBodyController->SetElectrocuting(1.5f * duration);
    mPendingShockDamage = 1.5f * damage;
    break;
  case kVN_Normal:
    mBodyController->SetElectrocuting(duration);
    mPendingShockDamage = damage;
    break;
  default:
    break;
  }
}

const CDamageVulnerability* CBeetle::GetDamageVulnerability(const CVector3f& position,
                                                            const CVector3f& direction,
                                                            const CDamageInfo& damage) const {
  if (mBurrowing) {
    return &CDamageVulnerability::PassThroughVulnerability();
  }

  if (mFlavor == kFT_One) {
    const CWeaponMode& mode = damage.GetWeaponMode();
    if (mode.IsComboed() && mode.GetType() == kWT_Wave) {
      return &mTailVuln;
    }

    CUnitVector3f centerToPositionDir(position - GetBoundingBox().GetCenterPoint());
    const CVector3f forward = GetTransform().GetForward();
    float forwardDotDirection = CVector3f::Dot(forward, direction);
    float forwardDotCenterPos = CVector3f::Dot(forward, centerToPositionDir);

    if (forwardDotDirection > 0.f && forwardDotCenterPos < -0.5f) {
      return &mTailVuln;
    }

    return &mPlatingVuln;
  }

  return static_cast< const CActor* >(this)->GetDamageVulnerability();
}

const CDamageVulnerability* CBeetle::GetDamageVulnerability() const {
  if (mBurrowing) {
    return &CDamageVulnerability::PassThroughVulnerability();
  }

  if (mFlavor == kFT_One) {
    if (mBodyController->IsOnFire()) {
      return &mTailVuln;
    }
    return &mPlatingVuln;
  }

  return CAi::GetDamageVulnerability();
}

EWeaponCollisionResponseTypes CBeetle::GetCollisionResponseType(const CVector3f& position,
                                                                const CVector3f& direction,
                                                                const CWeaponMode& mode,
                                                                int attrib) const {
  EWeaponCollisionResponseTypes ret = kWCR_Unknown19;

  if (mBodyController->IsFrozen() && mode.GetType() == kWT_Ice) {
    ret = kWCR_None;
  } else if (mBurrowing) {
    ret = kWCR_Unknown69;
  } else if (mFlavor == kFT_One) {
    CUnitVector3f centerToPositionDir(position - GetBoundingBox().GetCenterPoint());
    const CVector3f forward = GetTransform().GetForward();
    float forwardDotDirection = CVector3f::Dot(GetTransform().GetForward(), direction);
    float forwardDotCenterPos = CVector3f::Dot(forward, centerToPositionDir);

    if (forwardDotDirection > 0.f && forwardDotCenterPos < -0.5f) {
      ret = kWCR_Unknown44;
    } else if (!mPlatingVuln.WeaponHurts(mode, CDamageVulnerability::kRD_No)) {
      ret = kWCR_Unknown69;
    }
  }

  return ret;
}

void CBeetle::TakeDamage(const CVector3f& direction, float magnitude) {
  mDamageCooldownTimer = CPatterned::skDamageHitTime;
}

CVector3f CBeetle::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (mFlavor == kFT_One || mFlavor == kFT_Two) {
    CTransform4f tailXf = GetLctrTransform(rstl::string_l(kDamageLctrName));

    float minFactor = 10.f;
    CVector3f scaleRange = tailXf * mTailAimReference - GetTranslation();
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
  if (mFlavor == kFT_One && mAlive) {
    CTransform4f tailXf = GetLctrTransform(rstl::string_l(kBetaLctrName));

    if (mDamageCooldownTimer >= 0.f && mColor.GetAlphau8() == 255) {
      if (mTailModel.valid()) {
        mTailModel->Render(mgr, tailXf, GetActorLights(),
                               CModelFlags(CModelFlags::kT_Two, mColor));
      }
    } else if (mTailModel.valid()) {
      mTailModel->Render(mgr, tailXf, GetActorLights(),
                             CModelFlags(CModelFlags::kT_Opaque, 1.f));
    }
  }

  CPatterned::Render(mgr);
}

void CBeetle::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (mAlive) {
    if (mFlavor == kFT_One) {
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
  if (mAiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mAiMgr))) {
      aiMgr->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Unknown,
                              CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CBeetle::SquadRemove(CStateManager& mgr) {
  if (mAiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mAiMgr))) {
      if (aiMgr->IsPartOfTeam(GetUniqueId())) {
        aiMgr->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

CVector3f CBeetle::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                             const CVector3f& aimPos) const {
  const float midRange = 0.5f * (mMinAttackRange + mMaxAttackRange);
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
                CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, ai->GetUniqueId())) {
          if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee) {
            dist *= 2.f;
          }
        }

        const CVector3f move = mSteeringBehaviors.Separation(*this, ai->GetTranslation(), dist);
        if (move.IsNonZero()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CBeetle::SetupRetreatPoints(CStateManager& mgr) {
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->mState == kSS_Retreat && conn->mMsg == kSM_Follow) {
      TUniqueId wpId = mgr.GetIdForScript(conn->mObjId);
      if (const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wpId))) {
        mRetreatPoints.push_back(wp->GetTranslation());
        if (mRetreatPoints.size() == 8) {
          break;
        }
      }
    }
  }
}

s32 CBeetle::FindFurthestRetreatPoint(CStateManager& mgr) {
  s32 ret = -1;
  if (mRetreatPoints.size() > 0) {
    CVector2f playerPos = mgr.GetPlayer()->GetTranslation().ToVec2f();

    ret = mgr.Random()->Range(0, mRetreatPoints.size() - 1);

    CVector2f retreatPos(mRetreatPoints[ret].GetX(), mRetreatPoints[ret].GetY());
    CVector2f maxDelta = playerPos - retreatPos;
    float maxDist = maxDelta.MagSquared();
    if (maxDist < 100.f) {
      for (int i = 0; i < mRetreatPoints.size(); ++i) {
        CVector2f pointPos(mRetreatPoints[i].GetX(), mRetreatPoints[i].GetY());
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
  if (mAlive) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_XRay:
      mColor.SetAlpha(0.3f);
      break;
    case CPlayerState::kPV_Thermal:
      if (mBurrowing) {
        mColor.SetAlpha(mIntoGroundFactor);
      } else {
        mColor.SetAlpha(1.f);
      }
      break;
    default:
      mColor.SetAlpha(1.f);
      break;
    }
  }

  CPatterned::PreRender(mgr, frustum);
}

void CBeetle::RefinePathFindDest(CStateManager& mgr, CVector3f& dest) {
  dest = mgr.GetPlayer()->GetTranslation();

  if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId())) {
    dest = role->GetTeamPosition();
  } else {
    CVector3f thisToDest = dest - GetTranslation();
    const CVector3f thisToDestNorm =
        thisToDest.CanBeNormalized() ? -thisToDest.AsNormalized() : GetTransform().GetForward();
    const float midRange = 0.5f * (mMinAttackRange + mMaxAttackRange);
    dest += midRange * thisToDestNorm;
  }
}

float CBeetle::GetGravityConstant() const { return 4.f * GravityConstant(); }

bool CBeetle::IsListening() const { return true; }

bool CBeetle::PathShagged(CStateManager& mgr, float arg) { return false; }

bool CBeetle::NoPathNodes(CStateManager& mgr, float arg) { return false; }

CPathFindSearch* CBeetle::GetSearchPath() { return &mPathFindSearch; }
