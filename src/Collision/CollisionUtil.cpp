#include "Collision/CollisionUtil.hpp"

#include "Collision/CCollisionInfo.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/NormalTable.hpp"

#include "Collision/CMRay.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CVector3d.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "rstl/math.hpp"

static inline float float_min(float a, float b) { return a < b ? a : b; }

static inline float float_max(float a, float b) { return a > b ? a : b; }

inline int spanIntersection(float minA, float maxA, float minB, float maxB) {
  const bool minMin = minA <= minB;
  const bool minMax = minA <= maxB;
  const bool maxMax = maxA <= maxB;
  const bool maxMin = maxA <= minB;
  return (maxMax << 3) | (maxMin << 2) | (minMax << 1) | minMin;
}

static inline int planeBoxOverlap(float* normal, float d, const CVector3f& maxbox) {
  float vmin[3];
  float vmax[3];
  for (int q = 0; q < 3; ++q) {
    if (normal[q] > 0.f) {
      vmin[q] = -maxbox[q];
      vmax[q] = maxbox[q];
    } else {
      vmin[q] = maxbox[q];
      vmax[q] = -maxbox[q];
    }
  }
  if ((normal[0] * vmin[0] + normal[1] * vmin[1] + normal[2] * vmin[2]) + d > 0.f) {
    return 0;
  }
  if ((normal[0] * vmax[0] + normal[1] * vmax[1] + normal[2] * vmax[2]) + d >= 0.f) {
    return 1;
  }
  return 0;
}

