#include "Runtime/Collision/CCollidableCollisionSurface.hpp"

namespace metaforce {
constexpr CCollisionPrimitive::Type sType(CCollidableCollisionSurface::SetStaticTableIndex,
                                          "CCollidableCollisionSurface");

const CCollisionPrimitive::Type& CCollidableCollisionSurface::GetType() { return sType; }

void CCollidableCollisionSurface::SetStaticTableIndex(u32 index) { sTableIndex = index; }
} // namespace metaforce
