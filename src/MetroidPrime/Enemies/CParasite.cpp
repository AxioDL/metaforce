#include "MetroidPrime/Enemies/CParasite.hpp"
#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

static const SSphereJointInfo skIceJoints[] = {{"Ice_LCTR", 0.4f}};
float CParasite::skAttackTime = 2.f * CMath::SqrtF(2.5f / CPhysicsActor::GravityConstant());
float CParasite::skAttackVelocity = 15.f / skAttackTime;
float CParasite::skRetreatTime = 2.f * CMath::SqrtF(2.5f / CPhysicsActor::GravityConstant());
float CParasite::skRetreatVelocity = 3.f / skRetreatTime;
static TUniqueId lastParasite = TUniqueId(0, 0);

CParasite::CParasite(TUniqueId uid, const rstl::string& name, CPatterned::EFlavorType flavor,
                     const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
                     const CPatternedInfo& pInfo, EBodyType bodyType, float maxTelegraphReactDist,
                     float advanceWpRadius, float f3, float alignAngVel, float f5,
                     float stuckTimeThreshold, float collisionCloseMargin,
                     float parasiteSearchRadius, float parasiteSeparationDist,
                     float parasiteSeparationWeight, float parasiteAlignmentWeight,
                     float parasiteCohesionWeight, float destinationSeekWeight,
                     float forwardMoveWeight, float playerSeparationDist,
                     float playerSeparationWeight, float playerObstructionMinDist, float haltDelay,
                     bool disableMove, CWallWalker::EType wType, const CDamageVulnerability& dVuln,
                     const CDamageInfo& dInfo, ushort haltSfx, ushort getUpSfx, ushort crouchSfx,
                     CAssetId modelRes, CAssetId skinRes, float iceZoomerJointHP,
                     const CActorParameters& aParams)
: CWallWalker(kC_Parasite, uid, name, flavor, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero, bodyType,
              aParams, kCS_Small, collisionCloseMargin, wType, disableMove, alignAngVel,
              advanceWpRadius, playerObstructionMinDist)
, mDoorRepulsors()
, mStateProgress(-1)
, x5ec_(0.f, 0.f, 0.f)
, mTargetPos(0.f, 0.f, 0.f)
, mActiveSpeed(1.f)
, mTelegraphRemTime(0.f)
, mStuckTime(0.f)
, mLastStuckPos(CVector3f::Zero())
, mCollisionActorManager(nullptr)
, mExtraModel(nullptr)
, mParasiteSeparationMove(0.f, 0.f, 0.f)
, mParasiteCohesionMove(0.f, 0.f, 0.f)
, mParasiteAlignmentMove(0.f, 0.f, 0.f)
, mOculusHaltDVuln(dVuln)
, mOculusHaltDInfo(dInfo)
, mMaxTelegraphReactDist(maxTelegraphReactDist)
, x6d4_(f3)
, x6dc_(f5)
, mStuckTimeThreshold(stuckTimeThreshold)
, mParasiteSearchRadius(parasiteSearchRadius)
, mParasiteSeparationDist(parasiteSeparationDist)
, mParasiteSeparationWeight(parasiteSeparationWeight)
, mParasiteAlignmentWeight(parasiteAlignmentWeight)
, mParasiteCohesionWeight(parasiteCohesionWeight)
, mDestinationSeekWeight(destinationSeekWeight)
, mForwardMoveWeight(forwardMoveWeight)
, mPlayerSeparationDist(playerSeparationDist)
, mPlayerSeparationWeight(playerSeparationWeight)
, mUnmorphedRadius(pInfo.GetHeight() / 2.f)
, mHaltDelay(haltDelay)
, mIceZoomerJointHP(iceZoomerJointHP)
, x718_(0.f)
, x71c_(0.f)
, x720_(0.f)
, x724_(0.f)
, x728_(0.f)
, x72c_(0.f)
, x730_(0.f)
, x734_(0.f)
, x738_(0.f)
, mHaltSfx(CSfxManager::TranslateSFXID(haltSfx))
, mGetUpSfx(CSfxManager::TranslateSFXID(getUpSfx))
, mCrouchSfx(CSfxManager::TranslateSFXID(crouchSfx))
, mReceivedTelegraph(false)
, mJumpVelDirty(false)
, x742_26_(false)
, mLanded(false)
, mOnGround(true)
, x742_29_(false)
, mAttackOver(true)
, x742_31_(false)
, mHalted(false)
, mOculusShotAt(false)
, mInJump(false) {
  SetCallTouch(false);
  switch (mWalkerType) {
  case kWT_Geemer:
    mKnockBackController.SetEnableFreeze(false);
  case kWT_Oculus:
    mKnockBackController.SetAutoResetImpulse(false);
    break;
  case kWT_IceZoomer:
    mExtraModel = rs_new TLockedToken< CSkinnedModel >(
        rs_new CSkinnedModel(gpSimplePool->GetObj(SObjectTag('CMDL', modelRes)),
                             gpSimplePool->GetObj(SObjectTag('CSKR', skinRes)),
                             GetModelData()->GetAnimationData()->GetModelData()->GetLayoutInfo(),
                             CSkinnedModel::kDO_Owned));
    break;
  default:
    break;
  }
  if (mWalkerType == kWT_Oculus) {
    mKnockBackController.SetEnableShock(false);
    mKnockBackController.SetEnableBurn(false);
    mKnockBackController.SetEnableBurnDeath(false);
    mKnockBackController.SetEnableExplodeDeath(false);
    mKnockBackController.SetX82_24(false);
  }
}

