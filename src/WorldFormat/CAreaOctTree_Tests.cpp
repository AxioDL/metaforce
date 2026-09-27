#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Math/CLine.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector3i.hpp"
#include "WorldFormat/CAreaOctTree.hpp"
#include "math.h"
#include "rstl/algorithm.hpp"

struct SSubdivision {
  int count;
  CVector3i axes;

  SSubdivision(int count, int x, int y, int z) : count(count), axes(x, y, z) {}
};
CHECK_SIZEOF(SSubdivision, 0x10)

static bool _close_enough(const float a, const float b, const float epsilon) {
  return fabs(a - b) <= epsilon;
}

static bool BoxLineTest(const CAABox& box, const CLine& line, float& lT, float& hT) {

  const CVector3f& min = box.GetMinPoint();
  const CVector3f& max = box.GetMaxPoint();
  CVector3f lineRefPoint = line.GetRefPoint();
  CUnitVector3f lineNormal = line.GetNormal();
  lT = -FLT_MAX;
  hT = FLT_MAX;

  for (int i = 0; i < 3; i++) {
    if (_close_enough(lineNormal[i], 0.f, 0.0001f)) {
      if (lineRefPoint[i] < min[i] || lineRefPoint[i] > max[i]) {
        return false;
      }
    } else {

      if (lineNormal[i] < 0.f) {
        const float tmpN = (1.f / lineNormal[i]);
        const float tmpH = max[i] - lineRefPoint[i];
        const float tmpL = min[i] - lineRefPoint[i];
        if (tmpH < lT * lineNormal[i]) {
          lT = tmpH * tmpN;
        }
        if (tmpL > hT * lineNormal[i]) {
          hT = tmpL * tmpN;
        }
      } else {
        const float tmpN = (1.f / lineNormal[i]);
        const float tmpL = min[i] - lineRefPoint[i];
        const float tmpH = max[i] - lineRefPoint[i];
        if (tmpL > lT * lineNormal[i]) {
          lT = tmpL * tmpN;
        }
        if (tmpH < hT * lineNormal[i]) {
          hT = tmpH * tmpN;
        }
      }
    }
  }

  return lT <= hT;
}

void CAreaOctTree::Node::LineTestEx(const CLine& line, const CMaterialFilter& filter,
                                    SRayResult& res, float length) const {
  if (mNodeType == kTT_Invalid) {
    return;
  }

  float lT = 0.f;
  float hT = 0.f;
  if (!BoxLineTest(mAabb, line, lT, hT)) {
    return;
  }

  LineTestExInternal(line, filter, res, lT - 0.0001f, hT + 0.0001f, length,
                     CVector3f(1.f / line.GetNormal().GetX(), 1.f / line.GetNormal().GetY(),
                               1.f / line.GetNormal().GetZ()));
}

bool CAreaOctTree::Node::LineTest(const CLine& line, const CMaterialFilter& filter,
                                  const float length) const {
  if (mNodeType == kTT_Invalid) {
    return true;
  }

  float lT = 0.f;
  float hT = 0.f;
  if (!BoxLineTest(mAabb, line, lT, hT)) {
    return true;
  }

  return LineTestInternal(line, filter, lT - 0.0001f, hT + 0.0001f, length,
                          CVector3f(1.f / line.GetNormal().GetX(), 1.f / line.GetNormal().GetY(),
                                    1.f / line.GetNormal().GetZ()));
}

