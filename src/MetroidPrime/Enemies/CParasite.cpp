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
, x5d8_doorRepulsors()
, x5e8_stateProgress(-1)
, x5ec_(0.f, 0.f, 0.f)
, x5f8_targetPos(0.f, 0.f, 0.f)
, x604_activeSpeed(1.f)
, x608_telegraphRemTime(0.f)
, x60c_stuckTime(0.f)
, x614_lastStuckPos(CVector3f::Zero())
, x620_collisionActorManager(nullptr)
, x624_extraModel(nullptr)
, x628_parasiteSeparationMove(0.f, 0.f, 0.f)
, x634_parasiteCohesionMove(0.f, 0.f, 0.f)
, x640_parasiteAlignmentMove(0.f, 0.f, 0.f)
, x64c_oculusHaltDVuln(dVuln)
, x6b4_oculusHaltDInfo(dInfo)
, x6d0_maxTelegraphReactDist(maxTelegraphReactDist)
, x6d4_(f3)
, x6dc_(f5)
, x6e0_stuckTimeThreshold(stuckTimeThreshold)
, x6e4_parasiteSearchRadius(parasiteSearchRadius)
, x6e8_parasiteSeparationDist(parasiteSeparationDist)
, x6ec_parasiteSeparationWeight(parasiteSeparationWeight)
, x6f0_parasiteAlignmentWeight(parasiteAlignmentWeight)
, x6f4_parasiteCohesionWeight(parasiteCohesionWeight)
, x6f8_destinationSeekWeight(destinationSeekWeight)
, x6fc_forwardMoveWeight(forwardMoveWeight)
, x700_playerSeparationDist(playerSeparationDist)
, x704_playerSeparationWeight(playerSeparationWeight)
, x708_unmorphedRadius(pInfo.GetHeight() / 2.f)
, x710_haltDelay(haltDelay)
, x714_iceZoomerJointHP(iceZoomerJointHP)
, x718_(0.f)
, x71c_(0.f)
, x720_(0.f)
, x724_(0.f)
, x728_(0.f)
, x72c_(0.f)
, x730_(0.f)
, x734_(0.f)
, x738_(0.f)
, x73c_haltSfx(CSfxManager::TranslateSFXID(haltSfx))
, x73e_getUpSfx(CSfxManager::TranslateSFXID(getUpSfx))
, x740_crouchSfx(CSfxManager::TranslateSFXID(crouchSfx))
, x742_24_receivedTelegraph(false)
, x742_25_jumpVelDirty(false)
, x742_26_(false)
, x742_27_landed(false)
, x742_28_onGround(true)
, x742_29_(false)
, x742_30_attackOver(true)
, x742_31_(false)
, x743_24_halted(false)
, x743_26_oculusShotAt(false)
, x743_27_inJump(false) {
  SetCallTouch(false);
  switch (x5d0_walkerType) {
  case kWT_Geemer:
    x460_knockBackController.SetEnableFreeze(false);
  case kWT_Oculus:
    x460_knockBackController.SetAutoResetImpulse(false);
    break;
  case kWT_IceZoomer:
    x624_extraModel = rs_new TLockedToken< CSkinnedModel >(
        rs_new CSkinnedModel(gpSimplePool->GetObj(SObjectTag('CMDL', modelRes)),
                             gpSimplePool->GetObj(SObjectTag('CSKR', skinRes)),
                             GetModelData()->GetAnimationData()->GetModelData()->GetLayoutInfo(),
                             CSkinnedModel::kDO_Owned));
    break;
  default:
    break;
  }
  if (x5d0_walkerType == kWT_Oculus) {
    x460_knockBackController.SetEnableShock(false);
    x460_knockBackController.SetEnableBurn(false);
    x460_knockBackController.SetEnableBurnDeath(false);
    x460_knockBackController.SetEnableExplodeDeath(false);
    x460_knockBackController.SetX82_24(false);
  }
}

ENTITY_ACCEPT_IMPL(CParasite)

