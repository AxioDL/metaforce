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

namespace urde
{

void CGroundMovement::CheckFalling(CPhysicsActor& actor, CStateManager& mgr, float)
{
    bool oob = true;
    for (const CGameArea& area : *mgr.GetWorld())
    {
        if (area.GetAABB().intersects(*actor.GetTouchBounds()))
        {
            oob = false;
            break;
        }
    }

    if (!oob)
    {
        mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::Falling);
    }
    else
    {
        mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::OnFloor);
        actor.SetAngularVelocityWR(actor.GetAngularVelocityWR() * 0.98f);
        zeus::CVector3f vel = actor.GetTransform().transposeRotate(actor.GetVelocity());
        vel.z = 0.f;
        actor.SetVelocityOR(vel);
        actor.SetMomentumWR(zeus::CVector3f::skZero);
    }
}

void CGroundMovement::MoveGroundCollider(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                         const rstl::reserved_vector<TUniqueId, 1024>* nearList)
{
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
    if (actor.GetCollisionPrimitive()->GetPrimType() == FOURCC('OBTG'))
    {
        CGameCollision::BuildAreaCollisionCache(mgr, cache);
        if (deltaMag > 0.5f * CGameCollision::GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive()))
        {
            zeus::CVector3f point =
                actor.GetCollisionPrimitive()->CalculateAABox(actor.GetPrimitiveTransform()).center();
            TUniqueId intersectId = kInvalidUniqueId;
            CMaterialFilter filter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});
            CRayCastResult result =
                mgr.RayWorldIntersection(intersectId, point, newState.x0_translation.normalized(), deltaMag,
                                         filter, useColliderList);
            if (result.IsValid())
            {
                collideDt = dt * (result.GetT() / deltaMag);
                newState = actor.PredictMotion_Internal(collideDt);
            }
            actor.MoveCollisionPrimitive(newState.x0_translation);
            if (CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                       actor.GetPrimitiveTransform(), actor.GetMaterialFilter(),
                                                       useColliderList, idDetect, collisionList))
            {
                actor.AddMotionState(newState);
                float resolved = 0.f;
                if (ResolveUpDown(cache, mgr, actor, actor.GetMaterialFilter(), useColliderList,
                                  actor.GetStepUpHeight(), 0.f, resolved, collisionList))
                {
                    actor.SetMotionState(oldState);
                    MoveGroundColliderXY(cache, mgr, actor, actor.GetMaterialFilter(), useColliderList, collideDt);
                }

            }
            else
            {
                actor.AddMotionState(newState);
            }

            float stepDown = actor.GetStepDownHeight();
            float resolved = 0.f;
            collisionList.Clear();
            TUniqueId stepZId = kInvalidUniqueId;
            if (stepDown >= 0.f)
            {
                if (MoveGroundColliderZ(cache, mgr, actor, actor.GetMaterialFilter(), useColliderList,
                                        -stepDown, resolved, collisionList, stepZId))
                {
                    if (collisionList.GetCount() > 0)
                    {
                        CCollisionInfoList filteredList;
                        CollisionUtil::FilterByClosestNormal(zeus::CVector3f{0.f, 0.f, 1.f}, collisionList, filteredList);
                        if (filteredList.GetCount() > 0)
                        {
                            if (CGameCollision::IsFloor(filteredList.Front().GetMaterialLeft(),
                                                        filteredList.Front().GetNormalLeft()))
                            {
                                if (TCastToPtr<CScriptPlatform> plat = mgr.ObjectById(stepZId))
                                    mgr.SendScriptMsg(plat.GetPtr(), actor.GetUniqueId(),
                                                      EScriptObjectMessage::AddPlatformRider);
                                CGameCollision::SendMaterialMessage(mgr, filteredList.Front().GetMaterialLeft(), actor);
                                mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::OnFloor);
                            }
                            else
                            {
                                CheckFalling(actor, mgr, dt);
                            }
                        }
                    }
                }
            }
            else
            {
                CheckFalling(actor, mgr, dt);
            }

            actor.ClearForcesAndTorques();
            actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
            if (actor.GetMaterialList().HasMaterial(EMaterialTypes::Player))
            {
                CGameCollision::CollisionFailsafe(mgr, cache, actor, *actor.GetCollisionPrimitive(),
                                                  useColliderList, 0.f, 1);
            }
        }
    }
}