bool CAreaOctTree::Node::LineTestInternal(const CLine& line, const CMaterialFilter& filter,
                                          float lT, float hT, float maxT,
                                          const CVector3f& dirRecip) const {
  float lowT = (1.f - FLT_EPSILON * 100.f) * lT;
  float highT = (1.f + FLT_EPSILON * 100.f) * hT;
  if (maxT) {
    if (lowT < 0.f) {
      lowT = 0.f;
    }
    if (highT > maxT) {
      highT = maxT;
    }
    if (lowT > highT) {
      return true;
    }
  }

  if (GetTreeType() == kTT_Leaf) {
    TriListReference triangles = GetTriangleArray();
    const int count = triangles.GetSize();
    for (int i = 0; i < count; ++i) {
      CCollisionSurface triangle = GetOwner().GetMasterListTriangle(triangles.GetAt(i));
      CVector3f edge0 = triangle.GetVert(1) - triangle.GetVert(0);
      CVector3f edge1 = triangle.GetVert(2) - triangle.GetVert(0);
      CVector3f p = CVector3f::Cross(line.GetNormal(), edge1);
      float determinant = CVector3f::Dot(edge0, p);
      if (CMath::AbsF(determinant) < FLT_EPSILON * 10.f) {
        continue;
      }
      float invDet = 1.f / determinant;
      CVector3f relativeOrigin = line.GetRefPoint() - triangle.GetVert(0);
      float u = invDet * CVector3f::Dot(relativeOrigin, p);
      if (u < 0.f || u > 1.f) {
        continue;
      }
      CVector3f q = CVector3f::Cross(relativeOrigin, edge0);
      float t = invDet * CVector3f::Dot(q, edge1);
      if (t >= highT || t < lowT) {
        continue;
      }
      float v = invDet * CVector3f::Dot(q, line.GetNormal());
      if (v < 0.f || u + v > 1.f) {
        continue;
      }
      CMaterialList material(triangle.GetSurfaceFlags());
      if (filter.Passes(material)) {
        return false;
      }
    }
    return true;
  } else if (GetTreeType() == kTT_Branch) {
    if (GetChildFlags() == 0xa) {
      Node left = GetChild(0);
      CAABox leftBounds = left.GetBoundingBox();
      float leftLowT = lT;
      float leftHighT = hT;
      if (BoxLineTest(leftBounds, line, leftLowT, leftHighT)) {
        if (!left.LineTestInternal(line, filter, leftLowT, leftHighT, maxT, dirRecip)) {
          return false;
        }
      }

      Node right = GetChild(1);
      CAABox rightBounds = right.GetBoundingBox();
      float rightLowT = lT;
      float rightHighT = hT;
      if (BoxLineTest(rightBounds, line, rightLowT, rightHighT)) {
        if (!right.LineTestInternal(line, filter, rightLowT, rightHighT, maxT, dirRecip)) {
          return false;
        }
      }
      return true;
    }

    CVector3f center = mAabb.GetCenterPoint();
    CVector3f lowPoint = line.GetRefPoint() + lT * line.GetNormal();
    CVector3f highPoint = line.GetRefPoint() + hT * line.GetNormal();
    CVector3f crossingTimes(dirRecip.GetX() * (center.GetX() - line.GetRefPoint().GetX()),
                            dirRecip.GetY() * (center.GetY() - line.GetRefPoint().GetY()),
                            dirRecip.GetZ() * (center.GetZ() - line.GetRefPoint().GetZ()));
    int startOctant = 0;
    int endOctant = 0;
    int crossingOrder = 0;
    static const int skAxisBits[3] = {1, 2, 4};
    static const int skNextAxis[3] = {1, 2, 0};
    for (int i = 0; i < 3; ++i) {
      const int axisBit = skAxisBits[i];
      if (lowPoint[i] >= center[i]) {
        startOctant |= axisBit;
      }
      if (highPoint[i] >= center[i]) {
        endOctant |= axisBit;
      }
      if (crossingTimes[i] < crossingTimes[skNextAxis[i]]) {
        crossingOrder |= axisBit;
      }
    }

    static const int skSubdivisionIndices[8][8] = {
        {0, 1, 2, 4, 5, 6, 8, 10}, {0, 1, 2, 3, 5, 6, 8, 10}, {0, 1, 2, 4, 5, 6, 9, 11},
        {0, 1, 2, 3, 5, 6, 9, 12}, {0, 1, 2, 4, 5, 7, 8, 13}, {0, 1, 2, 3, 5, 7, 8, 14},
        {0, 1, 2, 4, 5, 7, 9, 15}, {0, 1, 2, 3, 5, 7, 9, 15}};
    static const SSubdivision skSubdivisions[16] = {
        SSubdivision(0, 0, 0, 0), SSubdivision(1, 0, 0, 0), SSubdivision(1, 1, 0, 0),
        SSubdivision(2, 0, 1, 0), SSubdivision(2, 1, 0, 0), SSubdivision(1, 2, 0, 0),
        SSubdivision(2, 0, 2, 0), SSubdivision(2, 2, 0, 0), SSubdivision(2, 2, 1, 0),
        SSubdivision(2, 1, 2, 0), SSubdivision(3, 0, 2, 1), SSubdivision(3, 1, 0, 2),
        SSubdivision(3, 0, 1, 2), SSubdivision(3, 2, 1, 0), SSubdivision(3, 2, 0, 1),
        SSubdivision(3, 1, 2, 0)};
    const SSubdivision& subdivision =
        skSubdivisions[skSubdivisionIndices[crossingOrder][startOctant ^ endOctant]];
    const CVector3i& axes = subdivision.axes;
    float childLowT = lT;
    int childIndex = startOctant;
    for (int i = 0; i <= subdivision.count; ++i) {
      float childHighT;
      if (i < subdivision.count) {
        childHighT = crossingTimes[axes[i]];
      } else {
        childHighT = hT;
      }
      if (childHighT > lowT && childLowT <= childHighT) {
        Node child = GetChild(childIndex);
        if (child.GetTreeType() != kTT_Invalid) {
          if (!child.LineTestInternal(line, filter, childLowT, childHighT, maxT, dirRecip)) {
            return false;
          }
        }
      }
      if (i < subdivision.count) {
        childIndex ^= 1 << axes[i];
      }
      childLowT = childHighT;
    }
  }
  return true;
}