void CParasite::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered: {
    x450_bodyController->Activate(mgr);
    mgr.ActiveParasites().push_back(GetUniqueId());
    CActor::SetDrawShadow(false);
    x604_activeSpeed = x3b4_speed;
    const CVector3f extent(x590_colSphere.GetSphere().GetRadius(),
                           x590_colSphere.GetSphere().GetRadius(),
                           x590_colSphere.GetSphere().GetRadius());
    CPhysicsActor::SetBoundingBox(CAABox(-extent, extent));
    lastParasite = GetUniqueId();
    AddDoorRepulsors(mgr);
    if (x5d0_walkerType == kWT_IceZoomer) {
      SetupIceZoomerCollision(mgr);
      SetupIceZoomerVulnerability(
          mgr, x64c_oculusHaltDVuln,
          CHealthInfo(x714_iceZoomerJointHP, HealthInfo(mgr)->GetKnockBackResistance()));
    }
    break;
  }
  case kSM_Deleted:
    mgr.ActiveParasites().remove(GetUniqueId());
    if (x5d0_walkerType == kWT_IceZoomer)
      DestroyActorManager(mgr);
    break;
  case kSM_Jumped:
    if (x742_25_jumpVelDirty) {
      UpdateJumpVelocity();
      x742_25_jumpVelDirty = false;
    }
    break;
  case kSM_Activate:
    x5d6_27_disableMove = false;
    if (x5d0_walkerType == kWT_Parasite)
      x450_bodyController->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kSM_InvulnDamage:
    if (x5d0_walkerType == kWT_Oculus) {
      if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
        float distSq = (act->GetTranslation() - GetTranslation()).MagSquared();
        float maxComp = rstl::max_val(
            GetTouchBounds()->GetWidth(),
            rstl::max_val(GetTouchBounds()->GetDepth(), GetTouchBounds()->GetHeight()));
        float maxCompSq = maxComp * maxComp + 1.f;
        if (distSq < maxCompSq * maxCompSq)
          x743_26_oculusShotAt = true;
      }
    }
    break;
  case kSM_SuspendedMove:
    if (x620_collisionActorManager.get())
      x620_collisionActorManager->SetMovable(mgr, false);
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
  x5d8_doorRepulsors.reserve(count);
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    CEntity* entity = objects[i];
    if (CScriptDoor* door = TCastToPtr< CScriptDoor >(entity)) {
      if (door->GetCurrentAreaId() == GetCurrentAreaId()) {
        rstl::optional_object< CAABox > bounds = door->GetTouchBounds();
        if (bounds.valid()) {
          float diagonal = (bounds->GetMinPoint() - bounds->GetMaxPoint()).Magnitude();
          x5d8_doorRepulsors.push_back(CRepulsor(bounds->GetCenterPoint(), 0.75f * diagonal));
        }
      }
    }
  }
}

void CParasite::PreThink(float dt, CStateManager& mgr) {
  CWallWalker::PreThink(dt, mgr);
  x743_26_oculusShotAt = false;
}

