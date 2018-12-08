#pragma once

#include "GCNTypes.hpp"
#include "zeus/zeus.hpp"
#include "CMaterialList.hpp"

namespace urde {
class CCollisionInfoList;
namespace CollisionUtil {
bool LineIntersectsOBBox(const zeus::COBBox&, const zeus::CMRay&, float&);
u32 RayAABoxIntersection(const zeus::CMRay&, const zeus::CAABox&, float&, float&);
u32 RayAABoxIntersection(const zeus::CMRay&, const zeus::CAABox&, zeus::CVector3f&, float&);
u32 RayAABoxIntersection_Double(const zeus::CMRay&, const zeus::CAABox&, zeus::CVector3f&, double&);
bool RaySphereIntersection_Double(const zeus::CSphere&, const zeus::CVector3f&, const zeus::CVector3f&, double&);
bool RaySphereIntersection(const zeus::CSphere& sphere, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                           float mag, float& T, zeus::CVector3f& point);
bool RayTriangleIntersection_Double(const zeus::CVector3f& point, const zeus::CVector3f& dir,
                                    const zeus::CVector3f* verts, double& d);
bool RayTriangleIntersection(const zeus::CVector3f& point, const zeus::CVector3f& dir, const zeus::CVector3f* verts,
                             float& d);
void FilterOutBackfaces(const zeus::CVector3f& vec, const CCollisionInfoList& in, CCollisionInfoList& out);
void FilterByClosestNormal(const zeus::CVector3f& norm, const CCollisionInfoList& in, CCollisionInfoList& out);
bool AABoxAABoxIntersection(const zeus::CAABox& aabb0, const CMaterialList& list0, const zeus::CAABox& aabb1,
                            const CMaterialList& list1, CCollisionInfoList& infoList);
bool AABoxAABoxIntersection(const zeus::CAABox& aabb0, const zeus::CAABox& aabb1);
bool TriBoxOverlap(const zeus::CVector3f& boxcenter, const zeus::CVector3f& boxhalfsize,
                   const zeus::CVector3f& trivert0, const zeus::CVector3f& trivert1, const zeus::CVector3f& trivert2);
double TriPointSqrDist(const zeus::CVector3f& point, const zeus::CVector3f& trivert0, const zeus::CVector3f& trivert1,
                       const zeus::CVector3f& trivert2, float* baryX, float* baryY);
bool TriSphereOverlap(const zeus::CSphere& sphere, const zeus::CVector3f& trivert0, const zeus::CVector3f& trivert1,
                      const zeus::CVector3f& trivert2);
bool TriSphereIntersection(const zeus::CSphere& sphere, const zeus::CVector3f& trivert0,
                           const zeus::CVector3f& trivert1, const zeus::CVector3f& trivert2, zeus::CVector3f& point,
                           zeus::CVector3f& normal);
bool BoxLineTest(const zeus::CAABox& aabb, const zeus::CVector3f& point, const zeus::CVector3f& dir, float& tMin,
                 float& tMax, int& axis, bool& sign);
bool LineCircleIntersection2d(const zeus::CVector3f& point, const zeus::CVector3f& dir, const zeus::CSphere& sphere,
                              int axis1, int axis2, float& d);
bool MovingSphereAABox(const zeus::CSphere& sphere, const zeus::CAABox& aabb, const zeus::CVector3f& dir, double& d,
                       zeus::CVector3f& point, zeus::CVector3f& normal);
bool AABox_AABox_Moving(const zeus::CAABox& aabb0, const zeus::CAABox& aabb1, const zeus::CVector3f& dir, double& d,
                        zeus::CVector3f& point, zeus::CVector3f& normal);
void AddAverageToFront(const CCollisionInfoList& in, CCollisionInfoList& out);
} // namespace CollisionUtil
} // namespace urde