void CAreaOctTree::Node::LineTestExInternal(const CLine& line, const CMaterialFilter& filter,
                                            SRayResult& result, float lT, float hT, float maxT,
                                            const CVector3f& dirRecip) const {
  float lowT = (1.f - FLT_EPSILON * 100.f) * lT;
  float highT = (1.f + FLT_EPSILON * 100.f) * hT;
  if (maxT) {
    if (lowT < 0.f) {
      lowT = 0.f;
    }
    if (highT > maxT) {
      highT = maxT;
    }
    if (lowT > highT) {
      return;
    }
  }

  if (GetTreeType() == kTT_Leaf) {
    TriListReference triangles = GetTriangleArray();
    float bestT = highT;
    bool foundTriangle = false;
    SRayResult candidate;
    const int count = triangles.GetSize();
    for (int i = 0; i < count; ++i) {
      CCollisionSurface triangle = GetOwner().GetMasterListTriangle(triangles.GetAt(i));
      CVector3f edge0 = triangle.GetVert(1) - triangle.GetVert(0);
      CVector3f edge1 = triangle.GetVert(2) - triangle.GetVert(0);
      CVector3f p = CVector3f::Cross(line.GetNormal(), edge1);
      float determinant = CVector3f::Dot(edge0, p);
      if (CMath::AbsF(determinant) < FLT_EPSILON * 10.f) {
        continue;
      }
      float invDet = 1.f / determinant;
      CVector3f relativeOrigin = line.GetRefPoint() - triangle.GetVert(0);
      float u = invDet * CVector3f::Dot(relativeOrigin, p);
      if (u < 0.f || u > 1.f) {
        continue;
      }
      CVector3f q = CVector3f::Cross(relativeOrigin, edge0);
      float t = invDet * CVector3f::Dot(q, edge1);
      if (t >= bestT || t < lowT) {
        continue;
      }
      float v = invDet * CVector3f::Dot(q, line.GetNormal());
      if (v < 0.f || u + v > 1.f) {
        continue;
      }
      CMaterialList material(triangle.GetSurfaceFlags());
      if (filter.Passes(material) && t <= bestT) {
        bestT = t;
        foundTriangle = true;
        candidate.mSurface = triangle;
        candidate.mT = t;
      }
    }
    if (foundTriangle) {
      result = candidate;
      result.mPlane = result.mSurface->GetPlane();
    }
  } else if (GetTreeType() == kTT_Branch) {
    if (GetChildFlags() == 0xa) {
      SRayResult leftResult;
      SRayResult rightResult;
      Node left = GetChild(0);
      CAABox leftBounds = left.GetBoundingBox();
      float leftLowT = lT;
      float leftHighT = hT;
      if (BoxLineTest(leftBounds, line, leftLowT, leftHighT)) {
        left.LineTestExInternal(line, filter, leftResult, leftLowT, leftHighT, maxT, dirRecip);
      }
      Node right = GetChild(1);
      CAABox rightBounds = right.GetBoundingBox();
      float rightLowT = lT;
      float rightHighT = hT;
      if (BoxLineTest(rightBounds, line, rightLowT, rightHighT)) {
        right.LineTestExInternal(line, filter, rightResult, rightLowT, rightHighT, maxT, dirRecip);
      }
      if (!leftResult.mSurface.valid() && !rightResult.mSurface.valid()) {
        result = SRayResult();
        return;
      } else {
        if (leftResult.mSurface.valid() && rightResult.mSurface.valid()) {
          if (leftResult.mT < rightResult.mT) {
            result = leftResult;
          } else {
            result = rightResult;
          }
        } else if (leftResult.mSurface.valid()) {
          result = leftResult;
        } else {
          result = rightResult;
        }
        if (result.mT > highT) {
          result = SRayResult();
        }
      }
      return;
    }

    float crossingTimes[3];
    int count = 0;
    int components[4] = {-1, -1, -1, 0};
    CVector3f center = mAabb.GetCenterPoint();
    CVector3f lowPoint = line.GetRefPoint() + lT * line.GetNormal();
    CVector3f highPoint = line.GetRefPoint() + hT * line.GetNormal();
    for (int i = 0; i < 3; ++i) {
      if (((lowPoint[i] < center[i] && highPoint[i] > center[i]) ||
           (highPoint[i] < center[i] && lowPoint[i] > center[i])) &&
          !_close_enough(line.GetNormal()[i], 0.f, 0.0001f)) {
        crossingTimes[i] = dirRecip[i] * (center[i] - line.GetRefPoint()[i]);
        components[count] = i;
        ++count;
      }
    }

    switch (count) {
    case 0:
    case 1:
      break;
    case 2:
      if (crossingTimes[components[1]] < crossingTimes[components[0]]) {
        rstl::swap(components[0], components[1]);
      }
      break;
    case 3: {
      bool xBeforeZ = crossingTimes[0] < crossingTimes[2];
      bool yBeforeZ = crossingTimes[1] < crossingTimes[2];
      if (crossingTimes[0] < crossingTimes[1]) {
        if (!xBeforeZ) {
          components[0] = 2;
          components[1] = 0;
          components[2] = 1;
        } else if (yBeforeZ) {
          components[0] = 0;
          components[1] = 1;
          components[2] = 2;
        } else {
          components[0] = 0;
          components[1] = 2;
          components[2] = 1;
        }
      } else {
        if (!yBeforeZ) {
          components[0] = 2;
          components[1] = 1;
          components[2] = 0;
        } else if (xBeforeZ) {
          components[0] = 1;
          components[1] = 0;
          components[2] = 2;
        } else {
          components[0] = 1;
          components[1] = 2;
          components[2] = 0;
        }
      }
      break;
    }
    default:
      return;
    }

    CVector3f startPoint = line.GetRefPoint() + lT * line.GetNormal();
    int childIndex = 0;
    for (int i = 0; i < 3; ++i) {
      if (startPoint[i] >= center[i]) {
        childIndex |= 1 << i;
      }
    }
    float childLowT = lT;
    for (int i = -1; i < count; ++i) {
      if (i >= 0) {
        childIndex ^= 1 << components[i];
      }
      float childHighT;
      if (i < count - 1) {
        childHighT = crossingTimes[components[i + 1]];
      } else {
        childHighT = hT;
      }
      if (childHighT > lowT && childLowT <= childHighT) {
        Node child = GetChild(childIndex);
        if (child.GetTreeType() != kTT_Invalid) {
          child.LineTestExInternal(line, filter, result, childLowT, childHighT, maxT, dirRecip);
        }
        if (result.mSurface.valid()) {
          if (result.mT > highT) {
            result = SRayResult();
          }
          return;
        }
      }
      childLowT = childHighT;
    }
  }
}

static void hack() {
  static float hack1 = 1.f;
  static float hack2 = 0.f;
}