#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include <float.h>

void CPatterned::Start(CStateManager&, EStateMsg, float) {}

void CPatterned::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mLastPatrolDest == kInvalidUniqueId) {
      mDestObj = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
      mBehaviourOrient = kBO_MoveDir;
      mMoveSpeed = 1.f;
      if (mDestObj != kInvalidUniqueId) {
        if (const CScriptWaypoint* waypoint =
                TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mDestObj))) {
          mBehaviourOrient = static_cast< EBehaviourOrient >(waypoint->GetBehaviourOrient());
          mMoveSpeed = waypoint->GetSpeed();
        }
      }
    } else {
      mDestObj = mLastPatrolDest;
    }
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
    mPatrolState = kPS_Patrol;
    mWaypointPauseRemTime = 0.f;
    break;
  case kStateMsg_Update:
    switch (mPatrolState) {
    case kPS_Patrol:
      if (mInPosition && mDestObj != kInvalidUniqueId) {
        if (const CScriptWaypoint* waypoint =
                TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mDestObj))) {
          if (waypoint->GetPause() > 0.f) {
            mWaypointPauseRemTime = waypoint->GetPause();
            mPatrolState = kPS_Pause;
          }
        }
      }
      if (mDestObj == kInvalidUniqueId) {
        mPatrolState = kPS_Done;
      }
      UpdateDest(mgr);
      ApproachDest(mgr);
      break;
    case kPS_Pause:
      if (mWaypointPauseRemTime <= 0.f) {
        mPatrolState = kPS_Patrol;
      }
      break;
    case kPS_Done:
      if (mDestObj != kInvalidUniqueId) {
        mPatrolState = kPS_Patrol;
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mLastPatrolDest = mDestObj;
    mPatrolState = kPS_Invalid;
    break;
  default:
    break;
  }
}

void CPatterned::FollowPattern(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetupPattern(mgr);
    if (mNoPatternShagging || !IsPatternObstructed(mgr, GetTranslation(), mDestPos)) {
      ApproachDest(mgr);
    } else {
      mCurPattern = mPatterns.size();
      mPatternShagged = true;
    }
    break;
  case kStateMsg_Update:
    if (mInPosition) {
      ++mCurPattern;
      UpdatePatternDestPos(mgr);
      if (!mNoPatternShagging && IsPatternObstructed(mgr, GetTranslation(), mDestPos)) {
        mCurPattern = mPatterns.size();
        mPatternShagged = true;
      } else if (mCurPattern < mPatterns.size()) {
        mReflectedDestPos = GetTranslation();
        mInPosition = false;
        mMoveSpeed = mPatterns[mCurPattern].GetSpeed();
        mBehaviour = static_cast< EBehaviour >(mPatterns[mCurPattern].GetBehaviour());
        mBehaviourOrient =
            static_cast< EBehaviourOrient >(mPatterns[mCurPattern].GetBehaviourOrient());
        mBehaviourModifiers = static_cast< EBehaviourModifiers >(
            mPatterns[mCurPattern].GetBehaviourModifiers());
      }
    } else {
      UpdatePatternDestPos(mgr);
    }
    ApproachDest(mgr);
    break;
  case kStateMsg_Deactivate:
    mPatterns.clear();
    mPatternShagged = false;
    break;
  default:
    break;
  }
}

void CPatterned::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mDestObj = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mDestObj))) {
      SetDestPos(actor->GetTranslation());
    }
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
    break;
  default:
    break;
  }
}

void CPatterned::TargetPlayer(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mDestObj = mgr.GetPlayer()->GetUniqueId();
    SetDestPos(mgr.GetPlayer()->GetTranslation());
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
    break;
  default:
    break;
  }
}

void CPatterned::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mFaceVec = CVector3f::Zero();
    break;
  case kStateMsg_Update:
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_Die));
    if (!mFadeToDeath) {
      if (mBodyController->GetBodyStateInfo().GetCurrentState()->IsDead()) {
        mFadeToDeath = true;
        mAlphaDelta = -1.f / 3.f;
        RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
        AddMaterial(kMT_ProjectilePassthrough, mgr);
      }
    }
    break;
  default:
    break;
  }
}

