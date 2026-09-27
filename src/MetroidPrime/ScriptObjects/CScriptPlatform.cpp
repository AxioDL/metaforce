#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"

#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

#include "Kyoto/Graphics/CGX_Impl.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "WorldFormat/CCollidableOBBTreeGroup.hpp"

#include "rstl/algorithm.hpp"

#include "dolphin/gx/GXCommandList.h"
#include "dolphin/gx/GXVert.h"

#ifndef TARGET_PC
struct GXData {
  ushort cpSRreg;
  ushort cpCRreg;
};
extern GXData* __GXData;

static inline void write_bp_cmd(u32 cmd) {
  GXWGFifo.u8 = GX_LOAD_BP_REG;
  GXWGFifo.u32 = cmd;
  __GXData->cpCRreg = 0;
}
#endif

void CGX::update_fog(uint flags) {
  if (sGXState.mFogType == 0) {
    return;
  }
  if ((sGXState.mBlendMode & 0xE0) == (flags & 0xE0)) {
    return;
  }
  if ((flags & 0xE0) == 0x20) {
#ifdef TARGET_PC
    static const GXColor sGXClear = {0, 0, 0, 0};
    GXSetFogColor(sGXClear);
#else
    write_bp_cmd(0xf2000000);
#endif
  } else {
#ifdef TARGET_PC
    GXSetFogColor(sGXState.mFogParams.mFogColor);
#else
    write_bp_cmd((sGXState.mFogParams.mFogColor.b) |
                 (sGXState.mFogParams.mFogColor.g << 8) |
                 (sGXState.mFogParams.mFogColor.r << 16) | 0xf2000000);
#endif
  }
}

CScriptPlatform::CScriptPlatform(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& mData, const CActorParameters& actParams, const CAABox& aabb, float speed,
    bool detectCollision, float xrayAlpha, bool active, const CHealthInfo& hInfo,
    const CDamageVulnerability& dVuln,
    const rstl::optional_object< TLockedToken< CCollidableOBBTreeGroupContainer > >& dcln,
    bool rainSplashes, uint maxRainSplashes, uint rainGenRate)
: CPhysicsActor(uid, active, name, info, xf, mData,
                CMaterialList(kMT_Solid, kMT_Immovable, kMT_Platform, kMT_Occluder), aabb,
                SMoverData(15000.f, CVector3f::Zero(), CAxisAngle::Identity(), CVector3f::Zero(),
                           CAxisAngle::Identity()),
                actParams, 0.3f, 0.1f)
, mCurrentWaypoint(kInvalidUniqueId)
, mTargetWaypoint(kInvalidUniqueId)
, mCurrentSpeed(speed)
, mMoveDelay(0.f)
, mCollisionRecoverDelay(0.f)
, mFadeInTime(actParams.GetFadeInTime())
, mFadeOutTime(actParams.GetFadeOutTime())
, mDragDelta(CVector3f::Zero())
, mRotDelta(CQuaternion::NoRotation())
, mInitialHealth(hInfo)
, mHealth(hInfo)
, mDamageVuln(dVuln)
, mTreeGroupContainer(dcln)
, mTreeGroup(nullptr)
, mXrayAlpha(xrayAlpha)
, mMaxRainSplashes(maxRainSplashes)
, mRainGenRate(rainGenRate)
, mBoundsTrigger(kInvalidUniqueId)
, mDead(false)
, mControlledAnimation(false)
, mDetectCollision(detectCollision)
, mSquishedRider(false)
, mRainSplashes(rainSplashes)
, mSetXrayDrawFlags(false)
, mDisableXrayAlpha(false)
, mXrayFog(true) {
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_NoStaticCollision, kMT_NoPlatformCollision, kMT_Platform)));
  SetMovable(false);
  if (HasAnimation()) {
    AnimationData()->EnableLooping(true);
    AnimationData()->SetIsAnimating(true);
  }
  if (mTreeGroupContainer) {
    mTreeGroup = rs_new CCollidableOBBTreeGroup(**mTreeGroupContainer, GetMaterialList());
  }
}

CScriptPlatform::~CScriptPlatform() {}

