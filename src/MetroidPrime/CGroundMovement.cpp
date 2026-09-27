#include "MetroidPrime/CGroundMovement.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "MetroidPrime/CAABoxFilter.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/UserNames.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "rstl/math.hpp"

#include <float.h>
#include <math.h>

void CGroundMovement::CheckFalling(CPhysicsActor& actor, CStateManager& mgr, float dt) {
  bool outOfBounds = true;
  const CAABox bounds = *actor.GetTouchBounds();
  for (AUTO(it, mgr.GetWorld()->GetChainHead(CWorld::kC_Alive)); it != CWorld::GetAliveAreasEnd();
       ++it) {
    if (it->GetAABB().DoBoundsOverlap(*actor.GetTouchBounds())) {
      outOfBounds = false;
      break;
    }
  }
  if (!outOfBounds) {
    mgr.DeliverScriptMsg(&actor, kInvalidUniqueId, kSM_Falling);
  } else {
    mgr.DeliverScriptMsg(&actor, kInvalidUniqueId, kSM_OnFloor);
    actor.SetAngularVelocityWR(actor.GetAngularVelocityWR() * 0.98f);
    CVector3f velocity = actor.GetTransform().TransposeRotate(actor.GetVelocityWR());
    velocity.SetZ(0.f);
    actor.SetVelocityOR(velocity);
    actor.SetMomentumWR(CVector3f::Zero());
  }
}

void CGroundMovement::MoveGroundCollider(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                         const TEntityList* colliderList) {
  CMotionState oldState = actor.GetMotionState();
  if (IsUser(0)) {
    actor.SetMotionState(oldState);
  }
  CMotionState newState = actor.PredictMotion_Internal(dt);
  float deltaMag = newState.GetTranslation().Magnitude();
  CCollisionInfoList collisionList;
  TUniqueId idDetect = kInvalidUniqueId;
  TEntityList nearList;
  const CMaterialFilter& filter = actor.GetMaterialFilter();
  CAABox motionVolume = actor.GetMotionVolume(dt);
  if (colliderList != nullptr) {
    nearList = *colliderList;
  } else {
    mgr.BuildColliderList(nearList, actor, motionVolume);
  }
  CAreaCollisionCache cache(motionVolume);
  float collideDt = dt;
  if (actor.GetCollisionPrimitive()->GetPrimType() != 'OBTG') {
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (deltaMag >
        0.5f * CGameCollision::GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive())) {
      CAABox bounds = actor.GetCollisionPrimitive()->CalculateAABox(actor.GetPrimitiveTransform());
      CVector3f point = bounds.GetCenterPoint();
      CVector3f direction = newState.GetTranslation() / deltaMag;
      TUniqueId intersectId = kInvalidUniqueId;
      const CMaterialFilter& rayFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
      CRayCastResult result =
          mgr.RayWorldIntersection(intersectId, point, direction, deltaMag, rayFilter, nearList);
      if (result.IsValid()) {
        collideDt = dt * (result.GetTime() / deltaMag);
        newState = actor.PredictMotion_Internal(collideDt);
      }
    }
  }
  actor.MoveCollisionPrimitive(newState.GetTranslation());
  if (CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                             actor.GetPrimitiveTransform(), filter, nearList,
                                             idDetect, collisionList)) {
    float stepUp = actor.GetStepUpHeight();
    float resolved = 0.f;
    actor.AddMotionState(newState);
    if (ResolveUpDown(cache, mgr, actor, filter, nearList, stepUp, 0.f, resolved, collisionList)) {
      actor.SetMotionState(oldState);
      MoveGroundColliderXY(cache, mgr, actor, filter, nearList, collideDt);
    }
  } else {
    actor.AddMotionState(newState);
  }
  float resolved = 0.f;
  float stepDown = actor.GetStepDownHeight();
  collisionList.Clear();
  TUniqueId stepZId = kInvalidUniqueId;
  if (stepDown >= 0.f && MoveGroundColliderZ(cache, mgr, actor, filter, nearList, -stepDown,
                                             resolved, collisionList, stepZId)) {
    if (collisionList.GetCount() > 0) {
      CCollisionInfoList filteredList;
      CollisionUtil::FilterByClosestNormal(CVector3f(0.f, 0.f, 1.f), collisionList, filteredList);
      if (filteredList.GetCount() > 0) {
        CCollisionInfo info = filteredList[0];
        if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft())) {
          CEntity* entity = mgr.ObjectById(stepZId);
          if (TCastToPtr< CScriptPlatform >(entity)) {
            mgr.DeliverScriptMsg(entity, actor.GetUniqueId(), kSM_AddPlatformRider);
          }
          CGameCollision::SendMaterialMessage(mgr, info.GetMaterialLeft(), actor);
          mgr.DeliverScriptMsg(&actor, kInvalidUniqueId, kSM_OnFloor);
        } else {
          CheckFalling(actor, mgr, dt);
        }
      }
    }
  } else {
    CheckFalling(actor, mgr, dt);
  }
  actor.ClearForcesAndTorques();
  actor.MoveCollisionPrimitive(CVector3f::Zero());
  if (actor.GetMaterialList().HasMaterial(kMT_Player)) {
    CGameCollision::CollisionFailsafe(mgr, cache, actor, *actor.GetCollisionPrimitive(), nearList,
                                      0.f, 1);
  }
}

