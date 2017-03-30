#include "CGameCollision.hpp"
#include "CCollidableOBBTreeGroup.hpp"
#include "CMaterialFilter.hpp"
#include "CMaterialList.hpp"
#include "World/CActor.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "World/CWorld.hpp"

namespace urde
{

float CollisionImpulseFiniteVsInfinite(float f1, float f2, float f3) { return f1 * ((1.f / f3) * f2); }

float CollisionImpulseFiniteVsFinite(float f1, float f2, float f3, float f4)
{
    return (-(1.f + f4) * f3) / ((1.f / f1) + (1.f / f2));
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

void CGameCollision::Move(CStateManager& mgr, CPhysicsActor& actor, float dt, const rstl::reserved_vector<TUniqueId, 1024>*)
{

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
        msg = EScriptObjectMessage::InternalMessage05;
    else if (mat.HasMaterial(EMaterialTypes::MudSlow))
        msg = EScriptObjectMessage::InternalMessage06;
    else
        msg = EScriptObjectMessage::InternalMessage07;

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
}