rstl::optional_object< CAABox > CScriptPlatform::GetTouchBounds() const {
  if (GetActive()) {
    if (!mTreeGroup.null()) {
      return mTreeGroup->CalculateAABox(GetTransform());
    } else {
      return GetBoundingBox();
    }
  } else {
    return rstl::optional_object_null();
  }
}

TUniqueId CScriptPlatform::GetWaypoint(CStateManager& mgr) {
  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->mMsg == kSM_Follow) {
      return mgr.GetIdForScript(conn->mObjId);
    }
  }
  return kInvalidUniqueId;
}

TUniqueId CScriptPlatform::GetNext(TUniqueId uid, CStateManager& mgr) {
  const CScriptWaypoint* nextWp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(uid));
  if (!nextWp) {
    return GetWaypoint(mgr);
  }
  TUniqueId next = nextWp->NextWaypoint(mgr);
  if (const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(next))) {
    mCurrentSpeed = wp->GetSpeed();
  }
  return next;
}

void CScriptPlatform::AddRider(rstl::vector< SRiders >& riders, TUniqueId riderId,
                               const CPhysicsActor* ridee, CStateManager& mgr) {
  rstl::vector< SRiders >::iterator it = rstl::find(riders.begin(), riders.end(), SRiders(riderId));
  if (it == riders.end()) {
    SRiders rider(riderId);
    if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(mgr.ObjectById(riderId))) {
      CVector3f rideePos = ridee->GetTranslation();
      rider.mTransform.SetTranslation(
          ridee->GetTransform().TransposeRotate(act->GetTranslation() - rideePos));
      mgr.DeliverScriptMsg(act, ridee->GetUniqueId(), kSM_AddPlatformRider);
    }
    riders.reserve(riders.size() + 1);
    riders.push_back(rider);
  } else {
    (*it).mDecayTimer = 1.f / 6.f;
  }
}

TEntityList CScriptPlatform::BuildNearListFromRiders(CStateManager& mgr,
                                                     const rstl::vector< SRiders >& riders) {
  TEntityList result;
  rstl::vector< SRiders >::const_iterator it = riders.begin();
  for (; it != riders.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->mUid))) {
      result.push_back(actor->GetUniqueId());
    }
  }
  return result;
}

void CScriptPlatform::DecayRiders(rstl::vector< SRiders >& riders, float dt, CStateManager& mgr) {
  rstl::vector< SRiders >::iterator it = riders.begin();
  while (it != riders.end()) {
    (*it).mDecayTimer -= dt;
    if ((*it).mDecayTimer <= 0.f) {
      mgr.SendScriptMsgAlways((*it).mUid, kInvalidUniqueId, kSM_AddPlatformRider);
#if NONMATCHING
      it = riders.erase(it);
#else
      // Oops, forgot to reassign the iterator
      riders.erase(it);
#endif
      continue;
    }
    ++it;
  }
}

void CScriptPlatform::MoveRiders(CStateManager& mgr, float dt, bool active,
                                 rstl::vector< SRiders >& riders,
                                 rstl::vector< SRiders >& collidedRiders, const CTransform4f& oldXf,
                                 const CTransform4f& newXf, const CVector3f& dragDelta,
                                 CQuaternion rotDelta) {
  rstl::vector< SRiders >::iterator it = riders.begin();
  while (it != riders.end()) {
    if (active) {
      CPhysicsActor* act = TCastToPtr< CPhysicsActor >(mgr.ObjectById(it->mUid));
      if (act == nullptr || !act->GetActive()) {
        ++it;
        continue;
      }
      // Retail retains the implicit copy helper for this unused physics state.
      if (false) {
        CPhysicsState state = act->GetPhysicsState();
      }

      const CTransform4f& xf = it->mTransform;
      CVector3f diff = newXf.Rotate(xf.GetTranslation()) - oldXf.Rotate(xf.GetTranslation());
      diff.SetZ(0.f);
      CVector3f delta = dragDelta + diff;
      CVector3f newPos = act->GetTranslation() + delta;
      act->MoveCollisionPrimitive(delta);
      bool collision = CGameCollision::DetectStaticCollisionBoolean(
          mgr, *act->GetCollisionPrimitive(), act->GetPrimitiveTransform(),
          act->GetMaterialFilter());
      act->MoveCollisionPrimitive(CVector3f::Zero());
      if (collision) {
        AddRider(collidedRiders, act->GetUniqueId(), act, mgr);
#if NONMATCHING
        it = riders.erase(it);
#else
        // Oops, forgot to reassign the iterator (again)
        riders.erase(it);
#endif
        continue;
      }
      act->SetTranslation(newPos);
      const CPlayer* player = TCastToConstPtr< CPlayer >(*act);
      if (player == nullptr || player->GetOrbitState() == CPlayer::kOS_NoOrbit) {
        act->SetRotation(act->TransformLocalToWorldRotation(rotDelta));
      }
    }
    ++it;
  }
}

