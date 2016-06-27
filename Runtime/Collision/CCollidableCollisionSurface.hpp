#ifndef __URDE_COLLIDABLECOLLISIONSURFACE_HPP__
#define __URDE_COLLIDABLECOLLISIONSURFACE_HPP__

#include "CCollisionPrimitive.hpp"

namespace urde
{
class CCollidableCollisionSurface
{
    static const CCollisionPrimitive::Type sType;
    static u32 sTableIndex;
public:
    static const CCollisionPrimitive::Type& GetType();
    static void SetStaticTableIndex(u32 index);
};
}
#endif // __URDE_COLLIDABLECOLLISIONSURFACE_HPP__
