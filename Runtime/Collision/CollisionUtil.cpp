#include "CollisionUtil.hpp"
#include "CCollisionInfo.hpp"
#include "CCollisionInfoList.hpp"

namespace urde::CollisionUtil {
bool LineIntersectsOBBox(const zeus::COBBox& obb, const zeus::CMRay& ray, float& d) {
  zeus::CVector3f norm;
  return RayAABoxIntersection(ray.getInvUnscaledTransformRay(obb.transform), {-obb.extents, obb.extents}, norm, d);
}

u32 RayAABoxIntersection(const zeus::CMRay& ray, const zeus::CAABox& aabb, float& tMin, float& tMax) {
  tMin = -999999.f;
  tMax = 999999.f;

  for (int i = 0; i < 3; ++i) {
    if (std::fabs(ray.dir[i]) < 0.00001f) {
      if (ray.start[i] < aabb.min[i] || ray.start[i] > aabb.max[i])
        return 0;
    } else {
      if (ray.dir[i] < 0.f) {
        float startToMax = aabb.max[i] - ray.start[i];
        float startToMin = aabb.min[i] - ray.start[i];
        float dirRecip = 1.f / ray.dir[i];
        if (startToMax < tMin * ray.dir[i])
          tMin = startToMax * dirRecip;
        if (startToMin > tMax * ray.dir[i])
          tMax = startToMin * dirRecip;
      } else {
        float startToMin = aabb.min[i] - ray.start[i];
        float startToMax = aabb.max[i] - ray.start[i];
        float dirRecip = 1.f / ray.dir[i];
        if (startToMin > tMin * ray.dir[i])
          tMin = startToMin * dirRecip;
        if (startToMax < tMax * ray.dir[i])
          tMax = startToMax * dirRecip;
      }
    }
  }

  return tMin <= tMax ? 2 : 0;
}

u32 RayAABoxIntersection(const zeus::CMRay& ray, const zeus::CAABox& aabb, zeus::CVector3f& norm, float& d) {
  int sign[] = {2, 2, 2};
  bool bad = true;
  zeus::CVector3f rayStart = ray.start;
  zeus::CVector3f rayDelta = ray.delta;
  zeus::CVector3f aabbMin = aabb.min;
  zeus::CVector3f aabbMax = aabb.max;

  zeus::CVector3f vec0 = {-1.f, -1.f, -1.f};
  zeus::CVector3f vec1;

  if (rayDelta.x() != 0.f && rayDelta.y() != 0.f && rayDelta.z() != 0.f) {
    for (int i = 0; i < 3; ++i) {
      if (rayStart[i] < aabbMin[i]) {
        sign[i] = 1;
        bad = false;
        vec0[i] = (aabbMin[i] - rayStart[i]) / rayDelta[i];
      } else if (rayStart[i] > aabbMax[i]) {
        sign[i] = 0;
        bad = false;
        vec0[i] = (aabbMax[i] - rayStart[i]) / rayDelta[i];
      }
    }

    if (bad) {
      d = 0.f;
      return 1;
    }
  } else {
    zeus::CVector3f end;
    for (int i = 0; i < 3; ++i) {
      if (rayStart[i] < aabbMin[i]) {
        sign[i] = 1;
        bad = false;
        end[i] = float(aabbMin[i]);
      } else if (rayStart[i] > aabbMax[i]) {
        sign[i] = 0;
        bad = false;
        end[i] = float(aabbMax[i]);
      }
    }

    if (bad) {
      d = 0.f;
      return 1;
    }

    for (int i = 0; i < 3; ++i)
      if (sign[i] != 2 && rayDelta[i] != 0.f)
        vec0[i] = (end[i] - rayStart[i]) / rayDelta[i];
  }

  float maxComp = vec0.x();
  int maxCompIdx = 0;
  if (maxComp < vec0.y()) {
    maxComp = vec0.y();
    maxCompIdx = 1;
  }
  if (maxComp < vec0.z()) {
    maxComp = vec0.z();
    maxCompIdx = 2;
  }

  if (maxComp < 0.f || maxComp > 1.f)
    return 0;

  for (int i = 0; i < 3; ++i) {
    if (maxCompIdx != i) {
      vec1[i] = maxComp * rayDelta[i] + rayStart[i];
      if (vec1[i] > aabbMax[i])
        return 0;
    }
  }

  d = maxComp;
  norm = zeus::CVector3f::skZero;
  norm[maxCompIdx] = (sign[maxCompIdx] == 1) ? -1.f : 1.f;
  return 2;
}

u32 RayAABoxIntersection_Double(const zeus::CMRay& ray, const zeus::CAABox& aabb, zeus::CVector3f& norm, double& d) {
  int sign[] = {2, 2, 2};
  bool bad = true;
  zeus::CVector3d rayStart = ray.start;
  zeus::CVector3d rayDelta = ray.delta;
  zeus::CVector3d aabbMin = aabb.min;
  zeus::CVector3d aabbMax = aabb.max;

  zeus::CVector3d vec0 = {-1.0, -1.0, -1.0};
  zeus::CVector3d vec1;

  if (rayDelta.x() != 0.0 && rayDelta.y() != 0.0 && rayDelta.z() != 0.0) {
    for (int i = 0; i < 3; ++i) {
      if (rayStart[i] < aabbMin[i]) {
        sign[i] = 1;
        bad = false;
        vec0[i] = (aabbMin[i] - rayStart[i]) / rayDelta[i];
      } else if (rayStart[i] > aabbMax[i]) {
        sign[i] = 0;
        bad = false;
        vec0[i] = (aabbMax[i] - rayStart[i]) / rayDelta[i];
      }
    }

    if (bad) {
      d = 0.0;
      return 1;
    }
  } else {
    zeus::CVector3d end;
    for (int i = 0; i < 3; ++i) {
      if (rayStart[i] < aabbMin[i]) {
        sign[i] = 1;
        bad = false;
        end[i] = double(aabbMin[i]);
      } else if (rayStart[i] > aabbMax[i]) {
        sign[i] = 0;
        bad = false;
        end[i] = double(aabbMax[i]);
      }
    }

    if (bad) {
      d = 0.0;
      return 1;
    }

    for (int i = 0; i < 3; ++i)
      if (sign[i] != 2 && rayDelta[i] != 0.0)
        vec0[i] = (end[i] - rayStart[i]) / rayDelta[i];
  }

  double maxComp = vec0.x();
  int maxCompIdx = 0;
  if (maxComp < vec0.y()) {
    maxComp = vec0.y();
    maxCompIdx = 1;
  }
  if (maxComp < vec0.z()) {
    maxComp = vec0.z();
    maxCompIdx = 2;
  }

  if (maxComp < 0.0 || maxComp > 1.0)
    return 0;

  for (int i = 0; i < 3; ++i) {
    if (maxCompIdx != i) {
      vec1[i] = maxComp * rayDelta[i] + rayStart[i];
      if (vec1[i] > aabbMax[i])
        return 0;
    }
  }

  d = maxComp;
  norm = zeus::CVector3f::skZero;
  norm[maxCompIdx] = (sign[maxCompIdx] == 1) ? -1.0 : 1.0;
  return 2;
}

bool RaySphereIntersection_Double(const zeus::CSphere& sphere, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                  double& T) {
  zeus::CVector3d sPosD = sphere.position;
  zeus::CVector3d posD = pos;
  zeus::CVector3d sphereToPos = posD - sPosD;
  double f30 = sphereToPos.dot(zeus::CVector3d(dir)) * 2.0;
  double f1 = f30 * f30 - 4.0 * (sphereToPos.magSquared() - sphere.radius * sphere.radius);
  if (f1 >= 0.0) {
    double intersectT = 0.5 * (-f30 - std::sqrt(f1));
    if (T == 0 || intersectT < T) {
      T = intersectT;
      return true;
    }
  }
  return false;
}

bool RaySphereIntersection(const zeus::CSphere& sphere, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                           float mag, float& T, zeus::CVector3f& point) {
  zeus::CVector3f rayToSphere = sphere.position - pos;
  float magSq = rayToSphere.magSquared();
  float dirDot = rayToSphere.dot(dir);
  float radSq = sphere.radius * sphere.radius;
  if (dirDot < 0.f && magSq > radSq)
    return false;
  float intersectSq = radSq - (magSq - dirDot * dirDot);
  if (intersectSq < 0.f)
    return false;
  T = magSq > radSq ? dirDot - std::sqrt(intersectSq) : dirDot + std::sqrt(intersectSq);
  if (T < mag || mag == 0.f) {
    point = pos + T * dir;
    return true;
  }
  return false;
}

bool RayTriangleIntersection_Double(const zeus::CVector3f& point, const zeus::CVector3f& dir,
                                    const zeus::CVector3f* verts, double& d) {
  zeus::CVector3d v0tov1 = verts[1] - verts[0];
  zeus::CVector3d v0tov2 = verts[2] - verts[0];
  zeus::CVector3d cross0 = zeus::CVector3d(dir).cross(v0tov2);
  double dot0 = v0tov1.dot(cross0);
  if (dot0 < DBL_EPSILON)
    return false;
  zeus::CVector3d v0toPoint = point - verts[0];
  double dot1 = v0toPoint.dot(cross0);
  if (dot1 < 0.0 || dot1 > dot0)
    return false;
  zeus::CVector3d cross1 = v0toPoint.cross(v0tov1);
  double dot2 = cross1.dot(dir);
  if (dot2 < 0.0 || dot1 + dot2 > dot0)
    return false;
  double final = 1.0 / dot0 * cross1.dot(v0tov2);
  if (final < 0.0 || final >= d)
    return false;
  d = final;
  return true;
}

bool RayTriangleIntersection(const zeus::CVector3f& point, const zeus::CVector3f& dir, const zeus::CVector3f* verts,
                             float& d) {
  zeus::CVector3f v0tov1 = verts[1] - verts[0];
  zeus::CVector3f v0tov2 = verts[2] - verts[0];
  zeus::CVector3f cross0 = dir.cross(v0tov2);
  float dot0 = v0tov1.dot(cross0);
  if (dot0 < DBL_EPSILON)
    return false;
  zeus::CVector3f v0toPoint = point - verts[0];
  float dot1 = v0toPoint.dot(cross0);
  if (dot1 < 0.f || dot1 > dot0)
    return false;
  zeus::CVector3f cross1 = v0toPoint.cross(v0tov1);
  float dot2 = cross1.dot(dir);
  if (dot2 < 0.f || dot1 + dot2 > dot0)
    return false;
  float final = 1.f / dot0 * cross1.dot(v0tov2);
  if (final < 0.f || final >= d)
    return false;
  d = final;
  return true;
}

void FilterOutBackfaces(const zeus::CVector3f& vec, const CCollisionInfoList& in, CCollisionInfoList& out) {
  if (vec.canBeNormalized()) {
    zeus::CVector3f norm = vec.normalized();
    for (const CCollisionInfo& info : in) {
      if (info.GetNormalLeft().dot(norm) < 0.001f)
        out.Add(info, false);
    }
  } else {
    out = in;
  }
}

void FilterByClosestNormal(const zeus::CVector3f& norm, const CCollisionInfoList& in, CCollisionInfoList& out) {
  float maxDot = -1.1f;
  int idx = -1;
  int i = 0;
  for (const CCollisionInfo& info : in) {
    float dot = info.GetNormalLeft().dot(norm);
    if (dot > maxDot) {
      maxDot = dot;
      idx = i;
    }
    ++i;
  }

  if (idx != -1)
    out.Add(in.GetItem(i), false);
}

static const zeus::CVector3f AABBNormalTable[] = {{-1.f, 0.f, 0.f}, {1.f, 0.f, 0.f},  {0.f, -1.f, 0.f},
                                                  {0.f, 1.f, 0.f},  {0.f, 0.f, -1.f}, {0.f, 0.f, 1.f}};

bool AABoxAABoxIntersection(const zeus::CAABox& aabb0, const CMaterialList& list0, const zeus::CAABox& aabb1,
                            const CMaterialList& list1, CCollisionInfoList& infoList) {
  zeus::CVector3f maxOfMin(std::max(aabb0.min.x(), aabb1.min.x()), std::max(aabb0.min.y(), aabb1.min.y()),
                           std::max(aabb0.min.z(), aabb1.min.z()));
  zeus::CVector3f minOfMax(std::min(aabb0.max.x(), aabb1.max.x()), std::min(aabb0.max.y(), aabb1.max.y()),
                           std::min(aabb0.max.z(), aabb1.max.z()));

  if (maxOfMin.x() >= minOfMax.x() || maxOfMin.y() >= minOfMax.y() || maxOfMin.z() >= minOfMax.z())
    return false;

  zeus::CAABox boolAABB(maxOfMin, minOfMax);

  int ineqFlags[] = {
      (aabb0.min.x() <= aabb1.min.x() ? 1 << 0 : 0) | (aabb0.min.x() <= aabb1.max.x() ? 1 << 1 : 0) |
          (aabb0.max.x() <= aabb1.min.x() ? 1 << 2 : 0) | (aabb0.max.x() <= aabb1.max.x() ? 1 << 3 : 0),
      (aabb0.min.y() <= aabb1.min.y() ? 1 << 0 : 0) | (aabb0.min.y() <= aabb1.max.y() ? 1 << 1 : 0) |
          (aabb0.max.y() <= aabb1.min.y() ? 1 << 2 : 0) | (aabb0.max.y() <= aabb1.max.y() ? 1 << 3 : 0),
      (aabb0.min.z() <= aabb1.min.z() ? 1 << 0 : 0) | (aabb0.min.z() <= aabb1.max.z() ? 1 << 1 : 0) |
          (aabb0.max.z() <= aabb1.min.z() ? 1 << 2 : 0) | (aabb0.max.z() <= aabb1.max.z() ? 1 << 3 : 0),
  };

  for (int i = 0; i < 3; ++i) {
    switch (ineqFlags[i]) {
    case 0x2: // aabb0.min <= aabb1.max
    {
      CCollisionInfo info(boolAABB, list0, list1, AABBNormalTable[i * 2 + 1], -AABBNormalTable[i * 2 + 1]);
      infoList.Add(info, false);
      break;
    }
    case 0xB: // aabb0.min <= aabb1.min && aabb0.max <= aabb1.min && aabb0.max <= aabb1.max
    {
      CCollisionInfo info(boolAABB, list0, list1, AABBNormalTable[i * 2], -AABBNormalTable[i * 2]);
      infoList.Add(info, false);
      break;
    }
    default:
      break;
    }
  }

  if (infoList.GetCount())
    return true;

  {
    CCollisionInfo info(boolAABB, list0, list1, AABBNormalTable[4], -AABBNormalTable[4]);
    infoList.Add(info, false);
  }

  {
    CCollisionInfo info(boolAABB, list0, list1, AABBNormalTable[5], -AABBNormalTable[5]);
    infoList.Add(info, false);
  }

  return true;
}

bool AABoxAABoxIntersection(const zeus::CAABox& aabb0, const zeus::CAABox& aabb1) { return aabb0.intersects(aabb1); }

/* http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox2.txt */
/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-Möller                              */
/* Function: int triBoxOverlap(float boxcenter[3],      */
/*          float boxhalfsize[3],float triverts[3][3]); */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-06-18: changed the order of the tests, faster */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/* Thanks to David Hunt for finding a ">="-bug!         */
/********************************************************/

#define FINDMINMAX(x0, x1, x2, min, max)                                                                               \
  min = max = x0;                                                                                                      \
  if (x1 < min)                                                                                                        \
    min = x1;                                                                                                          \
  if (x1 > max)                                                                                                        \
    max = x1;                                                                                                          \
  if (x2 < min)                                                                                                        \
    min = x2;                                                                                                          \
  if (x2 > max)                                                                                                        \
    max = x2;

static bool planeBoxOverlap(const zeus::CVector3f& normal, float d, const zeus::CVector3f& maxbox) {
  zeus::CVector3f vmin, vmax;
  for (int q = 0; q <= 2; q++) {
    if (normal[q] > 0.0f) {
      vmin[q] = -maxbox[q];
      vmax[q] = maxbox[q];
    } else {
      vmin[q] = maxbox[q];
      vmax[q] = -maxbox[q];
    }
  }
  if (normal.dot(vmin) + d > 0.0f)
    return false;
  if (normal.dot(vmax) + d >= 0.0f)
    return true;

  return false;
}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)                                                                                     \
  p0 = a * v0.y() - b * v0.z();                                                                                        \
  p2 = a * v2.y() - b * v2.z();                                                                                        \
  if (p0 < p2) {                                                                                                       \
    min = p0;                                                                                                          \
    max = p2;                                                                                                          \
  } else {                                                                                                             \
    min = p2;                                                                                                          \
    max = p0;                                                                                                          \
  }                                                                                                                    \
  rad = fa * boxhalfsize.y() + fb * boxhalfsize.z();                                                                   \
  if (min > rad || max < -rad)                                                                                         \
    return false;