bool CGroundMovement::ResolveUpDown(CAreaCollisionCache& cache, CStateManager& mgr,
                                    CPhysicsActor& actor, const CMaterialFilter& filter,
                                    const TEntityList& nearList, float stepUp, float stepDown,
                                    float& resolved, CCollisionInfoList& list) {
  float zExtent = stepDown;
  if (list.GetCount() > 0) {
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    CVector3f normal(0.f, 0.f, 0.f);
    for (int i = 0; i < list.GetCount(); ++i) {
      const CCollisionInfo& info = list[i];
      if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft())) {
        bounds.AccumulateBounds(info.GetPoint());
        bounds.AccumulateBounds(info.GetExtreme());
        normal += info.GetNormalLeft();
      }
    }
    if (normal.CanBeNormalized()) {
      normal = normal.AsNormalized();
    } else {
      return true;
    }
    const CAABox& actorBounds = actor.GetBoundingBox();
    if (normal.GetZ() >= 0.f) {
      zExtent = bounds.GetMaxPoint().GetZ() - actorBounds.GetMinPoint().GetZ() + 0.02f;
      if (zExtent > stepUp) {
        return true;
      }
    } else {
      zExtent = bounds.GetMinPoint().GetZ() - actorBounds.GetMaxPoint().GetZ() - 0.02f;
      if (zExtent < -stepDown) {
        return true;
      }
    }
    actor.MoveCollisionPrimitive(CVector3f(0.f, 0.f, zExtent));
    if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                       actor.GetPrimitiveTransform(), filter,
                                                       nearList)) {
      resolved = zExtent;
      actor.SetTranslation(actor.GetTranslation() + CVector3f(0.f, 0.f, zExtent));
      actor.MoveCollisionPrimitive(CVector3f::Zero());
      bool floor = false;
      for (int i = 0; i < list.GetCount(); ++i) {
        if (CGameCollision::IsFloor(list[i].GetMaterialLeft(), list[i].GetNormalLeft())) {
          floor = true;
          break;
        }
      }
      if (!floor) {
        mgr.DeliverScriptMsg(&actor, kInvalidUniqueId, kSM_LandOnNotFloor);
      }
      return false;
    }
  }
  return true;
}

bool CGroundMovement::MoveGroundColliderZ(CAreaCollisionCache& cache, CStateManager& mgr,
                                          CPhysicsActor& actor, const CMaterialFilter& filter,
                                          const TEntityList& nearList, float amount,
                                          float& resolved, CCollisionInfoList& list,
                                          TUniqueId& idOut) {
  actor.MoveCollisionPrimitive(CVector3f(0.f, 0.f, amount));
  idOut = kInvalidUniqueId;
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  bool collided = CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                         actor.GetPrimitiveTransform(), filter,
                                                         nearList, idOut, list);
  if (collided) {
    for (int i = 0; i < list.GetCount(); ++i) {
      bounds.AccumulateBounds(list[i].GetPoint());
      bounds.AccumulateBounds(list[i].GetExtreme());
    }
    const CAABox& actorBounds = actor.GetBoundingBox();
    float zExtent = 0.f;
    if (amount > 0.f) {
      zExtent = bounds.GetMinPoint().GetZ() - actorBounds.GetMaxPoint().GetZ() - 0.02f + amount;
    } else {
      zExtent = bounds.GetMaxPoint().GetZ() - actorBounds.GetMinPoint().GetZ() + 0.02f + amount;
    }
    actor.MoveCollisionPrimitive(CVector3f(0.f, 0.f, zExtent));
    if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                       actor.GetPrimitiveTransform(), filter,
                                                       nearList)) {
      resolved = zExtent;
      actor.SetTranslation(actor.GetTranslation() + CVector3f(0.f, 0.f, zExtent));
      actor.MoveCollisionPrimitive(CVector3f::Zero());
    }
    bool floor = false;
    for (int i = 0; i < list.GetCount(); ++i) {
      if (CGameCollision::IsFloor(list[i].GetMaterialLeft(), list[i].GetNormalLeft())) {
        floor = true;
        break;
      }
    }
    if (!floor) {
      mgr.DeliverScriptMsg(&actor, kInvalidUniqueId, kSM_LandOnNotFloor);
    }
    CCollisionInfoList filteredList;
    if (amount > 0.0) {
      CollisionUtil::FilterByClosestNormal(CVector3f(0.f, 0.f, -1.f), list, filteredList);
    } else {
      CollisionUtil::FilterByClosestNormal(CVector3f(0.f, 0.f, 1.f), list, filteredList);
    }
    if (filteredList.GetCount() > 0) {
      CGameCollision::MakeCollisionCallbacks(mgr, actor, idOut, filteredList);
    }
  }
  return collided;
}

