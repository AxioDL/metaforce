#include "CGameCollision.hpp"
#include "CCollidableOBBTreeGroup.hpp"
#include "CMaterialFilter.hpp"
#include "CMaterialList.hpp"
#include "World/CActor.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "World/CWorld.hpp"
#include "CAABoxFilter.hpp"
#include "CBallFilter.hpp"
#include "CMetroidAreaCollider.hpp"
#include "CollisionUtil.hpp"
#include "World/CScriptPlatform.hpp"
#include "CCollidableSphere.hpp"

namespace urde
{

static float CollisionImpulseFiniteVsInfinite(float mass, float velNormDot, float restitution)
{
    return mass * ((1.f / restitution) * velNormDot);
}

static float CollisionImpulseFiniteVsFinite(float mass, float velNormDot, float restitution, float f4)
{
    return (-(1.f + f4) * restitution) / ((1.f / mass) + (1.f / velNormDot));
}

void CGameCollision::InitCollision()
{
    /* Types */
    CCollisionPrimitive::InitBeginTypes();
    CCollisionPrimitive::InitAddType(CCollidableOBBTreeGroup::GetType());
    CCollisionPrimitive::InitEndTypes();

    /* Colliders */
    CCollisionPrimitive::InitBeginColliders();
    CCollisionPrimitive::InitAddCollider(CCollidableOBBTreeGroup::SphereCollide, "CCollidableSphere",
                                         "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitAddCollider(CCollidableOBBTreeGroup::AABoxCollide, "CCollidableAABox",
                                         "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitAddBooleanCollider(CCollidableOBBTreeGroup::SphereCollideBoolean, "CCollidableSphere",
                                                "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitAddBooleanCollider(CCollidableOBBTreeGroup::AABoxCollideBoolean, "CCollidableAABox",
                                                "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitAddMovingCollider(CCollidableOBBTreeGroup::CollideMovingAABox, "CCollidableAABox",
                                               "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitAddMovingCollider(CCollidableOBBTreeGroup::CollideMovingSphere, "CCollidableSphere",
                                               "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitAddCollider(CGameCollision::NullCollisionCollider, "CCollidableOBBTreeGroup",
                                         "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitAddBooleanCollider(CGameCollision::NullBooleanCollider, "CCollidableOBBTreeGroup",
                                                "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitAddMovingCollider(CGameCollision::NullMovingCollider, "CCollidableOBBTreeGroup",
                                               "CCollidableOBBTreeGroup");
    CCollisionPrimitive::InitEndColliders();
}

void CGameCollision::MovePlayer(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                const rstl::reserved_vector<TUniqueId, 1024>* colliderList)
{
    actor.SetAngularEnabled(true);
    actor.AddMotionState(actor.PredictAngularMotion(dt));
    if (actor.IsUseStandardCollider())
    {
        MoveAndCollide(mgr, actor, dt, CBallFilter(actor), colliderList);
    }
    else
    {
        if (actor.GetMaterialList().HasMaterial(EMaterialTypes::GroundCollider))
            MoveGroundCollider_New(mgr, actor, dt, colliderList);
        else
            MoveAndCollide(mgr, actor, dt, CBallFilter(actor), colliderList);
    }
    actor.SetAngularEnabled(false);
}

void CGameCollision::MoveGroundCollider(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                        const rstl::reserved_vector<TUniqueId, 1024>* colliderList)
{
    CMotionState oldState = actor.GetMotionState();
    CMotionState newState = actor.PredictMotion_Internal(dt);
    newState.x0_translation.magnitude();
    actor.GetMaterialFilter();
    zeus::CAABox motionVol = actor.GetMotionVolume(dt);
    rstl::reserved_vector<TUniqueId, 1024> augmentedColliderList;
    if (colliderList)
        augmentedColliderList = *colliderList;
    mgr.BuildColliderList(augmentedColliderList, actor, motionVol);
    CAreaCollisionCache cache(motionVol);
    actor.GetCollisionPrimitive()->GetPrimType();
    // TODO: finish
}

void CGameCollision::MoveGroundCollider_New(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                            const rstl::reserved_vector<TUniqueId, 1024>* colliderList)
{
    // TODO: finish
}

void CGameCollision::MoveAndCollide(CStateManager& mgr, CPhysicsActor& actor, float dt, const ICollisionFilter& filter,
                                    const rstl::reserved_vector<TUniqueId, 1024>* colliderList)
{
    bool isPlayer = actor.GetMaterialList().HasMaterial(EMaterialTypes::Player);
    bool r28 = false;
    bool r27 = false;
    int r26 = 0;
    float f31 = dt;
    float _4AC4 = dt;
    float _4AC8 = dt;
    CCollisionInfoList accumList;
    CMotionState mState = actor.PredictMotion_Internal(dt);
    float transMag = mState.x0_translation.magnitude();
    float m1 = 0.0005f / actor.GetCollisionAccuracyModifier();
    float m2 = transMag / (5.f * actor.GetCollisionAccuracyModifier());
    float mMax = std::max(m1, m2);
    float m3 = 0.001f / actor.GetCollisionAccuracyModifier();
    
    zeus::CAABox motionVol = actor.GetMotionVolume(dt);
    rstl::reserved_vector<TUniqueId, 1024> useColliderList;
    if (colliderList)
        useColliderList = *colliderList;
    else
        mgr.BuildColliderList(useColliderList, actor, zeus::CAABox(motionVol.min - 1.f, motionVol.max + 1.f));
    CAreaCollisionCache cache(motionVol);
    if (actor.GetCollisionPrimitive()->GetPrimType() != FOURCC('OBTG') &&
        !actor.GetMaterialFilter().GetExcludeList().HasMaterial(EMaterialTypes::ThirtyEight))
    {
        BuildAreaCollisionCache(mgr, cache);
        zeus::CVector3f pos = actor.GetCollisionPrimitive()->CalculateAABox(actor.GetPrimitiveTransform()).center();
        float halfExtent = 0.5f * GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive());
        if (transMag > halfExtent)
        {
            TUniqueId id = kInvalidUniqueId;
            zeus::CVector3f dir = (1.f / transMag) * mState.x0_translation;
            CRayCastResult intersectRes = mgr.RayWorldIntersection(id, pos, dir, transMag,
                                                                   actor.GetMaterialFilter(), useColliderList);
            if (intersectRes.IsValid())
            {
                f31 = dt * (intersectRes.GetT() / transMag);
                mState = actor.PredictMotion_Internal(f31);
                _4AC8 = halfExtent * (dt / transMag);
                mMax = std::min(mMax, halfExtent);
            }
        }
    }

