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

#include "Kyoto/Graphics/CGX.hpp"
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
  if (sGXState.x53_fogType == 0) {
    return;
  }
  if ((sGXState.x56_blendMode & 0xE0) == (flags & 0xE0)) {
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
    GXSetFogColor(sGXState.x24c_fogParams.x10_fogColor);
#else
    write_bp_cmd((sGXState.x24c_fogParams.x10_fogColor.b) |
                 (sGXState.x24c_fogParams.x10_fogColor.g << 8) |
                 (sGXState.x24c_fogParams.x10_fogColor.r << 16) | 0xf2000000);
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
, x258_currentWaypoint(kInvalidUniqueId)
, x25a_targetWaypoint(kInvalidUniqueId)
, x25c_currentSpeed(speed)
, x260_moveDelay(0.f)
, x264_collisionRecoverDelay(0.f)
, x268_fadeInTime(actParams.GetFadeInTime())
, x26c_fadeOutTime(actParams.GetFadeOutTime())
, x270_dragDelta(CVector3f::Zero())
, x27c_rotDelta(CQuaternion::NoRotation())
, x28c_initialHealth(hInfo)
, x294_health(hInfo)
, x29c_damageVuln(dVuln)
, x304_treeGroupContainer(dcln)
, x314_treeGroup(nullptr)
, x348_xrayAlpha(xrayAlpha)
, x34c_maxRainSplashes(maxRainSplashes)
, x350_rainGenRate(rainGenRate)
, x354_boundsTrigger(kInvalidUniqueId)
, x356_24_dead(false)
, x356_25_controlledAnimation(false)
, x356_26_detectCollision(detectCollision)
, x356_27_squishedRider(false)
, x356_28_rainSplashes(rainSplashes)
, x356_29_setXrayDrawFlags(false)
, x356_30_disableXrayAlpha(false)
, x356_31_xrayFog(true) {
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_NoStaticCollision, kMT_NoPlatformCollision, kMT_Platform)));
  SetMovable(false);
  if (HasAnimation()) {
    AnimationData()->EnableLooping(true);
    AnimationData()->SetIsAnimating(true);
  }
  if (x304_treeGroupContainer) {
    x314_treeGroup = rs_new CCollidableOBBTreeGroup(**x304_treeGroupContainer, GetMaterialList());
  }
}

CScriptPlatform::~CScriptPlatform() {}

rstl::optional_object< CAABox > CScriptPlatform::GetTouchBounds() const {
  if (GetActive()) {
    if (!x314_treeGroup.null()) {
      return x314_treeGroup->CalculateAABox(GetTransform());
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
    if (conn->x4_msg == kSM_Follow) {
      return mgr.GetIdForScript(conn->x8_objId);
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
    x25c_currentSpeed = wp->GetSpeed();
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
      rider.x8_transform.SetTranslation(
          ridee->GetTransform().TransposeRotate(act->GetTranslation() - rideePos));
      mgr.DeliverScriptMsg(act, ridee->GetUniqueId(), kSM_AddPlatformRider);
    }
    riders.reserve(riders.size() + 1);
    riders.push_back(rider);
  } else {
    (*it).x4_decayTimer = 1.f / 6.f;
  }
}

TEntityList CScriptPlatform::BuildNearListFromRiders(CStateManager& mgr,
                                                     const rstl::vector< SRiders >& riders) {
  TEntityList result;
  rstl::vector< SRiders >::const_iterator it = riders.begin();
  for (; it != riders.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->x0_uid))) {
      result.push_back(actor->GetUniqueId());
    }
  }
  return result;
}

