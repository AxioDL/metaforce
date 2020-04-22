#include "Runtime/MP1/World/CParasite.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptDoor.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

const float CParasite::flt_805A8FB0 = 2.f * std::sqrt(2.5f / CPhysicsActor::GravityConstant());
const float CParasite::skAttackVelocity = 15.f / 2.f * (std::sqrt(2.5f / CPhysicsActor::GravityConstant()));
short CParasite::word_805A8FC0 = 0;
const float CParasite::flt_805A8FB8 = 2.f * std::sqrt(2.5f / CPhysicsActor::GravityConstant());
const float CParasite::skRetreatVelocity = 3.f / 2.f * std::sqrt(2.5f / CPhysicsActor::GravityConstant());

CParasite::CParasite(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                     const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, EBodyType bodyType,
                     float maxTelegraphReactDist, float advanceWpRadius, float f3, float alignAngVel, float f5,
                     float stuckTimeThreshold, float collisionCloseMargin, float parasiteSearchRadius,
                     float parasiteSeparationDist, float parasiteSeparationWeight, float parasiteAlignmentWeight,
                     float parasiteCohesionWeight, float destinationSeekWeight, float forwardMoveWeight,
                     float playerSeparationDist, float playerSeparationWeight, float playerObstructionMinDist,
                     float haltDelay, bool disableMove, EWalkerType wType, const CDamageVulnerability& dVuln,
                     const CDamageInfo& parInfo, u16 haltSfx, u16 getUpSfx, u16 crouchSfx, CAssetId modelRes,
                     CAssetId skinRes, float iceZoomerJointHP, const CActorParameters& aParams)
: CWallWalker(ECharacter::Parasite, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
              EColliderType::Zero, bodyType, aParams, collisionCloseMargin, alignAngVel, EKnockBackVariant::Small,
              advanceWpRadius, wType, playerObstructionMinDist, disableMove)
, x64c_oculusHaltDVuln(dVuln)
, x6b4_oculusHaltDInfo(parInfo)
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
, x708_unmorphedRadius(pInfo.GetHeight() * 0.5f)
, x710_haltDelay(haltDelay)
, x714_iceZoomerJointHP(iceZoomerJointHP)
, x73c_haltSfx(CSfxManager::TranslateSFXID(haltSfx))
, x73e_getUpSfx(CSfxManager::TranslateSFXID(getUpSfx))
, x740_crouchSfx(CSfxManager::TranslateSFXID(crouchSfx)) {
  switch (x5d0_walkerType) {
  case EWalkerType::Geemer:
    x460_knockBackController.SetEnableFreeze(false);
    [[fallthrough]];
  case EWalkerType::Oculus:
    x460_knockBackController.SetAutoResetImpulse(false);
    break;
  case EWalkerType::IceZoomer: {
    TLockedToken<CModel> model = g_SimplePool->GetObj({FOURCC('CMDL'), modelRes});
    TLockedToken<CModel> skin = g_SimplePool->GetObj({FOURCC('CSKR'), skinRes});
    x624_extraModel =
        CToken(TObjOwnerDerivedFromIObj<CSkinnedModel>::GetNewDerivedObject(std::make_unique<CSkinnedModel>(
            model, skin, x64_modelData->GetAnimationData()->GetModelData()->GetLayoutInfo(), 1, 1)));
    break;
  }
  default:
    break;
  }
  if (x5d0_walkerType == EWalkerType::Oculus) {
    x460_knockBackController.SetEnableShock(false);
    x460_knockBackController.SetEnableBurn(false);
    x460_knockBackController.SetEnableBurnDeath(false);
    x460_knockBackController.SetEnableExplodeDeath(false);
    x460_knockBackController.SetX82_24(false);
  }
}

void CParasite::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CParasite::SetupIceZoomerCollision(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> descs;
  descs.reserve(2);
  descs.push_back(CJointCollisionDescription::SphereCollision(
      x64_modelData->GetAnimationData()->GetLocatorSegId("Ice_LCTR"sv), 0.4f, "Ice_LCTR"sv, 0.001f));
  RemoveMaterial(EMaterialTypes::Solid, mgr);
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
  x620_collisionActorManager =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), descs, GetActive());
}