void CScriptPlatform::PreThink(float dt, CStateManager& mgr) {
  DecayRiders(mRiders, dt, mgr);
  mCollisionRecoverDelay -= dt;
  mMoveDelay -= dt;
  if (!(mMoveDelay > 0.f)) {
    mDragDelta = CVector3f::Zero();
    CTransform4f oldXf = GetTransform();
    CMotionState mState = GetMotionState();
    if (GetActive()) {
      rstl::vector< SRiders >::iterator it = mRiders.begin();
      for (; it != mRiders.end(); ++it) {
        if (const CPhysicsActor* act =
                TCastToConstPtr< CPhysicsActor >(mgr.ObjectById(it->mUid))) {
          CVector3f actPos = act->GetTranslation();
          CVector3f pos = GetTranslation();
          it->mTransform.SetTranslation(GetTransform().TransposeRotate(actPos - pos));
        }
      }
      mRotDelta = Move(dt, mgr);
    }

    CTransform4f newXf = GetTransform();
    mDragDelta = newXf.GetTranslation() - oldXf.GetTranslation();

    rstl::vector< SRiders > collidedRiders;
    MoveRiders(mgr, dt, GetActive(), mRiders, collidedRiders, oldXf, newXf, mDragDelta,
               mRotDelta);
    mSquishedRider = false;
    if (!collidedRiders.empty()) {
      TEntityList nearList = BuildNearListFromRiders(mgr, collidedRiders);
      if (CGameCollision::DetectDynamicCollisionBoolean(*GetCollisionPrimitive(),
                                                        GetPrimitiveTransform(), nearList, mgr)) {
        SetMotionState(mState);
        Stop();
        mMoveDelay = 0.035f;
        MoveRiders(
            mgr, dt, GetActive(), mRiders, collidedRiders, newXf, oldXf, -mDragDelta,
#if VERSION >= VERSION_GM8E_02
            mRotDelta.BuildInverted());
#else
            CQuaternion::ScalarVector(mRotDelta.GetScalar(),
                                      static_cast< const CVector3f& >(-mRotDelta.GetVector())));
#endif
        mDragDelta = CVector3f::Zero();
        SendScriptMsgs(kSS_Modify, mgr, kSM_None);
        mSquishedRider = true;
      }
    }
  }
}

void CScriptPlatform::BuildSlaveList(CStateManager& mgr) {
  mSlavesStatic.reserve(GetConnectionList().size());
  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->mState == kSS_Play && conn->mMsg == kSM_Activate) {
      if (CActor* const act =
              TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(conn->mObjId)))) {
        act->AddMaterial(kMT_PlatformSlave, mgr);
        CTransform4f xf = act->GetTransform();
        xf.SetTranslation(act->GetTranslation() - GetTranslation());
        mSlavesStatic.push_back(SRiders(act->GetUniqueId(), 1.f / 6.f, xf));
      }
    } else if (conn->mState == kSS_InheritBounds && conn->mMsg == kSM_Activate) {
      CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->mObjId);
      CStateManager::TIdList::const_iterator current = search.first;
      CStateManager::TIdList::const_iterator end = search.second;
      while (current != end) {
        if (TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(current->second))) {
          mBoundsTrigger = current->second;
        }
        ++current;
      }
    }
  }
}