void CScriptPlatform::DecayRiders(rstl::vector< SRiders >& riders, float dt, CStateManager& mgr) {
  rstl::vector< SRiders >::iterator it = riders.begin();
  while (it != riders.end()) {
    (*it).x4_decayTimer -= dt;
    if ((*it).x4_decayTimer <= 0.f) {
      mgr.SendScriptMsgAlways((*it).x0_uid, kInvalidUniqueId, kSM_AddPlatformRider);
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
      CPhysicsActor* act = TCastToPtr< CPhysicsActor >(mgr.ObjectById(it->x0_uid));
      if (act == nullptr || !act->GetActive()) {
        ++it;
        continue;
      }
      const CTransform4f& xf = it->x8_transform;
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
  DecayRiders(x318_riders, dt, mgr);
  x264_collisionRecoverDelay -= dt;
  x260_moveDelay -= dt;
  if (!(x260_moveDelay > 0.f)) {
    x270_dragDelta = CVector3f::Zero();
    CTransform4f oldXf = GetTransform();
    CMotionState mState = GetMotionState();
    if (GetActive()) {
      rstl::vector< SRiders >::iterator it = x318_riders.begin();
      for (; it != x318_riders.end(); ++it) {
        if (const CPhysicsActor* act =
                TCastToConstPtr< CPhysicsActor >(mgr.ObjectById(it->x0_uid))) {
          CVector3f actPos = act->GetTranslation();
          CVector3f pos = GetTranslation();
          it->x8_transform.SetTranslation(GetTransform().TransposeRotate(actPos - pos));
        }
      }
      x27c_rotDelta = Move(dt, mgr);
    }

    CTransform4f newXf = GetTransform();
    x270_dragDelta = newXf.GetTranslation() - oldXf.GetTranslation();

    rstl::vector< SRiders > collidedRiders;
    MoveRiders(mgr, dt, GetActive(), x318_riders, collidedRiders, oldXf, newXf, x270_dragDelta,
               x27c_rotDelta);
    x356_27_squishedRider = false;
    if (!collidedRiders.empty()) {
      TEntityList nearList = BuildNearListFromRiders(mgr, collidedRiders);
      if (CGameCollision::DetectDynamicCollisionBoolean(*GetCollisionPrimitive(),
                                                        GetPrimitiveTransform(), nearList, mgr)) {
        SetMotionState(mState);
        Stop();
        x260_moveDelay = 0.035f;
        MoveRiders(
            mgr, dt, GetActive(), x318_riders, collidedRiders, newXf, oldXf, -x270_dragDelta,
            CQuaternion::ScalarVector(x27c_rotDelta.GetScalar(),
                                      static_cast< const CVector3f& >(-x27c_rotDelta.GetVector())));
        x270_dragDelta = CVector3f::Zero();
        SendScriptMsgs(kSS_Modify, mgr, kSM_None);
        x356_27_squishedRider = true;
      }
    }
  }
}

void CScriptPlatform::BuildSlaveList(CStateManager& mgr) {
  x328_slavesStatic.reserve(GetConnectionList().size());
  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->x0_state == kSS_Play && conn->x4_msg == kSM_Activate) {
      if (CActor* const act =
              TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(conn->x8_objId)))) {
        act->AddMaterial(kMT_PlatformSlave, mgr);
        CTransform4f xf = act->GetTransform();
        xf.SetTranslation(act->GetTranslation() - GetTranslation());
        x328_slavesStatic.push_back(SRiders(act->GetUniqueId(), 1.f / 6.f, xf));
      }
    } else if (conn->x0_state == kSS_InheritBounds && conn->x4_msg == kSM_Activate) {
      CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->x8_objId);
      CStateManager::TIdList::const_iterator current = search.first;
      CStateManager::TIdList::const_iterator end = search.second;
      while (current != end) {
        if (TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(current->second))) {
          x354_boundsTrigger = current->second;
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
  for (AUTO(it, x328_slavesStatic.begin()); it != x328_slavesStatic.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->x0_uid))) {
      DragSlave(mgr, moved, actor, delta);
    }
  }
  AUTO(it, x338_slavesDynamic.begin());
  while (it != x338_slavesDynamic.end()) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->x0_uid))) {
      DragSlave(mgr, moved, actor, delta);
      ++it;
    } else {
      it = x338_slavesDynamic.erase(it);
    }
  }
}

