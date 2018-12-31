#include "CGroundMovement.hpp"
#include "World/CPhysicsActor.hpp"
#include "Collision/CGameCollision.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Collision/CAABoxFilter.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "World/CScriptPlatform.hpp"
#include "World/CWorld.hpp"
#include "World/CPlayer.hpp"

namespace urde {

void CGroundMovement::CheckFalling(CPhysicsActor& actor, CStateManager& mgr, float) {
  bool oob = true;
  for (const CGameArea& area : *mgr.GetWorld()) {
    if (area.GetAABB().intersects(*actor.GetTouchBounds())) {
      oob = false;
      break;
    }
  }

  if (!oob) {
    mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::Falling);
  } else {
    mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::OnFloor);
    actor.SetAngularVelocityWR(actor.GetAngularVelocityWR() * 0.98f);
    zeus::CVector3f vel = actor.GetTransform().transposeRotate(actor.GetVelocity());
    vel.z() = 0.f;
    actor.SetVelocityOR(vel);
    actor.SetMomentumWR(zeus::CVector3f::skZero);
  }
}

void CGroundMovement::MoveGroundCollider(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                         const rstl::reserved_vector<TUniqueId, 1024>* nearList) {
  CMotionState oldState = actor.GetMotionState();
  CMotionState newState = actor.PredictMotion_Internal(dt);
  float deltaMag = newState.x0_translation.magnitude();
  TUniqueId idDetect = kInvalidUniqueId;
  CCollisionInfoList collisionList;
  zeus::CAABox motionVol = actor.GetMotionVolume(dt);
  rstl::reserved_vector<TUniqueId, 1024> useColliderList;
  if (nearList)
    useColliderList = *nearList;
  mgr.BuildColliderList(useColliderList, actor, motionVol);
  CAreaCollisionCache cache(motionVol);
  float collideDt = dt;
  if (actor.GetCollisionPrimitive()->GetPrimType() != FOURCC('OBTG')) {
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (deltaMag > 0.5f * CGameCollision::GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive())) {
      zeus::CVector3f point = actor.GetCollisionPrimitive()->CalculateAABox(actor.GetPrimitiveTransform()).center();
      TUniqueId intersectId = kInvalidUniqueId;
      CMaterialFilter filter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});
      CRayCastResult result = mgr.RayWorldIntersection(intersectId, point, newState.x0_translation.normalized(),
                                                       deltaMag, filter, useColliderList);
      if (result.IsValid()) {
        collideDt = dt * (result.GetT() / deltaMag);
        newState = actor.PredictMotion_Internal(collideDt);
      }
    }
  }
  actor.MoveCollisionPrimitive(newState.x0_translation);
  if (CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                             actor.GetPrimitiveTransform(), actor.GetMaterialFilter(),
                                             useColliderList, idDetect, collisionList)) {
    actor.AddMotionState(newState);
    float resolved = 0.f;
    if (ResolveUpDown(cache, mgr, actor, actor.GetMaterialFilter(), useColliderList, actor.GetStepUpHeight(), 0.f,
                      resolved, collisionList)) {
      actor.SetMotionState(oldState);
      MoveGroundColliderXY(cache, mgr, actor, actor.GetMaterialFilter(), useColliderList, collideDt);
    }

  } else {
    actor.AddMotionState(newState);
  }

  float stepDown = actor.GetStepDownHeight();
  float resolved = 0.f;
  collisionList.Clear();
  TUniqueId stepZId = kInvalidUniqueId;
  if (stepDown >= 0.f) {
    if (MoveGroundColliderZ(cache, mgr, actor, actor.GetMaterialFilter(), useColliderList, -stepDown, resolved,
                            collisionList, stepZId)) {
      if (collisionList.GetCount() > 0) {
        CCollisionInfoList filteredList;
        CollisionUtil::FilterByClosestNormal(zeus::CVector3f{0.f, 0.f, 1.f}, collisionList, filteredList);
        if (filteredList.GetCount() > 0) {
          if (CGameCollision::IsFloor(filteredList.Front().GetMaterialLeft(),
                                      filteredList.Front().GetNormalLeft())) {
            if (TCastToPtr<CScriptPlatform> plat = mgr.ObjectById(stepZId))
              mgr.SendScriptMsg(plat.GetPtr(), actor.GetUniqueId(), EScriptObjectMessage::AddPlatformRider);
            CGameCollision::SendMaterialMessage(mgr, filteredList.Front().GetMaterialLeft(), actor);
            mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::OnFloor);
          } else {
            CheckFalling(actor, mgr, dt);
          }
        }
      }
    }
  } else {
    CheckFalling(actor, mgr, dt);
  }

  actor.ClearForcesAndTorques();
  actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
  if (actor.GetMaterialList().HasMaterial(EMaterialTypes::Player)) {
    CGameCollision::CollisionFailsafe(mgr, cache, actor, *actor.GetCollisionPrimitive(), useColliderList, 0.f, 1);
  }
}