#define AXISTEST_X2(a, b, fa, fb)                                                                                      \
  p0 = a * v0.y() - b * v0.z();                                                                                        \
  p1 = a * v1.y() - b * v1.z();                                                                                        \
  if (p0 < p1) {                                                                                                       \
    min = p0;                                                                                                          \
    max = p1;                                                                                                          \
  } else {                                                                                                             \
    min = p1;                                                                                                          \
    max = p0;                                                                                                          \
  }                                                                                                                    \
  rad = fa * boxhalfsize.y() + fb * boxhalfsize.z();                                                                   \
  if (min > rad || max < -rad)                                                                                         \
    return false;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)                                                                                     \
  p0 = -a * v0.x() + b * v0.z();                                                                                       \
  p2 = -a * v2.x() + b * v2.z();                                                                                       \
  if (p0 < p2) {                                                                                                       \
    min = p0;                                                                                                          \
    max = p2;                                                                                                          \
  } else {                                                                                                             \
    min = p2;                                                                                                          \
    max = p0;                                                                                                          \
  }                                                                                                                    \
  rad = fa * boxhalfsize.x() + fb * boxhalfsize.z();                                                                   \
  if (min > rad || max < -rad)                                                                                         \
    return false;

#define AXISTEST_Y1(a, b, fa, fb)                                                                                      \
  p0 = -a * v0.x() + b * v0.z();                                                                                       \
  p1 = -a * v1.x() + b * v1.z();                                                                                       \
  if (p0 < p1) {                                                                                                       \
    min = p0;                                                                                                          \
    max = p1;                                                                                                          \
  } else {                                                                                                             \
    min = p1;                                                                                                          \
    max = p0;                                                                                                          \
  }                                                                                                                    \
  rad = fa * boxhalfsize.x() + fb * boxhalfsize.z();                                                                   \
  if (min > rad || max < -rad)                                                                                         \
    return false;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)                                                                                     \
  p1 = a * v1.x() - b * v1.y();                                                                                        \
  p2 = a * v2.x() - b * v2.y();                                                                                        \
  if (p2 < p1) {                                                                                                       \
    min = p2;                                                                                                          \
    max = p1;                                                                                                          \
  } else {                                                                                                             \
    min = p1;                                                                                                          \
    max = p2;                                                                                                          \
  }                                                                                                                    \
  rad = fa * boxhalfsize.x() + fb * boxhalfsize.y();                                                                   \
  if (min > rad || max < -rad)                                                                                         \
    return false;

