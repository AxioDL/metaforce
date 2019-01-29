#include "CScriptPlatform.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"
#include "Collision/CGameCollision.hpp"
#include "World/CScriptWaypoint.hpp"
#include "World/CScriptColorModulate.hpp"
#include "World/CWorld.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptTrigger.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde {

static CMaterialList MakePlatformMaterialList() {
  CMaterialList ret;
  ret.Add(EMaterialTypes::Solid);
  ret.Add(EMaterialTypes::Immovable);
  ret.Add(EMaterialTypes::Platform);
  ret.Add(EMaterialTypes::Occluder);
  return ret;
}

CScriptPlatform::CScriptPlatform(
    TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
    const CActorParameters& actParms, const zeus::CAABox& aabb, float speed, bool detectCollision, float xrayAlpha,
    bool active, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
    const rstl::optional<TLockedToken<CCollidableOBBTreeGroupContainer>>& dcln, bool rainSplashes,
    u32 maxRainSplashes, u32 rainGenRate)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakePlatformMaterialList(), aabb, SMoverData(15000.f),
                actParms, 0.3f, 0.1f)
, x25c_currentSpeed(speed)
, x28c_initialHealth(hInfo)
, x294_health(hInfo)
, x29c_damageVuln(dVuln)
, x304_treeGroupContainer(dcln) {
  x348_xrayAlpha = xrayAlpha;
  x34c_maxRainSplashes = maxRainSplashes;
  x350_rainGenRate = rainGenRate;
  x356_24_dead = false;
  ;
  x356_25_notAnimating = false;
  x356_26_detectCollision = detectCollision;
  x356_27_squishedRider = false;
  x356_28_rainSplashes = rainSplashes;
  x356_29_setXrayDrawFlags = false;
  x356_30_disableXrayAlpha = false;
  x356_31_xrayFog = true;
  CActor::SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(EMaterialTypes::Solid),
      CMaterialList(EMaterialTypes::NoStaticCollision, EMaterialTypes::NoPlatformCollision, EMaterialTypes::Platform)));
  xf8_24_movable = false;
  if (HasModelData() && ModelData()->HasAnimData())
    ModelData()->AnimationData()->EnableLooping(true);
  if (x304_treeGroupContainer)
    x314_treeGroup = std::make_unique<CCollidableOBBTreeGroup>(x304_treeGroupContainer->GetObj(), x68_material);
}

void CScriptPlatform::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptPlatform::DragSlave(CStateManager& mgr, rstl::reserved_vector<u16, 1024>& draggedSet, CActor* actor,
                                const zeus::CVector3f& delta) {
  if (std::find(draggedSet.begin(), draggedSet.end(), actor->GetUniqueId().Value()) == draggedSet.end()) {
    draggedSet.push_back(actor->GetUniqueId().Value());
    zeus::CTransform newXf = actor->GetTransform();
    newXf.origin += delta;
    actor->SetTransform(newXf);
    if (TCastToPtr<CScriptPlatform> plat = actor)
      plat->DragSlaves(mgr, draggedSet, delta);
  }
}

void CScriptPlatform::DragSlaves(CStateManager& mgr, rstl::reserved_vector<u16, 1024>& draggedSet,
                                 const zeus::CVector3f& delta) {
  for (SRiders& rider : x328_slavesStatic)
    if (TCastToPtr<CActor> act = mgr.ObjectById(rider.x0_uid))
      DragSlave(mgr, draggedSet, act.GetPtr(), delta);
  for (auto it = x338_slavesDynamic.begin(); it != x338_slavesDynamic.end();) {
    if (TCastToPtr<CActor> act = mgr.ObjectById(it->x0_uid)) {
      DragSlave(mgr, draggedSet, act.GetPtr(), delta);
      ++it;
    } else
      it = x338_slavesDynamic.erase(it);
  }
}