    float f27 = f31;
    while (true)
    {
        actor.MoveCollisionPrimitive(mState.x0_translation);
        if (DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                          actor.GetPrimitiveTransform(), actor.GetMaterialFilter(),
                                          useColliderList))
        {
            r28 = true;
            if (mState.x0_translation.magnitude() < mMax)
            {
                r27 = true;
                accumList.Clear();
                TUniqueId id = kInvalidUniqueId;
                DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(), actor.GetPrimitiveTransform(),
                                       actor.GetMaterialFilter(), useColliderList, id, accumList);
                TCastToPtr<CPhysicsActor> otherActor = mgr.ObjectById(id);
                actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
                zeus::CVector3f relVel = GetActorRelativeVelocities(actor, otherActor.GetPtr());
                CCollisionInfoList filterList0, filterList1;
                CollisionUtil::FilterOutBackfaces(relVel, accumList, filterList0);
                if (filterList0.GetCount() > 0)
                {
                    filter.Filter(filterList0, filterList1);
                    if (!filterList1.GetCount() && actor.GetMaterialList().HasMaterial(EMaterialTypes::Player))
                    {
                        CMotionState mState = actor.GetLastNonCollidingState();
                        mState.x1c_velocity *= 0.5f;
                        mState.x28_angularMomentum *= 0.5f;
                        actor.SetMotionState(mState);
                    }
                }
                MakeCollisionCallbacks(mgr, actor, id, filterList1);
                SendScriptMessages(mgr, actor, otherActor.GetPtr(), filterList1);
                ResolveCollisions(actor, otherActor.GetPtr(), filterList1);
                _4AC4 -= f31;
                f27 = std::min(_4AC4, _4AC8);
                f31 = f27;
            }
            else
            {
                f27 *= 0.5f;
                f31 *= 0.5f;
            }
        }
        else
        {
            actor.AddMotionState(mState);
            _4AC4 -= f31;
            f31 = f27;
            actor.ClearImpulses();
            actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
        }

        ++r26;
        if (_4AC4 > 0.f && ((mState.x0_translation.magnitude() > m3 && r27) || !r27) && r26 <= 1000)
            mState = actor.PredictMotion_Internal(f31);
        else
            break;
    }

    f27 = _4AC4 / dt;
    if (!r28 && !actor.GetMaterialList().HasMaterial(EMaterialTypes::GroundCollider))
        mgr.SendScriptMsg(&actor, kInvalidUniqueId, EScriptObjectMessage::Falling);

    if (isPlayer)
        CollisionFailsafe(mgr, cache, actor, *actor.GetCollisionPrimitive(), useColliderList, f27, 2);

    actor.ClearForcesAndTorques();
    actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
}