namespace CollisionUtil {
bool RayPlaneIntersection(const CVector3f& from, const CVector3f& to, const CPlane& plane,
                          CVector3f& point) {
  CVector3f delta = to - from;
  const CVector3f planeNorm(plane.GetNormal());
  if (CMath::AbsF(CVector3f::Dot(delta.AsNormalized(), planeNorm)) < 0.01f) {
    return false;
  }

  float tmp =
      -(CVector3f::Dot(from, planeNorm) - plane.GetConstant()) / CVector3f::Dot(delta, planeNorm);
  if (tmp < -0.f || tmp > 1.0001f) {
    return false;
  }

  point = from + tmp * delta;
  return true;
}

bool RaySphereIntersection(const CSphere& sphere, const CVector3f& pos, const CVector3f& dir,
                           float mag, float& T, CVector3f& point) {
  const CVector3f rayToSphere = sphere.GetCenter() - pos;
  const float dirDot = CVector3f::Dot(rayToSphere, dir);
  const float magSq = rayToSphere.MagSquared();
  const float radSq = sphere.GetRadius() * sphere.GetRadius();
  float intersectSq = (dirDot * dirDot);

  if (dirDot < 0.f && magSq > radSq) {
    return false;
  }
  intersectSq = radSq - (magSq - intersectSq);

  if (intersectSq < 0.f) {
    return false;
  }
  intersectSq = CMath::SqrtF(intersectSq);
  T = magSq > radSq ? dirDot - intersectSq : dirDot + intersectSq;

  if (T < mag || mag == 0.f) {
    point = pos + T * dir;
    return true;
  }

  return false;
}

bool RaySphereIntersection_Double(const CSphere& sphere, const CVector3f& pos, const CVector3f& dir,
                                  double& T) {
  bool result = false;
  CVector3d sphereToPosCopy = CVector3d(pos) - CVector3d(sphere.GetCenter());
  CVector3d dirDCopy = CVector3d(dir);
  const double rad = sphere.GetRadius();
  const double f30 = CVector3d::Dot(sphereToPosCopy, dirDCopy) * 2.0;
  const double f1 = f30 * f30 - 4.0 * (sphereToPosCopy.MagSquared() - rad * rad);

  if (f1 >= 0.0) {
    const double root = CMath::SqrtD(f1);
    const double intersectT = 0.5 * (-f30 - root);
    if (T == 0.0 || intersectT < T) {
      T = intersectT;
      result = true;
    }
  }

  return result;
}

int RayAABoxIntersection(const CMRay& ray, const CAABox& box, CVector3f& normal,
                         float& penetration) {
  int sign[3] = {2, 2, 2};
  float maxT[3] = {-1.f, -1.f, -1.f};
  const float zero = 0.f;
  CVector3f coord(CVector3f::Zero());
  float* const coordVals = &coord[0];
  bool inside = true;

  if (zero != ray.GetDelta().GetX() && zero != ray.GetDelta().GetY() &&
      zero != ray.GetDelta().GetZ()) {
    float startX = ray.GetStart().GetX();
    float minX = box.GetMinPoint().GetX();
    if (startX < minX) {
      sign[0] = 1;
      inside = false;
      maxT[0] = (minX - startX) / ray.GetDelta().GetX();
    } else {
      float maxX = box.GetMaxPoint().GetX();
      if (startX > maxX) {
        sign[0] = 0;
        inside = false;
        maxT[0] = (maxX - startX) / ray.GetDelta().GetX();
      }
    }

    float startY = ray.GetStart().GetY();
    float minY = box.GetMinPoint().GetY();
    if (startY < minY) {
      sign[1] = 1;
      inside = false;
      maxT[1] = (minY - startY) / ray.GetDelta().GetY();
    } else {
      float maxY = box.GetMaxPoint().GetY();
      if (startY > maxY) {
        sign[1] = 0;
        inside = false;
        maxT[1] = (maxY - startY) / ray.GetDelta().GetY();
      }
    }

    float startZ = ray.GetStart().GetZ();
    float minZ = box.GetMinPoint().GetZ();
    if (startZ < minZ) {
      sign[2] = 1;
      inside = false;
      maxT[2] = (minZ - startZ) / ray.GetDelta().GetZ();
    } else {
      float maxZ = box.GetMaxPoint().GetZ();
      if (startZ > maxZ) {
        sign[2] = 0;
        inside = false;
        maxT[2] = (maxZ - startZ) / ray.GetDelta().GetZ();
      }
    }

    if (inside) {
      penetration = 0.f;
      return 1;
    }
  } else {
    float candidateX;
    float candidateY;
    float candidateZ;
    float startX = ray.GetStart().GetX();
    float minX = box.GetMinPoint().GetX();
    if (startX < minX) {
      sign[0] = 1;
      inside = false;
      candidateX = minX;
    } else {
      float maxX = box.GetMaxPoint().GetX();
      if (startX > maxX) {
        sign[0] = 0;
        inside = false;
        candidateX = maxX;
      }
    }

    float startY = ray.GetStart().GetY();
    float minY = box.GetMinPoint().GetY();
    if (startY < minY) {
      sign[1] = 1;
      inside = false;
      candidateY = minY;
    } else {
      float maxY = box.GetMaxPoint().GetY();
      if (startY > maxY) {
        sign[1] = 0;
        inside = false;
        candidateY = maxY;
      }
    }

    float startZ = ray.GetStart().GetZ();
    float minZ = box.GetMinPoint().GetZ();
    if (startZ < minZ) {
      sign[2] = 1;
      inside = false;
      candidateZ = minZ;
    } else {
      float maxZ = box.GetMaxPoint().GetZ();
      if (startZ > maxZ) {
        sign[2] = 0;
        inside = false;
        candidateZ = maxZ;
      }
    }

    if (inside) {
      penetration = 0.f;
      return 1;
    }

    if (sign[0] != 2 && zero != ray.GetDelta().GetX()) {
      maxT[0] = (candidateX - startX) / ray.GetDelta().GetX();
    }
    if (sign[1] != 2 && zero != ray.GetDelta().GetY()) {
      maxT[1] = (candidateY - startY) / ray.GetDelta().GetY();
    }
    if (sign[2] != 2 && zero != ray.GetDelta().GetZ()) {
      maxT[2] = (candidateZ - startZ) / ray.GetDelta().GetZ();
    }
  }

  int whichPlane = 0;
  float maxCoord = maxT[0];
  if (maxCoord < maxT[1]) {
    whichPlane = 1;
    maxCoord = maxT[1];
  }
  if (maxCoord < maxT[2]) {
    whichPlane = 2;
    maxCoord = maxT[2];
  }

  if (maxCoord < zero) {
    return 0;
  }
  if (maxCoord > 1.f) {
    return 0;
  }

  if (whichPlane != 0) {
    coordVals[0] = maxCoord * ray.GetDelta().GetX() + ray.GetStart().GetX();
    if (coordVals[0] < box.GetMinPoint().GetX() || coordVals[0] > box.GetMaxPoint().GetX()) {
      return 0;
    }
  }
  if (whichPlane != 1) {
    coordVals[1] = maxCoord * ray.GetDelta().GetY() + ray.GetStart().GetY();
    if (coordVals[1] < box.GetMinPoint().GetY() || coordVals[1] > box.GetMaxPoint().GetY()) {
      return 0;
    }
  }
  if (whichPlane != 2) {
    coordVals[2] = maxCoord * ray.GetDelta().GetZ() + ray.GetStart().GetZ();
    if (coordVals[2] < box.GetMinPoint().GetZ() || coordVals[2] > box.GetMaxPoint().GetZ()) {
      return 0;
    }
  }

  penetration = maxCoord;
  normal = CVector3f::Zero();
  normal[whichPlane] = sign[whichPlane] == 1 ? -1.f : 1.f;
  return 2;
}

int RayAABoxIntersection_Double(const CMRay& ray, const CAABox& box, CVector3f& normal,
                                double& penetration) {
  bool inside = true;
  int sign[3] = {2, 2, 2};
  CVector3d rayStart(ray.GetStart());
  CVector3d rayDelta(ray.GetDelta());
  double maxT[3] = {-1.0, -1.0, -1.0};
  CVector3d boxMin(box.GetMinPoint());
  CVector3d boxMax(box.GetMaxPoint());
  CVector3d coord(0.0, 0.0, 0.0);

  if (0.0 != rayDelta.GetX() && 0.0 != rayDelta.GetY() && 0.0 != rayDelta.GetZ()) {
    if (rayStart.GetX() < boxMin[0]) {
      sign[0] = 1;
      inside = false;
      maxT[0] = (boxMin[0] - rayStart.GetX()) / rayDelta.GetX();
    } else {
      if (rayStart.GetX() > boxMax.GetX()) {
        sign[0] = 0;
        inside = false;
        maxT[0] = (boxMax.GetX() - rayStart.GetX()) / rayDelta.GetX();
      }
    }

    if (rayStart.GetY() < boxMin[1]) {
      sign[1] = 1;
      inside = false;
      maxT[1] = (boxMin[1] - rayStart.GetY()) / rayDelta.GetY();
    } else {
      if (rayStart.GetY() > boxMax.GetY()) {
        sign[1] = 0;
        inside = false;
        maxT[1] = (boxMax.GetY() - rayStart.GetY()) / rayDelta.GetY();
      }
    }

    if (rayStart.GetZ() < boxMin[2]) {
      sign[2] = 1;
      inside = false;
      maxT[2] = (boxMin[2] - rayStart.GetZ()) / rayDelta.GetZ();
    } else {
      if (rayStart.GetZ() > boxMax.GetZ()) {
        sign[2] = 0;
        inside = false;
        maxT[2] = (boxMax.GetZ() - rayStart.GetZ()) / rayDelta.GetZ();
      }
    }

    if (inside) {
      penetration = 0.0;
      return 1;
    }
  } else {
    double candidateX;
    double candidateY;
    double candidateZ;

    if (rayStart.GetX() < boxMin[0]) {
      sign[0] = 1;
      inside = false;
      candidateX = boxMin[0];
    } else {
      if (rayStart.GetX() > boxMax.GetX()) {
        sign[0] = 0;
        inside = false;
        candidateX = boxMax.GetX();
      }
    }

    if (rayStart.GetY() < boxMin[1]) {
      sign[1] = 1;
      inside = false;
      candidateY = boxMin[1];
    } else {
      if (rayStart.GetY() > boxMax.GetY()) {
        sign[1] = 0;
        inside = false;
        candidateY = boxMax.GetY();
      }
    }

    if (rayStart.GetZ() < boxMin[2]) {
      sign[2] = 1;
      inside = false;
      candidateZ = boxMin[2];
    } else {
      if (rayStart.GetZ() > boxMax.GetZ()) {
        sign[2] = 0;
        inside = false;
        candidateZ = boxMax.GetZ();
      }
    }

    if (inside) {
      penetration = 0.0;
      return 1;
    }

    if (sign[0] != 2 && 0.0 != rayDelta.GetX()) {
      maxT[0] = (candidateX - rayStart.GetX()) / rayDelta.GetX();
    }
    if (sign[1] != 2 && 0.0 != rayDelta.GetY()) {
      maxT[1] = (candidateY - rayStart.GetY()) / rayDelta.GetY();
    }
    if (sign[2] != 2 && 0.0 != rayDelta.GetZ()) {
      maxT[2] = (candidateZ - rayStart.GetZ()) / rayDelta.GetZ();
    }
  }

  int whichPlane = 0;
  double maxCoord = maxT[0];
  if (maxCoord < maxT[1]) {
    whichPlane = 1;
    maxCoord = maxT[1];
  }
  if (maxCoord < maxT[2]) {
    whichPlane = 2;
    maxCoord = maxT[2];
  }

  if (maxCoord < 0.0) {
    return 0;
  }
  if (maxCoord > 1.0) {
    return 0;
  }

  if (whichPlane != 0) {
    coord[0] = maxCoord * rayDelta.GetX() + rayStart.GetX();
    if (coord[0] < boxMin[0] || coord[0] > boxMax.GetX()) {
      return 0;
    }
  }

  if (whichPlane != 1) {
    coord[1] = maxCoord * rayDelta.GetY() + rayStart.GetY();
    if (coord[1] < boxMin[1] || coord[1] > boxMax.GetY()) {
      return 0;
    }
  }
  if (whichPlane != 2) {
    coord[2] = maxCoord * rayDelta.GetZ() + rayStart.GetZ();
    if (coord[2] < boxMin[2] || coord[2] > boxMax.GetZ()) {
      return 0;
    }
  }

  penetration = maxCoord;
  normal = CVector3f::Zero();
  normal[whichPlane] = sign[whichPlane] == 1 ? -1.f : 1.f;
  return 2;
}

int RayAABoxIntersection(const CMRay& ray, const CAABox& box, float& tMin, float& tMax) {
  const CVector3f start = ray.GetStart();
  const CVector3f dir = ray.GetDirection();
  const CVector3f& boxMin = box.GetMinPoint();
  const CVector3f& boxMax = box.GetMaxPoint();
  const float* dirPtr = &dir[kDX];
  const float* startPtr = &start[kDX];
  const float* minPtr = &boxMin[kDX];
  const float* maxPtr = &boxMax[kDX];
  tMin = -999999.f;
  tMax = 999999.f;

  for (int i = 0; i < 3; ++i) {
    const float boxMinI = *minPtr;
    const float startI = *startPtr;
    const float dirI = *dirPtr;
    const float boxMaxI = *maxPtr;

    if (close_enough(dirI, 0.f)) {
      if (startI < boxMinI || startI > boxMaxI) {
        return 0;
      }
    } else {
      if (dirI < 0.f) {
        const float dirRecip = 1.f / dirI;
        float maxMinusStart = boxMaxI - startI;
        float minMinusStart = boxMinI - startI;
        if (maxMinusStart < tMin * dirI) {
          tMin = maxMinusStart * dirRecip;
        }
        if (minMinusStart > tMax * dirI) {
          tMax = minMinusStart * dirRecip;
        }
      } else {
        const float dirRecip = 1.f / dirI;
        float minMinusStart = boxMinI - startI;
        float maxMinusStart = boxMaxI - startI;
        if (minMinusStart > tMin * dirI) {
          tMin = minMinusStart * dirRecip;
        }
        if (maxMinusStart < tMax * dirI) {
          tMax = maxMinusStart * dirRecip;
        }
      }
    }

    ++dirPtr;
    ++startPtr;
    ++minPtr;
    ++maxPtr;
  }

  if (tMin <= tMax) {
    return 2;
  }
  return 0;
}

bool AABoxAABoxIntersection(const CAABox& left, const CAABox& right) {
  const float minX = float_max(left.GetMinPoint().GetX(), right.GetMinPoint().GetX());
  const float minY = float_max(left.GetMinPoint().GetY(), right.GetMinPoint().GetY());
  const float minZ = float_max(left.GetMinPoint().GetZ(), right.GetMinPoint().GetZ());
  const float maxX = float_min(left.GetMaxPoint().GetX(), right.GetMaxPoint().GetX());
  const float maxY = float_min(left.GetMaxPoint().GetY(), right.GetMaxPoint().GetY());
  const float maxZ = float_min(left.GetMaxPoint().GetZ(), right.GetMaxPoint().GetZ());
  if (minX >= maxX || minY >= maxY || minZ >= maxZ) {
    return false;
  }
  return true;
}

bool AABoxAABoxIntersection(const CAABox& left, const CMaterialList& leftFilter,
                            const CAABox& right, const CMaterialList& rightFilter,
                            CCollisionInfoList& list) {
  int flags[3];
  CVector3f overlapMin(float_max(left.GetMinPoint().GetX(), right.GetMinPoint().GetX()),
                       float_max(left.GetMinPoint().GetY(), right.GetMinPoint().GetY()),
                       float_max(left.GetMinPoint().GetZ(), right.GetMinPoint().GetZ()));
  CVector3f overlapMax(float_min(left.GetMaxPoint().GetX(), right.GetMaxPoint().GetX()),
                       float_min(left.GetMaxPoint().GetY(), right.GetMaxPoint().GetY()),
                       float_min(left.GetMaxPoint().GetZ(), right.GetMaxPoint().GetZ()));
  if (overlapMin.GetX() >= overlapMax.GetX() || overlapMin.GetY() >= overlapMax.GetY() ||
      overlapMin.GetZ() >= overlapMax.GetZ()) {
    return false;
  }

  CAABox overlapBox(overlapMin, overlapMax);
  flags[0] = spanIntersection(left.GetMinPoint().GetX(), left.GetMaxPoint().GetX(),
                              right.GetMinPoint().GetX(), right.GetMaxPoint().GetX());
  flags[1] = spanIntersection(left.GetMinPoint().GetY(), left.GetMaxPoint().GetY(),
                              right.GetMinPoint().GetY(), right.GetMaxPoint().GetY());
  flags[2] = spanIntersection(left.GetMinPoint().GetZ(), left.GetMaxPoint().GetZ(),
                              right.GetMinPoint().GetZ(), right.GetMaxPoint().GetZ());

  for (int i = 0; i < 3; ++i) {
    switch (flags[i]) {
    case 2:
      list.Add(CCollisionInfo(overlapBox, leftFilter, rightFilter, normalTable[i * 2 + 1],
                              -normalTable[i * 2 + 1]),
               false);
      break;
    case 3:
    case 10:
      break;
    case 11:
      list.Add(CCollisionInfo(overlapBox, leftFilter, rightFilter, normalTable[i * 2],
                              -normalTable[i * 2]),
               false);
      break;
    default:
      break;
    }
  }

  if (list.GetCount() == 0) {
    list.Add(CCollisionInfo(overlapBox, leftFilter, rightFilter, normalTable[4], -normalTable[4]),
             false);
    list.Add(CCollisionInfo(overlapBox, leftFilter, rightFilter, normalTable[5], -normalTable[5]),
             false);
  }
  return true;
}

bool AABoxSphereIntersection(const CAABox& box, const CSphere& sphere) {
  const CVector3f& sphereCenter = sphere.GetCenter();
  const CVector3f& boxMaxVal = box.GetMaxPoint();
  const CVector3f& boxMinVal = box.GetMinPoint();

  int comps = 0;
  __REGISTER float dist = 0.f;
  for (int i = 0; i < 3; ++i) {
    if (sphereCenter[i] < boxMinVal[i]) {
      if (sphereCenter[i] + sphere.GetRadius() >= boxMinVal[i]) {
        float d = sphereCenter[i] - boxMinVal[i];
        comps |= 1 << (i * 2);
        float dsq = d * d;
        dist += dsq;
      } else {
        return false;
      }
    } else {
      if (sphereCenter[i] > boxMaxVal[i]) {
        if (sphereCenter[i] - sphere.GetRadius() <= boxMaxVal[i]) {
          float d = sphereCenter[i] - boxMaxVal[i];
          comps |= 1 << (i * 2 + 1);
          float dsq = d * d;
          dist += dsq;
        } else {
          return false;
        }
      }
    }
  }

  if (comps == 0) {
    return true;
  }
  return !(dist > sphere.GetRadius() * sphere.GetRadius());
}

float AABoxSphereIntersectionRadius(const CAABox& aabb, const CSphere& sphere) {
  const CVector3f& sphereCenter = sphere.GetCenter();
  const CVector3f& boxMaxVal = aabb.GetMaxPoint();
  const CVector3f& boxMinVal = aabb.GetMinPoint();

  int comps = 0;
  float dist = 0.f;
  for (int i = 0; i < 3; ++i) {
    if (sphereCenter[i] < boxMinVal[i]) {
      if (sphereCenter[i] + sphere.GetRadius() >= boxMinVal[i]) {
        float d = sphereCenter[i] - boxMinVal[i];
        comps |= 1 << (i * 2);
        float dsq = d * d;
        dist += dsq;
      } else {
        return -1.f;
      }
    } else {
      if (sphereCenter[i] > boxMaxVal[i]) {
        if (sphereCenter[i] - sphere.GetRadius() <= boxMaxVal[i]) {
          float d = sphereCenter[i] - boxMaxVal[i];
          comps |= 1 << (i * 2 + 1);
          float dsq = d * d;
          dist += dsq;
        } else {
          return -1.f;
        }
      }
    }
  }

  if (comps == 0) {
    return dist;
  }
  if (dist > sphere.GetRadius() * sphere.GetRadius()) {
    return -1.f;
  }
  return dist;
}

bool RayTriangleIntersection(const CVector3f& point, const CVector3f& dir, const CVector3f* verts,
                             float& d) {
  CVector3f v0tov1 = verts[1] - verts[0];
  CVector3f v0tov2 = verts[2] - verts[0];
  CVector3f cross0 = CVector3f(CVector3f::Cross(dir, v0tov2));
  const float dot0 = CVector3f::Dot(v0tov1, cross0);
  if (dot0 < 10.f * FLT_EPSILON) {
    return false;
  }

  CVector3f v0toPoint = point - verts[0];
  const float dot1 = CVector3f::Dot(v0toPoint, cross0);
  if (dot1 < 0.f || dot1 > dot0) {
    return false;
  }

  const CVector3f cross1 = CVector3f::Cross(v0toPoint, v0tov1);
  const float dot2 = CVector3f::Dot(cross1, dir);
  if (dot2 < 0.f || dot1 + dot2 > dot0) {
    return false;
  }

  const float finalT = (1.f / dot0) * CVector3f::Dot(cross1, v0tov2);
  if (finalT < 0.f || finalT >= d) {
    return false;
  }

  d = finalT;
  return true;
}

bool RayTriangleIntersection_Double(const CVector3f& point, const CVector3f& dir,
                                    const CVector3f* verts, double& d) {
  CVector3d v0tov1(verts[1] - verts[0]);
  CVector3d v0tov2(verts[2] - verts[0]);
  CVector3d cross0 = CVector3d::Cross(CVector3d(dir), v0tov2);
  const double dot0 = CVector3d::Dot(v0tov1, cross0);
  if (dot0 < FLT_EPSILON) {
    return false;
  }

  CVector3d v0toPoint(point - verts[0]);
  const double dot1 = CVector3d::Dot(v0toPoint, cross0);
  if (dot1 < 0.0 || dot1 > dot0) {
    return false;
  }

  CVector3d cross1 = CVector3d::Cross(v0toPoint, v0tov1);
  const double dot2 = CVector3d::Dot(cross1, CVector3d(dir));
  if (dot2 < 0.0 || dot1 + dot2 > dot0) {
    return false;
  }

  const double finalT = (1.0 / dot0) * CVector3d::Dot(cross1, v0tov2);
  if (finalT < 0.0 || finalT >= d) {
    return false;
  }

  d = finalT;
  return true;
}

void FilterByClosestNormal(const CVector3f& norm, const CCollisionInfoList& in,
                           CCollisionInfoList& out) {
  const CCollisionInfo* pInfo = &in[0];
  float maxDot = -1.1f;
  int i;
  int idx = -1;
  for (i = 0; i < in.GetCount(); ++i) {
    float dot = CVector3f::Dot(pInfo->GetNormalLeft(), norm);
    if (dot > maxDot) {
      maxDot = dot;
      idx = i;
    }
    pInfo++;
  }

  if (idx != -1) {
    out.Add(in[idx], false);
  }
}

void FilterOutBackfaces(const CVector3f& relVel, const CCollisionInfoList& in,
                        CCollisionInfoList& out) {
  if (relVel.CanBeNormalized()) {
    CVector3f norm = relVel.AsNormalized();
    float x = norm.GetX();
    float y = norm.GetY();
    float z = norm.GetZ();
    float limit = 0.001f;
    for (int i = 0; i < in.GetCount(); ++i) {
      if (x * in[i].GetNormalLeft().GetX() + y * in[i].GetNormalLeft().GetY() +
              z * in[i].GetNormalLeft().GetZ() <
          limit) {
        out.Add(in[i], false);
      }
    }
  } else {
    out = in;
  }
}

void AddAverageToFront(const CCollisionInfoList& in, CCollisionInfoList& out) {
  const int count = in.GetCount();
  CVector3f normAccum(0.f, 0.f, 0.f);
  CVector3f pointAccum(0.f, 0.f, 0.f);
  if (count > 1) {
    for (int i = 0; i < count; ++i) {
      pointAccum += in[i].GetPoint();
      normAccum += in[i].GetNormalLeft();
    }

    if (normAccum.CanBeNormalized()) {
      CVector3f normal = normAccum.AsNormalized();
      normAccum = normal;
      const float factor = 1.f / float(count);
      pointAccum *= factor;
      out.Add(
          CCollisionInfo(pointAccum, in[0].GetMaterialRight(), in[0].GetMaterialLeft(), normAccum),
          false);
    }
  }

  for (int i = 0; i < count; ++i) {
    out.Add(in[i], false);
  }
}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)                                                                 \
  do {                                                                                             \
    p0 = a * v0[1] - b * v0[2];                                                                    \
    p2 = a * v2[1] - b * v2[2];                                                                    \
    if (p0 < p2) {                                                                                 \
      min = p0;                                                                                    \
      max = p2;                                                                                    \
    } else {                                                                                       \
      min = p2;                                                                                    \
      max = p0;                                                                                    \
    }                                                                                              \
    rad = fa * boxhalfsize.GetY() + fb * boxhalfsize.GetZ();                                       \
    if (min > rad || max < -rad)                                                                   \
      return false;                                                                                \
  } while (false)