ENTITY_ACCEPT_IMPL(CParasite)

void CParasite::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered: {
    mBodyController->Activate(mgr);
    mgr.ActiveParasites().push_back(GetUniqueId());
    CActor::SetDrawShadow(false);
    mActiveSpeed = mSpeed;
    const CVector3f extent(mColSphere.GetSphere().GetRadius(),
                           mColSphere.GetSphere().GetRadius(),
                           mColSphere.GetSphere().GetRadius());
    CPhysicsActor::SetBoundingBox(CAABox(-extent, extent));
    lastParasite = GetUniqueId();
    AddDoorRepulsors(mgr);
    if (mWalkerType == kWT_IceZoomer) {
      SetupIceZoomerCollision(mgr);
      SetupIceZoomerVulnerability(
          mgr, mOculusHaltDVuln,
          CHealthInfo(mIceZoomerJointHP, HealthInfo(mgr)->GetKnockBackResistance()));
    }
    break;
  }
  case kSM_Deleted:
    mgr.ActiveParasites().remove(GetUniqueId());
    if (mWalkerType == kWT_IceZoomer)
      DestroyActorManager(mgr);
    break;
  case kSM_Jumped:
    if (mJumpVelDirty) {
      UpdateJumpVelocity();
      mJumpVelDirty = false;
    }
    break;
  case kSM_Activate:
    mDisableMove = false;
    if (mWalkerType == kWT_Parasite)
      mBodyController->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kSM_InvulnDamage:
    if (mWalkerType == kWT_Oculus) {
      if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
        float distSq = (act->GetTranslation() - GetTranslation()).MagSquared();
        float maxComp = rstl::max_val(
            GetTouchBounds()->GetWidth(),
            rstl::max_val(GetTouchBounds()->GetDepth(), GetTouchBounds()->GetHeight()));
        float maxCompSq = maxComp * maxComp + 1.f;
        if (distSq < maxCompSq * maxCompSq)
          mOculusShotAt = true;
      }
    }
    break;
  case kSM_SuspendedMove:
    if (mCollisionActorManager.get())
      mCollisionActorManager->SetMovable(mgr, false);
    break;
  default:
    break;
  }
}

void CParasite::AddDoorRepulsors(CStateManager& mgr) {
  CObjectList& objects = mgr.ObjectListById(kOL_PhysicsActor);
  int count = 0;
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    CEntity* entity = objects[i];
    if (CScriptDoor* door = TCastToPtr< CScriptDoor >(entity)) {
      if (door->GetCurrentAreaId() == GetCurrentAreaId()) {
        ++count;
      }
    }
  }
  mDoorRepulsors.reserve(count);
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    CEntity* entity = objects[i];
    if (CScriptDoor* door = TCastToPtr< CScriptDoor >(entity)) {
      if (door->GetCurrentAreaId() == GetCurrentAreaId()) {
        rstl::optional_object< CAABox > bounds = door->GetTouchBounds();
        if (bounds.valid()) {
          float diagonal = (bounds->GetMinPoint() - bounds->GetMaxPoint()).Magnitude();
          mDoorRepulsors.push_back(CRepulsor(bounds->GetCenterPoint(), 0.75f * diagonal));
        }
      }
    }
  }
}