zeus::CVector3f CGameCollision::GetActorRelativeVelocities(const CPhysicsActor& act0, const CPhysicsActor* act1)
{
    zeus::CVector3f ret = act0.GetVelocity();
    if (act1)
    {
        bool rider = false;
        if (TCastToConstPtr<CScriptPlatform> plat = act1)
            rider = plat->IsRider(act0.GetUniqueId());
        if (!rider)
            ret =- act1->GetVelocity();
    }
    return ret;
}

void CGameCollision::Move(CStateManager& mgr, CPhysicsActor& actor, float dt,
                          const rstl::reserved_vector<TUniqueId, 1024>* colliderList)
{
    if (!actor.IsMovable())
        return;
    if (actor.GetMaterialList().HasMaterial(EMaterialTypes::GroundCollider) || actor.WillMove(mgr))
    {
        if (actor.IsAngularEnabled())
            actor.AddMotionState(actor.PredictAngularMotion(dt));
        actor.UseCollisionImpulses();
        if (actor.GetMaterialList().HasMaterial(EMaterialTypes::Solid))
        {
            if (actor.GetMaterialList().HasMaterial(EMaterialTypes::Player))
                MovePlayer(mgr, actor, dt, colliderList);
            else if (actor.GetMaterialList().HasMaterial(EMaterialTypes::GroundCollider))
                MoveGroundCollider(mgr, actor, dt, colliderList);
            else
                MoveAndCollide(mgr, actor, dt, CAABoxFilter(actor), colliderList);
        }
        else
        {
            actor.AddMotionState(actor.PredictMotion_Internal(dt));
            actor.ClearForcesAndTorques();
        }
        mgr.UpdateActorInSortedLists(actor);
    }
}

bool CGameCollision::CanBlock(const CMaterialList& mat, const zeus::CVector3f& v)
{
    if ((mat.HasMaterial(EMaterialTypes::Character) && !mat.HasMaterial(EMaterialTypes::Scannable)) ||
        mat.HasMaterial(EMaterialTypes::Occluder))
        return false;

    if (mat.HasMaterial(EMaterialTypes::Occluder) || mat.HasMaterial(EMaterialTypes::Floor))
        return true;

    return (v.z > 0.85f);
}

bool CGameCollision::IsFloor(const CMaterialList& mat, const zeus::CVector3f& v)
{
    if (mat.HasMaterial(EMaterialTypes::Floor))
        return true;
    return (v.z > 0.85f);
}

void CGameCollision::SendMaterialMessage(CStateManager& mgr, const CMaterialList& mat, CActor& act)
{
    EScriptObjectMessage msg;
    if (mat.HasMaterial(EMaterialTypes::Ice))
        msg = EScriptObjectMessage::OnIceSurface;
    else if (mat.HasMaterial(EMaterialTypes::MudSlow))
        msg = EScriptObjectMessage::OnMudSlowSurface;
    else
        msg = EScriptObjectMessage::OnNormalSurface;

    mgr.SendScriptMsg(&act, kInvalidUniqueId, msg);
}