bool CGroundMovement::ResolveUpDown(CAreaCollisionCache& cache, CStateManager& mgr, CPhysicsActor& actor,
                                    const CMaterialFilter& filter, rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                    float stepUp, float stepDown, float& fOut, CCollisionInfoList& list)
{
    float zextent = stepDown;
    if (list.GetCount() <= 0)
        return true;

    zeus::CAABox aabb = zeus::CAABox::skInvertedBox;
    zeus::CVector3f normAccum = zeus::CVector3f::skZero;
    for (CCollisionInfo& info : list)
    {
        if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft()))
        {
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
    if (normAccum.z >= 0.f)
    {
        zextent = aabb.max.z - actorAABB.min.z + 0.02f;
        if (zextent > stepUp)
            return true;
    }
    else
    {
        zextent = aabb.min.z - actorAABB.max.z - 0.02f;
        if (zextent < -stepDown)
            return true;
    }

    actor.MoveCollisionPrimitive({0.f, 0.f, zextent});

    if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                       actor.GetPrimitiveTransform(), filter, nearList))
    {
        fOut = zextent;
        actor.SetTranslation(actor.GetTranslation() + zeus::CVector3f(0.f, 0.f, zextent));
        actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);

        bool floor = false;
        for (CCollisionInfo& info : list)
        {
            if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft()))
            {
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
                                          const CMaterialFilter& filter, rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                          float amt, float&, CCollisionInfoList& list, TUniqueId& idOut)
{
    actor.MoveCollisionPrimitive({0.f, 0.f, amt});

    zeus::CAABox aabb = zeus::CAABox::skInvertedBox;
    if (CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                               actor.GetPrimitiveTransform(), filter, nearList, idOut, list))
    {
        for (CCollisionInfo& info : list)
        {
            aabb.accumulateBounds(info.GetPoint());
            aabb.accumulateBounds(info.GetExtreme());
        }

        zeus::CAABox actorAABB = actor.GetBoundingBox();
        float zextent;
        if (amt > 0.f)
            zextent = aabb.min.z - actorAABB.max.z - 0.02f + amt;
        else
            zextent = aabb.max.z - actorAABB.min.z + 0.02f + amt;

        actor.MoveCollisionPrimitive({0.f, 0.f, zextent});

        if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                           actor.GetPrimitiveTransform(), filter, nearList))
        {
            actor.SetTranslation(actor.GetTranslation() + zeus::CVector3f(0.f, 0.f, zextent));
            actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
        }

        bool floor = false;
        for (CCollisionInfo& info : list)
        {
            if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft()))
            {
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
                                           const CMaterialFilter& filter, rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                           float dt)
{
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
    if (transMag > minExtent)
    {
        dt = minExtent * (dt / transMag);
        originalDt = dt;
        newMState = actor.PredictMotion_Internal(dt);
        divMag = std::min(divMag, minExtent);
    }

    float nonCollideDt = dt;
    do
    {
        actor.MoveCollisionPrimitive(newMState.x0_translation);
        collisionList.Clear();
        TUniqueId otherId = kInvalidUniqueId;
        bool collided = CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                               actor.GetPrimitiveTransform(), filter, nearList,
                                                               otherId, collisionList);
        if (collided)
            otherActor = mgr.ObjectById(otherId);
        actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
        if (collided)
        {
            didCollide = true;
            if (newMState.x0_translation.magnitude() < divMag)
            {
                CCollisionInfoList backfaceFilteredList, floorFilteredList;
                zeus::CVector3f deltaVel = actor.GetVelocity();
                if (otherActor)
                    deltaVel -= otherActor->GetVelocity();
                CollisionUtil::FilterOutBackfaces(deltaVel, collisionList, backfaceFilteredList);
                CAABoxFilter::FilterBoxFloorCollisions(backfaceFilteredList, floorFilteredList);
                CGameCollision::MakeCollisionCallbacks(mgr, actor, otherId, floorFilteredList);
                if (floorFilteredList.GetCount() == 0 && isPlayer)
                {
                    CMotionState lastNonCollideState = actor.GetLastNonCollidingState();
                    lastNonCollideState.x1c_velocity *= 0.5f;
                    lastNonCollideState.x28_angularMomentum *= 0.5f;
                    actor.SetMotionState(lastNonCollideState);
                }
                for (const CCollisionInfo& info : floorFilteredList)
                {
                    CCollisionInfo infoCopy = info;
                    float restitution = CGameCollision::GetCoefficientOfRestitution(infoCopy) +
                                        actor.GetCoefficientOfRestitutionModifier();
                    if (otherActor)
                        CGameCollision::CollideWithDynamicBodyNoRot(actor, *otherActor, infoCopy, restitution, true);
                    else
                        CGameCollision::CollideWithStaticBodyNoRot(actor, infoCopy.GetMaterialLeft(), infoCopy.GetMaterialRight(),
                                                                   infoCopy.GetNormalLeft(), restitution, true);
                }
                remDt -= dt;
                nonCollideDt = std::min(originalDt, remDt);
                dt = nonCollideDt;
            }
            else
            {
                nonCollideDt *= 0.5f;
                dt *= 0.5f;
            }
        }
        else
        {
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

void CGroundMovement::CollisionDamping(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&,
                                       float, float)
{

}

void CGroundMovement::MoveGroundCollider_New(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                             const rstl::reserved_vector<TUniqueId, 1024>* nearList)
{
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
    bool startingJump = player.x258_movementState == CPlayer::EPlayerMovementState::StartingJump;
    bool dampUnderwater = false;
    if (player.x9c4_31_dampUnderwaterMotion)
    {
        if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
            dampUnderwater = true;
    }

    bool noJump = (player.x258_movementState != CPlayer::EPlayerMovementState::StartingJump &&
                   player.x258_movementState != CPlayer::EPlayerMovementState::Jump);

    float stepDown = player.GetStepDownHeight();
    float stepUp = player.GetStepUpHeight();

    bool x108 = true;
    CMaterialList material(EMaterialTypes::Unknown);
    SMoveObjectResult result;

    if (!startingJump)
    {
        SMovementOptions opts;
        opts.x0_ = false;
        opts.x4_ = 0.f;
        opts.x8_ = 0.f;
        opts.xc_ = 0.37f;
        opts.x10_ = 0.25f;
        opts.x14_ = 0.f;
        opts.x18_ = true;
        opts.x19_ = false;
        opts.x1a_ = noJump;
        opts.x1c_ = 4;
        opts.x20_ = 0.002f;
        opts.x24_ = 0.f;
        opts.x28_ = 1.f;
        opts.x2c_ = 0.f;
        opts.x30_ = 0.02f;
        opts.x3c_ = player.GetLastFloorPlaneNormal();
        opts.x34_ = 0.2f;
        opts.x38_ = player.GetMaximumPlayerPositiveVerticalVelocity(mgr);

        if (noJump)
        {
            zeus::CVector3f vel = player.GetVelocity();
            vel.z = 0.f;
            actor.SetVelocityWR(vel);
            actor.x15c_force.z = 0.f;
            actor.x150_momentum.z = 0.f;
            actor.x168_impulse.z = 0.f;
        }

        CPhysicsState physStatePre = actor.GetPhysicsState();
        CMaterialList material = MoveObjectAnalytical(mgr, actor, dt, useNearList, cache, opts, result);
        CPhysicsState physStatePost = actor.GetPhysicsState();

        if (material.XOR({EMaterialTypes::Unknown}))
        {
            SMovementOptions optsCopy = opts;
            zeus::CVector3f postToPre = physStatePre.GetTranslation() - physStatePost.GetTranslation();
            float f27 = postToPre.magSquared();
            optsCopy.x19_ = noJump;
            float quarterStepUp = 0.25f * stepUp;
            rstl::reserved_vector<CPhysicsState, 2> physStateList;
            rstl::reserved_vector<float, 2> floatList;
            rstl::reserved_vector<CCollisionInfo, 2> collisionInfoList;
            rstl::reserved_vector<TUniqueId, 2> uniqueIdList;
            rstl::reserved_vector<CMaterialList, 2> materialListList;
            bool done = false;
            for (int i=0 ; i < 2 && !done ; ++i)
            {
                double useStepUp = (i == 0) ? quarterStepUp : stepUp;
                actor.SetPhysicsState(physStatePre);
                CCollisionInfo collisionInfo;
                TUniqueId id = kInvalidUniqueId;
                CGameCollision::DetectCollision_Cached_Moving(mgr, cache, *actor.GetCollisionPrimitive(),
                                                              actor.GetTransform(), actor.GetMaterialFilter(),
                                                              useNearList, {0.f, 0.f, 1.f}, id, collisionInfo,
                                                              useStepUp);
                if (collisionInfo.IsValid())
                {
                    useStepUp = std::max(0.0, useStepUp - optsCopy.x20_);
                    done = true;
                }

                if (useStepUp > 0.0005)
                {
                    actor.SetTranslation(actor.GetTranslation() + zeus::CVector3f(0.f, 0.f, useStepUp));

                    SMoveObjectResult result;
                    CMaterialList material = MoveObjectAnalytical(mgr, actor, dt, useNearList, cache, optsCopy, result);
                    CCollisionInfo collisionInfo;
                    double useStepDown = useStepUp + stepDown;
                    TUniqueId id = kInvalidUniqueId;
                    if (useStepDown > 0.0)
                    {
                        CGameCollision::DetectCollision_Cached_Moving(mgr, cache, *actor.GetCollisionPrimitive(),
                                                                      actor.GetTransform(), actor.GetMaterialFilter(),
                                                                      useNearList, {0.f, 0.f, -1.f}, id, collisionInfo,
                                                                      useStepDown);
                    }
                    else
                    {
                        useStepDown = 0.0;
                    }

                    float minStep = std::min(useStepUp, useStepDown);
                    zeus::CVector3f offsetStep = actor.GetTranslation() - zeus::CVector3f(0.f, 0.f, minStep);
                    bool canBlock = (collisionInfo.IsValid() &&
                                     CGameCollision::CanBlock(collisionInfo.GetMaterialLeft(), collisionInfo.GetNormalLeft()));
                    zeus::CVector3f postToPre = physStatePre.GetTranslation() - offsetStep;
                    float f26 = postToPre.magSquared();
                    if (canBlock && f27 < f26)
                    {
                        useStepDown = std::max(0.0, useStepDown - 0.0005);
                        actor.SetTranslation(actor.GetTranslation() - zeus::CVector3f(0.f, 0.f, useStepDown));
                        physStateList.push_back(actor.GetPhysicsState());
                        floatList.push_back(f26);
                        collisionInfoList.push_back(collisionInfo);
                        uniqueIdList.push_back(id);
                        materialListList.push_back(material);
                    }
                }
            }

            if (physStateList.size() == 0)
            {
                actor.SetPhysicsState(physStatePost);
            }
            else
            {
                float maxFloat = -1.0e10f;
                int maxIdx = -1;
                for (int i=0 ; i<physStateList.size() ; ++i)
                {
                    if (maxFloat < floatList[i])
                    {
                        maxFloat = floatList[i];
                        maxIdx = i;
                    }
                }

                actor.SetPhysicsState(physStateList[maxIdx]);
                mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::OnFloor);
                if (CEntity* ent = mgr.ObjectById(uniqueIdList[maxIdx]))
                {
                    result.x0_id.emplace(uniqueIdList[maxIdx]);
                    result.x8_collision.emplace(collisionInfoList[maxIdx]);
                    if (TCastToPtr<CScriptPlatform> plat = ent)
                        mgr.SendScriptMsg(ent, actor.GetUniqueId(), EScriptObjectMessage::AddPlatformRider);
                }

                CCollisionInfo& cInfo = collisionInfoList[maxIdx];
                CGameCollision::SendMaterialMessage(mgr, cInfo.GetMaterialLeft(), actor);
                x108 = false;
                actor.SetLastFloorPlaneNormal({cInfo.GetNormalLeft()});
            }
        }
    }
    else
    {
        SMovementOptions opts;
        opts.x0_ = true;
        opts.x4_ = dampUnderwater ? 35.f : 1.f;
        opts.x8_ = dampUnderwater ? 5.f : 0.f;
        opts.xc_ = dampUnderwater ? 0.05f : 0.37f;
        opts.x10_ = dampUnderwater ? 0.01f : 0.25f;
        opts.x14_ = dampUnderwater ? 0.2f : 0.f;
        opts.x18_ = false;
        opts.x19_ = false;
        opts.x1a_ = false;
        opts.x1c_ = 4;
        opts.x20_ = 0.002f;
        opts.x24_ = 0.f;
        opts.x28_ = 1.f;
        opts.x2c_ = 0.1f;
        opts.x30_ = dampUnderwater ? 0.2f : 0.2f;
        opts.x3c_ = player.GetLastFloorPlaneNormal();
        opts.x38_ = player.GetMaximumPlayerPositiveVerticalVelocity(mgr);

        material = MoveObjectAnalytical(mgr, actor, dt, useNearList, cache, opts, result);
    }

    if (x108)
    {
        // TODO: Finish
    }

    actor.ClearForcesAndTorques();
    if (material.HasMaterial(EMaterialTypes::Wall))
        player.SetPlayerHitWallDuringMove();

    if (result.x0_id)
    {
        CCollisionInfoList list;
        list.Add(*result.x8_collision, false);
        CGameCollision::MakeCollisionCallbacks(mgr, actor, *result.x0_id, list);
    }

    CMotionState mState = actor.GetMotionState();
    mState.x0_translation = actor.GetLastNonCollidingState().x0_translation;
    mState.x1c_velocity = actor.GetLastNonCollidingState().x1c_velocity;
    actor.SetLastNonCollidingState(mState);

    const CCollisionPrimitive* prim = actor.GetCollisionPrimitive();
    if (prim->GetPrimType() == FOURCC('AABX'))
    {
        // TODO: Finish
    }
    else if (prim->GetPrimType() == FOURCC('SPHR'))
    {
        // TODO: Finish
    }

    CGameCollision::CollisionFailsafe(mgr, cache, actor, *prim, useNearList, 0.f, 1);
}

void CGroundMovement::RemoveNormalComponent(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&, float&)
{

}

void CGroundMovement::RemoveNormalComponent(const zeus::CVector3f&, const zeus::CVector3f&)
{

}

CMaterialList CGroundMovement::MoveObjectAnalytical(CStateManager& mgr, CPhysicsActor& actor, float,
                                                    rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                                    CAreaCollisionCache& cache, const SMovementOptions& opts,
                                                    SMoveObjectResult& result)
{
    return {};
}

}