void CPatterned::PathFind(CStateManager& mgr, EStateMsg msg, float arg) {
  if (GetSearchPath()) {
    switch (msg) {
    case kStateMsg_Deactivate:
      break;
    case kStateMsg_Activate:
      if (GetSearchPath()->Search(GetTranslation(), mDestPos) == CPathFindSearch::kR_Success) {
        mReflectedDestPos = GetTranslation();
        SetDestPos(GetSearchPath()->GetPoint());
        mInPosition = false;
        ApproachDest(mgr);
      }
      break;
    case kStateMsg_Update:
      if (!GetSearchPath()->IsOver()) {
        if (mVerticalMovement || mOnGround) {
          mPathOverCount += 1;
          mPathOverCount &= 3;
        }
        CVector3f position = GetTranslation() + 0.3f * CVector3f::Up();
        mReflectedDestPos = position - (mDestPos - position);
        ApproachDest(mgr);
        CVector3f point = position + GetModelScale().GetY() * GetTransform().GetForward();
        GetSearchPath()->GetSplinePointWithLookahead(
            point, position, skActorApproachDistance * GetModelScale().GetY());
        SetDestPos(point);
        if (GetSearchPath()->SegmentOver(position)) {
          GetSearchPath()->Advance();
        }
      }
      break;
    }
  }
}

bool CPatterned::OffLine(CStateManager&, float arg) {
  CVector3f curLine = GetTranslation() - mReflectedDestPos;
  CVector3f pathLine = mDestPos - mReflectedDestPos;
  float distance = 0.f;
  if (CVector3f::Dot(pathLine, curLine) <= 0.f) {
    distance = curLine.MagSquared();
  } else {
    pathLine.Normalize();
    curLine -= CVector3f::Dot(pathLine, curLine) * pathLine;
    distance = curLine.MagSquared();
    const CVector3f delta = GetTranslation() - mDestPos;
    if (CVector3f::Dot(pathLine, delta) > 0.f) {
      distance = delta.MagSquared();
    }
  }
  return distance > arg * arg;
}

bool CPatterned::InRange(CStateManager& mgr, float arg) {
  float distance = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared();
  float range = 0.5f * (mMinAttackRange + mMaxAttackRange);
  return distance < range * range;
}

bool CPatterned::TooClose(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
         mMinAttackRange * mMinAttackRange;
}

bool CPatterned::InMaxRange(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
         mMaxAttackRange * mMaxAttackRange;
}

bool CPatterned::InDetectionRange(CStateManager& mgr, float arg) {
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  bool result = false;
  if (delta.MagSquared() < mDetectionRange * mDetectionRange) {
    result = true;
    if (mDetectionHeightRange > 0.f) {
      result = delta.GetZ() * delta.GetZ() < mDetectionHeightRange * mDetectionHeightRange;
    }
  }
  return result;
}

bool CPatterned::Leash(CStateManager&, float arg) {
  bool result = mCurPlayerLeashTime > mPlayerLeashTime;
  if (result) {
    const float distance = (mLatestLeashPosition - GetTranslation()).MagSquared();
    result = result && distance > mLeashRadius * mLeashRadius;
  }
  return result;
}

bool CPatterned::SpotPlayer(CStateManager& mgr, float arg) {
  bool result = false;
  CVector3f delta = mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetGunEyePos();
  float dot = CVector3f::Dot(delta, GetTransform().GetForward());
  if (dot > 0.f) {
    float distance = delta.MagSquared();
    result = dot * dot > distance * mDetectionAngle;
  }
  return result;
}

bool CPatterned::PlayerSpot(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    CVector3f center = GetBoundingBox().GetCenterPoint();
    CVector3f delta = center - aim;
    float distance = delta.Magnitude();
    delta *= 1.f / distance;
    CVector3f screen = mgr.GetCameraManager()->GetFirstPersonCamera()->ConvertToScreenSpace(center);
    if (screen.GetZ() > 0.f && screen.GetX() * screen.GetX() < 1.f &&
        screen.GetY() * screen.GetY() < 1.f) {
      const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
      CRayCastResult result = mgr.RayStaticIntersection(aim, delta, distance, filter);
      if (!result.IsValid()) {
        return true;
      }
    }
  }
  return false;
}

bool CPatterned::Landed(CStateManager&, float arg) {
  bool result = mOnGround && !mPrevOnGround;
  mPrevOnGround = mOnGround;
  return result;
}

