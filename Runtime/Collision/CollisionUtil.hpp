#ifndef __URDE_COLLISIONUTIL_HPP__
#define __URDE_COLLISIONUTIL_HPP__

#include "GCNTypes.hpp"
#include "zeus/zeus.hpp"
#include "CMaterialList.hpp"

namespace urde
{
class CCollisionInfoList;
namespace CollisionUtil
{
bool LineIntersectsOBBox(const zeus::COBBox&, const zeus::CMRay&, float&);
u32 RayAABoxIntersection(const zeus::CMRay&, const zeus::CAABox&, const zeus::CVector3f&, float&);
u32 RaySphereIntersection_Double(const zeus::CSphere&, const zeus::CVector3f&, const zeus::CVector3f&, double&);
bool RaySphereIntersection(const zeus::CSphere& sphere, const zeus::CVector3f& pos, const zeus::CVector3f& dir, 
                           float mag, float& T, zeus::CVector3f& point);
void FilterOutBackfaces(const zeus::CVector3f& vec, const CCollisionInfoList& in, CCollisionInfoList& out);
void FilterByClosestNormal(const zeus::CVector3f& norm, const CCollisionInfoList& in, CCollisionInfoList& out);
bool AABoxAABoxIntersection(const zeus::CAABox& aabb0, const CMaterialList& list0,
                            const zeus::CAABox& aabb1, const CMaterialList& list1,
                            CCollisionInfoList& infoList);
bool AABoxAABoxIntersection(const zeus::CAABox& aabb0, const zeus::CAABox& aabb1);
}
}
#endif // __URDE_COLLISIONUTIL_HPP__
