#include "CMaterialFilter.hpp"
#include "CGameCollision.hpp"
#include "CCollidableOBBTreeGroup.hpp"

namespace urde
{

void CGameCollision::InitCollision()
{
    CCollisionPrimitive::InitBeginTypes();
}

}