bool CGroundMovement::ResolveUpDown(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                                    const CMaterialFilter& filter, rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                    float stepUp, float stepDown, float& fOut, CCollisionInfoList& list) {
  float zextent = stepDown;
  if (list.GetCount() <= 0)
    return true;

  zeus::CAABox aabb = zeus::CAABox::skInvertedBox;
  zeus::CVector3f normAccum = zeus::CVector3f::skZero;
  for (CCollisionInfo& info : list) {
    if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft())) {
      aabb.accumulateBounds(info.GetPoint());
      aabb.accumulateBounds(info.GetExtreme());
      normAccum += info.GetNormalLeft();
    }
  }

  if (normAccum.canBeNormalized())
    normAccum.normalize();
  else
    return true;

  zeus::CAABox actorAABB = actor.GetBoundingBox();
  if (normAccum.z() >= 0.f) {
    zextent = aabb.max.z() - actorAABB.min.z() + 0.02f;
    if (zextent > stepUp)
      return true;
  } else {
    zextent = aabb.min.z() - actorAABB.max.z() - 0.02f;
    if (zextent < -stepDown)
      return true;
  }

  actor.MoveCollisionPrimitive({0.f, 0.f, zextent});

  if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                     actor.GetPrimitiveTransform(), filter, nearList)) {
    fOut = zextent;
    actor.SetTranslation(actor.GetTranslation() + zeus::CVector3f(0.f, 0.f, zextent));
    actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);

    bool floor = false;
    for (CCollisionInfo& info : list) {
      if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft())) {
        floor = true;
        break;
      }
    }

    if (!floor)
      mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::LandOnNotFloor);

    return false;
  }

  return true;
}

bool CGroundMovement::MoveGroundColliderZ(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                                          const CMaterialFilter& filter,
                                          rstl::reserved_vector<TUniqueId, 1024>& nearList, float amt, float&,
                                          CCollisionInfoList& list, TUniqueId& idOut) {
  actor.MoveCollisionPrimitive({0.f, 0.f, amt});

  zeus::CAABox aabb = zeus::CAABox::skInvertedBox;
  if (CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(), actor.GetPrimitiveTransform(),
                                             filter, nearList, idOut, list)) {
    for (CCollisionInfo& info : list) {
      aabb.accumulateBounds(info.GetPoint());
      aabb.accumulateBounds(info.GetExtreme());
    }

    zeus::CAABox actorAABB = actor.GetBoundingBox();
    float zextent;
    if (amt > 0.f)
      zextent = aabb.min.z() - actorAABB.max.z() - 0.02f + amt;
    else
      zextent = aabb.max.z() - actorAABB.min.z() + 0.02f + amt;

    actor.MoveCollisionPrimitive({0.f, 0.f, zextent});

    if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                       actor.GetPrimitiveTransform(), filter, nearList)) {
      actor.SetTranslation(actor.GetTranslation() + zeus::CVector3f(0.f, 0.f, zextent));
      actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
    }

    bool floor = false;
    for (CCollisionInfo& info : list) {
      if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft())) {
        floor = true;
        break;
      }
    }

    if (!floor)
      mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::LandOnNotFloor);

    CCollisionInfoList filteredList;
    if (amt > 0.f)
      CollisionUtil::FilterByClosestNormal({0.f, 0.f, -1.f}, list, filteredList);
    else
      CollisionUtil::FilterByClosestNormal({0.f, 0.f, 1.f}, list, filteredList);

    if (filteredList.GetCount() > 0)
      CGameCollision::MakeCollisionCallbacks(mgr, actor, idOut, filteredList);

    return true;
  }

  return false;
}