void CScriptPlatform::DragSlave(CStateManager& mgr, TMovedList& moved, CActor* actor,
                                const CVector3f& delta) {
  if (IsInMovedList(actor->GetUniqueId(), moved)) {
    return;
  }
  moved.push_back(actor->GetUniqueId().Value());
  CTransform4f xf = actor->GetTransform();
  xf.AddTranslation(delta);
  actor->SetTransform(xf);
  if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(actor)) {
    platform->DragSlaves(mgr, moved, delta);
  }
}

void CScriptPlatform::DragSlaves(CStateManager& mgr, TMovedList& moved, const CVector3f& delta) {
  for (AUTO(it, mSlavesStatic.begin()); it != mSlavesStatic.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->mUid))) {
      DragSlave(mgr, moved, actor, delta);
    }
  }
  AUTO(it, mSlavesDynamic.begin());
  while (it != mSlavesDynamic.end()) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->mUid))) {
      DragSlave(mgr, moved, actor, delta);
      ++it;
    } else {
      it = mSlavesDynamic.erase(it);
    }
  }
}

void CScriptPlatform::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (HasAnimation()) {
    if (!mControlledAnimation) {
      UpdateAnimation(dt, mgr, true);
    }
    if (mRainSplashes) {
      const CWorld* world = mgr.GetWorld();
      const CEnvFxManager* envFx = mgr.GetEnvFxManager();
      if (world->GetNeededEnvFx() == kEFX_Rain && HasModelData()) {
        if (envFx->GetRainMagnitude()) {
          mgr.ActorModelParticles()->AddRainSplashGenerator(*this, mgr, mMaxRainSplashes,
                                                            mRainGenRate, 0.f);
        }
      }
    }
  }
  if (!mSlavesStatic.empty() || !mSlavesDynamic.empty()) {
    TMovedList moved;
    DragSlaves(mgr, moved, mDragDelta);
  }
  if (!mDead && HealthInfo(mgr)->GetHP() <= 0.f) {
    mDead = true;
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
  }
}

bool CScriptPlatform::IsInMovedList(TUniqueId id, const TMovedList& moved) {
  const ushort index = id.Value();
  for (AUTO(it, moved.begin()); it != moved.end(); ++it) {
    if (index == *it) {
      return true;
    }
  }
  return false;
}

CHealthInfo* CScriptPlatform::HealthInfo(CStateManager&) { return &mHealth; }

const CDamageVulnerability* CScriptPlatform::GetDamageVulnerability() const {
  return &mDamageVuln;
}

void CScriptPlatform::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_InitializedInArea:
    BuildSlaveList(mgr);
    break;
  case kSM_AddPlatformRider:
    AddRider(mRiders, uid, this, mgr);
    break;
  case kSM_Stop:
    mCurrentSpeed = 0.f;
    Stop();
    break;
  case kSM_Next:
    mTargetWaypoint = GetNext(mCurrentWaypoint, mgr);
    if (mTargetWaypoint == kInvalidUniqueId) {
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Stop);
    } else if (CScriptWaypoint* wp =
                   TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mTargetWaypoint))) {
      mCurrentSpeed = 0.f;
      Stop();
      mDragDelta = wp->GetTranslation() - GetTranslation();
      SetTranslation(wp->GetTranslation());
      mCurrentWaypoint = mTargetWaypoint;
      mTargetWaypoint = GetNext(mCurrentWaypoint, mgr);
      mgr.DeliverScriptMsg(wp, GetUniqueId(), kSM_Arrived);
      if (!mSlavesStatic.empty() || !mSlavesDynamic.empty()) {
        TMovedList moved;
        DragSlaves(mgr, moved, mDragDelta);
      }
      mDragDelta = CVector3f::Zero();
    }
    break;
  case kSM_Start:
    mTargetWaypoint = GetNext(mCurrentWaypoint, mgr);
    if (mTargetWaypoint == kInvalidUniqueId) {
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Stop);
    } else if (const CScriptWaypoint* wp =
                   TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mTargetWaypoint))) {
      mCurrentSpeed = wp->GetSpeed();
    }
    break;
  case kSM_Reset:
    mDead = false;
    mHealth = mInitialHealth;
    break;
  case kSM_Increment:
    if (!GetActive()) {
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Activate);
    }
    CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), mFadeInTime);
    break;
  case kSM_Decrement:
    CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), mFadeOutTime);
    break;
  case kSM_Deleted:
    DecayRiders(mRiders, 1.66666675f, mgr);
    break;
  default:
    break;
  }
  CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptPlatform)