void CParasite::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  ++x5d4_thinkCounter;
  if (x5d0_walkerType == kWT_IceZoomer)
    UpdateCollisionActors(dt, mgr);

  x5d6_26_playerObstructed = false;
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible)
    x5d6_26_playerObstructed = true;

  if (!x5d6_26_playerObstructed) {
    CVector3f plVec = mgr.GetPlayer()->GetTranslation();
    float distance = (plVec - GetTranslation()).Magnitude();

    if (distance > x5c4_playerObstructionMinDist) {
      CRayCastResult res =
          mgr.RayStaticIntersection(plVec, (GetTranslation() - plVec).AsNormalized(), distance,
                                    CMaterialFilter::skPassEverything);
      if (res.IsValid())
        x5d6_26_playerObstructed = true;
    }
  }

  if (x5d6_26_playerObstructed) {
    xf8_24_movable = false;
    return;
  }

  xf8_24_movable = !x5d6_24_alignToFloor;

  if (!x5d6_27_disableMove) {
    if (close_enough(x450_bodyController->GetPercentageFrozen(), 0.f)) {
      static const float ksMinMoveDistance = 0.3f * dt * x3b4_speed;
      const CVector3f stuckDelta = GetTranslation() - x614_lastStuckPos;
      if (stuckDelta.MagSquared() < ksMinMoveDistance * dt)
        x60c_stuckTime += dt;
      else
        x60c_stuckTime = 0.f;

      x614_lastStuckPos = GetTranslation();
      if (x608_telegraphRemTime > 0.f)
        x608_telegraphRemTime -= dt;
      else
        x608_telegraphRemTime = 0.f;
    }
  }

  if (x400_25_alive) {
    CPlayer* pl = mgr.Player();
    bool useCollisionRadius =
        pl->GetMorphballTransitionState() == CPlayer::kMS_Morphed || !x742_30_attackOver;
    float radius =
        useCollisionRadius ? x590_colSphere.GetSphere().GetRadius() : x708_unmorphedRadius;

    const CVector3f extent(radius, radius, radius);
    CAABox aabox(GetTranslation() - extent, GetTranslation() + extent);
    rstl::optional_object< CAABox > plBox = pl->GetTouchBounds();

    if (plBox.valid() && plBox->DoBoundsOverlap(aabox)) {
      if (!x742_30_attackOver) {
        x742_30_attackOver = true;
        x742_27_landed = false;
      }

      if (x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
  }

  CWallWalker::Think(dt, mgr);

  if (x5d6_27_disableMove)
    return;

  if (!close_enough(x450_bodyController->GetPercentageFrozen(), 0.f))
    return;

  x3b4_speed = x604_activeSpeed;
  if (x5d6_24_alignToFloor)
    AlignToFloor(mgr, x590_colSphere.GetSphere().GetRadius(),
                 GetTranslation() + 2.f * (dt * GetVelocityWR()), dt);

  x742_27_landed = false;
}

CAdvancementDeltas CParasite::UpdateWalkerAnimation(CStateManager& mgr, float dt) {
  return CActor::UpdateAnimation(dt, mgr, true);
}

void CParasite::UpdateJumpVelocity() {
  SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
  CVector3f velocity = CVector3f::Zero();
  if (!x742_30_attackOver) {
    float speed = skAttackVelocity;
    velocity = speed * GetTransform().GetForward();
    velocity[kDZ] = 0.5f * skAttackVelocity;
  } else {
    float speed = skRetreatVelocity;
    velocity = speed * GetTransform().GetForward();
    velocity[kDZ] = 0.5f * skRetreatVelocity;
  }
  const CVector3f& position = GetTranslation();
  float height = x5f8_targetPos[kDZ] - position[kDZ];
  float acceleration = GetMomentumWR()[kDZ] / GetMass();
  CVector3f delta(x5f8_targetPos[kDX] - position[kDX], x5f8_targetPos[kDY] - position[kDY], 0.f);
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

bool CParasite::Stuck(CStateManager&, float) { return x60c_stuckTime > x6e0_stuckTimeThreshold; }

bool CParasite::AttackOver(CStateManager&, float) { return x742_30_attackOver; }

bool CParasite::ShotAt(CStateManager&, float) {
  if (x5d0_walkerType != kWT_Oculus)
    return x400_24_hitByPlayerProjectile;
  return x743_26_oculusShotAt;
}

bool CParasite::CloseToWall(CStateManager& mgr) {
  static const CMaterialFilter kSolidFilter =
      CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CAABox& bounds = CPhysicsActor::GetBoundingBox();
  float radius = x590_colSphere.GetSphere().GetRadius();
  float margin = radius + x5b0_collisionCloseMargin;
  CAABox expanded(bounds.GetMinPoint() - CVector3f(margin, margin, margin),
                  bounds.GetMaxPoint() + CVector3f(margin, margin, margin));
  CCollidableAABox collisionBox(expanded, GetMaterialList());
  return CGameCollision::DetectStaticCollisionBoolean(mgr, collisionBox, CTransform4f::Identity(),
                                                      kSolidFilter);
}

void CParasite::CollidedWith(const TUniqueId& uid, const CCollisionInfoList& list, CStateManager&) {
  static const CMaterialList testList = CMaterialList(kMT_Player, kMT_Character);
  if (x743_27_inJump) {
    for (AUTO(it, list.Begin()); it < list.End(); ++it) {
      const CCollisionInfo& info = *it;
      if (!x5d6_24_alignToFloor && !testList.SharesMaterials(info.GetMaterialLeft())) {
        const CVector3f normal = info.GetNormalLeft();
        OrientToSurfaceNormal(normal, 360.f);
        CPhysicsActor::Stop();
        SetVelocityWR(CVector3f::Zero());
        x742_27_landed = true;
        x742_28_onGround = true;
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
    if (conn.x0_state == kSS_Arrived && conn.x4_msg == kSM_Next) {
      TUniqueId nextId = mgr.GetIdForScript(conn.x8_objId);
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
    if (state == conn.x0_state && conn.x4_msg == kSM_Follow) {
      TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
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

bool CParasite::AnimOver(CStateManager&, float) { return x5e8_stateProgress == 2; }

bool CParasite::Landed(CStateManager&, float) { return x742_27_landed; }

bool CParasite::HitSomething(CStateManager& mgr, float) {
  if (x5d4_thinkCounter & 0x1)
    return true;
  return x5b8_tumbleAngle < 270.f && CloseToWall(mgr);
}

bool CParasite::ShouldAttack(CStateManager& mgr, float arg) {
  bool shouldAttack = false;
  if (x742_24_receivedTelegraph && x608_telegraphRemTime > 0.1f)
    shouldAttack = true;
  return !TooClose(mgr, arg) && InMaxRange(mgr, arg) &&
         (shouldAttack || InDetectionRange(mgr, 0.f));
}

void CParasite::Generate(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x5e8_stateProgress = 0;
    break;
  case kStateMsg_Update:
    switch (x5e8_stateProgress) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Generate)
        x5e8_stateProgress = 1;
      else
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Zero, CVector3f::Zero(), false, false));
      break;
    case 1:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Generate)
        x5e8_stateProgress = 2;
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
    x5e8_stateProgress = 0;
    SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
    mgr.DeleteObjectRequest(GetUniqueId());
    break;
  case kStateMsg_Update:
    switch (x5e8_stateProgress) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Generate)
        x5e8_stateProgress = 1;
      else
        x450_bodyController->CommandMgr().DeliverCmd(
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
    x5d6_24_alignToFloor = true;
    if (x5d0_walkerType == kWT_Parasite)
      x450_bodyController->SetLocomotionType(pas::kLT_Lurk);
    SetMomentumWR(CVector3f::Zero());
    xf8_24_movable = false;
    break;
  case kStateMsg_Update:
    UpdatePFDestination(mgr);
    DoFlockingBehavior(mgr);
    break;
  case kStateMsg_Deactivate:
    xf8_24_movable = true;
    x5d6_24_alignToFloor = false;
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
    x742_28_onGround = false;
    x5d6_24_alignToFloor = false;
    x742_27_landed = false;
    x743_27_inJump = true;
    break;
  case kStateMsg_Update:
    SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
    break;
  case kStateMsg_Deactivate:
    RemoveMaterial(kMT_GroundCollider, mgr);
    SetMomentumWR(CVector3f::Zero());
    x742_28_onGround = true;
    x742_27_landed = false;
    x743_27_inJump = false;
    break;
  }
}

void CParasite::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetMomentumWR(CVector3f::Zero());
    TUniqueId wpId = GetClosestWaypointForState(kSS_Patrol, mgr);
    if (wpId != kInvalidUniqueId)
      x2dc_destObj = wpId;
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
    x5d6_24_alignToFloor = true;
    if (!x5d6_27_disableMove && x5d0_walkerType == kWT_Parasite)
      x450_bodyController->SetLocomotionType(pas::kLT_Lurk);
    SetMomentumWR(CVector3f::Zero());
    x5d6_25_hasAlignSurface = false;
    xf8_24_movable = false;
    break;
  case kStateMsg_Update: {
    const float& pause = x5bc_patrolPauseRemTime;
    if (pause > 0.f) {
      x5bc_patrolPauseRemTime -= dt;
      if (x5bc_patrolPauseRemTime <= 0.f) {
        if (x5d0_walkerType == kWT_Parasite)
          x450_bodyController->SetLocomotionType(pas::kLT_Lurk);
        x5bc_patrolPauseRemTime = 0.f;
      }
    }
    GotoNextWaypoint(mgr);
    if (x5bc_patrolPauseRemTime <= 0.f && !x5d6_27_disableMove)
      DoFlockingBehavior(mgr);
    break;
  }
  case kStateMsg_Deactivate:
    x5d6_24_alignToFloor = false;
    xf8_24_movable = true;
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
    x608_telegraphRemTime = 0.f;
    CRandom16& random = *mgr.Random();
    if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      x5f8_targetPos =
          mgr.GetPlayer()->GetTranslation() +
          0.5f * CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, random.Float() - 0.5f);
    } else {
      x5f8_targetPos =
          GetTranslation() +
          15.f * (mgr.GetPlayer()->GetTranslation() +
                  CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, random.Float() + 0.5f) -
                  GetTranslation())
                     .AsNormalized();
    }
    FaceTarget(x5f8_targetPos);
    x5e8_stateProgress = 0;
    x742_30_attackOver = false;
    x742_24_receivedTelegraph = false;
    x742_28_onGround = false;
    break;
  }
  case kStateMsg_Update:
    switch (x5e8_stateProgress) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Jump) {
        x5e8_stateProgress = 1;
      } else {
        x742_25_jumpVelDirty = true;
        FaceTarget(x5f8_targetPos);
        x450_bodyController->CommandMgr().DeliverCmd(CBCJumpCmd(x5f8_targetPos, pas::kJT_Normal));
      }
      break;
    case 1:
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x742_28_onGround = true;
    x742_30_attackOver = true;
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
    x5f8_targetPos = GetTranslation() - dir * 3.f;
    FaceTarget(x5f8_targetPos);
    x5e8_stateProgress = 0;
    x742_27_landed = false;
    x742_28_onGround = false;
    x742_25_jumpVelDirty = true;
    x450_bodyController->CommandMgr().DeliverCmd(CBCJumpCmd(x5f8_targetPos, pas::kJT_One));
    break;
  }
  case kStateMsg_Update:
    x3b4_speed = 1.f;
    break;
  case kStateMsg_Deactivate:
    x742_28_onGround = true;
    break;
  }
}