void CParasite::SetupIceZoomerVulnerability(CStateManager& mgr, const CDamageVulnerability& dVuln,
                                            const CHealthInfo& hInfo) {
  for (u32 i = 0; i < x620_collisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& cDesc = x620_collisionActorManager->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(cDesc.GetCollisionActorId())) {
      act->SetDamageVulnerability(dVuln);
      *act->HealthInfo(mgr) = hInfo;
    }
  }
}

void CParasite::AddDoorRepulsors(CStateManager& mgr) {
  u32 doorCount = 0;
  for (CEntity* ent : mgr.GetPhysicsActorObjectList())
    if (TCastToPtr<CScriptDoor> door = ent)
      if (door->GetAreaIdAlways() == GetAreaIdAlways())
        ++doorCount;
  x5d8_doorRepulsors.reserve(doorCount);
  for (CEntity* ent : mgr.GetPhysicsActorObjectList())
    if (TCastToPtr<CScriptDoor> door = ent)
      if (door->GetAreaIdAlways() == GetAreaIdAlways()) {
        if (auto tb = door->GetTouchBounds()) {
          float diagMag = (tb->min - tb->max).magnitude() * 0.75f;
          x5d8_doorRepulsors.emplace_back(tb->center(), diagMag);
        }
      }
}

static TUniqueId lastParasite = kInvalidUniqueId;

void CParasite::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    mgr.GetActiveParasites().push_back(GetUniqueId());
    CActor::CreateShadow(false);
    x604_activeSpeed = x3b4_speed;
    CPhysicsActor::SetBoundingBox(zeus::CAABox(zeus::CVector3f(-x590_colSphere.GetSphere().radius),
                                               zeus::CVector3f(x590_colSphere.GetSphere().radius)));
    lastParasite = GetUniqueId();
    AddDoorRepulsors(mgr);
    if (x5d0_walkerType == EWalkerType::IceZoomer) {
      SetupIceZoomerCollision(mgr);
      SetupIceZoomerVulnerability(mgr, x64c_oculusHaltDVuln,
                                  CHealthInfo(x714_iceZoomerJointHP, HealthInfo(mgr)->GetKnockbackResistance()));
    }
    break;
  case EScriptObjectMessage::Deleted:
    mgr.GetActiveParasites().remove(GetUniqueId());
    if (x5d0_walkerType == EWalkerType::IceZoomer)
      DestroyActorManager(mgr);
    break;
  case EScriptObjectMessage::Jumped:
    if (x742_25_jumpVelDirty) {
      UpdateJumpVelocity();
      x742_25_jumpVelDirty = false;
    }
    break;
  case EScriptObjectMessage::Activate:
    x5d6_27_disableMove = false;
    if (x5d0_walkerType == EWalkerType::Parasite)
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    break;
  case EScriptObjectMessage::InvulnDamage:
    if (x5d0_walkerType == EWalkerType::Oculus) {
      if (TCastToConstPtr<CActor> act = mgr.GetObjectById(uid)) {
        float distSq = (act->GetTranslation() - GetTranslation()).magSquared();
        auto tb = GetTouchBounds();
        float maxComp =
            std::max(std::max(tb->max.y() - tb->min.y(), tb->max.z() - tb->min.z()), tb->max.x() - tb->min.x());
        float maxCompSq = maxComp * maxComp + 1.f;
        if (distSq < maxCompSq * maxCompSq)
          x743_26_oculusShotAt = true;
      }
    }
    break;
  case EScriptObjectMessage::SuspendedMove:
    if (x620_collisionActorManager)
      x620_collisionActorManager->SetMovable(mgr, false);
    break;
  default:
    break;
  }
}

void CParasite::PreThink(float dt, CStateManager& mgr) {
  CWallWalker::PreThink(dt, mgr);
  x743_26_oculusShotAt = false;
}

void CParasite::UpdateCollisionActors(float dt, CStateManager& mgr) {
  x620_collisionActorManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  if (!x743_25_vulnerable) {
    float totalHP = 0.f;
    for (u32 i = 0; i < x620_collisionActorManager->GetNumCollisionActors(); ++i) {
      const CJointCollisionDescription& cDesc = x620_collisionActorManager->GetCollisionDescFromIndex(i);
      if (TCastToPtr<CCollisionActor> cact = mgr.ObjectById(cDesc.GetCollisionActorId()))
        totalHP += cact->HealthInfo(mgr)->GetHP();
    }
    if (totalHP <= 0.f) {
      x743_25_vulnerable = true;
      AddMaterial(EMaterialTypes::Solid, mgr);
      RemoveMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
      DestroyActorManager(mgr);
      x64_modelData->GetAnimationData()->SubstituteModelData(x624_extraModel);
    }
  }
}