CRayCastResult
CGameCollision::RayStaticIntersection(const CStateManager& mgr, const zeus::CVector3f& pos,
                                      const zeus::CVector3f& dir, float length, const CMaterialFilter& filter)
{
    CRayCastResult ret;
    float bestT = length;
    if (bestT <= 0.f)
        bestT = 100000.f;

    zeus::CLine line(pos, dir);
    for (const CGameArea& area : *mgr.GetWorld())
    {
        CAreaOctTree::SRayResult rayRes;
        CAreaOctTree& collision = *area.GetPostConstructed()->x0_collision;
        collision.GetRootNode().LineTestEx(line, filter, rayRes, length);
        if (!rayRes.x10_surface || (length != 0.f && length < rayRes.x3c_t))
            continue;

        if (rayRes.x3c_t < bestT)
        {
            ret = CRayCastResult(rayRes.x3c_t, dir * rayRes.x3c_t + pos,
                                 rayRes.x0_plane, rayRes.x10_surface->GetSurfaceFlags());
            bestT = rayRes.x3c_t;
        }
    }

    return ret;
}

bool CGameCollision::RayStaticIntersectionBool(const CStateManager& mgr, const zeus::CVector3f& start,
                                               const zeus::CVector3f& dir, float length,
                                               const CMaterialFilter& filter)
{
    if (length <= 0.f)
        length = 100000.f;
    zeus::CLine line(start, dir);
    for (const CGameArea& area : *mgr.GetWorld())
    {
        const CAreaOctTree& collision = *area.GetPostConstructed()->x0_collision;
        CAreaOctTree::Node root = collision.GetRootNode();
        if (!root.LineTest(line, filter, length))
            return false;
    }

    return true;
}

CRayCastResult
CGameCollision::RayDynamicIntersection(const CStateManager& mgr, TUniqueId& idOut, const zeus::CVector3f& pos,
                                       const zeus::CVector3f& dir, float length, const CMaterialFilter& filter,
                                       const rstl::reserved_vector<TUniqueId, 1024>& nearList)
{
    CRayCastResult ret;
    float bestT = length;
    if (bestT <= 0.f)
        bestT = 100000.f;

    for (TUniqueId id : nearList)
    {
        CEntity* ent = const_cast<CEntity*>(mgr.GetObjectById(id));
        if (TCastToPtr<CPhysicsActor> physActor = ent)
        {
            zeus::CTransform xf = physActor->GetPrimitiveTransform();
            const CCollisionPrimitive* prim = physActor->GetCollisionPrimitive();
            CRayCastResult res = prim->CastRay(pos, dir, bestT, filter, xf);
            if (!res.IsInvalid() && res.GetT() < bestT)
            {
                bestT = res.GetT();
                ret = res;
                idOut = physActor->GetUniqueId();
            }
        }
    }

    return ret;
}

bool CGameCollision::RayDynamicIntersectionBool(const CStateManager& mgr,
                                                const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                const CMaterialFilter& filter,
                                                const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                                const CActor* damagee, float length)
{
    if (length <= 0.f)
        length = 100000.f;

    for (TUniqueId id : nearList)
    {
        const CEntity* ent = mgr.GetObjectById(id);
        if (TCastToConstPtr<CPhysicsActor> physActor = ent)
        {
            if (damagee && physActor->GetUniqueId() == damagee->GetUniqueId())
                continue;
            zeus::CTransform xf = physActor->GetPrimitiveTransform();
            const CCollisionPrimitive* prim = physActor->GetCollisionPrimitive();
            CRayCastResult res = prim->CastRay(pos, dir, length, filter, xf);
            if (!res.IsInvalid())
                return false;
        }
    }

    return true;
}

CRayCastResult
CGameCollision::RayWorldIntersection(const CStateManager& mgr, TUniqueId& idOut, const zeus::CVector3f& pos,
                                     const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter,
                                     const rstl::reserved_vector<TUniqueId, 1024>& nearList)
{
    CRayCastResult staticRes = RayStaticIntersection(mgr, pos, dir, mag, filter);
    CRayCastResult dynamicRes = RayDynamicIntersection(mgr, idOut, pos, dir, mag, filter, nearList);

    if (!dynamicRes.IsInvalid() && staticRes.IsInvalid())
        return dynamicRes;
    else if (staticRes.GetT() >= dynamicRes.GetT())
        return dynamicRes;
    else
        return staticRes;
}