void CParasite::PreThink(float dt, CStateManager& mgr) {
  CWallWalker::PreThink(dt, mgr);
  mOculusShotAt = false;
}

void CParasite::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  ++mThinkCounter;
  if (mWalkerType == kWT_IceZoomer)
    UpdateCollisionActors(dt, mgr);

  mPlayerObstructed = false;
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible)
    mPlayerObstructed = true;

  if (!mPlayerObstructed) {
    CVector3f plVec = mgr.GetPlayer()->GetTranslation();
    float distance = (plVec - GetTranslation()).Magnitude();

    if (distance > mPlayerObstructionMinDist) {
      CRayCastResult res =
          mgr.RayStaticIntersection(plVec, (GetTranslation() - plVec).AsNormalized(), distance,
                                    CMaterialFilter::skPassEverything);
      if (res.IsValid())
        mPlayerObstructed = true;
    }
  }

  if (mPlayerObstructed) {
    mMovable = false;
    return;
  }

  mMovable = !mAlignToFloor;

  if (!mDisableMove) {
    if (close_enough(mBodyController->GetPercentageFrozen(), 0.f)) {
      static const float ksMinMoveDistance = 0.3f * dt * mSpeed;
      const CVector3f stuckDelta = GetTranslation() - mLastStuckPos;
      if (stuckDelta.MagSquared() < ksMinMoveDistance * dt)
        mStuckTime += dt;
      else
        mStuckTime = 0.f;

      mLastStuckPos = GetTranslation();
      if (mTelegraphRemTime > 0.f)
        mTelegraphRemTime -= dt;
      else
        mTelegraphRemTime = 0.f;
    }
  }

  if (mAlive) {
    CPlayer* pl = mgr.Player();
    bool useCollisionRadius =
        pl->GetMorphballTransitionState() == CPlayer::kMS_Morphed || !mAttackOver;
    float radius =
        useCollisionRadius ? mColSphere.GetSphere().GetRadius() : mUnmorphedRadius;

    const CVector3f extent(radius, radius, radius);
    CAABox aabox(GetTranslation() - extent, GetTranslation() + extent);
    rstl::optional_object< CAABox > plBox = pl->GetTouchBounds();

    if (plBox.valid() && plBox->DoBoundsOverlap(aabox)) {
      if (!mAttackOver) {
        mAttackOver = true;
        mLanded = false;
      }

      if (mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
  }

  CWallWalker::Think(dt, mgr);

  if (mDisableMove)
    return;

  if (!close_enough(mBodyController->GetPercentageFrozen(), 0.f))
    return;

  mSpeed = mActiveSpeed;
  if (mAlignToFloor)
    AlignToFloor(mgr, mColSphere.GetSphere().GetRadius(),
                 GetTranslation() + 2.f * (dt * GetVelocityWR()), dt);

  mLanded = false;
}

CAdvancementDeltas CParasite::UpdateWalkerAnimation(CStateManager& mgr, float dt) {
  return CActor::UpdateAnimation(dt, mgr, true);
}

void CParasite::UpdateJumpVelocity() {
  SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
  CVector3f velocity = CVector3f::Zero();
  if (!mAttackOver) {
    float speed = skAttackVelocity;
    velocity = speed * GetTransform().GetForward();
    velocity[kDZ] = 0.5f * skAttackVelocity;
  } else {
    float speed = skRetreatVelocity;
    velocity = speed * GetTransform().GetForward();
    velocity[kDZ] = 0.5f * skRetreatVelocity;
  }
  const CVector3f& position = GetTranslation();
  float height = mTargetPos[kDZ] - position[kDZ];
  float acceleration = GetMomentumWR()[kDZ] / GetMass();
  CVector3f delta(mTargetPos[kDX] - position[kDX], mTargetPos[kDY] - position[kDY], 0.f);
  float distance = delta.Magnitude();
  if (distance > FLT_EPSILON) {
    delta *= 1.f / distance;
    float speed = CVector3f::Dot(delta, velocity);
    if (speed > FLT_EPSILON) {
      float time = 0.f;
      bool below = height < 0.f;
      float positiveRoot, negativeRoot;
      if (CSteeringBehaviors::SolveQuadratic(acceleration, velocity[kDZ], -height, positiveRoot,
                                             negativeRoot)) {
        time = below ? negativeRoot : positiveRoot;
      }
      if (!below) {
        time += distance / speed;
      }
      if (time < 10.f) {
        velocity = distance / time * delta;
        velocity[kDZ] = -(0.5f * acceleration * time - height / time);
      }
    }
  }
  SetVelocityWR(velocity);
}

CVector3f CParasite::GetAimPosition(const CStateManager&, float) const { return GetTranslation(); }

void CParasite::ThinkAboutMove(float dt) {
  if (!GetMaterialList().HasMaterial(kMT_GroundCollider))
    CPatterned::ThinkAboutMove(dt);
}

bool CParasite::Stuck(CStateManager&, float) { return mStuckTime > mStuckTimeThreshold; }

bool CParasite::AttackOver(CStateManager&, float) { return mAttackOver; }

bool CParasite::ShotAt(CStateManager&, float) {
  if (mWalkerType != kWT_Oculus)
    return mHitByPlayerProjectile;
  return mOculusShotAt;
}

bool CParasite::CloseToWall(CStateManager& mgr) {
  static const CMaterialFilter kSolidFilter =
      CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CAABox& bounds = CPhysicsActor::GetBoundingBox();
  float radius = mColSphere.GetSphere().GetRadius();
  float margin = radius + mCollisionCloseMargin;
  CAABox expanded(bounds.GetMinPoint() - CVector3f(margin, margin, margin),
                  bounds.GetMaxPoint() + CVector3f(margin, margin, margin));
  CCollidableAABox collisionBox(expanded, GetMaterialList());
  return CGameCollision::DetectStaticCollisionBoolean(mgr, collisionBox, CTransform4f::Identity(),
                                                      kSolidFilter);
}

void CParasite::CollidedWith(const TUniqueId& uid, const CCollisionInfoList& list, CStateManager&) {
  static const CMaterialList testList = CMaterialList(kMT_Player, kMT_Character);
  if (mInJump) {
    for (AUTO(it, list.Begin()); it < list.End(); ++it) {
      const CCollisionInfo& info = *it;
      if (!mAlignToFloor && !testList.SharesMaterials(info.GetMaterialLeft())) {
        const CVector3f normal = info.GetNormalLeft();
        OrientToSurfaceNormal(normal, 360.f);
        CPhysicsActor::Stop();
        SetVelocityWR(CVector3f::Zero());
        mLanded = true;
        mOnGround = true;
      }
    }
  }
}

TUniqueId CParasite::RecursiveFindClosestWayPoint(CStateManager& mgr, TUniqueId id,
                                                  float& dist) const {
  TUniqueId ret = id;
  CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id));
  if (!wp)
    return ret;
  wp->SetActive(false);
  dist = (wp->GetTranslation() - GetTranslation()).MagSquared();
  for (AUTO(it, wp->GetConnectionList().begin()); it != wp->GetConnectionList().end(); ++it) {
    const SConnection& conn = *it;
    if (conn.mState == kSS_Arrived && conn.mMsg == kSM_Next) {
      TUniqueId nextId = mgr.GetIdForScript(conn.mObjId);
      if (nextId != kInvalidUniqueId) {
        if (const CScriptWaypoint* wp2 =
                TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(nextId))) {
          if (wp2->GetActive()) {
            float nextDist;
            TUniqueId closestWp = RecursiveFindClosestWayPoint(mgr, nextId, nextDist);
            if (nextDist < dist) {
              dist = nextDist;
              ret = closestWp;
            }
          }
        }
      }
    }
  }
  wp->SetActive(true);
  return ret;
}