#define AXISTEST_X2(a, b, fa, fb)                                                                  \
  do {                                                                                             \
    p0 = a * v0[1] - b * v0[2];                                                                    \
    p1 = a * v1[1] - b * v1[2];                                                                    \
    if (p0 < p1) {                                                                                 \
      min = p0;                                                                                    \
      max = p1;                                                                                    \
    } else {                                                                                       \
      min = p1;                                                                                    \
      max = p0;                                                                                    \
    }                                                                                              \
    rad = fa * boxhalfsize.GetY() + fb * boxhalfsize.GetZ();                                       \
    if (min > rad || max < -rad)                                                                   \
      return false;                                                                                \
  } while (false)

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)                                                                 \
  do {                                                                                             \
    p0 = -a * v0[0] + b * v0[2];                                                                   \
    p2 = -a * v2[0] + b * v2[2];                                                                   \
    if (p0 < p2) {                                                                                 \
      min = p0;                                                                                    \
      max = p2;                                                                                    \
    } else {                                                                                       \
      min = p2;                                                                                    \
      max = p0;                                                                                    \
    }                                                                                              \
    rad = fa * boxhalfsize.GetX() + fb * boxhalfsize.GetZ();                                       \
    if (min > rad || max < -rad)                                                                   \
      return false;                                                                                \
  } while (false)