bool CGroundMovement::MoveGroundColliderXY(CAreaCollisionCache& cache, CStateManager& mgr,
                                           CPhysicsActor& actor, const CMaterialFilter& filter,
                                           const TEntityList& nearList, float dt) {
  bool didCollide = false;
  bool isPlayer = actor.GetMaterialList().HasMaterial(kMT_Player);
  CMotionState oldState = actor.GetMotionState();
  if (IsUser(0)) {
    actor.MoveCollisionPrimitive(CVector3f::Zero());
    actor.SetMotionState(oldState);
  }
  static int numCollisions = 0;
  static int totalIterations = 0;
  static int peakIterationCount = 1;
  int iterationCount = 0;
  float remainingDt = dt;
  float originalDt = dt;
  CPhysicsActor* otherActor = nullptr;
  CCollisionInfoList collisionList;
  CMotionState motion = actor.PredictMotion_Internal(dt);
  float translationMag = motion.GetTranslation().Magnitude();
  float minimumTranslation = isPlayer ? rstl::max_val(translationMag / 5.f, 0.005f)
                                      : rstl::max_val(translationMag / 3.f, 0.02f);
  float minExtent =
      0.5f * CGameCollision::GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive());
  if (translationMag > minExtent) {
    originalDt = minExtent * (dt / translationMag);
    dt = originalDt;
    motion = actor.PredictMotion_Internal(dt);
    minimumTranslation = rstl::min_val(minExtent, minimumTranslation);
  }
  float nonCollideDt = dt;
  bool loopContinue = true;
  while (loopContinue) {
    actor.MoveCollisionPrimitive(motion.GetTranslation());
    collisionList.Clear();
    TUniqueId otherId = kInvalidUniqueId;
    ++iterationCount;
    bool collided = CGameCollision::DetectCollision_Cached(
        mgr, cache, *actor.GetCollisionPrimitive(), actor.GetPrimitiveTransform(), filter, nearList,
        otherId, collisionList);
    if (collided) {
      otherActor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(otherId));
    }
    actor.MoveCollisionPrimitive(CVector3f::Zero());
    if (collided) {
      didCollide = true;
      if (motion.GetTranslation().Magnitude() < minimumTranslation) {
        CCollisionInfoList backfaceFilteredList;
        CCollisionInfoList floorFilteredList;
        CVector3f deltaVelocity = actor.GetVelocityWR();
        if (otherActor != nullptr) {
          deltaVelocity -= otherActor->GetVelocityWR();
        }
        CollisionUtil::FilterOutBackfaces(deltaVelocity, collisionList, backfaceFilteredList);
        CAABoxFilter::FilterBoxFloorCollisions(backfaceFilteredList, floorFilteredList);
        CGameCollision::MakeCollisionCallbacks(mgr, actor, otherId, floorFilteredList);
        if (floorFilteredList.GetCount() == 0 && isPlayer) {
          const CMotionState& lastState = actor.GetLastNonCollidingState();
          actor.SetMotionState(CMotionState(lastState.GetTranslation(), lastState.GetOrientation(),
                                            0.5f * lastState.GetVelocity(),
                                            lastState.GetAngularMomentum() * 0.5f));
        }
        if (IsUser(0)) {
          if (floorFilteredList.GetCount() > 0) {
            CGameCollision::ShowCollisionResults(floorFilteredList, CColor::White());
          } else {
            CGameCollision::ShowCollisionResults(collisionList, CColor::Yellow());
          }
        }
        for (int i = 0; i < floorFilteredList.GetCount(); ++i) {
          CCollisionInfo info = floorFilteredList[i];
          float restitution = CGameCollision::GetCoefficientOfRestitution(info) +
                              actor.GetCoefficientOfRestitutionModifier();
          CVector3f normal = info.GetNormalLeft();
          if (otherActor != nullptr) {
            CGameCollision::CollideWithDynamicBodyNoRot(actor, *otherActor, info, restitution,
                                                        true);
          } else {
            CGameCollision::CollideWithStaticBodyNoRot(actor, info.GetMaterialLeft(),
                                                       info.GetMaterialRight(),
                                                       CUnitVector3f(normal), restitution, true);
          }
        }
        remainingDt -= dt;
        nonCollideDt = rstl::min_val(remainingDt, originalDt);
        dt = nonCollideDt;
      } else {
        nonCollideDt *= 0.5f;
        dt *= 0.5f;
      }
    } else {
      actor.AddMotionState(motion);
      remainingDt -= dt;
      dt = nonCollideDt;
      actor.MoveCollisionPrimitive(CVector3f::Zero());
    }
    motion = actor.PredictMotion_Internal(dt);
    loopContinue = remainingDt > 0.f;
  }
  if (!didCollide && !actor.GetMaterialList().HasMaterial(kMT_GroundCollider)) {
    mgr.DeliverScriptMsg(&actor, kInvalidUniqueId, kSM_Falling);
  }
  actor.MoveCollisionPrimitive(CVector3f::Zero());
  if (IsUser(0)) {
    if (didCollide) {
      totalIterations += iterationCount;
      ++numCollisions;
    }
    if (iterationCount > peakIterationCount) {
      peakIterationCount = iterationCount;
    }
  }
  return didCollide;
}