#define AXISTEST_Z0(a, b, fa, fb)                                                                                      \
  p0 = a * v0.x() - b * v0.y();                                                                                        \
  p1 = a * v1.x() - b * v1.y();                                                                                        \
  if (p0 < p1) {                                                                                                       \
    min = p0;                                                                                                          \
    max = p1;                                                                                                          \
  } else {                                                                                                             \
    min = p1;                                                                                                          \
    max = p0;                                                                                                          \
  }                                                                                                                    \
  rad = fa * boxhalfsize.x() + fb * boxhalfsize.y();                                                                   \
  if (min > rad || max < -rad)                                                                                         \
    return false;

bool TriBoxOverlap(const zeus::CVector3f& boxcenter, const zeus::CVector3f& boxhalfsize,
                   const zeus::CVector3f& trivert0, const zeus::CVector3f& trivert1, const zeus::CVector3f& trivert2) {

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
  zeus::CVector3f v0, v1, v2;
  float min, max, d, p0, p1, p2, rad, fex, fey, fez;
  zeus::CVector3f normal, e0, e1, e2;

  /* This is the fastest branch on Sun */
  /* move everything so that the boxcenter is in (0,0,0) */
  v0 = trivert0 - boxcenter;
  v1 = trivert1 - boxcenter;
  v2 = trivert2 - boxcenter;

  /* compute triangle edges */
  e0 = v1 - v0; /* tri edge 0 */
  e1 = v2 - v1; /* tri edge 1 */
  e2 = v0 - v2; /* tri edge 2 */

  /* Bullet 3:  */
  /*  test the 9 tests first (this was faster) */
  fex = std::fabs(e0.x());
  fey = std::fabs(e0.y());
  fez = std::fabs(e0.z());
  AXISTEST_X01(e0.z(), e0.y(), fez, fey);
  AXISTEST_Y02(e0.z(), e0.x(), fez, fex);
  AXISTEST_Z12(e0.y(), e0.x(), fey, fex);

  fex = std::fabs(e1.x());
  fey = std::fabs(e1.y());
  fez = std::fabs(e1.z());
  AXISTEST_X01(e1.z(), e1.y(), fez, fey);
  AXISTEST_Y02(e1.z(), e1.x(), fez, fex);
  AXISTEST_Z0(e1.y(), e1.x(), fey, fex);

  fex = std::fabs(e2.x());
  fey = std::fabs(e2.y());
  fez = std::fabs(e2.z());
  AXISTEST_X2(e2.z(), e2.y(), fez, fey);
  AXISTEST_Y1(e2.z(), e2.x(), fez, fex);
  AXISTEST_Z12(e2.y(), e2.x(), fey, fex);

  /* Bullet 1: */
  /*  first test overlap in the {x,y,z}-directions */
  /*  find min, max of the triangle each direction, and test for overlap in */
  /*  that direction -- this is equivalent to testing a minimal AABB around */
  /*  the triangle against the AABB */

  /* test in X-direction */
  FINDMINMAX(v0.x(), v1.x(), v2.x(), min, max);
  if (min > boxhalfsize.x() || max < -boxhalfsize.x())
    return false;

  /* test in Y-direction */
  FINDMINMAX(v0.y(), v1.y(), v2.y(), min, max);
  if (min > boxhalfsize.y() || max < -boxhalfsize.y())
    return false;

  /* test in Z-direction */
  FINDMINMAX(v0.z(), v1.z(), v2.z(), min, max);
  if (min > boxhalfsize.z() || max < -boxhalfsize.z())
    return false;

  /* Bullet 2: */
  /*  test if the box intersects the plane of the triangle */
  /*  compute plane equation of triangle: normal*x+d=0 */
  normal = e0.cross(e1);
  d = -normal.dot(v0); /* plane eq: normal.x+d=0 */
  if (!planeBoxOverlap(normal, d, boxhalfsize))
    return false;

  return true; /* box and triangle overlaps */
}

