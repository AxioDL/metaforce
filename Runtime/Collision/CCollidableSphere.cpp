#include "CCollidableSphere.hpp"

namespace urde
{
const CCollisionPrimitive::Type CCollidableSphere::sType(CCollidableSphere::SetStaticTableIndex, "CCollidableSphere");
u32 CCollidableSphere::sTableIndex = -1;

u32 CCollidableSphere::GetTableIndex() const
{
    return sTableIndex;
}

zeus::CAABox CCollidableSphere::CalculateAABox(const zeus::CTransform &) const
{
    return {};
}

zeus::CAABox CCollidableSphere::CalculateLocalAABox() const
{
    return {};
}

FourCC CCollidableSphere::GetPrimType() const
{
    return SBIG('SPHR');
}

CRayCastResult CCollidableSphere::CastRayInternal(const CInternalRayCastStructure &) const
{
    return {};
}

const CCollisionPrimitive::Type& CCollidableSphere::GetType()
{
    return sType;
}

void CCollidableSphere::SetStaticTableIndex(u32 index)
{
    sTableIndex = index;
}

}
