#include "InternalColliders.hpp"
#include "CCollidableAABox.hpp"
#include "CCollidableCollisionSurface.hpp"
#include "CCollidableSphere.hpp"

namespace urde
{
namespace InternalColliders
{
void AddTypes()
{
    CCollisionPrimitive::InitAddType(CCollidableAABox::GetType());
    CCollisionPrimitive::InitAddType(CCollidableCollisionSurface::GetType());
    CCollisionPrimitive::InitAddType(CCollidableSphere::GetType());
}
}
}