TUniqueId CParasite::GetClosestWaypointForState(EScriptObjectState state,
                                                CStateManager& mgr) const {
  float minDist = FLT_MAX;
  TUniqueId ret = kInvalidUniqueId;
  for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
    const SConnection& conn = *it;
    if (state == conn.mState && conn.mMsg == kSM_Follow) {
      TUniqueId id = mgr.GetIdForScript(conn.mObjId);
      float dist;
      TUniqueId closestWp = RecursiveFindClosestWayPoint(mgr, id, dist);
      if (dist < minDist) {
        minDist = dist;
        ret = closestWp;
      }
    }
  }
  return ret;
}

bool CParasite::AnimOver(CStateManager&, float) { return mStateProgress == 2; }

bool CParasite::Landed(CStateManager&, float) { return mLanded; }

bool CParasite::HitSomething(CStateManager& mgr, float) {
  if (mThinkCounter & 0x1)
    return true;
  return mTumbleAngle < 270.f && CloseToWall(mgr);
}

bool CParasite::ShouldAttack(CStateManager& mgr, float arg) {
  bool shouldAttack = false;
  if (mReceivedTelegraph && mTelegraphRemTime > 0.1f)
    shouldAttack = true;
  return !TooClose(mgr, arg) && InMaxRange(mgr, arg) &&
         (shouldAttack || InDetectionRange(mgr, 0.f));
}