bool CPatterned::PathOver(CStateManager&, float arg) {
  if (GetSearchPath() && (mVerticalMovement || mOnGround)) {
    return !GetSearchPath()->IsShagged() && GetSearchPath()->IsOver();
  }
  return false;
}

bool CPatterned::PathFound(CStateManager&, float arg) {
  return GetSearchPath() && !GetSearchPath()->IsShagged();
}

bool CPatterned::PathShagged(CStateManager&, float arg) {
  if (GetSearchPath()) {
    if (GetSearchPath()->IsShagged()) {
      return true;
    }
    if (GetSearchPath()->GetCurrentWaypoint() > 0 && mPathOverCount == 0) {
      CVector3f original = GetTranslation() + 0.3f * CVector3f::Up();
      CVector3f point = original;
      GetSearchPath()->GetSplinePoint(point, GetTranslation());
      if ((point - original).MagSquared() >
          4.f * skActorApproachDistance * skActorApproachDistance) {
        return true;
      }
    }
  }
  return false;
}

bool CPatterned::NoPathNodes(CStateManager&, float arg) {
  if (GetSearchPath()) {
    return GetSearchPath()->OnPath(GetTranslation()) != CPathFindSearch::kR_Success;
  }
  return true;
}

bool CPatterned::HasAttackPattern(CStateManager& mgr, float arg) {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

bool CPatterned::HasRetreatPattern(CStateManager& mgr, float arg) {
  return GetConnectedObject(mgr, kSS_Retreat, kSM_Follow) != kInvalidUniqueId;
}

bool CPatterned::PatternOver(CStateManager&, float arg) {
  return mCurPattern >= mPatterns.size();
}

bool CPatterned::PatternShagged(CStateManager&, float arg) { return mPatternShagged; }

bool CPatterned::Attacked(CStateManager&, float arg) { return mHitByPlayerProjectile; }

bool CPatterned::HasPatrolPath(CStateManager& mgr, float arg) {
  return GetConnectedObject(mgr, kSS_Patrol, kSM_Follow) != kInvalidUniqueId;
}

bool CPatterned::InPosition(CStateManager&, float arg) { return mInPosition; }

bool CPatterned::AnimOver(CStateManager&, float arg) { return mAnimState == kAS_Over; }

bool CPatterned::Stuck(CStateManager&, float arg) { return mPredictedLeashTime > 0.2f; }

bool CPatterned::PatrolPathOver(CStateManager&, float arg) {
  return mDestObj == kInvalidUniqueId;
}

bool CPatterned::Delay(CStateManager&, float arg) { return mStateMachineState.GetTime() > arg; }

bool CPatterned::RandomDelay(CStateManager&, float arg) {
  return mStateMachineState.GetTime() > arg * mStateMachineState.GetRandom();
}

bool CPatterned::FixedDelay(CStateManager&, float arg) {
  return mStateMachineState.GetTime() > mStateMachineState.GetDelay();
}

bool CPatterned::CodeTrigger(CStateManager&, float arg) {
  return mStateMachineState.GetCodeTrigger();
}

void CPatterned::ApproachDest(CStateManager& mgr) {
  CVector3f face = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  CVector3f move = mDestPos - GetTranslation();
  if (!mVerticalMovement) {
    move.SetZ(0.f);
    face.SetZ(0.f);
  }
  const CVector3f path = mDestPos - mReflectedDestPos;
  if (CVector3f::Dot(path, move) <= 0.f) {
    mInPosition = true;
  } else if (move.MagSquared() < skActorApproachDistance * skActorApproachDistance) {
    move = path;
  }
  if (!mInPosition) {
    if (move.CanBeNormalized()) {
      move.Normalize();
    }
    switch (mBehaviourOrient) {
    case kBO_Constant:
    case kBO_Three:
      break;
    case kBO_MoveDir:
      face = move;
      break;
    case kBO_Destination:
      if (mCurPattern != 0 && mCurPattern < mPatterns.size()) {
        face = mPatterns[mCurPattern].GetForward();
      } else if (mDestObj != kInvalidUniqueId) {
        if (const CScriptWaypoint* waypoint =
                TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mDestObj))) {
          face = waypoint->GetTransform().GetForward();
        }
      }
      break;
    }
    mFaceVec = face;
    mMoveVec = mMoveSpeed * move;
    if (!KnockbackWhenFrozen()) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(mMoveVec, mFaceVec, 1.f));
    } else if (mBehaviourOrient == kBO_MoveDir ||
               !mBodyController->HasBodyState(pas::kAS_Step)) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(mMoveVec, CVector3f::Zero(), 1.f));
    } else {
      pas::EStepDirection step = FindBestStepDirection(mMoveVec);
      if (step != pas::kSD_Forward) {
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(step, pas::kStep_Normal));
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(mMoveVec, CVector3f::Zero(), 1.f));
      }
      mBodyController->CommandMgr().DeliverTargetVector(mFaceVec);
    }
  } else {
    float maxSpeed = mBodyController->GetBodyStateInfo().GetMaxSpeed();
    if (maxSpeed > FLT_EPSILON) {
      float speed = GetVelocityWR().Magnitude() / maxSpeed;
      CVector3f move = speed * GetTransform().GetForward();
      mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    }
  }
}