void CGroundMovement::MoveGroundColliderXY(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                                           const CMaterialFilter& filter,
                                           rstl::reserved_vector<TUniqueId, 1024>& nearList, float dt) {
  bool didCollide = false;
  bool isPlayer = actor.GetMaterialList().HasMaterial(EMaterialTypes::Player);
  float remDt = dt;
  float originalDt = dt;
  TCastToPtr<CPhysicsActor> otherActor;
  CCollisionInfoList collisionList;
  CMotionState newMState = actor.PredictMotion_Internal(dt);
  float transMag = newMState.x0_translation.magnitude();
  float divMag;
  if (isPlayer)
    divMag = std::max(transMag / 5.f, 0.005f);
  else
    divMag = std::max(transMag / 3.f, 0.02f);

  float minExtent = 0.5f * CGameCollision::GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive());
  if (transMag > minExtent) {
    dt = minExtent * (dt / transMag);
    originalDt = dt;
    newMState = actor.PredictMotion_Internal(dt);
    divMag = std::min(divMag, minExtent);
  }

  float nonCollideDt = dt;
  do {
    actor.MoveCollisionPrimitive(newMState.x0_translation);
    collisionList.Clear();
    TUniqueId otherId = kInvalidUniqueId;
    bool collided =
        CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                               actor.GetPrimitiveTransform(), filter, nearList, otherId, collisionList);
    if (collided)
      otherActor = mgr.ObjectById(otherId);
    actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
    if (collided) {
      didCollide = true;
      if (newMState.x0_translation.magnitude() < divMag) {
        CCollisionInfoList backfaceFilteredList, floorFilteredList;
        zeus::CVector3f deltaVel = actor.GetVelocity();
        if (otherActor)
          deltaVel -= otherActor->GetVelocity();
        CollisionUtil::FilterOutBackfaces(deltaVel, collisionList, backfaceFilteredList);
        CAABoxFilter::FilterBoxFloorCollisions(backfaceFilteredList, floorFilteredList);
        CGameCollision::MakeCollisionCallbacks(mgr, actor, otherId, floorFilteredList);
        if (floorFilteredList.GetCount() == 0 && isPlayer) {
          CMotionState lastNonCollideState = actor.GetLastNonCollidingState();
          lastNonCollideState.x1c_velocity *= zeus::CVector3f(0.5f);
          lastNonCollideState.x28_angularMomentum *= zeus::CVector3f(0.5f);
          actor.SetMotionState(lastNonCollideState);
        }
        for (const CCollisionInfo& info : floorFilteredList) {
          CCollisionInfo infoCopy = info;
          float restitution =
              CGameCollision::GetCoefficientOfRestitution(infoCopy) + actor.GetCoefficientOfRestitutionModifier();
          if (otherActor)
            CGameCollision::CollideWithDynamicBodyNoRot(actor, *otherActor, infoCopy, restitution, true);
          else
            CGameCollision::CollideWithStaticBodyNoRot(actor, infoCopy.GetMaterialLeft(), infoCopy.GetMaterialRight(),
                                                       infoCopy.GetNormalLeft(), restitution, true);
        }
        remDt -= dt;
        nonCollideDt = std::min(originalDt, remDt);
        dt = nonCollideDt;
      } else {
        nonCollideDt *= 0.5f;
        dt *= 0.5f;
      }
    } else {
      actor.AddMotionState(newMState);
      remDt -= dt;
      dt = nonCollideDt;
      actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
    }

    newMState = actor.PredictMotion_Internal(dt);
  } while (remDt > 0.f);

  if (!didCollide && !actor.GetMaterialList().HasMaterial(EMaterialTypes::GroundCollider))
    mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::Falling);

  actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
}