void CParasite::Generate(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProgress = 0;
    break;
  case kStateMsg_Update:
    switch (mStateProgress) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Generate)
        mStateProgress = 1;
      else
        mBodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Zero, CVector3f::Zero(), false, false));
      break;
    case 1:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Generate)
        mStateProgress = 2;
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CParasite::Deactivate(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProgress = 0;
    SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
    mgr.DeleteObjectRequest(GetUniqueId());
    break;
  case kStateMsg_Update:
    switch (mStateProgress) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Generate)
        mStateProgress = 1;
      else
        mBodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_One, CVector3f::Zero(), false, false));
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CParasite::Run(CStateManager&, EStateMsg, float) {
  // Empty
}

void CParasite::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x742_26_ = true;
    mAlignToFloor = true;
    if (mWalkerType == kWT_Parasite)
      mBodyController->SetLocomotionType(pas::kLT_Lurk);
    SetMomentumWR(CVector3f::Zero());
    mMovable = false;
    break;
  case kStateMsg_Update:
    UpdatePFDestination(mgr);
    DoFlockingBehavior(mgr);
    break;
  case kStateMsg_Deactivate:
    mMovable = true;
    mAlignToFloor = false;
    x742_26_ = false;
    break;
  default:
    break;
  }
}

void CParasite::Jump(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    AddMaterial(kMT_GroundCollider, mgr);
    SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
    mOnGround = false;
    mAlignToFloor = false;
    mLanded = false;
    mInJump = true;
    break;
  case kStateMsg_Update:
    SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
    break;
  case kStateMsg_Deactivate:
    RemoveMaterial(kMT_GroundCollider, mgr);
    SetMomentumWR(CVector3f::Zero());
    mOnGround = true;
    mLanded = false;
    mInJump = false;
    break;
  }
}

void CParasite::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetMomentumWR(CVector3f::Zero());
    TUniqueId wpId = GetClosestWaypointForState(kSS_Patrol, mgr);
    if (wpId != kInvalidUniqueId)
      mDestObj = wpId;
    break;
  }
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Update:
    break;
  }
}

void CParasite::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x742_26_ = true;
    mAlignToFloor = true;
    if (!mDisableMove && mWalkerType == kWT_Parasite)
      mBodyController->SetLocomotionType(pas::kLT_Lurk);
    SetMomentumWR(CVector3f::Zero());
    mHasAlignSurface = false;
    mMovable = false;
    break;
  case kStateMsg_Update: {
    const float& pause = mPatrolPauseRemTime;
    if (pause > 0.f) {
      mPatrolPauseRemTime -= dt;
      if (mPatrolPauseRemTime <= 0.f) {
        if (mWalkerType == kWT_Parasite)
          mBodyController->SetLocomotionType(pas::kLT_Lurk);
        mPatrolPauseRemTime = 0.f;
      }
    }
    GotoNextWaypoint(mgr);
    if (mPatrolPauseRemTime <= 0.f && !mDisableMove)
      DoFlockingBehavior(mgr);
    break;
  }
  case kStateMsg_Deactivate:
    mAlignToFloor = false;
    mMovable = true;
    break;
  default:
    break;
  }
}