void CParasite::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  ++x5d4_thinkCounter;
  if (x5d0_walkerType == EWalkerType::IceZoomer)
    UpdateCollisionActors(dt, mgr);

  x5d6_26_playerObstructed = false;
  CGameArea* area = mgr.GetWorld()->GetArea(GetAreaIdAlways());

  CGameArea::EOcclusionState r6 = CGameArea::EOcclusionState::Occluded;
  if (area->IsPostConstructed())
    r6 = area->GetPostConstructed()->x10dc_occlusionState;
  if (r6 != CGameArea::EOcclusionState::Visible)
    x5d6_26_playerObstructed = true;

  if (!x5d6_26_playerObstructed) {
    zeus::CVector3f plVec = mgr.GetPlayer().GetTranslation();
    float distance = (GetTranslation() - plVec).magnitude();

    if (distance > x5c4_playerObstructionMinDist) {
      CRayCastResult res = mgr.RayStaticIntersection(plVec, (GetTranslation() - plVec).normalized(), distance,
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
    if (x450_bodyController->IsFrozen()) {
      if ((GetTranslation() - x614_lastStuckPos).magSquared() < 0.3f /* <- Used to be a static variable */ * dt)
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
    float radius;
    if (pl->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed && !x742_30_attackOver)
      radius = x590_colSphere.GetSphere().radius;
    else
      radius = x708_unmorphedRadius;

    zeus::CAABox aabox{GetTranslation() - radius, GetTranslation() + radius};
    auto plBox = pl->GetTouchBounds();

    if (plBox && plBox->intersects(aabox)) {
      if (!x742_30_attackOver) {
        x742_30_attackOver = true;
        x742_27_landed = false;
      }

      if (x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
  }

  CWallWalker::Think(dt, mgr);

  if (x5d6_27_disableMove)
    return;

  if (x450_bodyController->IsFrozen())
    return;

  x3b4_speed = x604_activeSpeed;
  if (x5d6_24_alignToFloor)
    AlignToFloor(mgr, x590_colSphere.GetSphere().radius, GetTranslation() + 2.f * dt * x138_velocity, dt);

  x742_27_landed = false;
}

void CParasite::Render(CStateManager& mgr) { CWallWalker::Render(mgr); }

const CDamageVulnerability* CParasite::GetDamageVulnerability() const {
  switch (x5d0_walkerType) {
  case EWalkerType::Oculus:
    if (x743_24_halted)
      return &x64c_oculusHaltDVuln;
    break;
  case EWalkerType::IceZoomer:
    if (!x743_25_vulnerable)
      return &CDamageVulnerability::ImmuneVulnerabilty();
    break;
  default:
    break;
  }
  return CAi::GetDamageVulnerability();
}

CDamageInfo CParasite::GetContactDamage() const {
  if (x5d0_walkerType == EWalkerType::Oculus && x743_24_halted)
    return x6b4_oculusHaltDInfo;
  return CPatterned::GetContactDamage();
}

void CParasite::Touch(CActor& actor, CStateManager& mgr) { CPatterned::Touch(actor, mgr); }

zeus::CVector3f CParasite::GetAimPosition(const CStateManager&, float) const { return GetTranslation(); }

void CParasite::CollidedWith(TUniqueId uid, const CCollisionInfoList& list, CStateManager&) {
  static constexpr CMaterialList testList(EMaterialTypes::Character, EMaterialTypes::Player);
  if (x743_27_inJump) {
    for (const auto& info : list) {
      if (!x5d6_24_alignToFloor && info.GetMaterialLeft().Intersection(testList) == 0) {
        OrientToSurfaceNormal(info.GetNormalLeft(), 360.f);
        CPhysicsActor::Stop();
        SetVelocityWR(zeus::skZero3f);
        x742_27_landed = true;
        x742_28_onGround = true;
      }
    }
  }
}

void CParasite::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  CPhysicsActor::Stop();
  TelegraphAttack(mgr, EStateMsg::Activate, 0.f);
  SetMomentumWR({0.f, 0.f, -GetWeight()});
  CPatterned::Death(mgr, direction, state);
}

void CParasite::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x742_26_ = true;
    x5d6_24_alignToFloor = true;
    if (!x5d6_27_disableMove && x5d0_walkerType == EWalkerType::Parasite)
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    SetMomentumWR(zeus::skZero3f);
    x5d6_25_hasAlignSurface = false;
    xf8_24_movable = false;
    break;
  case EStateMsg::Update:
    if (x5bc_patrolPauseRemTime > 0.f) {
      x5bc_patrolPauseRemTime -= dt;
      if (x5bc_patrolPauseRemTime <= 0.f) {
        if (x5d0_walkerType == EWalkerType::Parasite)
          x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
        x5bc_patrolPauseRemTime = 0.f;
      }
    }
    GotoNextWaypoint(mgr);
    if (x5bc_patrolPauseRemTime <= 0.f && !x5d6_27_disableMove)
      DoFlockingBehavior(mgr);
    break;
  case EStateMsg::Deactivate:
    x5d6_24_alignToFloor = false;
    xf8_24_movable = true;
    break;
  default:
    break;
  }
}

void CParasite::UpdatePFDestination(CStateManager& mgr) {
  // Empty
}

void CParasite::DoFlockingBehavior(CStateManager& mgr) {
  zeus::CVector3f upVec = x34_transform.basis[2];
  rstl::reserved_vector<TUniqueId, 1024> parasiteList;
  zeus::CAABox aabb(GetTranslation() - x6e4_parasiteSearchRadius, GetTranslation() + x6e4_parasiteSearchRadius);
  if ((x5d4_thinkCounter % 6) == 0) {
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    static constexpr CMaterialFilter filter = CMaterialFilter::MakeInclude(EMaterialTypes::Character);
    CParasite* closestParasite = nullptr;
    float minDistSq = 2.f + x6e8_parasiteSeparationDist * x6e8_parasiteSeparationDist;
    mgr.BuildNearList(nearList, aabb, filter, nullptr);
    for (TUniqueId id : nearList) {
      if (CParasite* parasite = CPatterned::CastTo<CParasite>(mgr.ObjectById(id))) {
        if (parasite != this && parasite->IsAlive()) {
          parasiteList.push_back(parasite->GetUniqueId());
          float distSq = (parasite->GetTranslation() - GetTranslation()).magSquared();
          if (distSq < minDistSq) {
            minDistSq = distSq;
            closestParasite = parasite;
          }
        }
      }
    }
    if (closestParasite && x6ec_parasiteSeparationWeight > 0.f && x6e8_parasiteSeparationDist > 0.f)
      x628_parasiteSeparationMove =
          x45c_steeringBehaviors.Separation(*this, closestParasite->GetTranslation(), x6e8_parasiteSeparationDist) *
          x604_activeSpeed;
    else
      x628_parasiteSeparationMove = zeus::skZero3f;
    x634_parasiteCohesionMove = x45c_steeringBehaviors.Cohesion(*this, parasiteList, 0.6f, mgr) * x604_activeSpeed;
    x640_parasiteAlignmentMove = x45c_steeringBehaviors.Alignment(*this, parasiteList, mgr) * x604_activeSpeed;
  }

  if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() <
      x700_playerSeparationDist * x700_playerSeparationDist) {
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(x45c_steeringBehaviors.Separation(*this, mgr.GetPlayer().GetTranslation(),
                                                                                x700_playerSeparationDist),
                                              upVec) *
                             x604_activeSpeed,
                         zeus::skZero3f, x704_playerSeparationWeight));
  }

  if (x628_parasiteSeparationMove != zeus::skZero3f) {
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(x628_parasiteSeparationMove, upVec), zeus::skZero3f,
                         x6ec_parasiteSeparationWeight));
  }

  for (const auto& r : x5d8_doorRepulsors) {
    if ((r.GetVector() - GetTranslation()).magSquared() < r.GetFloat() * r.GetFloat()) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
          ProjectVectorToPlane(x45c_steeringBehaviors.Separation(*this, r.GetVector(), r.GetFloat()) * x604_activeSpeed,
                               upVec),
          zeus::skZero3f, 1.f));
    }
  }

  if (x608_telegraphRemTime <= 0.f) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
        ProjectVectorToPlane(x634_parasiteCohesionMove, upVec), zeus::skZero3f, x6f4_parasiteCohesionWeight));
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(x640_parasiteAlignmentMove, upVec), zeus::skZero3f,
                         x6f0_parasiteAlignmentWeight));
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
        ProjectVectorToPlane(
            ProjectVectorToPlane(x45c_steeringBehaviors.Seek(*this, x2e0_destPos), upVec) * x604_activeSpeed, upVec),
        zeus::skZero3f, x6f8_destinationSeekWeight));
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(x34_transform.basis[1] * x604_activeSpeed, zeus::skZero3f, x6fc_forwardMoveWeight));
  }
}

