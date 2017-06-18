#include "CCollidableSphere.hpp"

namespace urde
{
const CCollisionPrimitive::Type CCollidableSphere::sType(CCollidableSphere::SetStaticTableIndex, "CCollidableSphere");
u32 CCollidableSphere::sTableIndex = -1;

namespace Collide
{

bool Sphere_AABox(const CInternalCollisionStructure&, CCollisionInfoList&) { return false; }

bool Sphere_AABox_Bool(const CInternalCollisionStructure&)
{
    return false;
}

bool Sphere_Sphere(const CInternalCollisionStructure&, CCollisionInfoList&) { return false; }

bool Sphere_Sphere_Bool(const CInternalCollisionStructure&) { return false; }
}

CCollidableSphere::CCollidableSphere(const zeus::CSphere& sphere, const CMaterialList& list)
: CCollisionPrimitive(list), x10_sphere(sphere)
{
}

const zeus::CSphere& CCollidableSphere::GetSphere() const { return x10_sphere; }

void CCollidableSphere::SetSphereCenter(const zeus::CVector3f&)
{

}

zeus::CSphere CCollidableSphere::Transform(const zeus::CTransform& xf) const
{
    return zeus::CSphere(xf * x10_sphere.position, x10_sphere.radius);
}

u32 CCollidableSphere::GetTableIndex() const { return sTableIndex; }

zeus::CAABox CCollidableSphere::CalculateAABox(const zeus::CTransform&) const { return {}; }

zeus::CAABox CCollidableSphere::CalculateLocalAABox() const { return {}; }

FourCC CCollidableSphere::GetPrimType() const { return SBIG('SPHR'); }

CRayCastResult CCollidableSphere::CastRayInternal(const CInternalRayCastStructure&) const { return {}; }

const CCollisionPrimitive::Type& CCollidableSphere::GetType() { return sType; }

void CCollidableSphere::SetStaticTableIndex(u32 index) { sTableIndex = index; }

bool CCollidableSphere::CollideMovingAABox(const CInternalCollisionStructure&, const zeus::CVector3f&, double&,
                                           CCollisionInfo&)
{
    return false;
}

bool CCollidableSphere::CollideMovingSphere(const CInternalCollisionStructure&, const zeus::CVector3f&, double&,
                                            CCollisionInfo&)
{
    return false;
}

bool CCollidableSphere::Sphere_AABox_Bool(const zeus::CSphere& sphere, const zeus::CAABox& aabb)
{
    float mag = 0.f;

    for (int i=0 ; i<3 ; ++i)
    {
        if (sphere.position[i] < aabb.min[i])
        {
            float tmp = sphere.position[i] - aabb.min[i];
            mag += tmp * tmp;
        }
        else if (sphere.position[i] > aabb.max[i])
        {
            float tmp = sphere.position[i] - aabb.max[i];
            mag += tmp * tmp;
        }
    }

    return mag <= sphere.radius * sphere.radius;
}
}