void CParasite::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x5f8_targetPos = mgr.GetPlayer()->GetTranslation() + CVector3f(0.f, 0.f, 1.5f);
    break;
  case kStateMsg_Update:
    x450_bodyController->FaceDirection3D(
        ProjectVectorToPlane(x5f8_targetPos - GetTranslation(), GetTransform().GetUp()),
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
      CParasite* other =
          CPatterned::CastTo< CParasite >(TPatternedCast< CParasite >(mgr.ObjectById(*it)));
      if (!other) {
        it = parasites.erase(it);
        continue;
      }
      if (other != this && other->IsAlive() &&
          (other->GetTranslation() - GetTranslation()).MagSquared() <
              x6d0_maxTelegraphReactDist * x6d0_maxTelegraphReactDist) {
        other->x742_24_receivedTelegraph = true;
        other->x608_telegraphRemTime = mgr.Random()->Float() * 0.5f + 0.5f;
        other->x5f8_targetPos = GetTranslation();
      }
      ++it;
    }
    x400_24_hitByPlayerProjectile = false;
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
    x330_stateMachineState.SetDelay(x710_haltDelay);
    x32c_animState = kAS_Ready;
    x743_24_halted = true;
    x5d6_24_alignToFloor = true;
    if (x5d0_walkerType == kWT_Geemer)
      CSfxManager::AddEmitter(x73c_haltSfx, GetTranslation(), CVector3f::Zero(), true, false);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_LoopReaction, &CPatterned::TryLoopedReaction, 1);
    x400_24_hitByPlayerProjectile = false;
    break;
  case kStateMsg_Deactivate:
    x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    x32c_animState = kAS_NotReady;
    x743_24_halted = false;
    x5d6_24_alignToFloor = false;
    break;
  default:
    break;
  }
}