bool CGameCollision::RayStaticIntersectionArea(const CGameArea& area, const zeus::CVector3f& pos,
                                               const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter)
{
    if (mag <= 0.f)
        mag = 100000.f;
    CAreaOctTree::Node node = area.GetPostConstructed()->x0_collision->GetRootNode();
    zeus::CLine line(pos, dir);
    return node.LineTest(line, filter, mag);
}

void CGameCollision::BuildAreaCollisionCache(CStateManager& mgr, CAreaCollisionCache& cache)
{
    cache.ClearCache();
    for (const CGameArea& area : *mgr.GetWorld())
    {
        const CAreaOctTree& areaCollision = *area.GetPostConstructed()->x0_collision;
        CMetroidAreaCollider::COctreeLeafCache octreeCache(areaCollision);
        CMetroidAreaCollider::BuildOctreeLeafCache(areaCollision.GetRootNode(),
                                                   cache.GetCacheBounds(), octreeCache);
        cache.AddOctreeLeafCache(octreeCache);
    }
}

float CGameCollision::GetMinExtentForCollisionPrimitive(const CCollisionPrimitive& prim)
{
    if (prim.GetPrimType() == FOURCC('SPHR'))
    {
        const CCollidableSphere& sphere = static_cast<const CCollidableSphere&>(prim);
        return 2.f * sphere.GetSphere().radius;
    }
    else if (prim.GetPrimType() == FOURCC('AABX'))
    {
        const CCollidableAABox& aabx = static_cast<const CCollidableAABox&>(prim);
        zeus::CVector3f extent = aabx.GetAABB().max - aabx.GetAABB().min;
        float minExtent = std::min(extent.x, extent.y);
        minExtent = std::min(minExtent, extent.z);
        return minExtent;
    }
    else if (prim.GetPrimType() == FOURCC('ABSH'))
    {
        // Combination AABB / Sphere cut from game
    }
    return 1.f;
}

bool CGameCollision::DetectCollisionBoolean_Cached(CStateManager& mgr, CAreaCollisionCache& cache,
                                                   const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                                   const CMaterialFilter& filter,
                                                   const rstl::reserved_vector<TUniqueId, 1024>& nearList)
{
    if (!filter.GetExcludeList().HasMaterial(EMaterialTypes::ThirtyEight) &&
        DetectStaticCollisionBoolean_Cached(mgr, cache, prim, xf, filter))
        return true;
    if (DetectDynamicCollisionBoolean(prim, xf, nearList, mgr))
        return true;
    return false;
}

bool CGameCollision::DetectStaticCollisionBoolean(CStateManager& mgr, const CCollisionPrimitive& prim,
                                                  const zeus::CTransform& xf, const CMaterialFilter& filter)
{
    if (prim.GetPrimType() == FOURCC('OBTG'))
        return false;

    if (prim.GetPrimType() == FOURCC('AABX'))
    {
        zeus::CAABox aabb = prim.CalculateAABox(xf);
        for (const CGameArea& area : *mgr.GetWorld())
        {
            if (CMetroidAreaCollider::AABoxCollisionCheckBoolean(
                *area.GetPostConstructed()->x0_collision, aabb, filter))
                return true;
        }
    }
    else if (prim.GetPrimType() == FOURCC('SPHR'))
    {
        const CCollidableSphere& sphere = static_cast<const CCollidableSphere&>(prim);
        zeus::CAABox aabb = prim.CalculateAABox(xf);
        zeus::CSphere xfSphere = sphere.Transform(xf);
        for (const CGameArea& area : *mgr.GetWorld())
        {
            if (CMetroidAreaCollider::SphereCollisionCheckBoolean(
                *area.GetPostConstructed()->x0_collision, aabb, xfSphere, filter))
                return true;
        }
    }
    else if (prim.GetPrimType() == FOURCC('ABSH'))
    {
        // Combination AABB / Sphere cut from game
    }

    return false;
}