CVector3f CGroundMovement::CollisionDamping(const CVector3f& velocity, const CVector3f& direction,
                                            const CVector3f& normal, float normalCoefficient,
                                            float deltaCoefficient) {
  const CVector3f dampedDirection =
      (direction + (-2.f * normal) * CVector3f::Dot(normal, direction)).AsNormalized();
  const CVector3f dampedNormal = CVector3f::Dot(normal, dampedDirection) * normal;
  return normalCoefficient * (velocity.Magnitude() * dampedNormal) +
         deltaCoefficient * (velocity.Magnitude() * (dampedDirection - dampedNormal));
}

void CGroundMovement::MoveGroundCollider_New(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                             const TEntityList* colliderList) {
  TEntityList nearList;
  CAABox motionVolume = actor.GetMotionVolume(dt);
  if (colliderList != nullptr) {
    nearList = *colliderList;
  } else {
    mgr.BuildColliderList(nearList, actor, motionVolume);
  }
  CAreaCollisionCache cache(motionVolume);
  CGameCollision::BuildAreaCollisionCache(mgr, cache);
  CPlayer& player = *TCastToPtr< CPlayer >(&actor);
  player.SetPlayerIsSlidingOnWall(false);
  bool applyJump = player.GetPlayerMovementState() == NPlayer::kMS_ApplyJump;
  bool dampUnderwater = false;
  if (player.IsInsideFluid() && !mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
    dampUnderwater = true;
  }
  bool noJump = player.GetPlayerMovementState() != NPlayer::kMS_ApplyJump &&
                player.GetPlayerMovementState() != NPlayer::kMS_Jump;
  float stepDown = actor.GetStepDownHeight();
  float stepUp = actor.GetStepUpHeight();
  bool doStepDown = true;
  CMaterialList materials(kMT_NoStepLogic);
  SMoveObjectResult result;
  if (!applyJump) {
    SMovementOptions options;
    options.mSetWaterLandingForce = false;
    options.mWaterLandingForceCoefficient = 0.f;
    options.mMinimumWaterLandingForce = 0.f;
    options.mAnyZThreshold = 0.37f;
    options.mDownwardZThreshold = 0.25f;
    options.mWaterLandingVelocityReduction = 0.f;
    options.mDampForceAndMomentum = true;
    options.mAlwaysClip = false;
    options.mDisableClipForFloorOnly = noJump;
    options.mMaxCollisionCycles = 4;
    options.mMinimumTranslationDelta = 0.002f;
    options.mDampedNormalCoefficient = 0.f;
    options.mDampedDeltaCoefficient = 1.f;
    options.mFloorElasticForce = 0.f;
    options.mWallElasticConstant = 0.02f;
    options.mFloorPlaneNormal = actor.GetLastFloorPlaneNormal();
    options.mWallElasticLinear = 0.2f;
    options.mMaxPositiveVerticalVelocity = player.GetMaximumPlayerPositiveVerticalVelocity(mgr);
    if (noJump) {
      CVector3f velocity = actor.GetVelocityWR();
      velocity.SetZ(0.f);
      actor.SetVelocityWR(velocity);
      actor.ForceWR().SetZ(0.f);
      actor.MomentumWR().SetZ(0.f);
      actor.ImpulseWR().SetZ(0.f);
    }
    CPhysicsState before = actor.GetPhysicsState();
    CMaterialList moveMaterials =
        MoveObjectAnalytical(mgr, actor, dt, nearList, cache, options, result);
    CPhysicsState after = actor.GetPhysicsState();
    if (moveMaterials.GetValue() != 1ull) {
      SMovementOptions stepOptions = options;
      stepOptions.mAlwaysClip = noJump;
      stepOptions.mWallElasticConstant = 0.03f;
      const CVector3f& movement = before.GetTranslation() - after.GetTranslation();
      float movementMag = movement.MagSquared();
      float quarterStepUp = 0.25f * stepUp;
      rstl::reserved_vector< CPhysicsState, 2 > states;
      rstl::reserved_vector< float, 2 > stepDeltas;
      rstl::reserved_vector< CCollisionInfo, 2 > collisions;
      rstl::reserved_vector< TUniqueId, 2 > ids;
      rstl::reserved_vector< CMaterialList, 2 > stepMaterials;
      bool done = false;
      for (int i = 0; i < 2 && !done; ++i) {
        float stepAmount = i == 0 ? quarterStepUp : stepUp;
        actor.SetPhysicsState(before);
        CCollisionInfo upCollision;
        TUniqueId upId = kInvalidUniqueId;
        double useStepUp = stepAmount;
        CGameCollision::DetectCollision_Cached_Moving(
            mgr, cache, *actor.GetCollisionPrimitive(), actor.GetTransform(),
            actor.GetMaterialFilter(), nearList, CVector3f(0.f, 0.f, 1.f), upId, upCollision,
            useStepUp);
        if (upCollision.IsValid()) {
          useStepUp = rstl::max_val(0.0, useStepUp - stepOptions.mMinimumTranslationDelta);
          done = true;
        }
        if (useStepUp > 0.0005f) {
          actor.SetTranslation(actor.GetTranslation() +
                               CVector3f(0.f, 0.f, static_cast< float >(useStepUp)));
          SMoveObjectResult stepResult;
          CMaterialList stepMaterial =
              MoveObjectAnalytical(mgr, actor, dt, nearList, cache, stepOptions, stepResult);
          CCollisionInfo downCollision;
          double useStepDown = useStepUp + stepDown;
          TUniqueId downId = kInvalidUniqueId;
          if (useStepDown > 0.0) {
            CGameCollision::DetectCollision_Cached_Moving(
                mgr, cache, *actor.GetCollisionPrimitive(), actor.GetTransform(),
                actor.GetMaterialFilter(), nearList, CVector3f(0.f, 0.f, -1.f), downId,
                downCollision, useStepDown);
          } else {
            useStepDown = 0.0;
          }
          float minStep = rstl::min_val(useStepDown, useStepUp);
          CVector3f endPosition = actor.GetTranslation() - minStep * CVector3f(0.f, 0.f, 1.f);
          bool floor =
              downCollision.IsValid() && CGameCollision::CanBlock(downCollision.GetMaterialLeft(),
                                                                  downCollision.GetNormalLeft());
          const CVector3f& stepMovement = before.GetTranslation() - endPosition;
          float stepDelta = stepMovement.MagSquared();
          if (floor && movementMag < stepDelta) {
            useStepDown = rstl::max_val(useStepDown - 0.0005f, 0.0);
            const CVector3f& up = CVector3f(0.f, 0.f, 1.f);
            actor.SetTranslation(actor.GetTranslation() - static_cast< float >(useStepDown) * up);
            states.push_back(actor.GetPhysicsState());
            stepDeltas.push_back(stepDelta);
            collisions.push_back(upCollision);
            ids.push_back(downId);
            stepMaterials.push_back(stepMaterial);
          }
        }
      }
      if (states.empty()) {
        actor.SetPhysicsState(after);
        materials = moveMaterials;
      } else {
        float maxDelta = -1.e10f;
        int maxIndex = -1;
        for (int i = 0; i < states.size(); ++i) {
          if (maxDelta < stepDeltas[i]) {
            maxDelta = stepDeltas[i];
            maxIndex = i;
          }
        }
        actor.SetPhysicsState(states[maxIndex]);
        mgr.DeliverScriptMsg(&actor, kInvalidUniqueId, kSM_OnFloor);
        materials = stepMaterials[maxIndex];
        CEntity* entity = mgr.ObjectById(ids[maxIndex]);
        if (entity != nullptr) {
          result.mId = ids[maxIndex];
          result.mCollision = collisions[maxIndex];
          if (TCastToPtr< CScriptPlatform >(entity)) {
            mgr.DeliverScriptMsg(entity, actor.GetUniqueId(), kSM_AddPlatformRider);
          }
        }
        CCollisionInfo& info = collisions[maxIndex];
        CGameCollision::SendMaterialMessage(mgr, info.GetMaterialLeft(), actor);
        doStepDown = false;
        actor.SetLastFloorPlaneNormal(info.GetNormalLeft());
      }
    }
  } else {
    SMovementOptions options;
    options.mSetWaterLandingForce = true;
    options.mWaterLandingForceCoefficient = 1.f;
    options.mMinimumWaterLandingForce = 0.f;
    options.mAnyZThreshold = 0.37f;
    options.mDownwardZThreshold = 0.25f;
    options.mWaterLandingVelocityReduction = 0.f;
    if (dampUnderwater) {
      options.mWaterLandingForceCoefficient = 35.f;
      options.mMinimumWaterLandingForce = 5.f;
      options.mAnyZThreshold = 0.05f;
      options.mDownwardZThreshold = 0.01f;
      options.mWaterLandingVelocityReduction = 0.2f;
    }
    options.mDampForceAndMomentum = false;
    options.mAlwaysClip = false;
    options.mDisableClipForFloorOnly = false;
    options.mMaxCollisionCycles = 4;
    options.mMinimumTranslationDelta = 0.002f;
    options.mDampedNormalCoefficient = 0.f;
    options.mDampedDeltaCoefficient = 1.f;
    options.mFloorElasticForce = 0.1f;
    options.mWallElasticConstant = 0.2f;
    options.mMaxPositiveVerticalVelocity = player.GetMaximumPlayerPositiveVerticalVelocity(mgr);
    materials = MoveObjectAnalytical(mgr, actor, dt, nearList, cache, options, result);
  }
  if (doStepDown) {
    CCollisionInfo info;
    double useStepDown = actor.GetStepDownHeight();
    float zOffset = 0.f;
    TUniqueId id = kInvalidUniqueId;
    if (useStepDown > static_cast< double >(FLT_EPSILON)) {
      CTransform4f transform = actor.GetTransform();
      transform.SetTranslation(transform.GetTranslation() + CVector3f(0.f, 0.f, 0.0005f));
      if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                         transform, actor.GetMaterialFilter(),
                                                         nearList)) {
        actor.SetTranslation(transform.GetTranslation());
        zOffset = 0.0005f;
        useStepDown += 0.0005f;
      }
      CGameCollision::DetectCollision_Cached_Moving(
          mgr, cache, *actor.GetCollisionPrimitive(), actor.GetTransform(),
          actor.GetMaterialFilter(), nearList, CVector3f(0.f, 0.f, -1.f), id, info, useStepDown);
    }
    if (id != kInvalidUniqueId) {
      result.mId = id;
      result.mCollision = info;
    }
    if (!info.IsValid() ||
        !CGameCollision::CanBlock(info.GetMaterialLeft(), info.GetNormalLeft())) {
      if (zOffset > 0.f) {
        CTransform4f transform = actor.GetTransform();
        transform.SetTranslation(transform.GetTranslation() - CVector3f(0.f, 0.f, zOffset));
      }
      if (info.IsValid()) {
        player.SetPlayerIsSlidingOnWall(true);
      }
      CheckFalling(actor, mgr, dt);
      actor.SetLastFloorPlaneNormal(rstl::optional_object_null());
    } else {
      mgr.DeliverScriptMsg(&actor, kInvalidUniqueId, kSM_OnFloor);
      useStepDown = rstl::max_val(useStepDown - 0.0005f, 0.0);
      const CVector3f& up = CVector3f(0.f, 0.f, 1.f);
      actor.SetTranslation(actor.GetTranslation() - static_cast< float >(useStepDown) * up);
      CEntity* entity = mgr.ObjectById(id);
      if (TCastToPtr< CScriptPlatform >(entity)) {
        mgr.DeliverScriptMsg(entity, actor.GetUniqueId(), kSM_AddPlatformRider);
      }
      CGameCollision::SendMaterialMessage(mgr, info.GetMaterialLeft(), actor);
      actor.SetLastFloorPlaneNormal(info.GetNormalLeft());
    }
  }
  actor.ClearForcesAndTorques();
  if (materials.HasMaterial(kMT_Wall)) {
    player.SetPlayerHitWallDuringMove();
  }
  if (result.mId) {
    CCollisionInfoList collisionList;
    collisionList.Add(*result.mCollision, false);
    CGameCollision::MakeCollisionCallbacks(mgr, actor, *result.mId, collisionList);
  }
  CMotionState motion = actor.GetMotionState();
  motion.SetTranslation(actor.GetLastNonCollidingState().GetTranslation());
  motion.SetVelocity(actor.GetLastNonCollidingState().GetVelocity());
  actor.SetLastNonCollidingState(motion);
  uchar primitiveStorage[64];
  const CCollisionPrimitive* primitive = actor.GetCollisionPrimitive();
  const CCollisionPrimitive* usePrimitive = primitive;
  void* storage = primitiveStorage;
  if (primitive->GetPrimType() == 'AABX') {
    const CCollidableAABox& box = static_cast< const CCollidableAABox& >(*primitive);
    usePrimitive = new (storage)
        CCollidableAABox(CAABox(box.GetBox().GetMinPoint() + CVector3f(0.0001f, 0.0001f, 0.0001f),
                                box.GetBox().GetMaxPoint() - CVector3f(0.0001f, 0.0001f, 0.0001f)),
                         primitive->GetMaterial());
  } else if (primitive->GetPrimType() == 'SPHR') {
    const CCollidableSphere& sphere = static_cast< const CCollidableSphere& >(*primitive);
    usePrimitive = new (storage) CCollidableSphere(
        CSphere(sphere.GetSphere().GetCenter(), sphere.GetSphere().GetRadius() - 0.0001f),
        primitive->GetMaterial());
  }
  CGameCollision::CollisionFailsafe(mgr, cache, actor, *usePrimitive, nearList, 0.f, 1);
}