void CParasite::Crouch(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    x450_bodyController->SetLocomotionType(pas::kLT_Crouch);
    if (x5d0_walkerType == kWT_Geemer)
      CSfxManager::AddEmitter(x740_crouchSfx, GetTranslation(), CVector3f::Zero(), true, false);
  }
}

void CParasite::GetUp(CStateManager&, EStateMsg msg, float) {
  if (msg == kStateMsg_Activate) {
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    if (x5d0_walkerType == kWT_Geemer)
      CSfxManager::AddEmitter(x73e_getUpSfx, GetTranslation(), CVector3f::Zero(), true, false);
  }
}

void CParasite::Touch(CActor& actor, CStateManager& mgr) { CPatterned::Touch(actor, mgr); }

void CParasite::UpdatePFDestination(CStateManager& mgr) {
  // Empty
}

void CParasite::DoFlockingBehavior(CStateManager& mgr) {
  CVector3f upVec = GetTransform().GetUp();
  rstl::reserved_vector< TUniqueId, 1024 > parasiteList;
  float radius = x6e4_parasiteSearchRadius;
  const CVector3f position = GetTranslation();
  CAABox aabb(position - CVector3f(radius, radius, radius),
              position + CVector3f(radius, radius, radius));
  if ((x5d4_thinkCounter % 6) == 0) {
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    nearList.clear();
    static const CMaterialFilter filter =
        CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
    CParasite* closestParasite = nullptr;
    float minDistSq = 2.f + x6e8_parasiteSeparationDist * x6e8_parasiteSeparationDist;
    mgr.BuildNearList(nearList, aabb, filter, nullptr);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      if (CParasite* parasite =
              CPatterned::CastTo< CParasite >(TPatternedCast< CParasite >(mgr.ObjectById(*it)))) {
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
    if (closestParasite && x6ec_parasiteSeparationWeight > 0.f && x6e8_parasiteSeparationDist > 0.f)
      x628_parasiteSeparationMove =
          x45c_steeringBehaviors.Separation(*this, closestParasite->GetTranslation(),
                                            x6e8_parasiteSeparationDist) *
          x604_activeSpeed;
    else
      x628_parasiteSeparationMove = CVector3f::Zero();
    x634_parasiteCohesionMove =
        x45c_steeringBehaviors.Cohesion(*this, parasiteList, 0.6f, mgr) * x604_activeSpeed;
    x640_parasiteAlignmentMove =
        x45c_steeringBehaviors.Alignment(*this, parasiteList, mgr) * x604_activeSpeed;
  }

  if ((mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
      x700_playerSeparationDist * x700_playerSeparationDist) {
    const CVector3f playerSeparation =
        ProjectVectorToPlane(x45c_steeringBehaviors.Separation(*this,
                                                               mgr.GetPlayer()->GetTranslation(),
                                                               x700_playerSeparationDist),
                             upVec) *
        x604_activeSpeed;
    x450_bodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(playerSeparation, CVector3f::Zero(), x704_playerSeparationWeight));
  }

  if (!(x628_parasiteSeparationMove == CVector3f::Zero())) {
    x450_bodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(x628_parasiteSeparationMove, upVec),
                         CVector3f::Zero(), x6ec_parasiteSeparationWeight));
  }

  for (AUTO(it, x5d8_doorRepulsors.begin()); it != x5d8_doorRepulsors.end(); ++it) {
    const CRepulsor& r = *it;
    if ((r.GetPos() - GetTranslation()).MagSquared() < r.GetRadius() * r.GetRadius()) {
      const CVector3f doorSeparation =
          x45c_steeringBehaviors.Separation(*this, r.GetPos(), r.GetRadius()) * x604_activeSpeed;
      x450_bodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(ProjectVectorToPlane(doorSeparation, upVec), CVector3f::Zero(), 1.f));
    }
  }

  if (x608_telegraphRemTime <= 0.f) {
    x450_bodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(x634_parasiteCohesionMove, upVec), CVector3f::Zero(),
                         x6f4_parasiteCohesionWeight));
    x450_bodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(x640_parasiteAlignmentMove, upVec), CVector3f::Zero(),
                         x6f0_parasiteAlignmentWeight));
    const CVector3f seek =
        ProjectVectorToPlane(x45c_steeringBehaviors.Seek(*this, x2e0_destPos), upVec) *
        x604_activeSpeed;
    x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        ProjectVectorToPlane(seek, upVec), CVector3f::Zero(), x6f8_destinationSeekWeight));
    x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        GetTransform().GetForward() * x604_activeSpeed, CVector3f::Zero(), x6fc_forwardMoveWeight));
  }
}