bool CGameCollision::DetectStaticCollisionBoolean_Cached(CStateManager& mgr, CAreaCollisionCache& cache,
                                                         const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                                         const CMaterialFilter& filter)
{
    if (prim.GetPrimType() == FOURCC('OBTG'))
        return false;

    zeus::CAABox aabb = prim.CalculateAABox(xf);
    if (!aabb.inside(cache.GetCacheBounds()))
    {
        zeus::CAABox newAABB(aabb.min - 0.2f, aabb.max + 0.2f);
        newAABB.accumulateBounds(cache.GetCacheBounds());
        cache.SetCacheBounds(newAABB);
        BuildAreaCollisionCache(mgr, cache);
    }

    if (cache.HasCacheOverflowed())
        return DetectStaticCollisionBoolean(mgr, prim, xf, filter);

    if (prim.GetPrimType() == FOURCC('AABX'))
    {
        for (CMetroidAreaCollider::COctreeLeafCache& leafCache : cache)
            if (CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(leafCache, aabb, filter))
                return true;
    }
    else if (prim.GetPrimType() == FOURCC('SPHR'))
    {
        const CCollidableSphere& sphere = static_cast<const CCollidableSphere&>(prim);
        zeus::CSphere xfSphere = sphere.Transform(xf);
        for (CMetroidAreaCollider::COctreeLeafCache& leafCache : cache)
            if (CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(leafCache, aabb, xfSphere, filter))
                return true;
    }
    else if (prim.GetPrimType() == FOURCC('ABSH'))
    {
        // Combination AABB / Sphere cut from game
    }

    return false;
}

bool CGameCollision::DetectDynamicCollisionBoolean(const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                                   const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                                   CStateManager& mgr)
{
    for (TUniqueId id : nearList)
    {
        if (TCastToPtr<CPhysicsActor> actor = mgr.ObjectById(id))
        {
            CInternalCollisionStructure::CPrimDesc p0(prim, CMaterialFilter::skPassEverything, xf);
            CInternalCollisionStructure::CPrimDesc p1(*actor->GetCollisionPrimitive(),
                                                      CMaterialFilter::skPassEverything,
                                                      actor->GetPrimitiveTransform());
            if (CCollisionPrimitive::CollideBoolean(p0, p1))
                return true;
        }
    }

    return false;
}

bool CGameCollision::DetectCollision_Cached(CStateManager& mgr, CAreaCollisionCache& cache,
                                            const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                            const CMaterialFilter& filter,
                                            const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                            TUniqueId& idOut, CCollisionInfoList& infoList)
{
    bool ret = false;
    if (!filter.GetExcludeList().HasMaterial(EMaterialTypes::ThirtyEight))
        if (DetectStaticCollision_Cached(mgr, cache, prim, xf, filter, infoList))
            ret = true;

    TUniqueId id = kInvalidUniqueId;
    if (DetectDynamicCollision(prim, xf, nearList, id, infoList, mgr))
    {
        ret = true;
        idOut = id;
    }

    return ret;
}

bool CGameCollision::DetectStaticCollision(CStateManager& mgr, const CCollisionPrimitive& prim,
                                           const zeus::CTransform& xf, const CMaterialFilter& filter,
                                           CCollisionInfoList& list)
{
    if (prim.GetPrimType() == FOURCC('OBTG'))
        return false;

    if (prim.GetPrimType() == FOURCC('AABX'))
    {
        zeus::CAABox aabb = prim.CalculateAABox(xf);
        for (const CGameArea& area : *mgr.GetWorld())
        {
            if (CMetroidAreaCollider::AABoxCollisionCheck(
                *area.GetPostConstructed()->x0_collision, aabb, filter,
                prim.GetMaterial(), list))
                return true;
        }
    }
    else if (prim.GetPrimType() == FOURCC('SPHR'))
    {
        const CCollidableSphere& sphere = static_cast<const CCollidableSphere&>(prim);
        zeus::CAABox aabb = prim.CalculateAABox(xf);
        zeus::CSphere xfSphere = sphere.Transform(xf);
        for (const CGameArea& area : *mgr.GetWorld())
        {
            if (CMetroidAreaCollider::SphereCollisionCheck(
                *area.GetPostConstructed()->x0_collision, aabb, xfSphere,
                prim.GetMaterial(), filter, list))
                return true;
        }
    }
    else if (prim.GetPrimType() == FOURCC('ABSH'))
    {
        // Combination AABB / Sphere cut from game
    }

    return false;
}

