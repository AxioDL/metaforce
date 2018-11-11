#pragma once

#include "CPatterned.hpp"
#include "Collision/CCollisionSurface.hpp"
#include "Collision/CCollidableSphere.hpp"

namespace urde
{
class CWallWalker : public CPatterned
{
protected:
    CCollisionSurface x568_ = CCollisionSurface(zeus::CVector3f(),
        zeus::CVector3f::skForward,
        zeus::CVector3f::skRight, -1);
    CCollidableSphere x590_colSphere;
    float x5b0_;
    float x5b4_;
    float x5b8_ = 0.f;
    float x5bc_ = 0.f;
    float x5c0_advanceWpRadius;
    float x5c4_;
    float x5c8_bendingHackWeight = 0.f;
    s32 x5cc_bendingHackAnim;
    u32 x5d0_;
    s16 x5d4_ = 0;
    bool x5d6_24_ : 1;
    bool x5d6_25_ : 1;
    bool x5d6_26_ : 1;
    bool x5d6_27_ : 1;
    bool x5d6_28_addBendingWeight : 1;
    bool x5d6_29_applyBendingHack : 1;
public:
    CWallWalker(ECharacter, TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&,
                CModelData&&, const CPatternedInfo&, EMovementType, EColliderType, EBodyType,
                const CActorParameters&, float, float, EKnockBackVariant, float, u32, float, bool);

    void PreThink(float, CStateManager&);
    void Think(float, CStateManager&);
    const CCollisionPrimitive* GetCollisionPrimitive() const { return &x590_colSphere; }
    void UpdateWPDestination(CStateManager&);
};
}