void CParasite::FaceTarget(CVector3f target) {
  CVector3f delta = target - GetTranslation();
  delta[kDZ] = 0.f;
  CQuaternion rotation = CQuaternion::LookAt(
      CUnitVector3f(CTransform4f::Identity().GetForward(), CUnitVector3f::kN_No),
      CUnitVector3f(delta), CRelAngle::FromDegrees(360.f));
  SetTransform(rotation.BuildTransform4f(GetTranslation()));
}

void CParasite::Attack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    mTelegraphRemTime = 0.f;
    CRandom16& random = *mgr.Random();
    if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      mTargetPos =
          mgr.GetPlayer()->GetTranslation() +
          0.5f * CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, random.Float() - 0.5f);
    } else {
      mTargetPos =
          GetTranslation() +
          15.f * (mgr.GetPlayer()->GetTranslation() +
                  CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, random.Float() + 0.5f) -
                  GetTranslation())
                     .AsNormalized();
    }
    FaceTarget(mTargetPos);
    mStateProgress = 0;
    mAttackOver = false;
    mReceivedTelegraph = false;
    mOnGround = false;
    break;
  }
  case kStateMsg_Update:
    switch (mStateProgress) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Jump) {
        mStateProgress = 1;
      } else {
        mJumpVelDirty = true;
        FaceTarget(mTargetPos);
        mBodyController->CommandMgr().DeliverCmd(CBCJumpCmd(mTargetPos, pas::kJT_Normal));
      }
      break;
    case 1:
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mOnGround = true;
    mAttackOver = true;
    break;
  default:
    break;
  }
}

void CParasite::Retreat(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f dir = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    dir[kDZ] = 0.f;
    if (dir.CanBeNormalized())
      dir = dir.AsNormalized();
    else
      dir = mgr.GetPlayer()->GetTransform().GetForward();
    mTargetPos = GetTranslation() - dir * 3.f;
    FaceTarget(mTargetPos);
    mStateProgress = 0;
    mLanded = false;
    mOnGround = false;
    mJumpVelDirty = true;
    mBodyController->CommandMgr().DeliverCmd(CBCJumpCmd(mTargetPos, pas::kJT_One));
    break;
  }
  case kStateMsg_Update:
    mSpeed = 1.f;
    break;
  case kStateMsg_Deactivate:
    mOnGround = true;
    break;
  }
}

void CParasite::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mTargetPos = mgr.GetPlayer()->GetTranslation() + CVector3f(0.f, 0.f, 1.5f);
    break;
  case kStateMsg_Update:
    mBodyController->FaceDirection3D(
        ProjectVectorToPlane(mTargetPos - GetTranslation(), GetTransform().GetUp()),
        GetTransform().GetForward(), 2.f);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CParasite::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  CPhysicsActor::Stop();
  TelegraphAttack(mgr, kStateMsg_Activate, 0.f);
  SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
  CPatterned::Death(mgr, direction, state);
}

void CParasite::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    rstl::list< TUniqueId >& parasites = mgr.ActiveParasites();
    for (AUTO(it, parasites.begin()); it != parasites.end();) {
      CParasite* other = PATTERNED_CAST_TO(CParasite, mgr.ObjectById(*it));
      if (!other) {
        it = parasites.erase(it);
        continue;
      }
      if (other != this && other->IsAlive() &&
          (other->GetTranslation() - GetTranslation()).MagSquared() <
              mMaxTelegraphReactDist * mMaxTelegraphReactDist) {
        other->mReceivedTelegraph = true;
        other->mTelegraphRemTime = mgr.Random()->Float() * 0.5f + 0.5f;
        other->mTargetPos = GetTranslation();
      }
      ++it;
    }
    mHitByPlayerProjectile = false;
    break;
  }
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Update:
    break;
  }
}