zeus::CVector3f CGroundMovement::CollisionDamping(const zeus::CVector3f& vel, const zeus::CVector3f& dir,
                                                  const zeus::CVector3f& cNorm, float normCoeff, float deltaCoeff) {
  zeus::CVector3f dampedDir = (cNorm * -2.f * cNorm.dot(dir) + dir).normalized();
  zeus::CVector3f dampedNorm = cNorm * cNorm.dot(dampedDir);
  return (dampedDir - dampedNorm) * vel.magnitude() * deltaCoeff + normCoeff * vel.magnitude() * dampedNorm;
}

void CGroundMovement::MoveGroundCollider_New(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                             const rstl::reserved_vector<TUniqueId, 1024>* nearList) {
  zeus::CAABox motionVol = actor.GetMotionVolume(dt);
  rstl::reserved_vector<TUniqueId, 1024> useNearList;
  if (nearList)
    useNearList = *nearList;
  else
    mgr.BuildColliderList(useNearList, actor, motionVol);

  CAreaCollisionCache cache(motionVol);
  CGameCollision::BuildAreaCollisionCache(mgr, cache);
  CPlayer& player = static_cast<CPlayer&>(actor);
  player.x9c5_28_slidingOnWall = false;
  bool applyJump = player.x258_movementState == CPlayer::EPlayerMovementState::ApplyJump;
  bool dampUnderwater = false;
  if (player.x9c4_31_dampUnderwaterMotion)
    if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
      dampUnderwater = true;

  bool noJump = (player.x258_movementState != CPlayer::EPlayerMovementState::ApplyJump &&
                 player.x258_movementState != CPlayer::EPlayerMovementState::Jump);

  float stepDown = player.GetStepDownHeight();
  float stepUp = player.GetStepUpHeight();

  bool doStepDown = true;
  CMaterialList material(EMaterialTypes::NoStepLogic);
  SMoveObjectResult result;

  if (!applyJump) {
    SMovementOptions opts;
    opts.x0_setWaterLandingForce = false;
    opts.x4_waterLandingForceCoefficient = 0.f;
    opts.x8_minimumWaterLandingForce = 0.f;
    opts.xc_anyZThreshold = 0.37f;
    opts.x10_downwardZThreshold = 0.25f;
    opts.x14_waterLandingVelocityReduction = 0.f;
    opts.x18_dampForceAndMomentum = true;
    opts.x19_alwaysClip = false;
    opts.x1a_disableClipForFloorOnly = noJump;
    opts.x1c_maxCollisionCycles = 4;
    opts.x20_minimumTranslationDelta = 0.002f;
    opts.x24_dampedNormalCoefficient = 0.f;
    opts.x28_dampedDeltaCoefficient = 1.f;
    opts.x2c_floorElasticForce = 0.f;
    opts.x30_wallElasticConstant = 0.02f;
    opts.x3c_floorPlaneNormal = player.GetLastFloorPlaneNormal();
    opts.x34_wallElasticLinear = 0.2f;
    opts.x38_maxPositiveVerticalVelocity = player.GetMaximumPlayerPositiveVerticalVelocity(mgr);

    if (noJump) {
      zeus::CVector3f vel = player.GetVelocity();
      vel.z() = 0.f;
      actor.SetVelocityWR(vel);
      actor.x15c_force.z() = 0.f;
      actor.x150_momentum.z() = 0.f;
      actor.x168_impulse.z() = 0.f;
    }

    CPhysicsState physStatePre = actor.GetPhysicsState();
    CMaterialList material2 = MoveObjectAnalytical(mgr, actor, dt, useNearList, cache, opts, result);
    CPhysicsState physStatePost = actor.GetPhysicsState();

    /* NoStepLogic must be the only set material bit to bypass step logic */
    if (material2.XOR({EMaterialTypes::NoStepLogic})) {
      SMovementOptions optsCopy = opts;
      zeus::CVector3f postToPre = physStatePre.GetTranslation() - physStatePost.GetTranslation();
      float postToPreMag = postToPre.magSquared();
      optsCopy.x19_alwaysClip = noJump;
      float quarterStepUp = 0.25f * stepUp;
      rstl::reserved_vector<CPhysicsState, 2> physStateList;
      rstl::reserved_vector<float, 2> stepDeltaList;
      rstl::reserved_vector<CCollisionInfo, 2> collisionInfoList;
      rstl::reserved_vector<TUniqueId, 2> uniqueIdList;
      rstl::reserved_vector<CMaterialList, 2> materialListList;
      bool done = false;
      for (int i = 0; i < 2 && !done; ++i) {
        double useStepUp = (i == 0) ? quarterStepUp : stepUp;
        actor.SetPhysicsState(physStatePre);
        CCollisionInfo collisionInfo;
        TUniqueId id = kInvalidUniqueId;
        CGameCollision::DetectCollision_Cached_Moving(mgr, cache, *actor.GetCollisionPrimitive(), actor.GetTransform(),
                                                      actor.GetMaterialFilter(), useNearList, {0.f, 0.f, 1.f}, id,
                                                      collisionInfo, useStepUp);
        if (collisionInfo.IsValid()) {
          useStepUp = std::max(0.0, useStepUp - optsCopy.x20_minimumTranslationDelta);
          done = true;
        }

        if (useStepUp > 0.0005) {
          actor.SetTranslation(actor.GetTranslation() + zeus::CVector3f(0.f, 0.f, useStepUp));

          SMoveObjectResult result2;
          CMaterialList material3 = MoveObjectAnalytical(mgr, actor, dt, useNearList, cache, optsCopy, result2);
          CCollisionInfo collisionInfo2;
          double useStepDown2 = useStepUp + stepDown;
          TUniqueId id2 = kInvalidUniqueId;
          if (useStepDown2 > 0.0) {
            CGameCollision::DetectCollision_Cached_Moving(mgr, cache, *actor.GetCollisionPrimitive(),
                                                          actor.GetTransform(), actor.GetMaterialFilter(), useNearList,
                                                          {0.f, 0.f, -1.f}, id2, collisionInfo2, useStepDown2);
          } else {
            useStepDown2 = 0.0;
          }

          float minStep = std::min(useStepUp, useStepDown2);
          zeus::CVector3f offsetStep = actor.GetTranslation() - zeus::CVector3f(0.f, 0.f, minStep);
          bool floor = (collisionInfo2.IsValid() &&
                        CGameCollision::CanBlock(collisionInfo2.GetMaterialLeft(), collisionInfo2.GetNormalLeft()));
          zeus::CVector3f postToPre2 = physStatePre.GetTranslation() - offsetStep;
          float stepDelta = postToPre2.magSquared();
          if (floor && postToPreMag < stepDelta) {
            useStepDown2 = std::max(0.0, useStepDown2 - 0.0005);
            actor.SetTranslation(actor.GetTranslation() - zeus::CVector3f(0.f, 0.f, useStepDown2));
            physStateList.push_back(actor.GetPhysicsState());
            stepDeltaList.push_back(stepDelta);
            collisionInfoList.push_back(collisionInfo2);
            uniqueIdList.push_back(id2);
            materialListList.push_back(material3);
          }
        }
      }

      if (physStateList.size() == 0) {
        actor.SetPhysicsState(physStatePost);
        material = material2;
      } else {
        float maxFloat = -1.0e10f;
        int maxIdx = -1;
        for (int i = 0; i < physStateList.size(); ++i) {
          if (maxFloat < stepDeltaList[i]) {
            maxFloat = stepDeltaList[i];
            maxIdx = i;
          }
        }

        actor.SetPhysicsState(physStateList[maxIdx]);
        mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::OnFloor);
        if (CEntity* ent = mgr.ObjectById(uniqueIdList[maxIdx])) {
          result.x0_id.emplace(uniqueIdList[maxIdx]);
          result.x8_collision.emplace(collisionInfoList[maxIdx]);
          if (TCastToPtr<CScriptPlatform>(ent))
            mgr.SendScriptMsg(ent, actor.GetUniqueId(), EScriptObjectMessage::AddPlatformRider);
        }

        CCollisionInfo& cInfo = collisionInfoList[maxIdx];
        CGameCollision::SendMaterialMessage(mgr, cInfo.GetMaterialLeft(), actor);
        doStepDown = false;
        actor.SetLastFloorPlaneNormal({cInfo.GetNormalLeft()});
      }
    }
  } else {
    SMovementOptions opts;
    opts.x0_setWaterLandingForce = true;
    opts.x4_waterLandingForceCoefficient = dampUnderwater ? 35.f : 1.f;
    opts.x8_minimumWaterLandingForce = dampUnderwater ? 5.f : 0.f;
    opts.xc_anyZThreshold = dampUnderwater ? 0.05f : 0.37f;
    opts.x10_downwardZThreshold = dampUnderwater ? 0.01f : 0.25f;
    opts.x14_waterLandingVelocityReduction = dampUnderwater ? 0.2f : 0.f;
    opts.x18_dampForceAndMomentum = false;
    opts.x19_alwaysClip = false;
    opts.x1a_disableClipForFloorOnly = false;
    opts.x1c_maxCollisionCycles = 4;
    opts.x20_minimumTranslationDelta = 0.002f;
    opts.x24_dampedNormalCoefficient = 0.f;
    opts.x28_dampedDeltaCoefficient = 1.f;
    opts.x2c_floorElasticForce = 0.1f;
    opts.x30_wallElasticConstant = dampUnderwater ? 0.2f : 0.2f;
    opts.x3c_floorPlaneNormal = player.GetLastFloorPlaneNormal();
    opts.x38_maxPositiveVerticalVelocity = player.GetMaximumPlayerPositiveVerticalVelocity(mgr);

    material = MoveObjectAnalytical(mgr, actor, dt, useNearList, cache, opts, result);
  }

  if (doStepDown) {
    CCollisionInfo collisionInfo;
    double stepDown2 = actor.GetStepDownHeight();
    float zOffset = 0.f;
    TUniqueId id = kInvalidUniqueId;
    if (stepDown2 > FLT_EPSILON) {
      zeus::CTransform xf = actor.GetTransform();
      xf.origin += zeus::CVector3f(0.f, 0.f, 0.0005f);
      if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(), xf,
                                                         actor.GetMaterialFilter(), useNearList)) {
        actor.SetTranslation(xf.origin);
        zOffset = 0.0005f;
        stepDown2 += 0.0005;
      }

      CGameCollision::DetectCollision_Cached_Moving(mgr, cache, *actor.GetCollisionPrimitive(), actor.GetTransform(),
                                                    actor.GetMaterialFilter(), useNearList, {0.f, 0.f, -1.f}, id,
                                                    collisionInfo, stepDown2);
    }

    if (id != kInvalidUniqueId) {
      result.x0_id.emplace(id);
      result.x8_collision.emplace(collisionInfo);
    }

    if (!collisionInfo.IsValid() ||
        !CGameCollision::CanBlock(collisionInfo.GetMaterialLeft(), collisionInfo.GetNormalLeft())) {
      if (zOffset > 0.f) {
        zeus::CTransform xf = actor.GetTransform();
        xf.origin -= zeus::CVector3f(0.f, 0.f, zOffset);
      }

      if (collisionInfo.IsValid())
        player.x9c5_28_slidingOnWall = true;
      CheckFalling(actor, mgr, dt);
      player.SetLastFloorPlaneNormal({});
    } else {
      mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::OnFloor);
      stepDown2 = std::max(0.0, stepDown2 - 0.0005);
      actor.SetTranslation(actor.GetTranslation() - zeus::CVector3f(0.f, 0.f, stepDown2));
      if (TCastToPtr<CScriptPlatform> plat = mgr.ObjectById(id))
        mgr.SendScriptMsg(plat.GetPtr(), actor.GetUniqueId(), EScriptObjectMessage::AddPlatformRider);
      CGameCollision::SendMaterialMessage(mgr, collisionInfo.GetMaterialLeft(), actor);
      actor.SetLastFloorPlaneNormal({collisionInfo.GetNormalLeft()});
    }
  }

  actor.ClearForcesAndTorques();
  if (material.HasMaterial(EMaterialTypes::Wall))
    player.SetPlayerHitWallDuringMove();

  if (result.x0_id) {
    CCollisionInfoList list;
    list.Add(*result.x8_collision, false);
    CGameCollision::MakeCollisionCallbacks(mgr, actor, *result.x0_id, list);
  }

  CMotionState mState = actor.GetMotionState();
  mState.x0_translation = actor.GetLastNonCollidingState().x0_translation;
  mState.x1c_velocity = actor.GetLastNonCollidingState().x1c_velocity;
  actor.SetLastNonCollidingState(mState);

  const CCollisionPrimitive* usePrim = actor.GetCollisionPrimitive();
  std::unique_ptr<CCollisionPrimitive> prim;
  if (usePrim->GetPrimType() == FOURCC('AABX')) {
    const CCollidableAABox& existingAABB = static_cast<const CCollidableAABox&>(*usePrim);
    prim.reset(
        new CCollidableAABox(zeus::CAABox(existingAABB.GetBox().min + 0.0001f, existingAABB.GetBox().max - 0.0001f),
                             usePrim->GetMaterial()));
    usePrim = prim.get();
  } else if (usePrim->GetPrimType() == FOURCC('SPHR')) {
    const CCollidableSphere& existingSphere = static_cast<const CCollidableSphere&>(*usePrim);
    prim.reset(new CCollidableSphere(
        zeus::CSphere(existingSphere.GetSphere().position, existingSphere.GetSphere().radius - 0.0001f),
        usePrim->GetMaterial()));
    usePrim = prim.get();
  }

  CGameCollision::CollisionFailsafe(mgr, cache, actor, *usePrim, useNearList, 0.f, 1);
}