void CScriptPlatform::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (HasAnimation()) {
    if (!x356_25_controlledAnimation) {
      UpdateAnimation(dt, mgr, true);
    }
    if (x356_28_rainSplashes) {
      const CWorld* world = mgr.GetWorld();
      const CEnvFxManager* envFx = mgr.GetEnvFxManager();
      if (world->GetNeededEnvFx() == kEFX_Rain && HasModelData()) {
        if (envFx->GetRainMagnitude()) {
          mgr.ActorModelParticles()->AddRainSplashGenerator(*this, mgr, x34c_maxRainSplashes,
                                                            x350_rainGenRate, 0.f);
        }
      }
    }
  }
  if (!x328_slavesStatic.empty() || !x338_slavesDynamic.empty()) {
    TMovedList moved;
    DragSlaves(mgr, moved, x270_dragDelta);
  }
  if (!x356_24_dead && HealthInfo(mgr)->GetHP() <= 0.f) {
    x356_24_dead = true;
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

CHealthInfo* CScriptPlatform::HealthInfo(CStateManager&) { return &x294_health; }

const CDamageVulnerability* CScriptPlatform::GetDamageVulnerability() const {
  return &x29c_damageVuln;
}

void CScriptPlatform::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_InitializedInArea:
    BuildSlaveList(mgr);
    break;
  case kSM_AddPlatformRider:
    AddRider(x318_riders, uid, this, mgr);
    break;
  case kSM_Stop:
    x25c_currentSpeed = 0.f;
    Stop();
    break;
  case kSM_Next:
    x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
    if (x25a_targetWaypoint == kInvalidUniqueId) {
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Stop);
    } else if (CScriptWaypoint* wp =
                   TCastToPtr< CScriptWaypoint >(mgr.ObjectById(x25a_targetWaypoint))) {
      x25c_currentSpeed = 0.f;
      Stop();
      x270_dragDelta = wp->GetTranslation() - GetTranslation();
      SetTranslation(wp->GetTranslation());
      x258_currentWaypoint = x25a_targetWaypoint;
      x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
      mgr.DeliverScriptMsg(wp, GetUniqueId(), kSM_Arrived);
      if (!x328_slavesStatic.empty() || !x338_slavesDynamic.empty()) {
        TMovedList moved;
        DragSlaves(mgr, moved, x270_dragDelta);
      }
      x270_dragDelta = CVector3f::Zero();
    }
    break;
  case kSM_Start:
    x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
    if (x25a_targetWaypoint == kInvalidUniqueId) {
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Stop);
    } else if (const CScriptWaypoint* wp =
                   TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(x25a_targetWaypoint))) {
      x25c_currentSpeed = wp->GetSpeed();
    }
    break;
  case kSM_Reset:
    x356_24_dead = false;
    x294_health = x28c_initialHealth;
    break;
  case kSM_Increment:
    if (!GetActive()) {
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Activate);
    }
    CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), x268_fadeInTime);
    break;
  case kSM_Decrement:
    CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), x26c_fadeOutTime);
    break;
  case kSM_Deleted:
    DecayRiders(x318_riders, 1.66666675f, mgr);
    break;
  default:
    break;
  }
  CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptPlatform)

const CCollisionPrimitive* CScriptPlatform::GetCollisionPrimitive() const {
  if (x314_treeGroup.null()) {
    return CPhysicsActor::GetCollisionPrimitive();
  }
  return x314_treeGroup.get();
}

CTransform4f CScriptPlatform::GetPrimitiveTransform() const {
  CTransform4f xf = GetTransform();
  xf.AddTranslation(GetPrimitiveOffset());
  return xf;
}

void CScriptPlatform::SplashThink(const CAABox&, const CFluidPlane&, float, CStateManager&) const {}

void CScriptPlatform::AddSlave(TUniqueId id, CStateManager& mgr) {
  if (rstl::find(x338_slavesDynamic.begin(), x338_slavesDynamic.end(), SRiders(id)) ==
      x338_slavesDynamic.end()) {
    if (CActor* const act = TCastToPtr< CActor >(mgr.ObjectById(id))) {
      act->AddMaterial(kMT_PlatformSlave, mgr);
      CTransform4f xf = GetTransform().GetQuickInverse() * act->GetTransform();
      x338_slavesDynamic.reserve(x338_slavesDynamic.size() + 1);
      x338_slavesDynamic.push_back(SRiders(id, 1.f / 6.f, xf));
    }
  }
}

bool CScriptPlatform::IsRider(TUniqueId id) const {
  return rstl::find(x318_riders.begin(), x318_riders.end(), SRiders(id)) != x318_riders.end();
}

bool CScriptPlatform::IsSlave(TUniqueId id) const {
  return rstl::find(x328_slavesStatic.begin(), x328_slavesStatic.end(), SRiders(id)) !=
             x328_slavesStatic.end() ||
         rstl::find(x338_slavesDynamic.begin(), x338_slavesDynamic.end(), SRiders(id)) !=
             x338_slavesDynamic.end();
}