void CParasite::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x742_26_ = true;
    x5d6_24_alignToFloor = true;
    if (x5d0_walkerType == EWalkerType::Parasite)
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    SetMomentumWR(zeus::skZero3f);
    xf8_24_movable = false;
    break;
  case EStateMsg::Update:
    UpdatePFDestination(mgr);
    DoFlockingBehavior(mgr);
    break;
  case EStateMsg::Deactivate:
    xf8_24_movable = true;
    x5d6_24_alignToFloor = false;
    x742_26_ = false;
    break;
  default:
    break;
  }
}

void CParasite::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x5f8_targetPos = mgr.GetPlayer().GetTranslation() + zeus::CVector3f(0.f, 0.f, 1.5f);
    break;
  case EStateMsg::Update:
    x450_bodyController->FaceDirection3D(
        ProjectVectorToPlane(x5f8_targetPos - GetTranslation(), x34_transform.basis[2]), x34_transform.basis[1], 2.f);
    break;
  default:
    break;
  }
}

TUniqueId CParasite::RecursiveFindClosestWayPoint(CStateManager& mgr, TUniqueId id, float& dist) {
  TUniqueId ret = id;
  TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(id);
  if (!wp)
    return ret;
  wp->SetActive(false);
  dist = (wp->GetTranslation() - GetTranslation()).magSquared();
  for (const auto& conn : wp->GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next) {
      TUniqueId nextId = mgr.GetIdForScript(conn.x8_objId);
      if (nextId != kInvalidUniqueId) {
        if (TCastToConstPtr<CScriptWaypoint> wp2 = mgr.GetObjectById(nextId)) {
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

TUniqueId CParasite::GetClosestWaypointForState(EScriptObjectState state, CStateManager& mgr) {
  float minDist = FLT_MAX;
  TUniqueId ret = kInvalidUniqueId;
  for (const auto& conn : GetConnectionList()) {
    if (conn.x0_state == state && conn.x4_msg == EScriptObjectMessage::Follow) {
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

void CParasite::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetMomentumWR(zeus::skZero3f);
    TUniqueId wpId = GetClosestWaypointForState(EScriptObjectState::Patrol, mgr);
    if (wpId != kInvalidUniqueId)
      x2dc_destObj = wpId;
  }
}

void CParasite::Halt(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case EStateMsg::Activate:
    x330_stateMachineState.SetDelay(x710_haltDelay);
    x32c_animState = EAnimState::Ready;
    x743_24_halted = true;
    x5d6_24_alignToFloor = true;
    if (x5d0_walkerType == EWalkerType::Geemer)
      CSfxManager::AddEmitter(x73c_haltSfx, GetTranslation(), zeus::skZero3f, true, false, 0x7f,
                              kInvalidAreaId);
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::LoopReaction, &CPatterned::TryLoopReaction, 1);
    x400_24_hitByPlayerProjectile = false;
    break;
  case EStateMsg::Deactivate:
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
    x32c_animState = EAnimState::NotReady;
    x743_24_halted = false;
    x5d6_24_alignToFloor = false;
    break;
  default:
    break;
  }
}

void CParasite::Run(CStateManager&, EStateMsg, float) {
  // Empty
}

void CParasite::Generate(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case EStateMsg::Activate:
    x5e8_stateProgress = 0;
    break;
  case EStateMsg::Update:
    switch (x5e8_stateProgress) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate)
        x5e8_stateProgress = 1;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
      break;
    case 1:
      if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Generate)
        x5e8_stateProgress = 2;
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void CParasite::Deactivate(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case EStateMsg::Activate:
    x5e8_stateProgress = 0;
    SendScriptMsgs(EScriptObjectState::DeactivateState, mgr, EScriptObjectMessage::None);
    mgr.FreeScriptObject(GetUniqueId());
    break;
  case EStateMsg::Update:
    if (x5e8_stateProgress == 0) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate)
        x5e8_stateProgress = 1;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::One));
    }
    break;
  default:
    break;
  }
}