void CScriptPlatform::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::InitializedInArea:
    BuildSlaveList(mgr);
    break;
  case EScriptObjectMessage::AddPlatformRider:
    AddRider(x318_riders, uid, this, mgr);
    break;
  case EScriptObjectMessage::Stop: {
    x25c_currentSpeed = 0.f;
    Stop();
    break;
  }
  case EScriptObjectMessage::Next: {
    x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
    if (x25a_targetWaypoint == kInvalidUniqueId)
      mgr.SendScriptMsg(this, GetUniqueId(), EScriptObjectMessage::Stop);
    else if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x25a_targetWaypoint)) {
      x25c_currentSpeed = 0.f;
      Stop();
      x270_dragDelta = wp->GetTranslation() - GetTranslation();
      SetTranslation(wp->GetTranslation());

      x258_currentWaypoint = x25a_targetWaypoint;
      x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
      mgr.SendScriptMsg(wp, GetUniqueId(), EScriptObjectMessage::Arrived);
      if (!x328_slavesStatic.empty() || !x338_slavesDynamic.empty()) {
        rstl::reserved_vector<u16, 1024> draggedSet;
        DragSlaves(mgr, draggedSet, x270_dragDelta);
      }
      x270_dragDelta = zeus::CVector3f::skZero;
    }
    break;
  }
  case EScriptObjectMessage::Start: {
    x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
    if (x25a_targetWaypoint == kInvalidUniqueId)
      mgr.SendScriptMsg(this, GetUniqueId(), EScriptObjectMessage::Stop);
    else if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x25a_targetWaypoint))
      x25c_currentSpeed = wp->GetSpeed();
    break;
  }
  case EScriptObjectMessage::Reset: {
    x356_24_dead = false;
    x294_health = x28c_initialHealth;
    break;
  }
  case EScriptObjectMessage::Increment: {
    if (GetActive())
      CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), x268_fadeInTime);
    else
      mgr.SendScriptMsg(this, GetUniqueId(), EScriptObjectMessage::Activate);
    break;
  }
  case EScriptObjectMessage::Decrement:
    CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), x26c_fadeOutTime);
    break;
  case EScriptObjectMessage::Deleted:
    DecayRiders(x318_riders, 0.f, mgr);
    break;
  default:
    break;
  }

  CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptPlatform::DecayRiders(std::vector<SRiders>& riders, float dt, CStateManager& mgr) {
  for (auto it = riders.begin(); it != riders.end();) {
    it->x4_decayTimer -= dt;
    if (it->x4_decayTimer <= 0.f) {
      mgr.SendScriptMsgAlways(it->x0_uid, kInvalidUniqueId, EScriptObjectMessage::AddPlatformRider);
      it = riders.erase(it);
      continue;
    }
    ++it;
  }
}

void CScriptPlatform::MoveRiders(CStateManager& mgr, float dt, bool active, std::vector<SRiders>& riders,
                                 std::vector<SRiders>& collidedRiders, const zeus::CTransform& oldXf,
                                 const zeus::CTransform& newXf, const zeus::CVector3f& dragDelta,
                                 const zeus::CQuaternion& rotDelta) {
  for (auto it = riders.begin(); it != riders.end();) {
    if (active) {
      if (TCastToPtr<CPhysicsActor> act = mgr.ObjectById(it->x0_uid)) {
        if (act->GetActive()) {
          zeus::CVector3f delta =
              newXf.rotate(it->x8_transform.origin) - oldXf.rotate(it->x8_transform.origin) + dragDelta;
          zeus::CVector3f newPos = act->GetTranslation() + delta;
          act->MoveCollisionPrimitive(delta);
          bool collision = CGameCollision::DetectStaticCollisionBoolean(
              mgr, *act->GetCollisionPrimitive(), act->GetPrimitiveTransform(), act->GetMaterialFilter());
          act->MoveCollisionPrimitive(zeus::CVector3f::skZero);
          if (collision) {
            AddRider(collidedRiders, act->GetUniqueId(), act.GetPtr(), mgr);
            it = riders.erase(it);
            continue;
          }
          act->SetTranslation(newPos);
          if (TCastToPtr<CPlayer> player = act.GetPtr()) {
            if (player->GetOrbitState() != CPlayer::EPlayerOrbitState::NoOrbit) {
              ++it;
              continue;
            }
          }
          zeus::CTransform xf = (rotDelta * zeus::CQuaternion(act->GetTransform().basis)).toTransform();
          xf.origin = act->GetTranslation();
          act->SetTransform(xf);
        }
      }
    }
    ++it;
  }
}

rstl::reserved_vector<TUniqueId, 1024>
CScriptPlatform::BuildNearListFromRiders(CStateManager& mgr, const std::vector<SRiders>& movedRiders) {
  rstl::reserved_vector<TUniqueId, 1024> ret;
  for (const SRiders& rider : movedRiders)
    if (TCastToPtr<CActor> act = mgr.ObjectById(rider.x0_uid))
      ret.push_back(act->GetUniqueId());
  return ret;
}

