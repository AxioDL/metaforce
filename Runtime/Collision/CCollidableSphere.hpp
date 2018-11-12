#pragma once

#include "CCollisionPrimitive.hpp"
#include "zeus/CSphere.hpp"

namespace urde
{
namespace Collide
{
bool Sphere_AABox(const CInternalCollisionStructure&, CCollisionInfoList&);
bool Sphere_AABox_Bool(const CInternalCollisionStructure&);
bool Sphere_Sphere(const CInternalCollisionStructure&, CCollisionInfoList&);
bool Sphere_Sphere_Bool(const CInternalCollisionStructure&);
}
class CCollidableSphere : public CCollisionPrimitive
{
    static const Type sType;
    static u32 sTableIndex;

    zeus::CSphere x10_sphere;

public:
    CCollidableSphere(const zeus::CSphere&, const CMaterialList&);

    const zeus::CSphere& GetSphere() const { return x10_sphere; }
    void SetSphereCenter(const zeus::CVector3f& center) { x10_sphere.position = center; }
    zeus::CSphere Transform(const zeus::CTransform& xf) const;

    virtual u32 GetTableIndex() const;
    virtual zeus::CAABox CalculateAABox(const zeus::CTransform&) const;
    virtual zeus::CAABox CalculateLocalAABox() const;
    virtual FourCC GetPrimType() const;
    virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const;

    static const Type& GetType() { return sType; }
    static void SetStaticTableIndex(u32 index) { sTableIndex = index; }
    static bool CollideMovingAABox(const CInternalCollisionStructure&, const zeus::CVector3f&, double&,
                                   CCollisionInfo&);
    static bool CollideMovingSphere(const CInternalCollisionStructure&, const zeus::CVector3f&, double&,
                                    CCollisionInfo&);
    static bool Sphere_AABox_Bool(const zeus::CSphere& sphere, const zeus::CAABox& aabb);
};
}