void CParasite::Attack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case EStateMsg::Activate:
    x608_telegraphRemTime = 0.f;
    if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
      float rz = mgr.GetActiveRandom()->Float();
      float ry = mgr.GetActiveRandom()->Float();
      float rx = mgr.GetActiveRandom()->Float();
      x5f8_targetPos = (zeus::CVector3f(rx, ry, rz) - 0.5f) * 0.5f + mgr.GetPlayer().GetTranslation();
    } else {
      float rz = mgr.GetActiveRandom()->Float();
      float ry = mgr.GetActiveRandom()->Float();
      float rx = mgr.GetActiveRandom()->Float();
      x5f8_targetPos =
          (zeus::CVector3f(rx, ry, rz) + mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized() * 15.f +
          GetTranslation();
    }
    FaceTarget(x5f8_targetPos);
    x5e8_stateProgress = 0;
    x742_30_attackOver = false;
    x742_24_receivedTelegraph = false;
    x742_28_onGround = false;
    break;
  case EStateMsg::Update:
    switch (x5e8_stateProgress) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Jump) {
        x5e8_stateProgress = 1;
      } else {
        x742_25_jumpVelDirty = true;
        FaceTarget(x5f8_targetPos);
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCJumpCmd(x5f8_targetPos, pas::EJumpType::Normal));
      }
      break;
    default:
      break;
    }
    break;
  case EStateMsg::Deactivate:
    x742_28_onGround = true;
    x742_30_attackOver = true;
    break;
  default:
    break;
  }
}