bool CGameCollision::DetectStaticCollision_Cached(CStateManager& mgr, CAreaCollisionCache& cache,
                                                  const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                                  const CMaterialFilter& filter, CCollisionInfoList& list)
{
    if (prim.GetPrimType() == FOURCC('OBTG'))
        return false;

    zeus::CAABox calcAABB = prim.CalculateAABox(xf);
    if (!calcAABB.inside(cache.GetCacheBounds()))
    {
        zeus::CAABox newAABB(calcAABB.min - 0.2f, calcAABB.max + 0.2f);
        newAABB.accumulateBounds(cache.GetCacheBounds());
        cache.SetCacheBounds(newAABB);
        BuildAreaCollisionCache(mgr, cache);
    }

    if (cache.HasCacheOverflowed())
        return DetectStaticCollision(mgr, prim, xf, filter, list);

    if (prim.GetPrimType() == FOURCC('AABX'))
    {
        for (CMetroidAreaCollider::COctreeLeafCache& leafCache : cache)
            if (CMetroidAreaCollider::AABoxCollisionCheck_Cached(leafCache, calcAABB, filter,
                                                                 prim.GetMaterial(), list))
                return true;
    }
    else if (prim.GetPrimType() == FOURCC('SPHR'))
    {
        const CCollidableSphere& sphere = static_cast<const CCollidableSphere&>(prim);
        zeus::CSphere xfSphere = sphere.Transform(xf);
        for (CMetroidAreaCollider::COctreeLeafCache& leafCache : cache)
            if (CMetroidAreaCollider::SphereCollisionCheck_Cached(leafCache, calcAABB, xfSphere,
                                                                  prim.GetMaterial(), filter, list))
                return true;
    }
    else if (prim.GetPrimType() == FOURCC('ABSH'))
    {
        // Combination AABB / Sphere cut from game
    }

    return false;
}

bool CGameCollision::DetectDynamicCollision(const CCollisionPrimitive& prim, const zeus::CTransform& xf,
                                            const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                            TUniqueId& idOut, CCollisionInfoList& list, CStateManager& mgr)
{
    for (TUniqueId id : nearList)
    {
        if (TCastToPtr<CPhysicsActor> actor = mgr.ObjectById(id))
        {
            CInternalCollisionStructure::CPrimDesc p0(prim, CMaterialFilter::skPassEverything, xf);
            CInternalCollisionStructure::CPrimDesc p1(*actor->GetCollisionPrimitive(),
                                                      CMaterialFilter::skPassEverything,
                                                      actor->GetPrimitiveTransform());
            if (CCollisionPrimitive::Collide(p0, p1, list))
            {
                idOut = actor->GetUniqueId();
                return true;
            }
        }
    }

    return false;
}

void CGameCollision::MakeCollisionCallbacks(CStateManager& mgr, CPhysicsActor& actor,
                                            TUniqueId id, const CCollisionInfoList& list)
{
    actor.CollidedWith(id, list, mgr);
    if (id != kInvalidUniqueId)
    {
        if (TCastToPtr<CPhysicsActor> actor = mgr.ObjectById(id))
        {
            CCollisionInfoList swappedList = list;
            for (CCollisionInfo& info : swappedList)
                info.Swap();
            actor->CollidedWith(actor->GetUniqueId(), list, mgr);
        }
    }
}