void CScriptPlatform::PreThink(float dt, CStateManager& mgr) {
  DecayRiders(x318_riders, dt, mgr);
  x264_collisionRecoverDelay -= dt;
  x260_moveDelay -= dt;
  if (x260_moveDelay <= 0.f) {
    x270_dragDelta = zeus::CVector3f::skZero;
    zeus::CTransform oldXf = x34_transform;
    CMotionState mState = GetMotionState();
    if (GetActive()) {
      for (SRiders& rider : x318_riders)
        if (TCastToPtr<CPhysicsActor> act = mgr.ObjectById(rider.x0_uid))
          rider.x8_transform.origin = x34_transform.transposeRotate(act->GetTranslation() - GetTranslation());
      x27c_rotDelta = Move(dt, mgr);
    }

    x270_dragDelta = x34_transform.origin - oldXf.origin;

    std::vector<SRiders> collidedRiders;
    MoveRiders(mgr, dt, GetActive(), x318_riders, collidedRiders, oldXf, x34_transform, x270_dragDelta, x27c_rotDelta);
    x356_27_squishedRider = false;
    if (!collidedRiders.empty()) {
      rstl::reserved_vector<TUniqueId, 1024> nearList = BuildNearListFromRiders(mgr, collidedRiders);
      if (CGameCollision::DetectDynamicCollisionBoolean(*GetCollisionPrimitive(), GetPrimitiveTransform(), nearList,
                                                        mgr)) {
        SetMotionState(mState);
        Stop();
        x260_moveDelay = 0.035f;
        MoveRiders(mgr, dt, GetActive(), x318_riders, collidedRiders, x34_transform, oldXf, -x270_dragDelta,
                   x27c_rotDelta.inverse());
        x270_dragDelta = zeus::CVector3f::skZero;
        SendScriptMsgs(EScriptObjectState::Modify, mgr, EScriptObjectMessage::None);
        x356_27_squishedRider = true;
      }
    }
  }
}

void CScriptPlatform::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (HasModelData() && GetModelData()->HasAnimData()) {
    if (!x356_25_notAnimating)
      UpdateAnimation(dt, mgr, true);
    if (x356_28_rainSplashes && mgr.GetWorld()->GetNeededEnvFx() == EEnvFxType::Rain) {
      if (HasModelData() && !GetModelData()->IsNull() && mgr.GetEnvFxManager()->GetRainMagnitude() != 0.f)
        mgr.GetActorModelParticles()->AddRainSplashGenerator(*this, mgr, x34c_maxRainSplashes, x350_rainGenRate, 0.f);
    }
  }

  if (!x328_slavesStatic.empty() || !x338_slavesDynamic.empty()) {
    rstl::reserved_vector<u16, 1024> draggedSet;
    DragSlaves(mgr, draggedSet, x270_dragDelta);
  }

  if (x356_24_dead)
    return;

  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    x356_24_dead = true;
    SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
  }
}

void CScriptPlatform::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CActor::PreRender(mgr, frustum);
  if (!xe4_30_outOfFrustum && !zeus::close_enough(x348_xrayAlpha, 1.f)) {
    CModelFlags flags(5, 0, 3, {1.f, x348_xrayAlpha});
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay && !x356_30_disableXrayAlpha) {
      xb4_drawFlags = flags;
      x356_29_setXrayDrawFlags = true;
    } else if (x356_29_setXrayDrawFlags) {
      x356_29_setXrayDrawFlags = false;
      if (xb4_drawFlags == flags && !x356_30_disableXrayAlpha)
        xb4_drawFlags = CModelFlags(0, 0, 3, zeus::CColor::skWhite);
    }
  }
  if (!mgr.GetObjectById(x354_boundsTrigger))
    x354_boundsTrigger = kInvalidUniqueId;
}

void CScriptPlatform::Render(const CStateManager& mgr) const {
  bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay;
  if (xray && !x356_31_xrayFog)
    g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::CColor::skBlack);
  CPhysicsActor::Render(mgr);
  if (xray && !x356_31_xrayFog)
    mgr.SetupFogForArea(x4_areaId);
}

rstl::optional<zeus::CAABox> CScriptPlatform::GetTouchBounds() const {
  if (x314_treeGroup)
    return {x314_treeGroup->CalculateAABox(GetTransform())};

  return {CPhysicsActor::GetBoundingBox()};
}

zeus::CTransform CScriptPlatform::GetPrimitiveTransform() const {
  zeus::CTransform ret = GetTransform();
  ret.origin += GetPrimitiveOffset();
  return ret;
}

const CCollisionPrimitive* CScriptPlatform::GetCollisionPrimitive() const {
  if (!x314_treeGroup)
    return CPhysicsActor::GetCollisionPrimitive();
  return x314_treeGroup.get();
}

