#include "CGameCollision.hpp"
#include "CCollidableOBBTreeGroup.hpp"
#include "CMaterialFilter.hpp"
#include "CMaterialList.hpp"
#include "World/CActor.hpp"
#include "CStateManager.hpp"

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
}