void CParasite::Render(const CStateManager& mgr) const { CWallWalker::Render(mgr); }

const CDamageVulnerability* CParasite::GetDamageVulnerability() const {
  switch (x5d0_walkerType) {
  case kWT_Oculus:
    if (x743_24_halted)
      return &x64c_oculusHaltDVuln;
    break;
  case kWT_IceZoomer:
    if (!x743_25_vulnerable)
      return &CDamageVulnerability::ImmuneVulnerability();
    break;
  default:
    break;
  }
  return CAi::GetDamageVulnerability();
}

CDamageInfo CParasite::GetContactDamage() const {
  switch (x5d0_walkerType) {
  case kWT_Oculus:
    if (x743_24_halted)
      return x6b4_oculusHaltDInfo;
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
  x620_collisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), descs, GetActive());
}

void CParasite::DestroyActorManager(CStateManager& mgr) {
  x620_collisionActorManager->Destroy(mgr);
}

void CParasite::SetupIceZoomerVulnerability(CStateManager& mgr, const CDamageVulnerability& dVuln,
                                            const CHealthInfo& hInfo) {
  for (uint i = 0; i < x620_collisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& cDesc =
        x620_collisionActorManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = cDesc.GetCollisionActorId();
    if (CCollisionActor* const act = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      act->SetDamageVulnerability(dVuln);
      *act->HealthInfo(mgr) = hInfo;
    }
  }
}