void CParasite::Halt(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateMachineState.SetDelay(mHaltDelay);
    mAnimState = kAS_Ready;
    mHalted = true;
    mAlignToFloor = true;
    if (mWalkerType == kWT_Geemer)
      CSfxManager::AddEmitter(mHaltSfx, GetTranslation(), CVector3f::Zero(), true, false);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_LoopReaction, &CPatterned::TryLoopedReaction, 1);
    mHitByPlayerProjectile = false;
    break;
  case kStateMsg_Deactivate:
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    mAnimState = kAS_NotReady;
    mHalted = false;
    mAlignToFloor = false;
    break;
  default:
    break;
  }
}

void CParasite::Crouch(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    if (mWalkerType == kWT_Geemer)
      CSfxManager::AddEmitter(mCrouchSfx, GetTranslation(), CVector3f::Zero(), true, false);
  }
}

void CParasite::GetUp(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    if (mWalkerType == kWT_Geemer)
      CSfxManager::AddEmitter(mGetUpSfx, GetTranslation(), CVector3f::Zero(), true, false);
  }
}

void CParasite::Touch(CActor& actor, CStateManager& mgr) { CPatterned::Touch(actor, mgr); }

void CParasite::UpdatePFDestination(CStateManager& mgr) {
  // Empty
}

void CParasite::DoFlockingBehavior(CStateManager& mgr) {
  CVector3f upVec = GetTransform().GetUp();
  rstl::reserved_vector< TUniqueId, 1024 > parasiteList;
  float radius = mParasiteSearchRadius;
  const CVector3f position = GetTranslation();
  CAABox aabb(position - CVector3f(radius, radius, radius),
              position + CVector3f(radius, radius, radius));
  if ((mThinkCounter % 6) == 0) {
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    nearList.clear();
    static const CMaterialFilter filter =
        CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
    CParasite* closestParasite = nullptr;
    float minDistSq = 2.f + mParasiteSeparationDist * mParasiteSeparationDist;
    mgr.BuildNearList(nearList, aabb, filter, nullptr);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      if (CParasite* parasite = PATTERNED_CAST_TO(CParasite, mgr.ObjectById(*it))) {
        if (parasite->GetUniqueId() != GetUniqueId() && parasite->IsAlive()) {
          parasiteList.push_back(parasite->GetUniqueId());
          float distSq = (parasite->GetTranslation() - GetTranslation()).MagSquared();
          if (distSq < minDistSq) {
            minDistSq = distSq;
            closestParasite = parasite;
          }
        }
      }
    }
    if (closestParasite && mParasiteSeparationWeight > 0.f && mParasiteSeparationDist > 0.f)
      mParasiteSeparationMove =
          mSteeringBehaviors.Separation(*this, closestParasite->GetTranslation(),
                                            mParasiteSeparationDist) *
          mActiveSpeed;
    else
      mParasiteSeparationMove = CVector3f::Zero();
    mParasiteCohesionMove =
        mSteeringBehaviors.Cohesion(*this, parasiteList, 0.6f, mgr) * mActiveSpeed;
    mParasiteAlignmentMove =
        mSteeringBehaviors.Alignment(*this, parasiteList, mgr) * mActiveSpeed;
  }

  if ((mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
      mPlayerSeparationDist * mPlayerSeparationDist) {
    const CVector3f playerSeparation =
        ProjectVectorToPlane(mSteeringBehaviors.Separation(*this,
                                                               mgr.GetPlayer()->GetTranslation(),
                                                               mPlayerSeparationDist),
                             upVec) *
        mActiveSpeed;
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(playerSeparation, CVector3f::Zero(), mPlayerSeparationWeight));
  }

  if (!(mParasiteSeparationMove == CVector3f::Zero())) {
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(mParasiteSeparationMove, upVec),
                         CVector3f::Zero(), mParasiteSeparationWeight));
  }

  for (AUTO(it, mDoorRepulsors.begin()); it != mDoorRepulsors.end(); ++it) {
    const CRepulsor& r = *it;
    if ((r.GetPos() - GetTranslation()).MagSquared() < r.GetRadius() * r.GetRadius()) {
      const CVector3f doorSeparation =
          mSteeringBehaviors.Separation(*this, r.GetPos(), r.GetRadius()) * mActiveSpeed;
      mBodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(ProjectVectorToPlane(doorSeparation, upVec), CVector3f::Zero(), 1.f));
    }
  }

  if (mTelegraphRemTime <= 0.f) {
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(mParasiteCohesionMove, upVec), CVector3f::Zero(),
                         mParasiteCohesionWeight));
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(mParasiteAlignmentMove, upVec), CVector3f::Zero(),
                         mParasiteAlignmentWeight));
    const CVector3f seek =
        ProjectVectorToPlane(mSteeringBehaviors.Seek(*this, mDestPos), upVec) *
        mActiveSpeed;
    mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        ProjectVectorToPlane(seek, upVec), CVector3f::Zero(), mDestinationSeekWeight));
    mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        GetTransform().GetForward() * mActiveSpeed, CVector3f::Zero(), mForwardMoveWeight));
  }
}