bool CGroundMovement::RemoveNormalComponent(const zeus::CVector3f& a, const zeus::CVector3f& b, zeus::CVector3f& c,
                                            float& d) {
  float dot = a.dot(c);
  if (std::fabs(dot) > 0.99f)
    return false;
  float dot2 = b.dot(c);
  float dot3 = b.dot((c - a * dot).normalized());
  if (dot2 > 0.f && dot3 < 0.f)
    return false;
  if (std::fabs(dot2) > 0.01f && std::fabs(dot3 / dot2) > 4.f)
    return false;
  c -= dot * a;
  d = dot;
  return true;
}

bool CGroundMovement::RemoveNormalComponent(const zeus::CVector3f& a, zeus::CVector3f& b) {
  float dot = a.dot(b);
  if (std::fabs(dot) > 0.99f)
    return false;
  b -= a * dot;
  return true;
}

static bool RemovePositiveZComponentFromNormal(zeus::CVector3f& vec) {
  if (vec.z() > 0.f && vec.z() < 0.99f) {
    vec.z() = 0.f;
    vec.normalize();
    return true;
  }
  return false;
}

CMaterialList CGroundMovement::MoveObjectAnalytical(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                                    rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                                    CAreaCollisionCache& cache, const SMovementOptions& opts,
                                                    SMoveObjectResult& result) {
  result.x6c_processedCollisions = 0;
  CMaterialList ret;
  zeus::CVector3f floorPlaneNormal = opts.x3c_floorPlaneNormal ? *opts.x3c_floorPlaneNormal : zeus::CVector3f::skZero;
  bool floorCollision = opts.x3c_floorPlaneNormal.operator bool();
  float remDt = dt;
  for (int i = 0; remDt > 0.f; ++i) {
    float collideDt = remDt;

    CMotionState mState = actor.PredictMotion_Internal(remDt);
    double mag = mState.x0_translation.magnitude();
    zeus::CVector3f normTrans = (1.f / ((float(mag) > FLT_EPSILON) ? float(mag) : 1.f)) * mState.x0_translation;
    TUniqueId id = kInvalidUniqueId;
    CCollisionInfo collisionInfo;

    if (mag > opts.x20_minimumTranslationDelta) {
      double oldMag = mag;
      CGameCollision::DetectCollision_Cached_Moving(mgr, cache, *actor.GetCollisionPrimitive(),
                                                    actor.GetPrimitiveTransform(), actor.GetMaterialFilter(), nearList,
                                                    normTrans, id, collisionInfo, mag);
      if (id != kInvalidUniqueId) {
        result.x0_id.emplace(id);
        result.x8_collision.emplace(collisionInfo);
      }
      collideDt = mag / oldMag * remDt;
    }

    mag = std::max(0.f, float(mag) - opts.x20_minimumTranslationDelta);

    zeus::CVector3f collisionNorm = collisionInfo.GetNormalLeft();
    bool floor = CGameCollision::CanBlock(collisionInfo.GetMaterialLeft(), collisionNorm);
    bool clipCollision = true;
    if (!opts.x19_alwaysClip) {
      if (!opts.x1a_disableClipForFloorOnly || floor)
        clipCollision = false;
    }

    float collisionFloorDot = 0.f;

    if (collisionInfo.IsValid()) {
      result.x6c_processedCollisions += 1;
      if (floor) {
        ret.Add(EMaterialTypes::Floor);
        floorPlaneNormal = collisionInfo.GetNormalLeft();
        floorCollision = true;
      } else {
        ret.Add(EMaterialTypes::Wall);
      }

      if (clipCollision) {
        if (floorCollision) {
          if (!CGroundMovement::RemoveNormalComponent(floorPlaneNormal, normTrans, collisionNorm, collisionFloorDot))
            RemovePositiveZComponentFromNormal(collisionNorm);
          else
            collisionNorm.normalize();
        } else {
          RemovePositiveZComponentFromNormal(collisionNorm);
        }
      }

      mState = actor.PredictMotion_Internal(collideDt);
    }

    mState.x0_translation = normTrans * mag;
    actor.AddMotionState(mState);

    if (collisionInfo.IsValid()) {
      zeus::CVector3f vel =
          actor.GetVelocity().canBeNormalized()
              ? CGroundMovement::CollisionDamping(actor.GetVelocity(), actor.GetVelocity().normalized(), collisionNorm,
                                                  opts.x24_dampedNormalCoefficient, opts.x28_dampedDeltaCoefficient)
              : zeus::CVector3f::skZero;
      float elasticForce = floor ? opts.x2c_floorElasticForce
                                 : opts.x34_wallElasticLinear * collisionFloorDot + opts.x30_wallElasticConstant;
      float dot = collisionNorm.dot(vel);
      if (dot < elasticForce)
        vel += (elasticForce - dot) * collisionNorm;
      if (clipCollision && floorCollision)
        if (!CGroundMovement::RemoveNormalComponent(floorPlaneNormal, vel))
          vel.z() = 0.f;
      if (vel.z() > opts.x38_maxPositiveVerticalVelocity)
        vel *= zeus::CVector3f(opts.x38_maxPositiveVerticalVelocity / vel.z());

      if (opts.x18_dampForceAndMomentum) {
        if (actor.x15c_force.canBeNormalized()) {
          // zeus::CVector3f prevForce = actor.x15c_force;
          actor.x15c_force = CGroundMovement::CollisionDamping(actor.x15c_force, actor.x15c_force.normalized(),
                                                               collisionNorm, 0.f, 1.f);
        }
        if (actor.x150_momentum.canBeNormalized()) {
          actor.x150_momentum = CGroundMovement::CollisionDamping(actor.x150_momentum, actor.x150_momentum.normalized(),
                                                                  collisionNorm, 0.f, 1.f);
        }
      }

      if (opts.x0_setWaterLandingForce && !floor) {
        if (collisionInfo.GetNormalLeft().z() < -0.1f && vel.z() > 0.f)
          vel.z() *= 0.5f;

        float zNormAbs = std::fabs(collisionInfo.GetNormalLeft().z());
        if ((zNormAbs > opts.x10_downwardZThreshold && vel.z() < 0.f) || zNormAbs > opts.xc_anyZThreshold) {
          actor.x15c_force = zeus::CVector3f(
              0.f, 0.f,
              -(1.f + std::max(opts.x4_waterLandingForceCoefficient * zNormAbs, opts.x8_minimumWaterLandingForce)) *
                  actor.GetWeight());
          vel *= zeus::CVector3f(1.f - opts.x14_waterLandingVelocityReduction);
        }
      }

      actor.SetVelocityWR(vel);
    } else {
      zeus::CVector3f vel = actor.x138_velocity;
      if (actor.x138_velocity.z() > opts.x38_maxPositiveVerticalVelocity)
        vel *= zeus::CVector3f(opts.x38_maxPositiveVerticalVelocity / vel.z());

      actor.SetVelocityWR(vel);
    }

    actor.ClearImpulses();

    remDt -= collideDt;
    if (i >= opts.x1c_maxCollisionCycles)
      break;
  }

  result.x70_processedDt = dt - remDt;
  return ret;
}

} // namespace urde