const CCollisionPrimitive* CScriptPlatform::GetCollisionPrimitive() const {
  if (mTreeGroup.null()) {
    return CPhysicsActor::GetCollisionPrimitive();
  }
  return mTreeGroup.get();
}

CTransform4f CScriptPlatform::GetPrimitiveTransform() const {
  CTransform4f xf = GetTransform();
  xf.AddTranslation(GetPrimitiveOffset());
  return xf;
}

void CScriptPlatform::SplashThink(const CAABox&, const CFluidPlane&, float, CStateManager&) const {}

void CScriptPlatform::AddSlave(TUniqueId id, CStateManager& mgr) {
  if (rstl::find(mSlavesDynamic.begin(), mSlavesDynamic.end(), SRiders(id)) ==
      mSlavesDynamic.end()) {
    if (CActor* const act = TCastToPtr< CActor >(mgr.ObjectById(id))) {
      act->AddMaterial(kMT_PlatformSlave, mgr);
      CTransform4f xf = GetTransform().GetQuickInverse() * act->GetTransform();
      mSlavesDynamic.reserve(mSlavesDynamic.size() + 1);
      mSlavesDynamic.push_back(SRiders(id, 1.f / 6.f, xf));
    }
  }
}

bool CScriptPlatform::IsRider(TUniqueId id) const {
  // Retain both vector erase instantiations in retail order.
  if (false) {
    rstl::vector< SRiders >& riders = const_cast< rstl::vector< SRiders >& >(mRiders);
    riders.erase(riders.begin());
  }

  return rstl::find(mRiders.begin(), mRiders.end(), SRiders(id)) != mRiders.end();
}

bool CScriptPlatform::IsSlave(TUniqueId id) const {
  return rstl::find(mSlavesStatic.begin(), mSlavesStatic.end(), SRiders(id)) !=
             mSlavesStatic.end() ||
         rstl::find(mSlavesDynamic.begin(), mSlavesDynamic.end(), SRiders(id)) !=
             mSlavesDynamic.end();
}