double TriPointSqrDist(const zeus::CVector3f& point, const zeus::CVector3f& trivert0, const zeus::CVector3f& trivert1,
                       const zeus::CVector3f& trivert2, float* baryX, float* baryY) {
  zeus::CVector3d A = trivert0 - point;
  zeus::CVector3d B = trivert1 - trivert0;
  zeus::CVector3d C = trivert2 - trivert0;

  double bMag = B.magSquared();
  double cMag = C.magSquared();
  double bDotC = B.dot(C);
  double aDotB = A.dot(B);
  double aDotC = A.dot(C);
  double ret = A.magSquared();

  double rej = std::fabs(bMag * cMag - bDotC * bDotC);
  double retB = bDotC * aDotC - cMag * aDotB;
  double retA = bDotC * aDotB - bMag * aDotC;

  if (retB + retA <= rej) {
    if (retB < 0.0) {
      if (retA < 0.0) {
        if (aDotB < 0.0) {
          retA = 0.0;
          if (-aDotB >= bMag) {
            retB = 1.0;
            ret += 2.0 * aDotB + bMag;
          } else {
            retB = -aDotB / bMag;
            ret += aDotB * retB;
          }
        } else {
          retB = 0.0;
          if (aDotC >= 0.0) {
            retA = 0.0;
          } else if (-aDotC >= cMag) {
            retA = 1.0;
            ret += 2.0 * aDotC + cMag;
          } else {
            retA = -aDotC / cMag;
            ret += aDotC * retA;
          }
        }
      } else {
        retB = 0.0;
        if (aDotC >= 0.0) {
          retA = 0.0;
        } else if (-aDotC >= cMag) {
          retA = 1.0;
          ret += 2.0 * aDotC + cMag;
        } else {
          retA = -aDotC / cMag;
          ret += aDotC * retA;
        }
      }
    } else if (retA < 0.0) {
      retA = 0.0;
      if (aDotB >= 0.0) {
        retB = 0.0;
      } else if (-aDotB >= bMag) {
        retB = 1.0;
        ret += 2.0 * aDotB + bMag;
      } else {
        retB = -aDotB / bMag;
        ret += aDotB * retB;
      }
    } else {
      float f3 = 1.0 / rej;
      retA *= f3;
      retB *= f3;
      ret += retB * (2.0 * aDotB + (bMag * retB + bDotC * retA)) + retA * (2.0 * aDotC + (bDotC * retB + cMag * retA));
    }
  } else if (retB < 0.0) {
    retB = bDotC + aDotB;
    retA = cMag + aDotC;
    if (retA > retB) {
      retA -= retB;
      retB = bMag - 2.0 * bDotC;
      retB += cMag;
      if (retA >= retB) {
        retB = 1.0;
        retA = 0.0;
        ret += 2.0 * aDotB + bMag;
      } else {
        retB = retA / retB;
        retA = 1.0 - retB;
        ret +=
            retB * (2.0 * aDotB + (bMag * retB + bDotC * retA)) + retA * (2.0 * aDotC + (bDotC * retB + cMag * retA));
      }
    } else {
      retB = 0.0;
      if (retA <= 0.0) {
        retA = 1.0;
        ret += 2.0 * aDotC + cMag;
      } else if (aDotC >= 0.0) {
        retA = 0.0;
      } else {
        retA = -aDotC / cMag;
        ret += aDotC * retA;
      }
    }
  } else {
    if (retA < 0.0) {
      retB = bDotC + aDotC;
      retA = bMag + aDotB;
      if (retA > retB) {
        retA -= retB;
        retB = bMag - 2.0 * bDotC;
        retB += cMag;
        if (retA >= retB) {
          retA = 1.0;
          retB = 0.0;
          ret += 2.0 * aDotC + cMag;
        } else {
          retA /= retB;
          retB = 1.0 - retA;
          ret +=
              retB * (2.0 * aDotB + (bMag * retB + bDotC * retA)) + retA * (2.0 * aDotC + (bDotC * retB + cMag * retA));
        }
      } else {
        retA = 0.0;
        if (retA <= 0.0) {
          retB = 1.0;
          ret += 2.0 * aDotB + bMag;
        } else if (aDotB >= 0.0) {
          retB = 0.0;
        } else {
          retB = -aDotB / bMag;
          ret += aDotB * retB;
        }
      }
    } else {
      retB = cMag + aDotC;
      retB -= bDotC;
      retA = retB - aDotB;
      if (retA <= 0.0) {
        retB = 0.0;
        retA = 1.0;
        ret += 2.0 * aDotC + cMag;
      } else {
        retB = bMag - 2.0 * bDotC;
        retB += cMag;
        if (retA >= retB) {
          retB = 1.0;
          retA = 0.0;
          ret += 2.0 * aDotB + bMag;
        } else {
          retB = retA / retB;
          retA = 1.0 - retB;
          ret +=
              retB * (2.0 * aDotB + (bMag * retB + bDotC * retA)) + retA * (2.0 * aDotC + (bDotC * retB + cMag * retA));
        }
      }
    }
  }

  if (baryX)
    *baryX = retA;
  if (baryY)
    *baryY = retB;

  return ret;
}

