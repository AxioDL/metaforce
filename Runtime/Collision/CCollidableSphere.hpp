#ifndef __URDE_CCOLLIDALBESPHERE_HPP
#define __URDE_CCOLLIDALBESPHERE_HPP

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

    const zeus::CSphere& GetSphere() const;
    void SetSphereCenter(const zeus::CVector3f&);

    virtual u32 GetTableIndex() const;
    virtual zeus::CAABox CalculateAABox(const zeus::CTransform&) const;
    virtual zeus::CAABox CalculateLocalAABox() const;
    virtual FourCC GetPrimType() const;
    virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const;

    static const Type& GetType();
    static void SetStaticTableIndex(u32 index);
    static bool CollideMovingAABox(const CInternalCollisionStructure&, const zeus::CVector3f&, double&,
                                   CCollisionInfo&);
    static bool CollideMovingSphere(const CInternalCollisionStructure&, const zeus::CVector3f&, double&,
                                    CCollisionInfo&);
};
}

#endif // __URDE_CCOLLIDALBESPHERE_HPP