void CParasite::Crouch(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    if (x5d0_walkerType == EWalkerType::Geemer)
      CSfxManager::AddEmitter(x740_crouchSfx, GetTranslation(), zeus::skZero3f, true, false, 0x7f,
                              kInvalidAreaId);
  }
}

void CParasite::GetUp(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    if (x5d0_walkerType == EWalkerType::Geemer)
      CSfxManager::AddEmitter(x73e_getUpSfx, GetTranslation(), zeus::skZero3f, true, false, 0x7f,
                              kInvalidAreaId);
  }
}

void CParasite::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case EStateMsg::Activate:
    for (auto it = mgr.GetActiveParasites().begin(); it != mgr.GetActiveParasites().end();) {
      CParasite* other = CPatterned::CastTo<CParasite>(mgr.ObjectById(*it));
      if (!other) {
        it = mgr.GetActiveParasites().erase(it);
        continue;
      }
      if (other != this && other->IsAlive() &&
          (other->GetTranslation() - GetTranslation()).magSquared() <
              x6d0_maxTelegraphReactDist * x6d0_maxTelegraphReactDist) {
        other->x742_24_receivedTelegraph = true;
        other->x608_telegraphRemTime = mgr.GetActiveRandom()->Float() * 0.5f + 0.5f;
        other->x5f8_targetPos = GetTranslation();
      }
      ++it;
    }
    x400_24_hitByPlayerProjectile = false;
    break;
  default:
    break;
  }
}

void CParasite::Jump(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case EStateMsg::Activate:
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    SetMomentumWR({0.f, 0.f, -GetWeight()});
    x742_28_onGround = false;
    x5d6_24_alignToFloor = false;
    x742_27_landed = false;
    x743_27_inJump = true;
    break;
  case EStateMsg::Update:
    SetMomentumWR({0.f, 0.f, -GetWeight()});
    break;
  case EStateMsg::Deactivate:
    RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
    SetMomentumWR(zeus::skZero3f);
    x742_28_onGround = true;
    x742_27_landed = false;
    x743_27_inJump = false;
    break;
  }
}

void CParasite::FaceTarget(const zeus::CVector3f& target) {
  zeus::CQuaternion q = zeus::CQuaternion::lookAt(zeus::CTransform().basis[1], target - GetTranslation(),
                                                  zeus::degToRad(360.f));
  SetTransform(q.toTransform(GetTranslation()));
}

void CParasite::Retreat(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case EStateMsg::Activate: {
    zeus::CVector3f dir = mgr.GetPlayer().GetTranslation() - GetTranslation();
    dir.z() = 0.f;
    if (dir.canBeNormalized())
      dir.normalize();
    else
      dir = mgr.GetPlayer().GetTransform().basis[1];
    x5f8_targetPos = GetTranslation() - dir * 3.f;
    FaceTarget(x5f8_targetPos);
    x5e8_stateProgress = 0;
    x742_27_landed = false;
    x742_28_onGround = false;
    x742_25_jumpVelDirty = true;
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCJumpCmd(x5f8_targetPos, pas::EJumpType::One));
    break;
  }
  case EStateMsg::Update:
    x3b4_speed = 1.f;
    break;
  case EStateMsg::Deactivate:
    x742_28_onGround = true;
    break;
  }
}