void CGameCollision::SendScriptMessages(CStateManager& mgr, CActor& a0, CActor* a1, const CCollisionInfoList& list)
{
    bool onFloor = false;
    bool platform = false;
    bool platform2 = false;
    for (const CCollisionInfo& info : list)
    {
        if (IsFloor(info.GetMaterialLeft(), info.GetNormalLeft()))
        {
            onFloor = true;
            if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Platform))
                platform = true;
            SendMaterialMessage(mgr, info.GetMaterialLeft(), a0);
        }
    }

    if (onFloor)
    {
        mgr.SendScriptMsg(&a0, kInvalidUniqueId, EScriptObjectMessage::OnFloor);
        if (platform)
        {
            if (TCastToPtr<CScriptPlatform> plat = a1)
            {
                mgr.SendScriptMsg(plat.GetPtr(), a0.GetUniqueId(),
                                  EScriptObjectMessage::AddPlatformRider);
            }
        }
        else if (a1)
        {
            if (TCastToPtr<CScriptPlatform> plat = a0)
            {
                for (const CCollisionInfo& info : list)
                {
                    if (IsFloor(info.GetMaterialRight(), info.GetNormalRight()))
                    {
                        if (info.GetMaterialRight().HasMaterial(EMaterialTypes::Platform))
                            platform2 = true;
                        SendMaterialMessage(mgr, info.GetMaterialLeft(), a0);
                    }
                }
                if (platform2)
                {
                    mgr.SendScriptMsg(plat.GetPtr(), a1->GetUniqueId(),
                                      EScriptObjectMessage::AddPlatformRider);
                }
            }
        }
    }
}

void CGameCollision::ResolveCollisions(CPhysicsActor& a0, CPhysicsActor* a1, const CCollisionInfoList& list)
{
    for (const CCollisionInfo& info : list)
    {
        CCollisionInfo infoCopy = info;
        float restitution = GetCoefficientOfRestitution(infoCopy) + a0.GetCoefficientOfRestitutionModifier();
        if (a1)
        {
            CollideWithDynamicBodyNoRot(a0, *a1, infoCopy, restitution, false);
        }
        else
        {
            CollideWithStaticBodyNoRot(a0, infoCopy.GetMaterialLeft(), infoCopy.GetMaterialRight(),
                                       infoCopy.GetNormalLeft(), restitution, false);
        }
    }
}

void CGameCollision::CollideWithDynamicBodyNoRot(CPhysicsActor& a0, CPhysicsActor& a1,
                                                 const CCollisionInfo& info, float restitution, bool)
{
    /* TODO: Finish */
}

void CGameCollision::CollideWithStaticBodyNoRot(CPhysicsActor& a0, const CMaterialList& m0, const CMaterialList& m1,
                                                const zeus::CUnitVector3f& normal, float restitution, bool b)
{
    zeus::CUnitVector3f useNorm = normal;
    if (b && m0.HasMaterial(EMaterialTypes::Player) && !m1.HasMaterial(EMaterialTypes::Floor))
        useNorm.z = 0.f;

    if (useNorm.canBeNormalized())
    {
        useNorm.normalize();
        float velNormDot = a0.GetVelocity().dot(useNorm);
        if (velNormDot < 0.0001f)
        {
            a0.ApplyImpulseWR(
                useNorm * CollisionImpulseFiniteVsInfinite(a0.GetMass(), velNormDot, restitution),
                zeus::CAxisAngle::sIdentity);
            a0.UseCollisionImpulses();
        }
        else if (velNormDot < 0.001f)
        {
            a0.ApplyImpulseWR(0.05f * a0.GetMass() * useNorm, zeus::CAxisAngle::sIdentity);
            a0.UseCollisionImpulses();
        }
    }
}

void CGameCollision::CollisionFailsafe(CStateManager& mgr, CAreaCollisionCache& cache,
                                       CPhysicsActor& actor, const CCollisionPrimitive& prim,
                                       const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                       float f1, u32 failsafeTicks)
{
    actor.MoveCollisionPrimitive(zeus::CVector3f::skZero);
    if (f1 > 0.5f)
        actor.SetNumTicksPartialUpdate(actor.GetNumTicksPartialUpdate() + 1);

    if (actor.GetNumTicksPartialUpdate() > 1 ||
        DetectCollisionBoolean_Cached(mgr, cache, prim, actor.GetPrimitiveTransform(),
                                      actor.GetMaterialFilter(), nearList))
    {
        actor.SetNumTicksPartialUpdate(0);
        actor.SetNumTicksStuck(actor.GetNumTicksStuck() + 1);
        if (actor.GetNumTicksStuck() < failsafeTicks)
            return;
        /* TODO: Finish */
    }
    else
    {
        actor.SetLastNonCollidingState(actor.GetMotionState());
        actor.SetNumTicksStuck(0);
    }
}
}
