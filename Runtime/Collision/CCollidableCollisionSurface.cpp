#include "CCollidableCollisionSurface.hpp"

namespace urde
{
const CCollisionPrimitive::Type& CCollidableCollisionSurface::GetType()
{
    return sType;
}

void CCollidableCollisionSurface::SetStaticTableIndex(u32 index)
{
    sTableIndex = index;
}
}