void CPatterned::UpdateDest(CStateManager& mgr) {
  if (mInPosition && mDestObj != kInvalidUniqueId) {
    if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mDestObj))) {
      UpdateActorKeyframe(mgr);
      mDestObj = waypoint->NextWaypoint(mgr);
      if (mDestObj != kInvalidUniqueId) {
        mReflectedDestPos = GetTranslation();
        mInPosition = false;
        if (const CScriptWaypoint* next =
                TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mDestObj))) {
          mMoveSpeed = waypoint->GetSpeed();
          mBehaviourOrient = static_cast< EBehaviourOrient >(waypoint->GetBehaviourOrient());
          if (waypoint->GetBehaviourModifiers() & 2) {
            CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
            cmdMgr.DeliverCmd(CBCJumpCmd(next->GetTranslation(), pas::kJT_Normal));
          } else if (waypoint->GetBehaviourModifiers() & 4) {
            TUniqueId nextId = next->NextWaypoint(mgr);
            if (nextId != kInvalidUniqueId) {
              if (const CScriptWaypoint* end =
                      TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(nextId))) {
                CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
                cmdMgr.DeliverCmd(
                    CBCJumpCmd(next->GetTranslation(), end->GetTranslation(), pas::kJT_Normal));
              }
            }
          }
        }
      }
      mgr.DeliverScriptMsg(waypoint, GetUniqueId(), kSM_Arrived);
    }
  }
  if (mDestObj != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mDestObj))) {
      SetDestPos(actor->GetTranslation());
    }
  }
}

void CPatterned::UpdateActorKeyframe(CStateManager& mgr) {
  if (const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mDestObj))) {
    const rstl::vector< SConnection >& connections = waypoint->GetConnectionList();
    for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
      if (it->mState == kSS_Arrived && it->mMsg == kSM_Action) {
        TUniqueId id = mgr.GetIdForScript(it->mObjId);
        if (CScriptActorKeyframe* keyframe =
                TCastToPtr< CScriptActorKeyframe >(mgr.ObjectById(id))) {
          if (keyframe->GetActive() && keyframe->IsPassive()) {
            keyframe->UpdateEntity(GetUniqueId(), mgr);
          }
        }
      }
    }
  }
}

TUniqueId CPatterned::GetConnectedObject(CStateManager& mgr, EScriptObjectState state,
                                         EScriptObjectMessage msg) {
  rstl::reserved_vector< TUniqueId, 8 > ids;
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->mState == state && it->mMsg == msg) {
      TUniqueId id = mgr.GetIdForScript(it->mObjId);
      if (const CEntity* entity = mgr.GetObjectById(id)) {
        if (entity->GetActive()) {
          ids.push_back(id);
          if (ids.capacity() - ids.size() <= 0) {
            break;
          }
        }
      }
    }
  }
  if (ids.size() != 0) {
    return ids[mgr.Random()->Next() % ids.size()];
  }
  return kInvalidUniqueId;
}