void CParasite::Render(const CStateManager& mgr) const { CWallWalker::Render(mgr); }

const CDamageVulnerability* CParasite::GetDamageVulnerability() const {
  switch (mWalkerType) {
  case kWT_Oculus:
    if (mHalted)
      return &mOculusHaltDVuln;
    break;
  case kWT_IceZoomer:
    if (!mVulnerable)
      return &CDamageVulnerability::ImmuneVulnerability();
    break;
  default:
    break;
  }
  return CAi::GetDamageVulnerability();
}

CDamageInfo CParasite::GetContactDamage() const {
  switch (mWalkerType) {
  case kWT_Oculus:
    if (mHalted)
      return mOculusHaltDInfo;
    return CPatterned::GetContactDamage();
  default:
    return CPatterned::GetContactDamage();
  }
}

void CParasite::SetupIceZoomerCollision(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > descs;
  descs.reserve(2);
  CAnimData& animData = *ModelData()->AnimationData();
  for (int i = 0; i < 1; ++i) {
    const SSphereJointInfo& joint = skIceJoints[i];
    CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
        animData.GetLocatorSegId(rstl::string_l(joint.name)), joint.radius,
        rstl::string_l(joint.name), 0.001f);
    descs.push_back(desc);
  }
  RemoveMaterial(kMT_Solid, mgr);
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  mCollisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), descs, GetActive());
}

void CParasite::DestroyActorManager(CStateManager& mgr) {
  mCollisionActorManager->Destroy(mgr);
}

void CParasite::SetupIceZoomerVulnerability(CStateManager& mgr, const CDamageVulnerability& dVuln,
                                            const CHealthInfo& hInfo) {
  for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& cDesc =
        mCollisionActorManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = cDesc.GetCollisionActorId();
    if (CCollisionActor* const act = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      act->SetDamageVulnerability(dVuln);
      *act->HealthInfo(mgr) = hInfo;
    }
  }
}

void CParasite::UpdateCollisionActors(float dt, CStateManager& mgr) {
  mCollisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  if (!mVulnerable) {
    float totalHP = 0.f;
    for (uint i = 0; i < mCollisionActorManager->GetNumCollisionActors(); ++i) {
      const CJointCollisionDescription& cDesc =
          mCollisionActorManager->GetCollisionDescFromIndex(i);
      const TUniqueId id = cDesc.GetCollisionActorId();
      if (CCollisionActor* cact = TCastToPtr< CCollisionActor >(mgr.ObjectById(id)))
        totalHP += cact->HealthInfo(mgr)->GetHP();
    }
    if (totalHP <= 0.f) {
      mVulnerable = true;
      AddMaterial(kMT_Solid, mgr);
      RemoveMaterial(kMT_ProjectilePassthrough, mgr);
      DestroyActorManager(mgr);
      ModelData()->AnimationData()->SubstituteModelData(*mExtraModel);
    }
  }
}

void CParasite::MassiveDeath(CStateManager& mgr) { CPatterned::MassiveDeath(mgr); }

void CParasite::MassiveFrozenDeath(CStateManager& mgr) { CPatterned::MassiveFrozenDeath(mgr); }

bool CParasite::IsOnGround() const { return mOnGround; }

CParasite::~CParasite() {}