bool TriSphereOverlap(const zeus::CSphere& sphere, const zeus::CVector3f& trivert0, const zeus::CVector3f& trivert1,
                      const zeus::CVector3f& trivert2) {
  return TriPointSqrDist(sphere.position, trivert0, trivert1, trivert2, nullptr, nullptr) <=
         sphere.radius * sphere.radius;
}

bool TriSphereIntersection(const zeus::CSphere& sphere, const zeus::CVector3f& trivert0,
                           const zeus::CVector3f& trivert1, const zeus::CVector3f& trivert2, zeus::CVector3f& point,
                           zeus::CVector3f& normal) {
  float baryX, baryY;
  if (TriPointSqrDist(sphere.position, trivert0, trivert1, trivert2, &baryX, &baryY) > sphere.radius * sphere.radius)
    return false;

  zeus::CVector3f barys(baryX, baryY, 1.f - (baryX + baryY));
  point = zeus::baryToWorld(trivert2, trivert1, trivert0, barys);

  if (baryX == 0.f || baryX == 1.f || baryY == 0.f || baryY == 1.f || barys.z() == 0.f || barys.z() == 1.f)
    normal = -sphere.getSurfaceNormal(point);
  else
    normal = (trivert1 - trivert0).cross(trivert2 - trivert0).normalized();

  return true;
}