#define AXISTEST_Y1(a, b, fa, fb)                                                                  \
  do {                                                                                             \
    p0 = -a * v0[0] + b * v0[2];                                                                   \
    p1 = -a * v1[0] + b * v1[2];                                                                   \
    if (p0 < p1) {                                                                                 \
      min = p0;                                                                                    \
      max = p1;                                                                                    \
    } else {                                                                                       \
      min = p1;                                                                                    \
      max = p0;                                                                                    \
    }                                                                                              \
    rad = fa * boxhalfsize.GetX() + fb * boxhalfsize.GetZ();                                       \
    if (min > rad || max < -rad)                                                                   \
      return false;                                                                                \
  } while (false)

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)                                                                 \
  do {                                                                                             \
    p1 = a * v1[0] - b * v1[1];                                                                    \
    p2 = a * v2[0] - b * v2[1];                                                                    \
    if (p2 < p1) {                                                                                 \
      min = p2;                                                                                    \
      max = p1;                                                                                    \
    } else {                                                                                       \
      min = p1;                                                                                    \
      max = p2;                                                                                    \
    }                                                                                              \
    rad = fa * boxhalfsize.GetX() + fb * boxhalfsize.GetY();                                       \
    if (min > rad || max < -rad)                                                                   \
      return false;                                                                                \
  } while (false)

