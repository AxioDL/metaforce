#include "CCollisionPrimitive.hpp"
#include "CInternalRayCastStructure.hpp"
#include "CMaterialFilter.hpp"

namespace urde
{
CCollisionPrimitive::CCollisionPrimitive(const CMaterialList& list)
    : x8_material(list)
{
}

void CCollisionPrimitive::SetMaterial(const CMaterialList& material)
{
    x8_material = material;
}

const CMaterialList&CCollisionPrimitive::GetMaterial() const
{
    return x8_material;

}

CRayCastResult CCollisionPrimitive::CastRay(const zeus::CVector3f& start, const zeus::CVector3f& end, float d, const
                                            CMaterialFilter& filter, const zeus::CTransform& xf) const
{
    return CastRayInternal(CInternalRayCastStructure(start, end, d, xf, filter));
}

}
