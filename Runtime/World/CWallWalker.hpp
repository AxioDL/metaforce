#pragma once

#include "CPatterned.hpp"
#include "Collision/CCollisionSurface.hpp"
#include "Collision/CCollidableSphere.hpp"

namespace urde
{
class CWallWalker : public CPatterned
{
public:
    enum class EWalkerType
    {
        Parasite = 0,
        Oculus = 1,
        Geemer = 2,
        IceZoomer = 3,
        Seedling = 4
    };
protected:
    CCollisionSurface x568_ = CCollisionSurface(zeus::CVector3f(),
        zeus::CVector3f::skForward,
        zeus::CVector3f::skRight, -1);
    CCollidableSphere x590_colSphere;
    float x5b0_collisionCloseMargin;
    float x5b4_;
    float x5b8_ = 0.f;
    float x5bc_ = 0.f;
    float x5c0_advanceWpRadius;
    float x5c4_;
    float x5c8_bendingHackWeight = 0.f;
    s32 x5cc_bendingHackAnim;
    EWalkerType x5d0_walkerType;
    s16 x5d4_ = 0;
    bool x5d6_24_ : 1;
    bool x5d6_25_ : 1;
    bool x5d6_26_ : 1;
    bool x5d6_27_ : 1;
    bool x5d6_28_addBendingWeight : 1;
    bool x5d6_29_applyBendingHack : 1;
    static zeus::CVector3f ProjectVectorToPlane(const zeus::CVector3f& v0, const zeus::CVector3f& v1)
    {
        return v0 - v1 * v0.dot(v1);
    }
    void OrientToSurfaceNormal(const zeus::CVector3f& normal, float clampAngle);
    void GotoNextWaypoint(CStateManager& mgr);
public:
    CWallWalker(ECharacter, TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&,
                CModelData&&, const CPatternedInfo&, EMovementType, EColliderType, EBodyType,
                const CActorParameters&, float, float, EKnockBackVariant, float, EWalkerType wType, float, bool);

    void PreThink(float, CStateManager&);
    void Think(float, CStateManager&);
    void Render(const CStateManager&) const;
    const CCollisionPrimitive* GetCollisionPrimitive() const { return &x590_colSphere; }
    void UpdateWPDestination(CStateManager&);
};
}