zeus::CVector3f CScriptPlatform::GetOrbitPosition(const CStateManager& mgr) const { return GetAimPosition(mgr, 0.f); }

zeus::CVector3f CScriptPlatform::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (auto tb = GetTouchBounds())
    return {tb->center()};
  return CPhysicsActor::GetAimPosition(mgr, dt);
}

zeus::CAABox CScriptPlatform::GetSortingBounds(const CStateManager& mgr) const {
  if (x354_boundsTrigger != kInvalidUniqueId)
    if (TCastToConstPtr<CScriptTrigger> trig = mgr.GetObjectById(x354_boundsTrigger))
      return trig->GetTriggerBoundsWR();
  return CActor::GetSortingBounds(mgr);
}

bool CScriptPlatform::IsRider(TUniqueId id) const {
  for (const SRiders& rider : x318_riders)
    if (rider.x0_uid == id)
      return true;
  return false;
}

bool CScriptPlatform::IsSlave(TUniqueId id) const {
  auto search = std::find_if(x328_slavesStatic.begin(), x328_slavesStatic.end(),
                             [id](const SRiders& rider) { return rider.x0_uid == id; });
  if (search != x328_slavesStatic.end())
    return true;
  search = std::find_if(x338_slavesDynamic.begin(), x338_slavesDynamic.end(),
                        [id](const SRiders& rider) { return rider.x0_uid == id; });
  return search != x338_slavesDynamic.end();
}

void CScriptPlatform::BuildSlaveList(CStateManager& mgr) {
  x328_slavesStatic.reserve(GetConnectionList().size());
  for (const SConnection& conn : GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::Play && conn.x4_msg == EScriptObjectMessage::Activate) {
      if (TCastToPtr<CActor> act = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
        act->AddMaterial(EMaterialTypes::PlatformSlave, mgr);
        zeus::CTransform xf = act->GetTransform();
        xf.origin = act->GetTranslation() - GetTranslation();
        x328_slavesStatic.emplace_back(act->GetUniqueId(), 0.166667f, xf);
      }
    } else if (conn.x0_state == EScriptObjectState::InheritBounds && conn.x4_msg == EScriptObjectMessage::Activate) {
      auto list = mgr.GetIdListForScript(conn.x8_objId);
      for (auto it = list.first; it != list.second; ++it)
        if (TCastToConstPtr<CScriptTrigger>(mgr.GetObjectById(it->second)))
          x354_boundsTrigger = it->second;
    }
  }
}

void CScriptPlatform::AddRider(std::vector<SRiders>& riders, TUniqueId riderId, const CPhysicsActor* ridee,
                               CStateManager& mgr) {
  auto search = std::find_if(riders.begin(), riders.end(), [riderId](const SRiders& r) { return r.x0_uid == riderId; });
  if (search == riders.end()) {
    zeus::CTransform xf;
    if (TCastToPtr<CPhysicsActor> act = mgr.ObjectById(riderId)) {
      xf.origin = ridee->GetTransform().transposeRotate(act->GetTranslation() - ridee->GetTranslation());
      mgr.SendScriptMsg(act.GetPtr(), ridee->GetUniqueId(), EScriptObjectMessage::AddPlatformRider);
    }
    riders.emplace_back(riderId, 0.166667f, xf);
  } else {
    search->x4_decayTimer = 0.166667f;
  }
}

void CScriptPlatform::AddSlave(TUniqueId id, CStateManager& mgr) {
  auto search = std::find_if(x338_slavesDynamic.begin(), x338_slavesDynamic.end(),
                             [id](const SRiders& r) { return r.x0_uid == id; });
  if (search == x338_slavesDynamic.end()) {
    if (TCastToPtr<CActor> act = mgr.ObjectById(id)) {
      act->AddMaterial(EMaterialTypes::PlatformSlave, mgr);
      zeus::CTransform localXf = x34_transform.inverse() * act->GetTransform();
      x338_slavesDynamic.emplace_back(id, 0.166667f, localXf);
    }
  }
}

TUniqueId CScriptPlatform::GetNext(TUniqueId uid, CStateManager& mgr) {
  TCastToConstPtr<CScriptWaypoint> nextWp = mgr.GetObjectById(uid);
  if (!nextWp)
    return GetWaypoint(mgr);

  TUniqueId next = nextWp->NextWaypoint(mgr);
  if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(next))
    x25c_currentSpeed = wp->GetSpeed();

  return next;
}

TUniqueId CScriptPlatform::GetWaypoint(CStateManager& mgr) {
  for (const SConnection& conn : x20_conns) {
    if (conn.x4_msg == EScriptObjectMessage::Follow)
      return mgr.GetIdForScript(conn.x8_objId);
  }

  return kInvalidUniqueId;
}