bool BoxLineTest(const zeus::CAABox& aabb, const zeus::CVector3f& point, const zeus::CVector3f& dir, float& tMin,
                 float& tMax, int& axis, bool& sign) {
  tMin = -999999.f;
  tMax = 999999.f;

  for (int i = 0; i < 3; ++i) {
    if (dir[i] == 0.f)
      if (point[i] < aabb.min[i] || point[i] > aabb.max[i])
        return false;

    float dirRecip = 1.f / dir[i];
    float tmpMin, tmpMax;
    if (dir[i] < 0.f) {
      tmpMin = (aabb.max[i] - point[i]) * dirRecip;
      tmpMax = (aabb.min[i] - point[i]) * dirRecip;
    } else {
      tmpMin = (aabb.min[i] - point[i]) * dirRecip;
      tmpMax = (aabb.max[i] - point[i]) * dirRecip;
    }

    if (tmpMin > tMin) {
      sign = dir[i] < 0.f;
      axis = i;
      tMin = tmpMin;
    }

    if (tmpMax < tMax)
      tMax = tmpMax;
  }

  return tMin <= tMax;
}

bool LineCircleIntersection2d(const zeus::CVector3f& point, const zeus::CVector3f& dir, const zeus::CSphere& sphere,
                              int axis1, int axis2, float& d) {
  zeus::CVector3f delta = sphere.position - point;
  zeus::CVector2f deltaVec(delta[axis1], delta[axis2]);
  zeus::CVector2f dirVec(dir[axis1], dir[axis2]);

  float dirVecMag = dirVec.magnitude();
  if (dirVecMag < FLT_EPSILON)
    return false;

  float deltaVecDot = deltaVec.dot(dirVec / dirVecMag);
  float deltaVecMagSq = deltaVec.magSquared();

  float sphereRadSq = sphere.radius * sphere.radius;
  if (deltaVecDot < 0.f && deltaVecMagSq > sphereRadSq)
    return false;

  float tSq = sphereRadSq - (deltaVecMagSq - deltaVecDot * deltaVecDot);
  if (tSq < 0.f)
    return false;

  float t = std::sqrt(tSq);

  d = (deltaVecMagSq > sphereRadSq) ? deltaVecDot - t : deltaVecDot + t;
  d /= dirVecMag;

  return true;
}