bool CPatterned::IsPatternObstructed(CStateManager& mgr, const CVector3f& from,
                                     const CVector3f& to) const {
  TEntityList nearList;
  bool obstructed = false;
  CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
  CVector3f delta = to - from;
  if (delta.CanBeNormalized()) {
    CVector3f direction = delta.AsNormalized();
    float distance = delta.Magnitude();
    mgr.BuildNearList(nearList, from, direction, distance, filter, this);
    TUniqueId id = kInvalidUniqueId;
    const CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    CRayCastResult result =
        mgr.RayWorldIntersection(id, from, direction, distance, solidFilter, nearList);
    obstructed = result.GetValid();
  }
  return obstructed;
}

pas::EStepDirection CPatterned::FindBestStepDirection(const CVector3f& dir) const {
  const CVector3f localDir = GetTransform().TransposeRotate(dir);
  float angle = CVector3f::GetAngleDiff(localDir, CVector3f::Forward());
  if (angle < CMath::Deg2Rad(45.f)) {
    return pas::kSD_Forward;
  }
  if (angle > CMath::Deg2Rad(135.f)) {
    return pas::kSD_Backward;
  }
  if (CVector3f::Dot(localDir, CVector3f::Right()) > 0.f) {
    return pas::kSD_Right;
  }
  return pas::kSD_Left;
}

CVector3f CPatterned::FindPatternDir(CStateManager& mgr) {
  CVector3f dir = CVector3f::Zero();
  switch (mPatternOrient) {
  case kPO_StartToPlayerStart:
    dir = mPatternStartPlayerPos - mPatternStartPos;
    break;
  case kPO_StartToPlayer:
    dir = mgr.GetPlayer()->GetTranslation() - mPatternStartPos;
    break;
  case kPO_ReversePlayerForward:
    dir = -mgr.GetPlayer()->GetTransform().GetForward();
    break;
  case kPO_Forward:
    dir = GetTransform().GetForward();
    break;
  default:
    break;
  }
  return dir;
}

CQuaternion CPatterned::FindPatternRotation(const CVector3f& dir) {
  CVector3f flatDir = dir;
  CVector3f flatDelta = mDestWPDelta;
  flatDir.SetZ(0.f);
  flatDelta.SetZ(0.f);
  flatDelta.Normalize();
  flatDir.Normalize();
  CQuaternion rotation = (flatDelta - flatDir).MagSquared() > 3.99f
                             ? CQuaternion::ZRotation(CRelAngle::FromDegrees(180.f))
                             : CQuaternion::ShortestRotationArc(flatDelta, flatDir);
  if (mVerticalMovement) {
    CVector3f rotated =
        (rotation * CQuaternion(0.f, mDestWPDelta) * rotation.BuildInverted()).GetVector();
    rotation =
        CQuaternion::ShortestRotationArc(rotated.AsNormalized(), dir.AsNormalized()) * rotation;
  }
  return rotation;
}

rstl::pair< CScriptWaypoint*, CScriptWaypoint* >
CPatterned::GetDestWaypoints(CStateManager& mgr) const {
  CScriptWaypoint* first = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mDestObj));
  CScriptWaypoint* second = nullptr;
  if (first) {
    second = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(first->FollowWaypoint(mgr)));
  }
  return rstl::pair< CScriptWaypoint*, CScriptWaypoint* >(first, second);
}

EScriptObjectState CPatterned::GetDesiredAttackState(CStateManager& mgr) const {
  float distance = (GetTranslation() - mgr.GetPlayer()->GetTranslation()).MagSquared();
  if (distance < mMinAttackRange * mMinAttackRange) {
    return kSS_Retreat;
  }
  if (distance > mMaxAttackRange * mMaxAttackRange) {
    return kSS_CloseIn;
  }
  return kSS_Attack;
}