CQuaternion CScriptPlatform::Move(float dt, CStateManager& mgr) {
  TUniqueId nextWaypoint = mTargetWaypoint;
  if (nextWaypoint == kInvalidUniqueId) {
    nextWaypoint = GetNext(mCurrentWaypoint, mgr);
  }
  const CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(nextWaypoint));
  if (mCurrentWaypoint != kInvalidUniqueId && wp && !wp->GetActive()) {
    nextWaypoint = GetNext(mCurrentWaypoint, mgr);
    if (nextWaypoint == kInvalidUniqueId) {
      if (const CScriptWaypoint* current =
              TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mCurrentWaypoint))) {
        if (current->GetActive()) {
          nextWaypoint = mCurrentWaypoint;
        }
      }
    }
  }
  if (nextWaypoint == kInvalidUniqueId) {
    return CQuaternion::NoRotation();
  }

  while (nextWaypoint != kInvalidUniqueId) {
    CScriptWaypoint* const waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(nextWaypoint));
    if (!waypoint) {
      nextWaypoint = kInvalidUniqueId;
      break;
    }
    const CVector3f delta = waypoint->GetTranslation() - GetTranslation();
    if (close_enough(delta, CVector3f::Zero(), 0.02f)) {
      mCurrentWaypoint = nextWaypoint;
      mgr.DeliverScriptMsg(waypoint, GetUniqueId(), kSM_Arrived);
      if (close_enough(mCurrentSpeed, 0.f, 0.02f)) {
        nextWaypoint = GetNext(mCurrentWaypoint, mgr);
        mCurrentSpeed = 0.f;
        Stop();
      } else {
        nextWaypoint = GetNext(mCurrentWaypoint, mgr);
      }
      if (kInvalidUniqueId != nextWaypoint) {
        continue;
      }
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Stop);
    }

    if (close_enough(delta, CVector3f::Zero(), 0.02f)) {
      mDragDelta = waypoint->GetTranslation() - GetTranslation();
      MoveToWR(GetTranslation(), dt);
    } else {
      const CVector3f moveDelta = dt * (mCurrentSpeed * delta.AsNormalized());
      if (moveDelta.MagSquared() > delta.MagSquared()) {
        mDragDelta = waypoint->GetTranslation() - GetTranslation();
        MoveToWR(waypoint->GetTranslation(), dt);
      } else {
        mDragDelta = moveDelta;
        MoveToWR(GetTranslation() + moveDelta, dt);
      }
    }

    TEntityList nearList;
    const CAABox volume = GetMotionVolume(dt);
    mgr.BuildColliderList(nearList, *this, volume);
    TEntityList nonRiders;
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      if (!IsRider(*it) && !IsSlave(*it)) {
        nonRiders.push_back(*it);
      }
    }

    if (mDetectCollision) {
      const CMotionState state = PredictMotion(dt);
      MoveCollisionPrimitive(state.GetTranslation());
      const bool collision = CGameCollision::DetectDynamicCollisionBoolean(
          *GetCollisionPrimitive(), GetPrimitiveTransform(), nonRiders, mgr);
      MoveCollisionPrimitive(CVector3f::Zero());
      if (collision || mSquishedRider) {
        if (mDetectCollision) {
          if (mCollisionRecoverDelay <= 0.f && !mSquishedRider) {
            mCollisionRecoverDelay = 0.035f;
            break;
          } else {
            mSquishedRider = false;
            const TUniqueId previousWaypoint = nextWaypoint;
            nextWaypoint = GetNext(nextWaypoint, mgr);
            if (nextWaypoint == mTargetWaypoint || previousWaypoint == mTargetWaypoint) {
              mMoveDelay = 0.035f;
              break;
            }
          }
        } else {
          break;
        }
      } else {
        AddMotionState(state);
        break;
      }
    } else {
      SetMovable(true);
      CGameCollision::Move(mgr, *this, dt, &nonRiders);
      SetMovable(false);
      break;
    }
  }
  mTargetWaypoint = nextWaypoint;
  return CQuaternion::NoRotation();
}

void CScriptPlatform::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);
  if (!GetPreRenderClipped() && !close_enough(mXrayAlpha, 1.f)) {
    const CModelFlags flags = CModelFlags::AlphaBlended(mXrayAlpha);
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay &&
        !mDisableXrayAlpha) {
      SetModelFlags(flags);
      mSetXrayDrawFlags = true;
    } else if (mSetXrayDrawFlags) {
      mSetXrayDrawFlags = false;
      if (GetModelFlags() == flags && !mDisableXrayAlpha) {
        SetModelFlags(CModelFlags::Normal());
      }
    }
  }
  if (!mgr.GetObjectById(mBoundsTrigger)) {
    mBoundsTrigger = kInvalidUniqueId;
  }
}

void CScriptPlatform::SetDamageVulnerability(const CDamageVulnerability& vuln) {
  mDamageVuln = vuln;
}

CVector3f CScriptPlatform::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

CVector3f CScriptPlatform::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (GetTouchBounds()) {
    return GetTouchBounds()->GetCenterPoint();
  }
  return CPhysicsActor::GetAimPosition(mgr, dt);
}

CAABox CScriptPlatform::GetSortingBounds(const CStateManager& mgr) const {
  if (mBoundsTrigger != kInvalidUniqueId) {
    if (const CScriptTrigger* trigger =
            static_cast< const CScriptTrigger* >(mgr.GetObjectById(mBoundsTrigger))) {
      return trigger->GetTriggerBoundsWR();
    }
  }
  return CActor::GetSortingBounds(mgr);
}

void CScriptPlatform::Render(const CStateManager& mgr) const {
  const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
  if (xray && !mXrayFog) {
    gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
  }
  CPhysicsActor::Render(mgr);
  if (xray && !mXrayFog) {
    mgr.SetupFogForArea(GetCurrentAreaId());
  }
}