bool MovingSphereAABox(const zeus::CSphere& sphere, const zeus::CAABox& aabb, const zeus::CVector3f& dir, double& dOut,
                       zeus::CVector3f& point, zeus::CVector3f& normal) {
  zeus::CAABox expAABB(aabb.min - sphere.radius, aabb.max + sphere.radius);
  float tMin, tMax;
  int axis;
  bool sign;
  if (!BoxLineTest(expAABB, sphere.position, dir, tMin, tMax, axis, sign))
    return false;

  point = sphere.position + tMin * dir;

  int nextAxis1 = (axis + 1) % 3; // r0
  int nextAxis2 = (axis + 2) % 3; // r5

  bool inMin1 = point[nextAxis1] >= aabb.min[nextAxis1]; // r6
  bool inMax1 = point[nextAxis1] <= aabb.max[nextAxis1]; // r8
  bool inBounds1 = inMin1 && inMax1;                     // r9
  bool inMin2 = point[nextAxis2] >= aabb.min[nextAxis2]; // r7
  bool inMax2 = point[nextAxis2] <= aabb.max[nextAxis2]; // r4
  bool inBounds2 = inMin2 && inMax2;                     // r8

  if (inBounds1 && inBounds2) {
    if (tMin < 0.f || tMin > dOut)
      return false;
    normal[axis] = sign ? 1.f : -1.f;
    dOut = tMin;
    point -= normal * sphere.radius;
    return true;
  } else if (!inBounds1 && !inBounds2) {
    int pointFlags = (1 << axis) * sign | (1 << nextAxis1) * inMin1 | (1 << nextAxis2) * inMin2;
    zeus::CVector3f aabbPoint = aabb.getPoint(pointFlags);
    float d;
    if (CollisionUtil::RaySphereIntersection(zeus::CSphere(aabbPoint, sphere.radius), sphere.position, dir, dOut, d,
                                             point)) {
      int useAxis = -1;
      for (int i = 0; i < 3; ++i) {
        if ((pointFlags & (1 << i)) ? aabbPoint[i] > point[i] : aabbPoint[i] < point[i]) {
          useAxis = i;
          break;
        }
      }

      if (useAxis == -1) {
        normal = (point - aabbPoint).normalized();
        point -= sphere.radius * normal;
        return true;
      }

      int useAxisNext1 = (useAxis + 1) % 3;
      int useAxisNext2 = (useAxis + 2) % 3;

      float d;
      if (CollisionUtil::LineCircleIntersection2d(sphere.position, dir, zeus::CSphere(aabbPoint, sphere.radius),
                                                  useAxisNext1, useAxisNext2, d) &&
          d > 0.f && d < dOut) {
        if (point[useAxis] > aabb.max[useAxis]) {
          int useAxisBit = 1 << useAxis;
          if (pointFlags & useAxisBit)
            return false;

          zeus::CVector3f aabbPoint1 = aabb.getPoint(pointFlags | useAxisBit);
          if (CollisionUtil::RaySphereIntersection(zeus::CSphere(aabbPoint1, sphere.radius), sphere.position, dir, dOut,
                                                   d, point)) {
            dOut = d;
            normal = (point - aabbPoint1).normalized();
            point -= normal * sphere.radius;
            return true;
          } else {
            return false;
          }
        } else if (point[useAxis] < aabb.min[useAxis]) {
          int useAxisBit = 1 << useAxis;
          if (!(pointFlags & useAxisBit))
            return false;

          zeus::CVector3f aabbPoint1 = aabb.getPoint(pointFlags ^ useAxisBit);
          if (CollisionUtil::RaySphereIntersection(zeus::CSphere(aabbPoint1, sphere.radius), sphere.position, dir, dOut,
                                                   d, point)) {
            dOut = d;
            normal = (point - aabbPoint1).normalized();
            point -= normal * sphere.radius;
            return true;
          } else {
            return false;
          }
        } else {
          normal = point - aabbPoint;
          normal.normalize();
          point -= normal * sphere.radius;
          return true;
        }
      }
    } else {
      int reverseCount = 0;
      float dMin = 1.0e10f;
      int minAxis = 0;
      for (int i = 0; i < 3; ++i) {
        if (std::fabs(dir[i]) > FLT_EPSILON) {
          bool pointMax = pointFlags & (1 << i);
          if (pointMax != dir[i] > 0.f) {
            ++reverseCount;
            float d = 1.f / dir[i] * ((pointMax ? aabb.max[i] : aabb.min[i]) - sphere.position[i]);
            if (d < 0.f)
              return false;
            if (d < dMin) {
              dMin = d;
              minAxis = i;
            }
          }
        }
      }

      if (reverseCount < 2)
        return false;

      int useAxisNext1 = (minAxis + 1) % 3;
      int useAxisNext2 = (minAxis + 2) % 3;
      float d;
      if (CollisionUtil::LineCircleIntersection2d(sphere.position, dir, zeus::CSphere(aabbPoint, sphere.radius),
                                                  useAxisNext1, useAxisNext2, d) &&
          d > 0.f && d < dOut) {
        point = sphere.position + d * dir;
        if (point[minAxis] > aabb.max[minAxis])
          return false;
        if (point[minAxis] < aabb.min[minAxis])
          return false;

        dOut = d;
        normal = point - aabbPoint;
        normal.normalize();
        point -= sphere.radius * normal;
        return true;
      } else {
        return false;
      }
    }
  }

  bool useNextAxis1 = inBounds1 ? nextAxis2 : nextAxis1;
  bool useNextAxis2 = inBounds1 ? nextAxis1 : nextAxis2;

  int pointFlags = ((1 << int(useNextAxis1)) * (inBounds1 ? inMin2 : inMin1)) | ((1 << axis) * sign);
  zeus::CVector3f aabbPoint2 = aabb.getPoint(pointFlags);
  float d;
  if (LineCircleIntersection2d(sphere.position, dir, zeus::CSphere(aabbPoint2, sphere.radius), axis, useNextAxis1, d) &&
      d > 0.f && d < dOut) {
    point = sphere.position + d * dir;
    if (point[useNextAxis2] > aabb.max[useNextAxis2]) {
      zeus::CVector3f aabbPoint3 = aabb.getPoint(pointFlags | (1 << int(useNextAxis2)));
      if (point[useNextAxis2] < expAABB.max[useNextAxis2]) {
        if (RaySphereIntersection(zeus::CSphere(aabbPoint3, sphere.radius), sphere.position, dir, dOut, d, point)) {
          dOut = d;
          normal = (point - aabbPoint3).normalized();
          point -= sphere.radius * normal;
          return true;
        }
      }
      return false;
    } else if (point[useNextAxis2] < aabb.min[useNextAxis2]) {
      if (point[useNextAxis2] > expAABB.min[useNextAxis2]) {
        if (RaySphereIntersection(zeus::CSphere(aabbPoint2, sphere.radius), sphere.position, dir, dOut, d, point)) {
          dOut = d;
          normal = (point - aabbPoint2).normalized();
          point -= sphere.radius * normal;
          return true;
        }
      }
      return false;
    } else {
      dOut = d;
      normal = point - aabbPoint2;
      normal.normalize();
      point -= sphere.radius * normal;
      return true;
    }
  }

  return false;
}