bool CParasite::AnimOver(CStateManager&, float) { return x5e8_stateProgress == 2; }

bool CParasite::ShouldAttack(CStateManager& mgr, float arg) {
  bool shouldAttack = false;
  if (x742_24_receivedTelegraph && x608_telegraphRemTime > 0.1f)
    shouldAttack = true;
  if (!TooClose(mgr, arg) && InMaxRange(mgr, arg))
    return shouldAttack || InDetectionRange(mgr, 0.f);
  return false;
}

bool CParasite::CloseToWall(const CStateManager& mgr) const {
  static constexpr CMaterialFilter filter = CMaterialFilter::MakeInclude(EMaterialTypes::Solid);
  zeus::CAABox aabb = CPhysicsActor::GetBoundingBox();
  const float margin = x590_colSphere.GetSphere().radius + x5b0_collisionCloseMargin;
  aabb.min -= zeus::CVector3f(margin);
  aabb.max += zeus::CVector3f(margin);
  const CCollidableAABox cAABB(aabb, x68_material);
  return CGameCollision::DetectStaticCollisionBoolean(mgr, cAABB, {}, filter);
}

bool CParasite::HitSomething(CStateManager& mgr, float) {
  if (x5d4_thinkCounter & 0x1)
    return true;
  return x5b8_tumbleAngle < 270.f && CloseToWall(mgr);
}

bool CParasite::Stuck(CStateManager&, float) { return x60c_stuckTime > x6e0_stuckTimeThreshold; }

bool CParasite::Landed(CStateManager&, float) { return x742_27_landed; }

bool CParasite::AttackOver(CStateManager&, float) { return x742_30_attackOver; }

bool CParasite::ShotAt(CStateManager&, float) {
  if (x5d0_walkerType != EWalkerType::Oculus)
    return x400_24_hitByPlayerProjectile;
  return x743_26_oculusShotAt;
}

void CParasite::MassiveDeath(CStateManager& mgr) { CPatterned::MassiveDeath(mgr); }

void CParasite::MassiveFrozenDeath(CStateManager& mgr) { CPatterned::MassiveFrozenDeath(mgr); }

void CParasite::ThinkAboutMove(float dt) {
  if (!x68_material.HasMaterial(EMaterialTypes::GroundCollider))
    CPatterned::ThinkAboutMove(dt);
}

bool CParasite::IsOnGround() const { return x742_28_onGround; }

void CParasite::UpdateWalkerAnimation(CStateManager& mgr, float dt) { CActor::UpdateAnimation(dt, mgr, true); }

void CParasite::DestroyActorManager(CStateManager& mgr) { x620_collisionActorManager->Destroy(mgr); }

void CParasite::UpdateJumpVelocity() {
  SetMomentumWR({0.f, 0.f, -GetWeight()});
  zeus::CVector3f vec;

  if (!x742_30_attackOver) {
    vec = skAttackVelocity * GetTransform().frontVector();
    vec.z() = 0.5f * skRetreatVelocity;
  } else {
    vec = skRetreatVelocity * GetTransform().frontVector();
    vec.z() = 0.5f * skAttackVelocity;
  }

  float f30 = x150_momentum.z() / xe8_mass;
  float f31 = x5f8_targetPos.z() - GetTranslation().z();
  zeus::CVector3f vec2 = x5f8_targetPos - GetTranslation();
  vec2.z() = 0.f;
  float f29 = vec2.magnitude();

  if (f29 > FLT_EPSILON) {
    vec2 *= zeus::CVector3f{1.f / f29};
    float f28 = vec2.dot(vec);
    if (f28 > FLT_EPSILON) {
      float f27 = 0.f;
      bool isNeg = f31 < 0.f;
      float xPos, xNeg;
      if (CSteeringBehaviors::SolveQuadratic(f30, vec.z(), -f31, xPos, xNeg))
        f27 = isNeg ? xPos : xNeg;

      if (!isNeg)
        f27 = f27 * f29 / f28;

      if (f27 < 10.f) {
        vec = f29 / f27 * vec2;
        vec.z() = (0.5f * f30 * f27 + f31 / f27);
      }
    }
  }
  SetVelocityWR(vec);
}

} // namespace urde::MP1