void CScriptPlatform::SplashThink(const zeus::CAABox&, const CFluidPlane&, float, CStateManager&) const {
  // Empty
}

zeus::CQuaternion CScriptPlatform::Move(float dt, CStateManager& mgr) {
  TUniqueId nextWaypoint = x25a_targetWaypoint;
  if (x25a_targetWaypoint == kInvalidUniqueId)
    nextWaypoint = GetNext(x258_currentWaypoint, mgr);

  TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(nextWaypoint);
  if (x258_currentWaypoint != kInvalidUniqueId && wp && !wp->GetActive()) {
    nextWaypoint = GetNext(x258_currentWaypoint, mgr);
    if (nextWaypoint == kInvalidUniqueId)
      if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x258_currentWaypoint))
        if (wp->GetActive())
          nextWaypoint = x258_currentWaypoint;
  }

  if (nextWaypoint == kInvalidUniqueId)
    return zeus::CQuaternion::skNoRotation;

  while (nextWaypoint != kInvalidUniqueId) {
    if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(nextWaypoint)) {
      zeus::CVector3f platToWp = wp->GetTranslation() - GetTranslation();
      if (zeus::close_enough(platToWp, zeus::CVector3f::skZero)) {
        x258_currentWaypoint = nextWaypoint;
        mgr.SendScriptMsg(wp.GetPtr(), GetUniqueId(), EScriptObjectMessage::Arrived);
        if (zeus::close_enough(x25c_currentSpeed, 0.f, 0.02)) {
          nextWaypoint = GetNext(x258_currentWaypoint, mgr);
          x25c_currentSpeed = 0.f;
          Stop();
        } else {
          nextWaypoint = GetNext(x258_currentWaypoint, mgr);
        }

        if (nextWaypoint != kInvalidUniqueId)
          continue;

        mgr.SendScriptMsg(this, GetUniqueId(), EScriptObjectMessage::Stop);
      }

      if (zeus::close_enough(platToWp, zeus::CVector3f::skZero)) {
        x270_dragDelta = wp->GetTranslation() - GetTranslation();
        MoveToWR(GetTranslation(), dt);

      } else if ((platToWp.normalized() * x25c_currentSpeed * dt).magSquared() > platToWp.magSquared()) {
        x270_dragDelta = wp->GetTranslation() - GetTranslation();
        MoveToWR(wp->GetTranslation(), dt);
      } else {
        x270_dragDelta = platToWp.normalized() * x25c_currentSpeed * dt;
        MoveToWR(GetTranslation() + x270_dragDelta, dt);
      }

      rstl::reserved_vector<TUniqueId, 1024> nearList;
      mgr.BuildColliderList(nearList, *this, GetMotionVolume(dt));
      rstl::reserved_vector<TUniqueId, 1024> nonRiders;
      for (TUniqueId id : nearList)
        if (!IsRider(id) && !IsSlave(id))
          nonRiders.push_back(id);

      if (x356_26_detectCollision) {
        CMotionState mState = PredictMotion(dt);
        MoveCollisionPrimitive(mState.x0_translation);
        bool collision = CGameCollision::DetectDynamicCollisionBoolean(*GetCollisionPrimitive(),
                                                                       GetPrimitiveTransform(), nonRiders, mgr);
        MoveCollisionPrimitive(zeus::CVector3f::skZero);
        if (collision || x356_27_squishedRider) {
          if (x356_26_detectCollision) {
            if (x264_collisionRecoverDelay <= 0.f && !x356_27_squishedRider) {
              x264_collisionRecoverDelay = 0.035f;
              break;
            } else {
              x356_27_squishedRider = false;
              TUniqueId prevWaypoint = nextWaypoint;
              nextWaypoint = GetNext(nextWaypoint, mgr);
              if (x25a_targetWaypoint == nextWaypoint || x25a_targetWaypoint == prevWaypoint) {
                x260_moveDelay = 0.035f;
                break;
              }
            }
          } else {
            break;
          }
        } else {
          AddMotionState(mState);
          break;
        }
      } else {
        xf8_24_movable = true;
        CGameCollision::Move(mgr, *this, dt, &nonRiders);
        xf8_24_movable = false;
        break;
      }
    } else {
      nextWaypoint = kInvalidUniqueId;
      break;
    }
  }

  x25a_targetWaypoint = nextWaypoint;
  return zeus::CQuaternion::skNoRotation;
}

} // namespace urde