bool RemovePositiveZComponentFromNormal(CVector3f& normal) {
  if (normal.GetZ() > 0.f && normal.GetZ() < 0.99f) {
    normal = CVector3f(normal.GetX(), normal.GetY(), 0.f).AsNormalized();
    return true;
  }
  return false;
}

bool CGroundMovement::RemoveNormalComponent(const CVector3f& normal, const CVector3f& direction,
                                            CVector3f& collisionNormal, float& normalDot) {
  const float dot = CVector3f::Dot(normal, collisionNormal);
  if (fabsf(dot) > 0.99f) {
    return false;
  }
  const float oldDot = CVector3f::Dot(direction, collisionNormal);
  const float newDot = CVector3f::Dot(direction, (collisionNormal - dot * normal).AsNormalized());
  if (oldDot > 0.f && newDot < 0.f) {
    return false;
  }
  if (fabsf(oldDot) > 0.01f && fabsf(newDot / oldDot) > 4.f) {
    return false;
  }
  collisionNormal -= dot * normal;
  normalDot = dot;
  return true;
}

bool CGroundMovement::RemoveNormalComponent(const CVector3f& normal, CVector3f& velocity) {
  const float dot = CVector3f::Dot(normal, velocity);
  if (fabsf(dot) > 0.99f) {
    return false;
  }
  velocity -= dot * normal;
  return true;
}