void CParasite::UpdateCollisionActors(float dt, CStateManager& mgr) {
  x620_collisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  if (!x743_25_vulnerable) {
    float totalHP = 0.f;
    for (uint i = 0; i < x620_collisionActorManager->GetNumCollisionActors(); ++i) {
      const CJointCollisionDescription& cDesc =
          x620_collisionActorManager->GetCollisionDescFromIndex(i);
      const TUniqueId id = cDesc.GetCollisionActorId();
      if (CCollisionActor* cact = TCastToPtr< CCollisionActor >(mgr.ObjectById(id)))
        totalHP += cact->HealthInfo(mgr)->GetHP();
    }
    if (totalHP <= 0.f) {
      x743_25_vulnerable = true;
      AddMaterial(kMT_Solid, mgr);
      RemoveMaterial(kMT_ProjectilePassthrough, mgr);
      DestroyActorManager(mgr);
      ModelData()->AnimationData()->SubstituteModelData(*x624_extraModel);
    }
  }
}

void CParasite::MassiveDeath(CStateManager& mgr) { CPatterned::MassiveDeath(mgr); }

void CParasite::MassiveFrozenDeath(CStateManager& mgr) { CPatterned::MassiveFrozenDeath(mgr); }

bool CParasite::IsOnGround() const { return x742_28_onGround; }

CParasite::~CParasite() {}