void CPatterned::SetupPattern(CStateManager& mgr) {
  EScriptObjectState state = GetDesiredAttackState(mgr);
  mDestObj = GetConnectedObject(mgr, state, kSM_Follow);
  if (kInvalidUniqueId == mDestObj && state != kSS_Attack) {
    mDestObj = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
  }
  mPatterns.clear();
  if (kInvalidUniqueId != mDestObj) {
    mPatternStartPos = GetTranslation();
    mPatternStartPlayerPos = mgr.GetPlayer()->GetTranslation();
    const rstl::pair< CScriptWaypoint*, CScriptWaypoint* > waypoints = GetDestWaypoints(mgr);
    if (waypoints.first) {
      mPatternTranslate =
          static_cast< EPatternTranslate >(waypoints.first->GetPatternTranslate());
      mPatternOrient = static_cast< EPatternOrient >(waypoints.first->GetPatternOrient());
      mPatternFit = static_cast< EPatternFit >(waypoints.first->GetPatternFit());
      if (waypoints.second) {
        mDestWPDelta = waypoints.second->GetTranslation() - waypoints.first->GetTranslation();
      } else {
        mDestWPDelta = CVector3f::Zero();
      }
      int count = 0;
      CScriptWaypoint* current = waypoints.first;
      do {
        ++count;
        current = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(current->NextWaypoint(mgr)));
        if (!current) {
          break;
        }
      } while (current->GetUniqueId() != waypoints.first->GetUniqueId());
      mPatterns.reserve(count);
      CVector3f origin = CVector3f::Zero();
      switch (mPatternTranslate) {
      case kPT_RelativePlayerStart:
        if (waypoints.second) {
          origin = waypoints.second->GetTranslation();
        }
        break;
      case kPT_Absolute:
        break;
      default:
        origin = waypoints.first->GetTranslation();
        break;
      }
      current = waypoints.first;
      do {
        if (mPatterns.size() >= mPatterns.capacity()) {
          break;
        }
        CVector3f forward = current->GetTransform().GetForward();
        if (mDestWPDelta.IsNonZero()) {
          CVector3f dir = FindPatternDir(mgr);
          forward = FindPatternRotation(dir).Transform(forward);
        }
        mPatterns.push_back(
            CPatternNode(current->GetTranslation() - origin, forward, current->GetSpeed(),
                         current->GetBehaviour(), current->GetBehaviourOrient(),
                         current->GetBehaviourModifiers(), current->GetAnimation()));
        current = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(current->NextWaypoint(mgr)));
        if (!current) {
          break;
        }
      } while (current->GetUniqueId() != waypoints.first->GetUniqueId());
    }
  }
  mPatternShagged = false;
  mCurPattern = 0;
  mInPosition = false;
  mReflectedDestPos = GetTranslation();
  if (!mPatterns.empty()) {
    mMoveSpeed = mPatterns.front().GetSpeed();
    mBehaviour = static_cast< EBehaviour >(mPatterns.front().GetBehaviour());
    mBehaviourOrient =
        static_cast< EBehaviourOrient >(mPatterns.front().GetBehaviourOrient());
    mBehaviourModifiers =
        static_cast< EBehaviourModifiers >(mPatterns.front().GetBehaviourModifiers());
  }
}

void CPatterned::UpdatePatternDestPos(CStateManager& mgr) {
  if (mCurPattern < mPatterns.size()) {
    if (mDestWPDelta.IsNonZero()) {
      CVector3f dir = FindPatternDir(mgr);
      SetDestPos(FindPatternRotation(dir).Transform(mPatterns[mCurPattern].GetPos()));
      switch (mPatternFit) {
      case kPF_Zero: {
        float ratio = mVerticalMovement
                          ? dir.MagSquared() / mDestWPDelta.MagSquared()
                          : (dir.GetX() * dir.GetX() + dir.GetY() * dir.GetY()) /
                                (mDestWPDelta.GetX() * mDestWPDelta.GetX() +
                                 mDestWPDelta.GetY() * mDestWPDelta.GetY());
        SetDestPos(CMath::SqrtF(ratio) * mDestPos);
        break;
      }
      default:
        break;
      }
    } else {
      SetDestPos(mPatterns[mCurPattern].GetPos());
    }
  }
  switch (mPatternTranslate) {
  case kPT_RelativeStart:
    SetDestPos(mDestPos + mPatternStartPos);
    break;
  case kPT_RelativePlayerStart:
    SetDestPos(mDestPos + mPatternStartPlayerPos);
    break;
  case kPT_RelativePlayer:
    SetDestPos(mDestPos + mgr.GetPlayer()->GetTranslation());
    break;
  default:
    break;
  }
}

bool CPatterned::Random(CStateManager&, float arg) {
  return mStateMachineState.GetRandom() < arg;
}

bool CPatterned::FixedRandom(CStateManager&, float arg) {
  return mStateMachineState.GetRandom() < mStateMachineState.GetFixedRandom();
}
