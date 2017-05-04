#ifndef __URDE_COLLISIONUTIL_HPP__
#define __URDE_COLLISIONUTIL_HPP__

#include "GCNTypes.hpp"
#include "zeus/zeus.hpp"

namespace urde
{
namespace CollisionUtil
{
bool LineIntersectsOBBox(const zeus::COBBox&, const zeus::CMRay&, float&);
u32 RayAABoxIntersection(const zeus::CMRay&, const zeus::CAABox&, const zeus::CVector3f&, float&);
u32 RaySphereIntersection_Double(const zeus::CSphere&, const zeus::CVector3f&, const zeus::CVector3f&, double&);
bool RaySphereIntersection(const zeus::CSphere&, const zeus::CVector3f&, const zeus::CVector3f&, float, float&, zeus::CVector3f&);
}
}
#endif // __URDE_COLLISIONUTIL_HPP__