CMaterialList CGroundMovement::MoveObjectAnalytical(CStateManager& mgr, CPhysicsActor& actor,
                                                    float dt, const TEntityList& nearList,
                                                    CAreaCollisionCache& cache,
                                                    const SMovementOptions& options,
                                                    SMoveObjectResult& result) {
  CMaterialList materials;
  result.mProcessedCollisions = 0;
  float remainingDt = dt;
  uint cycle = 0;
  bool floorCollision = options.mFloorPlaneNormal;
  CVector3f floorPlaneNormal = floorCollision ? *options.mFloorPlaneNormal : CVector3f::Zero();
  while (remainingDt > 0.f) {
    float collideDt = remainingDt;
    CMotionState motion = actor.PredictMotion_Internal(remainingDt);
    float translationMag = motion.GetTranslation().Magnitude();
    float inverseMagnitude = 1.f / (translationMag > FLT_EPSILON ? translationMag : 1.f);
    CVector3f direction = inverseMagnitude * motion.GetTranslation();
    actor.GetCollisionPrimitive()->CalculateAABox(actor.GetPrimitiveTransform());
    double distance = translationMag;
    CCollisionInfo info;
    if (translationMag > options.mMinimumTranslationDelta) {
      TUniqueId id = kInvalidUniqueId;
      CGameCollision::DetectCollision_Cached_Moving(
          mgr, cache, *actor.GetCollisionPrimitive(), actor.GetPrimitiveTransform(),
          actor.GetMaterialFilter(), nearList, direction, id, info, distance);
      if (id != kInvalidUniqueId) {
        result.mId = id;
        result.mCollision = info;
      }
      collideDt = remainingDt * static_cast< float >(distance / translationMag);
    }
    float moveDistance =
        rstl::max_val(static_cast< float >(distance - options.mMinimumTranslationDelta), 0.f);
    CVector3f collisionNormal = info.GetNormalLeft();
    bool floor = CGameCollision::CanBlock(info.GetMaterialLeft(), collisionNormal);
    const bool clipCollision =
        options.mAlwaysClip || (options.mDisableClipForFloorOnly && !floor);
    float collisionFloorDot = 0.f;
    if (info.IsValid()) {
      ++result.mProcessedCollisions;
      if (floor) {
        materials.Add(kMT_Floor);
        floorPlaneNormal = info.GetNormalLeft();
        floorCollision = true;
      } else {
        materials.Add(kMT_Wall);
      }
      if (clipCollision) {
        if (floorCollision) {
          if (!RemoveNormalComponent(floorPlaneNormal, direction, collisionNormal,
                                     collisionFloorDot)) {
            RemovePositiveZComponentFromNormal(collisionNormal);
          } else {
            collisionNormal.Normalize();
          }
        } else {
          RemovePositiveZComponentFromNormal(collisionNormal);
        }
      }
      motion = actor.PredictMotion_Internal(collideDt);
    }
    motion.SetTranslation(moveDistance * direction);
    actor.AddMotionState(motion);
    if (info.IsValid()) {
      CVector3f oldVelocity = actor.GetVelocityWR();
      CVector3f velocity =
          oldVelocity.CanBeNormalized()
              ? CollisionDamping(oldVelocity, oldVelocity.AsNormalized(), collisionNormal,
                                 options.mDampedNormalCoefficient,
                                 options.mDampedDeltaCoefficient)
              : CVector3f::Zero();
      float elasticForce = floor ? options.mFloorElasticForce
                                 : options.mWallElasticLinear * collisionFloorDot +
                                       options.mWallElasticConstant;
      float dot = CVector3f::Dot(collisionNormal, velocity);
      if (dot < elasticForce) {
        velocity += (elasticForce - dot) * collisionNormal;
      }
      if (clipCollision && floorCollision) {
        if (!RemoveNormalComponent(floorPlaneNormal, velocity)) {
          velocity.SetZ(0.f);
        }
      }
      if (velocity[kDZ] > options.mMaxPositiveVerticalVelocity) {
        float scale = options.mMaxPositiveVerticalVelocity / velocity[kDZ];
        velocity *= scale;
      }
      if (options.mDampForceAndMomentum) {
        CVector3f force = actor.GetForceWR();
        if (force.CanBeNormalized()) {
          force = CollisionDamping(force, force.AsNormalized(), collisionNormal, 0.f, 1.f);
          actor.SetForceWR(force);
        }
        CVector3f momentum = actor.GetMomentumWR();
        if (momentum.CanBeNormalized()) {
          momentum = CollisionDamping(momentum, momentum.AsNormalized(), collisionNormal, 0.f, 1.f);
          actor.SetMomentumWR(momentum);
        }
      }
      if (options.mSetWaterLandingForce && !floor) {
        if (info.GetNormalLeft().GetZ() < -0.1f && velocity.GetZ() > 0.f) {
          velocity.SetZ(0.5f * velocity.GetZ());
        }
        float normalZ = fabsf(info.GetNormalLeft().GetZ());
        if ((normalZ > options.mDownwardZThreshold && velocity.GetZ() < 0.f) ||
            normalZ > options.mAnyZThreshold) {
          float landingForce = rstl::max_val(options.mWaterLandingForceCoefficient * normalZ,
                                             options.mMinimumWaterLandingForce);
          actor.SetForceWR(CVector3f(0.f, 0.f, -(1.f + landingForce) * actor.GetWeight()));
          velocity *= 1.f - options.mWaterLandingVelocityReduction;
        }
      }
      actor.SetVelocityWR(velocity);
    } else {
      CVector3f velocity = actor.GetVelocityWR();
      if (velocity[kDZ] > options.mMaxPositiveVerticalVelocity) {
        float scale = options.mMaxPositiveVerticalVelocity / velocity[kDZ];
        velocity *= scale;
      }
      actor.SetVelocityWR(velocity);
    }
    actor.ClearImpulses();
    remainingDt -= collideDt;
    if (++cycle >= options.mMaxCollisionCycles) {
      break;
    }
  }
  result.mProcessedDt = dt - remainingDt;
  return materials;
}