CQuaternion CScriptPlatform::Move(float dt, CStateManager& mgr) {
  TUniqueId nextWaypoint = x25a_targetWaypoint;
  if (nextWaypoint == kInvalidUniqueId) {
    nextWaypoint = GetNext(x258_currentWaypoint, mgr);
  }
  const CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(nextWaypoint));
  if (x258_currentWaypoint != kInvalidUniqueId && wp && !wp->GetActive()) {
    nextWaypoint = GetNext(x258_currentWaypoint, mgr);
    if (nextWaypoint == kInvalidUniqueId) {
      if (const CScriptWaypoint* current =
              TCastToPtr< CScriptWaypoint >(mgr.ObjectById(x258_currentWaypoint))) {
        if (current->GetActive()) {
          nextWaypoint = x258_currentWaypoint;
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
      x258_currentWaypoint = nextWaypoint;
      mgr.DeliverScriptMsg(waypoint, GetUniqueId(), kSM_Arrived);
      if (close_enough(x25c_currentSpeed, 0.f, 0.02f)) {
        nextWaypoint = GetNext(x258_currentWaypoint, mgr);
        x25c_currentSpeed = 0.f;
        Stop();
      } else {
        nextWaypoint = GetNext(x258_currentWaypoint, mgr);
      }
      if (kInvalidUniqueId != nextWaypoint) {
        continue;
      }
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Stop);
    }

    if (close_enough(delta, CVector3f::Zero(), 0.02f)) {
      x270_dragDelta = waypoint->GetTranslation() - GetTranslation();
      MoveToWR(GetTranslation(), dt);
    } else {
      const CVector3f moveDelta = dt * (x25c_currentSpeed * delta.AsNormalized());
      if (moveDelta.MagSquared() > delta.MagSquared()) {
        x270_dragDelta = waypoint->GetTranslation() - GetTranslation();
        MoveToWR(waypoint->GetTranslation(), dt);
      } else {
        x270_dragDelta = moveDelta;
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

    if (x356_26_detectCollision) {
      const CMotionState state = PredictMotion(dt);
      MoveCollisionPrimitive(state.GetTranslation());
      const bool collision = CGameCollision::DetectDynamicCollisionBoolean(
          *GetCollisionPrimitive(), GetPrimitiveTransform(), nonRiders, mgr);
      MoveCollisionPrimitive(CVector3f::Zero());
      if (collision || x356_27_squishedRider) {
        if (x356_26_detectCollision) {
          if (x264_collisionRecoverDelay <= 0.f && !x356_27_squishedRider) {
            x264_collisionRecoverDelay = 0.035f;
            break;
          } else {
            x356_27_squishedRider = false;
            const TUniqueId previousWaypoint = nextWaypoint;
            nextWaypoint = GetNext(nextWaypoint, mgr);
            if (nextWaypoint == x25a_targetWaypoint || previousWaypoint == x25a_targetWaypoint) {
              x260_moveDelay = 0.035f;
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
  x25a_targetWaypoint = nextWaypoint;
  return CQuaternion::NoRotation();
}

void CScriptPlatform::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);
  if (!GetPreRenderClipped() && !close_enough(x348_xrayAlpha, 1.f)) {
    const CModelFlags flags = CModelFlags::AlphaBlended(x348_xrayAlpha);
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay &&
        !x356_30_disableXrayAlpha) {
      SetModelFlags(flags);
      x356_29_setXrayDrawFlags = true;
    } else if (x356_29_setXrayDrawFlags) {
      x356_29_setXrayDrawFlags = false;
      if (GetModelFlags() == flags && !x356_30_disableXrayAlpha) {
        SetModelFlags(CModelFlags::Normal());
      }
    }
  }
  if (!mgr.GetObjectById(x354_boundsTrigger)) {
    x354_boundsTrigger = kInvalidUniqueId;
  }
}

void CScriptPlatform::SetDamageVulnerability(const CDamageVulnerability& vuln) {
  x29c_damageVuln = vuln;
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
  if (x354_boundsTrigger != kInvalidUniqueId) {
    if (const CScriptTrigger* trigger =
            static_cast< const CScriptTrigger* >(mgr.GetObjectById(x354_boundsTrigger))) {
      return trigger->GetTriggerBoundsWR();
    }
  }
  return CActor::GetSortingBounds(mgr);
}

void CScriptPlatform::Render(const CStateManager& mgr) const {
  const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
  if (xray && !x356_31_xrayFog) {
    gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
  }
  CPhysicsActor::Render(mgr);
  if (xray && !x356_31_xrayFog) {
    mgr.SetupFogForArea(GetCurrentAreaId());
  }
}