#define AXISTEST_Z0(a, b, fa, fb)                                                                  \
  do {                                                                                             \
    p0 = a * v0[0] - b * v0[1];                                                                    \
    p1 = a * v1[0] - b * v1[1];                                                                    \
    if (p0 < p1) {                                                                                 \
      min = p0;                                                                                    \
      max = p1;                                                                                    \
    } else {                                                                                       \
      min = p1;                                                                                    \
      max = p0;                                                                                    \
    }                                                                                              \
    rad = fa * boxhalfsize.GetX() + fb * boxhalfsize.GetY();                                       \
    if (min > rad || max < -rad)                                                                   \
      return false;                                                                                \
  } while (false)

bool AABox_ABBox_Moving(const CAABox& aabb0, const CAABox& aabb1, const CVector3f& dir, double& d,
                        CVector3f& point, CVector3f& normal) {
  CVector3d vecMin(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  CVector3d vecMax(FLT_MAX, FLT_MAX, FLT_MAX);

  const CVector3f& leftMin = aabb0.GetMinPoint();
  const CVector3f& rightMin = aabb1.GetMinPoint();
  for (int i = 0; i < 3; ++i) {
    if (CMath::AbsF(dir[i]) < FLT_EPSILON) {
      if (leftMin[i] >= rightMin[i] && leftMin[i] <= aabb1.GetMaxPoint()[i]) {
        continue;
      }
      if (aabb0.GetMaxPoint()[i] >= rightMin[i] &&
          aabb0.GetMaxPoint()[i] <= aabb1.GetMaxPoint()[i]) {
        continue;
      }
      if (leftMin[i] < rightMin[i] && aabb0.GetMaxPoint()[i] > aabb1.GetMaxPoint()[i]) {
        continue;
      }
      return false;
    }

    if (aabb0.GetMaxPoint()[i] < rightMin[i] && dir[i] > 0.f) {
      vecMin[i] = (rightMin[i] - aabb0.GetMaxPoint()[i]) / dir[i];
    } else if (aabb1.GetMaxPoint()[i] < leftMin[i] && dir[i] < 0.f) {
      vecMin[i] = (aabb1.GetMaxPoint()[i] - leftMin[i]) / dir[i];
    } else if (aabb1.GetMaxPoint()[i] > leftMin[i] && dir[i] < 0.f) {
      vecMin[i] = (aabb1.GetMaxPoint()[i] - leftMin[i]) / dir[i];
    } else if (aabb0.GetMaxPoint()[i] > rightMin[i] && dir[i] > 0.f) {
      vecMin[i] = (rightMin[i] - aabb0.GetMaxPoint()[i]) / dir[i];
    }

    if (aabb1.GetMaxPoint()[i] > leftMin[i] && dir[i] > 0.f) {
      vecMax[i] = (aabb1.GetMaxPoint()[i] - leftMin[i]) / dir[i];
    } else if (aabb0.GetMaxPoint()[i] > rightMin[i] && dir[i] < 0.f) {
      vecMax[i] = (rightMin[i] - aabb0.GetMaxPoint()[i]) / dir[i];
    } else if (aabb0.GetMaxPoint()[i] < rightMin[i] && dir[i] < 0.f) {
      vecMax[i] = (rightMin[i] - aabb0.GetMaxPoint()[i]) / dir[i];
    } else if (aabb1.GetMaxPoint()[i] < leftMin[i] && dir[i] > 0.f) {
      vecMax[i] = (aabb1.GetMaxPoint()[i] - leftMin[i]) / dir[i];
    }
  }

  int maxAxis = 0;
  if (vecMin[1] > vecMin[0]) {
    maxAxis = 1;
  }
  if (vecMin[2] > vecMin[maxAxis]) {
    maxAxis = 2;
  }

  const double& minMax = rstl::min_val(vecMax[0], rstl::min_val(vecMax[1], vecMax[2]));
  if (vecMin[maxAxis] > minMax) {
    return false;
  }

  d = vecMin[maxAxis];
  normal = CVector3f::Zero();
  normal[maxAxis] = dir[maxAxis] > 0.f ? -1.f : 1.f;

  point[0] = dir[0] > 0.f ? aabb0.GetMaxPoint()[0] : aabb0.GetMinPoint()[0];
  point[1] = dir[1] > 0.f ? aabb0.GetMaxPoint()[1] : aabb0.GetMinPoint()[1];
  point[2] = dir[2] > 0.f ? aabb0.GetMaxPoint()[2] : aabb0.GetMinPoint()[2];

  point += CCast::ToReal32(d) * dir;
  return true;
}

bool BoxLineTest(const CAABox& aabb, const CVector3f& point, const CVector3f& dir, float& tMin,
                 float& tMax, int& axis, bool& sign) {
  const float* minPt = reinterpret_cast< const float* >(&aabb);
  const float* maxPt = reinterpret_cast< const float* >(&aabb.GetMaxPoint());
  const float* pointPtr = reinterpret_cast< const float* >(&point);
  const float* dirPtr = reinterpret_cast< const float* >(&dir);
  tMin = -999999.f;
  tMax = 999999.f;

  for (int i = 0; i < 3; ++i) {
    float dirI = *dirPtr;
    if (dirI == 0.f) {
      if (*pointPtr < *minPt || *pointPtr > *maxPt) {
        return false;
      }
    } else {
      const float dirRecip = 1.f / dirI;
      float tmpMin;
      float tmpMax;
      if (dirI < 0.f) {
        tmpMin = (*maxPt - *pointPtr) * dirRecip;
        tmpMax = (*minPt - *pointPtr) * dirRecip;
      } else {
        tmpMin = (*minPt - *pointPtr) * dirRecip;
        tmpMax = (*maxPt - *pointPtr) * dirRecip;
      }

      if (tmpMin > tMin) {
        sign = dirI < 0.f;
        axis = i;
        tMin = tmpMin;
      }

      if (tmpMax < tMax) {
        tMax = tmpMax;
      }
    }

    ++dirPtr;
    ++pointPtr;
    ++minPt;
    ++maxPt;
  }

  return tMin <= tMax;
}

bool TriBoxOverlap(const CVector3f& boxcenter, const CVector3f& boxhalfsize,
                   const CVector3f& trivert0, const CVector3f& trivert1,
                   const CVector3f& trivert2) {
  float min, max, d, p0, p1, p2, rad, fex, fey, fez;

  float v0[3];
  v0[0] = trivert0.GetX() - boxcenter.GetX();
  v0[1] = trivert0.GetY() - boxcenter.GetY();
  v0[2] = trivert0.GetZ() - boxcenter.GetZ();
  float v1[3];
  v1[0] = trivert1.GetX() - boxcenter.GetX();
  v1[1] = trivert1.GetY() - boxcenter.GetY();
  v1[2] = trivert1.GetZ() - boxcenter.GetZ();
  float v2[3];
  v2[0] = trivert2.GetX() - boxcenter.GetX();
  v2[1] = trivert2.GetY() - boxcenter.GetY();
  v2[2] = trivert2.GetZ() - boxcenter.GetZ();

  float e0[3];
  e0[0] = v1[0] - v0[0];
  e0[1] = v1[1] - v0[1];
  e0[2] = v1[2] - v0[2];
  float e1[3];
  e1[0] = v2[0] - v1[0];
  e1[1] = v2[1] - v1[1];
  e1[2] = v2[2] - v1[2];
  float e2[3];
  e2[0] = v0[0] - v2[0];
  e2[1] = v0[1] - v2[1];
  e2[2] = v0[2] - v2[2];

  const float e0x = e0[0];
  const float e0y = e0[1];
  const float e0z = e0[2];
  fex = CMath::AbsF(e0x);
  fey = CMath::AbsF(e0y);
  fez = CMath::AbsF(e0z);
  AXISTEST_X01(e0z, e0y, fez, fey);
  AXISTEST_Y02(e0z, e0x, fez, fex);
  AXISTEST_Z12(e0y, e0x, fey, fex);

  const float e1x = e1[0];
  const float e1y = e1[1];
  const float e1z = e1[2];
  fex = CMath::AbsF(e1x);
  fey = CMath::AbsF(e1y);
  fez = CMath::AbsF(e1z);
  AXISTEST_X01(e1z, e1y, fez, fey);
  AXISTEST_Y02(e1z, e1x, fez, fex);
  AXISTEST_Z0(e1y, e1x, fey, fex);

  const float e2x = e2[0];
  const float e2y = e2[1];
  const float e2z = e2[2];
  fex = CMath::AbsF(e2x);
  fey = CMath::AbsF(e2y);
  fez = CMath::AbsF(e2z);
  AXISTEST_X2(e2z, e2y, fez, fey);
  AXISTEST_Y1(e2z, e2x, fez, fex);
  AXISTEST_Z12(e2y, e2x, fey, fex);

  min = max = v0[0];
  if (v1[0] < v0[0]) {
    min = v1[0];
  }
  if (v1[0] > v0[0]) {
    max = v1[0];
  }
  if (v2[0] < min) {
    min = v2[0];
  }
  if (v2[0] > max) {
    max = v2[0];
  }
  if (min > boxhalfsize.GetX() || max < -boxhalfsize.GetX()) {
    return false;
  }

  min = max = v0[1];
  if (v1[1] < v0[1]) {
    min = v1[1];
  }
  if (v1[1] > v0[1]) {
    max = v1[1];
  }
  if (v2[1] < min) {
    min = v2[1];
  }
  if (v2[1] > max) {
    max = v2[1];
  }
  if (min > boxhalfsize.GetY() || max < -boxhalfsize.GetY()) {
    return false;
  }

  min = max = v0[2];
  if (v1[2] < v0[2]) {
    min = v1[2];
  }
  if (v1[2] > v0[2]) {
    max = v1[2];
  }
  if (v2[2] < min) {
    min = v2[2];
  }
  if (v2[2] > max) {
    max = v2[2];
  }
  if (min > boxhalfsize.GetZ() || max < -boxhalfsize.GetZ()) {
    return false;
  }

  float normal[3];
  normal[0] = e0[1] * e1[2] - e0[2] * e1[1];
  normal[1] = e0[2] * e1[0] - e0[0] * e1[2];
  normal[2] = e0[0] * e1[1] - e0[1] * e1[0];
  d = -(normal[0] * v0[0] + normal[1] * v0[1] + normal[2] * v0[2]);
  return planeBoxOverlap(&normal[0], d, boxhalfsize);
}

bool LineCircleIntersection2d(const CVector3f& point, const CVector3f& dir, const CSphere& sphere,
                              int axis1, int axis2, float& d) {
  CVector3f delta;
  delta.SetY(sphere.GetCenter().GetY() - point.GetY());
  delta.SetZ(sphere.GetCenter().GetZ() - point.GetZ());
  delta.SetX(sphere.GetCenter().GetX() - point.GetX());
  const float& deltaY = delta[axis2];
  const float& deltaX = delta[axis1];
  const CVector2f deltaVec(deltaX, deltaY);
  const CVector2f dirVec(dir[axis1], dir[axis2]);

  const float dirVecMag = dirVec.Magnitude();
  if (dirVecMag < FLT_EPSILON) {
    return false;
  }

  CVector2f dirVecNorm = CVector2f(dir[axis1], dir[axis2]) / dirVecMag;
  const float deltaVecDot = CVector2f::Dot(deltaVec, dirVecNorm);
  const float deltaVecDotSq = deltaVecDot * deltaVecDot;
  const float deltaVecMagSq = CVector2f::Dot(deltaVec, deltaVec);
  const float sphereRadSq = sphere.GetRadius() * sphere.GetRadius();
  if (deltaVecDot < 0.f && deltaVecMagSq > sphereRadSq) {
    return false;
  }

  const float tSq = sphereRadSq - (deltaVecMagSq - deltaVecDotSq);
  if (tSq < 0.f) {
    return false;
  }

  const float t = CMath::SqrtF(tSq);
  d = deltaVecMagSq > sphereRadSq ? deltaVecDot - t : deltaVecDot + t;
  d /= dirVecMag;
  return true;
}

bool MovingSphereAABox(const CSphere& sphere, const CAABox& aabb, const CVector3f& dir,
                       double& dOut, CVector3f& point, CVector3f& normal) {
  const CVector3f& aabbMax = aabb.GetMaxPoint();
  const CVector3f radiusVec(sphere.GetRadius(), sphere.GetRadius(), sphere.GetRadius());
  const CAABox expAABB(aabb.GetMinPoint() - radiusVec, aabbMax + radiusVec);
  float tMin;
  float tMax;
  int axis;
  bool sign;
  if (!BoxLineTest(expAABB, sphere.GetCenter(), dir, tMin, tMax, axis, sign)) {
    return false;
  }

  point = sphere.GetCenter() + tMin * dir;

  const int nextAxis1 = (axis + 1) % 3;
  const int nextAxis2 = (axis + 2) % 3;

  const float coord1 = point[nextAxis1];
  const bool inMin1 = coord1 >= aabb.GetMinPoint()[nextAxis1];
  const bool inMax1 = coord1 <= aabbMax[nextAxis1];
  const float coord2 = point[nextAxis2];
  const bool inMin2 = coord2 >= aabb.GetMinPoint()[nextAxis2];
  const bool inMax2 = coord2 <= aabbMax[nextAxis2];
  const bool inBounds1 = inMin1 && inMax1;
  const bool inBounds2 = inMin2 && inMax2;

  if (inBounds1 && inBounds2) {
    if (tMin < 0.f || tMin > dOut) {
      return false;
    }
    normal[axis] = sign ? 1.f : -1.f;
    dOut = tMin;
    point -= normal * sphere.GetRadius();
    return true;
  }

  if (!inBounds1 && !inBounds2) {
    const int pointFlags =
        (1 << axis) * sign | (1 << nextAxis1) * inMin1 | (1 << nextAxis2) * inMin2;
    const CVector3f& aabbPoint = aabb.GetPoint(pointFlags);
    float rayTime;
    if (CollisionUtil::RaySphereIntersection(CSphere(aabbPoint, sphere.GetRadius()),
                                             sphere.GetCenter(), dir, CCast::ToReal32(dOut),
                                             rayTime, point)) {
      int useAxis = -1;
      for (int i = 0; i < 3; ++i) {
        if ((pointFlags & (1 << i)) ? aabbPoint[i] > point[i] : aabbPoint[i] < point[i]) {
          useAxis = i;
          break;
        }
      }

      if (useAxis == -1) {
        dOut = rayTime;
        normal = (point - aabbPoint).AsNormalized();
        point -= sphere.GetRadius() * normal;
        return true;
      }

      float d;
      const int useAxisNext1 = (useAxis + 1) % 3;
      const int useAxisNext2 = (useAxis + 2) % 3;
      if (CollisionUtil::LineCircleIntersection2d(sphere.GetCenter(), dir,
                                                  CSphere(aabbPoint, sphere.GetRadius()),
                                                  useAxisNext1, useAxisNext2, d) &&
          d > 0.f && d < dOut) {
        point = sphere.GetCenter() + d * dir;
        if (point[useAxis] > aabbMax[useAxis]) {
          const int useAxisBit = 1 << useAxis;
          if (pointFlags & useAxisBit) {
            return false;
          }

          const CVector3f& aabbPoint1 = aabb.GetPoint(pointFlags | useAxisBit);
          if (CollisionUtil::RaySphereIntersection(CSphere(aabbPoint1, sphere.GetRadius()),
                                                   sphere.GetCenter(), dir, CCast::ToReal32(dOut),
                                                   d, point)) {
            dOut = d;
            normal = (point - aabbPoint1).AsNormalized();
            point -= normal * sphere.GetRadius();
            return true;
          }
          return false;
        } else if (point[useAxis] < aabb.GetMinPoint()[useAxis]) {
          const int useAxisBit = 1 << useAxis;
          if (!(pointFlags & useAxisBit)) {
            return false;
          }

          const CVector3f& aabbPoint1 = aabb.GetPoint(pointFlags ^ useAxisBit);
          if (CollisionUtil::RaySphereIntersection(CSphere(aabbPoint1, sphere.GetRadius()),
                                                   sphere.GetCenter(), dir, CCast::ToReal32(dOut),
                                                   d, point)) {
            dOut = d;
            normal = (point - aabbPoint1).AsNormalized();
            point -= normal * sphere.GetRadius();
            return true;
          }
          return false;
        } else {
          dOut = d;
          normal = point - aabbPoint;
          normal[useAxis] = 0.f;
          normal.Normalize();
          point -= normal * sphere.GetRadius();
          return true;
        }
      }
    } else {
      int minAxis = 0;
      int reverseCount = 0;
      float dMin = 1.0e10f;
      for (int i = 0; i < 3; ++i) {
        if (!(CMath::AbsF(dir[i]) < FLT_EPSILON)) {
          const bool pointMax = (pointFlags & (1 << i)) != 0;
          if (pointMax != (dir[i] > 0.f)) {
            ++reverseCount;
            const float d = (1.f / dir[i]) * ((pointMax ? aabbMax[i] : aabb.GetMinPoint()[i]) -
                                              sphere.GetCenter()[i]);
            if (d < 0.f) {
              return false;
            }
            if (d < dMin) {
              dMin = d;
              minAxis = i;
            }
          }
        }
      }

      if (reverseCount < 2) {
        return false;
      }

      float d;
      const int useAxisNext1 = (minAxis + 1) % 3;
      const int useAxisNext2 = (minAxis + 2) % 3;
      if (CollisionUtil::LineCircleIntersection2d(sphere.GetCenter(), dir,
                                                  CSphere(aabbPoint, sphere.GetRadius()),
                                                  useAxisNext1, useAxisNext2, d) &&
          d > 0.f && d < dOut) {
        point = sphere.GetCenter() + d * dir;
        if (point[minAxis] > aabbMax[minAxis]) {
          return false;
        }
        if (point[minAxis] < aabb.GetMinPoint()[minAxis]) {
          return false;
        }

        dOut = d;
        normal = point - aabbPoint;
        normal[minAxis] = 0.f;
        normal.Normalize();
        point -= sphere.GetRadius() * normal;
        return true;
      }
    }
    return false;
  } else {
    const int useNextAxis1 = inBounds1 ? nextAxis2 : nextAxis1;
    const bool pointSign = inBounds1 ? inMin2 : inMin1;
    const int pointFlag1 = (1 << useNextAxis1) * pointSign;
    const int useNextAxis2 = inBounds1 ? nextAxis1 : nextAxis2;
    const int pointFlags = pointFlag1 | ((1 << axis) * sign);
    const CVector3f& aabbPoint2 = aabb.GetPoint(pointFlags);
    float d;
    if (LineCircleIntersection2d(sphere.GetCenter(), dir, CSphere(aabbPoint2, sphere.GetRadius()),
                                 axis, useNextAxis1, d) &&
        d > 0.f && d < dOut) {
      point = sphere.GetCenter() + d * dir;
      if (point[useNextAxis2] > aabbMax[useNextAxis2]) {
        const CVector3f& aabbPoint3 = aabb.GetPoint(pointFlags | (1 << useNextAxis2));
        if (point[useNextAxis2] < expAABB.GetMaxPoint()[useNextAxis2]) {
          if (RaySphereIntersection(CSphere(aabbPoint3, sphere.GetRadius()), sphere.GetCenter(),
                                    dir, CCast::ToReal32(dOut), d, point)) {
            dOut = d;
            normal = (point - aabbPoint3).AsNormalized();
            point -= sphere.GetRadius() * normal;
            return true;
          }
        }
        return false;
      }

      if (point[useNextAxis2] < aabb.GetMinPoint()[useNextAxis2]) {
        if (point[useNextAxis2] > expAABB.GetMinPoint()[useNextAxis2]) {
          if (RaySphereIntersection(CSphere(aabbPoint2, sphere.GetRadius()), sphere.GetCenter(),
                                    dir, CCast::ToReal32(dOut), d, point)) {
            dOut = d;
            normal = (point - aabbPoint2).AsNormalized();
            point -= sphere.GetRadius() * normal;
            return true;
          }
        }
        return false;
      } else {
        dOut = d;
        normal = point - aabbPoint2;
        normal[useNextAxis2] = 0.f;
        normal.Normalize();
        point -= sphere.GetRadius() * normal;
        return true;
      }
    }
  }
  return false;
}

bool TriSphereOverlap(const CSphere& sphere, const CVector3f& trivert0, const CVector3f& trivert1,
                      const CVector3f& trivert2) {
  return !(TriPointSqrDist(sphere.GetCenter(), trivert0, trivert1, trivert2, 0, 0) >
           sphere.GetRadius() * sphere.GetRadius());
}

bool TriSphereIntersection(const CSphere& sphere, const CVector3f& trivert0,
                           const CVector3f& trivert1, const CVector3f& trivert2, CVector3f& point,
                           CVector3f& normal) {
  float baryX;
  float baryY;
  if (TriPointSqrDist(sphere.GetCenter(), trivert0, trivert1, trivert2, &baryX, &baryY) >
      sphere.GetRadius() * sphere.GetRadius()) {
    return false;
  }

  float baryZ = 1.f - (baryX + baryY);
  point = CMath::BaryToWorld(trivert2, trivert1, trivert0, CVector3f(baryX, baryY, baryZ));

  if (baryX == 0.f || baryX == 1.f || baryY == 0.f || baryY == 1.f || baryZ == 0.f ||
      baryZ == 1.f) {
    normal = -sphere.GetSurfaceNormal(point);
  } else {
    normal = CVector3f::Cross(trivert1 - trivert0, trivert2 - trivert0).AsNormalized();
  }

  return true;
}

double TriPointSqrDist(const CVector3f& point, const CVector3f& trivert0, const CVector3f& trivert1,
                       const CVector3f& trivert2, float* baryX, float* baryY) {
  const CVector3d A(trivert0 - point);
  const CVector3d B(trivert1 - trivert0);
  const CVector3d C(trivert2 - trivert0);

  const double bMag = B.MagSquared();
  const double cMag = C.MagSquared();
  const double bDotC = CVector3d::Dot(B, C);
  const double aDotB = CVector3d::Dot(A, B);
  const double aDotC = CVector3d::Dot(A, C);
  double ret = A.MagSquared();

  const double rej = CMath::AbsD(bMag * cMag - bDotC * bDotC);
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
      const double f3 = 1.0 / rej;
      retA *= f3;
      retB *= f3;
      ret += retB * (2.0 * aDotB + (bMag * retB + bDotC * retA)) +
             retA * (2.0 * aDotC + (bDotC * retB + cMag * retA));
    }
  } else if (retB < 0.0) {
    retB = bDotC + aDotB;
    retA = cMag + aDotC;
    if (retA > retB) {
      retA -= retB;
      retB = bMag - 2.0 * bDotC;
      retB = cMag + retB;
      if (retA >= retB) {
        retB = 1.0;
        retA = 0.0;
        ret += 2.0 * aDotB + bMag;
      } else {
        retB = retA / retB;
        retA = 1.0 - retB;
        ret += retB * (2.0 * aDotB + (bMag * retB + bDotC * retA)) +
               retA * (2.0 * aDotC + (bDotC * retB + cMag * retA));
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
  } else if (retA < 0.0) {
    double tmp1 = bDotC + aDotC;
    double tmp2 = bMag + aDotB;
    retB = tmp1;
    retA = tmp2;
    if (retA > retB) {
      retA -= retB;
      retB = bMag - 2.0 * bDotC;
      retB = cMag + retB;
      if (retA >= retB) {
        retA = 1.0;
        retB = 0.0;
        ret += 2.0 * aDotC + cMag;
      } else {
        retA /= retB;
        retB = 1.0 - retA;
        ret += retB * (2.0 * aDotB + (bMag * retB + bDotC * retA)) +
               retA * (2.0 * aDotC + (bDotC * retB + cMag * retA));
      }
    } else {
      retA = 0.0;
      if (tmp2 <= 0.0) {
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
      retB = cMag + retB;
      if (retA >= retB) {
        retB = 1.0;
        retA = 0.0;
        ret += 2.0 * aDotB + bMag;
      } else {
        retB = retA / retB;
        retA = 1.0 - retB;
        ret += retB * (2.0 * aDotB + (bMag * retB + bDotC * retA)) +
               retA * (2.0 * aDotC + (bDotC * retB + cMag * retA));
      }
    }
  }

  if (baryX != 0) {
    *baryX = float(retA);
  }
  if (baryY != 0) {
    *baryY = float(retB);
  }

  return ret;
}

} // namespace CollisionUtil