bool AABox_AABox_Moving(const zeus::CAABox& aabb0, const zeus::CAABox& aabb1, const zeus::CVector3f& dir, double& d,
                        zeus::CVector3f& point, zeus::CVector3f& normal) {
  zeus::CVector3d vecMin(-FLT_MAX);
  zeus::CVector3d vecMax(FLT_MAX);

  for (int i = 0; i < 3; ++i) {
    if (std::fabs(dir[i]) < FLT_EPSILON) {
      if (aabb0.min[i] >= aabb1.min[i] && aabb0.min[i] <= aabb1.max[i])
        continue;
      if (aabb0.max[i] >= aabb1.min[i] && aabb0.max[i] <= aabb1.max[i])
        continue;
      if (aabb0.min[i] < aabb1.min[i] && aabb0.max[i] > aabb1.max[i])
        continue;
      return false;
    } else {
      if (aabb0.max[i] < aabb1.min[i] && dir[i] > 0.f)
        vecMin[i] = (aabb1.min[i] - aabb0.max[i]) / dir[i];
      else if (aabb1.max[i] < aabb0.min[i] && dir[i] < 0.f)
        vecMin[i] = (aabb1.max[i] - aabb0.min[i]) / dir[i];
      else if (aabb1.max[i] > aabb0.min[i] && dir[i] < 0.f)
        vecMin[i] = (aabb1.max[i] - aabb0.min[i]) / dir[i];
      else if (aabb0.max[i] > aabb1.min[i] && dir[i] > 0.f)
        vecMin[i] = (aabb1.min[i] - aabb0.max[i]) / dir[i];

      if (aabb1.max[i] > aabb0.min[i] && dir[i] > 0.f)
        vecMax[i] = (aabb1.max[i] - aabb0.min[i]) / dir[i];
      else if (aabb0.max[i] > aabb1.min[i] && dir[i] < 0.f)
        vecMax[i] = (aabb1.min[i] - aabb0.max[i]) / dir[i];
      else if (aabb0.max[i] < aabb1.min[i] && dir[i] < 0.f)
        vecMax[i] = (aabb1.min[i] - aabb0.max[i]) / dir[i];
      else if (aabb1.max[i] < aabb0.min[i] && dir[i] > 0.f)
        vecMax[i] = (aabb1.max[i] - aabb0.min[i]) / dir[i];
    }
  }

  int maxAxis = 0;
  if (vecMin[1] > vecMin[0])
    maxAxis = 1;
  if (vecMin[2] > vecMin[maxAxis])
    maxAxis = 2;

  double minMax = std::min(std::min(vecMax[2], vecMax[1]), vecMax[0]);
  if (vecMin[maxAxis] > minMax)
    return false;
  d = vecMin[maxAxis];

  normal = zeus::CVector3f::skZero;
  normal[maxAxis] = dir[maxAxis] > 0.f ? -1.f : 1.f;

  for (int i = 0; i < 3; ++i)
    point[i] = dir[i] > 0.f ? aabb0.max[i] : aabb0.min[i];

  point += float(d) * dir;
  return true;
}

void AddAverageToFront(const CCollisionInfoList& in, CCollisionInfoList& out) {
  if (in.GetCount() > 1) {
    zeus::CVector3f pointAccum, normAccum;

    for (const CCollisionInfo& info : in) {
      pointAccum += info.GetPoint();
      normAccum += info.GetNormalLeft();
    }

    if (normAccum.canBeNormalized()) {
      out.Add(CCollisionInfo(pointAccum / float(in.GetCount()), in.GetItem(0).GetMaterialRight(),
                             in.GetItem(0).GetMaterialLeft(), normAccum.normalized()),
              false);
    }
  }

  for (const CCollisionInfo& info : in)
    out.Add(info, false);
}
} // namespace urde::CollisionUtil
