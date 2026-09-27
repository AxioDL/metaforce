#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"

#include "MetroidPrime/PathFinding/CPathFindArea.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector2f.hpp"

#include <float.h>

CPFRegionData::CPFRegionData()
: mBestPointDistSq(0.f)
, mBestPoint(CVector3f::Zero())
, mCookie(-1)
, mCost(0.f)
, mG(0.f)
, mH(0.f)
, mParent(nullptr)
, mOpenLess(nullptr)
, mOpenMore(nullptr)
, mParentLink(0) {}

#if !defined(TARGET_PC)
void CPFRegion::Fixup(CPFArea& area, int& numNodes) {
  mStartNode = mNumNodes ? &area.GetNode(reinterpret_cast< intptr_t >(mStartNode)) : nullptr;
  mStartLink = mNumLinks ? &area.GetLink(reinterpret_cast< intptr_t >(mStartLink)) : nullptr;
  mData = &area.GetRegionData(mRegionIdx);
  if (mNumNodes > numNodes) {
    numNodes = mNumNodes;
  }
}
#endif

CPFRegion::CPFRegion()
: mNumNodes(0)
, mStartNode(nullptr)
, mNumLinks(0)
, mStartLink(nullptr)
, mFlags(0)
, mHeight(0.f)
, mNormal(CVector3f::Zero())
, mRegionIdx(0)
, mCentroid(CVector3f::Zero())
, mBounds(CAABox::MakeMaxInvertedBox())
#if defined(TARGET_PC)
, mData(nullptr)
#endif
{
}

bool CPFRegion::IsPointInside(const CVector3f& point) const {
  bool inside = false;
  if (mBounds.PointInside(point)) {
    int i;
    for (i = 0; i < GetNumNodes(); ++i) {
      const CPFNode& node = GetNode(i);
      const CVector3f delta = point - node.GetPos();
      if (CVector3f::Dot(delta, node.GetNormal()) < 0.f) {
        break;
      }
    }
    if (i == GetNumNodes()) {
      CVector3f delta = point - GetNode(0).GetPos();
      if (CVector3f::Dot(delta, GetNormal()) >= 0.f) {
        const CVector3f height = GetHeight() * CVector3f::Up();
        delta -= height;
        if (CVector3f::Dot(delta, GetNormal()) <= 0.f) {
          inside = true;
        }
      }
    }
  }
  return inside;
}

float CPFRegion::PointHeight(const CVector3f& point) const {
  return CVector3f::Dot(point - GetNode(0).GetPos(), GetNormal());
}

bool CPFRegion::FindClosestPointOnPolygon(const rstl::vector< CVector3f >& polyPoints,
                                          const CVector3f& normal, const CVector3f& point,
                                          bool excludePolyPoints) {
  bool found = false;
  const int numPoints = polyPoints.size();
  int i;
  for (i = 0; i < numPoints; ++i) {
    const CVector3f delta = point - polyPoints[i];
    const CVector3f edge = polyPoints[(i + 1) % numPoints] - polyPoints[i];
    const CVector3f edgeNormal = CVector3f::Cross(edge, normal);
    if (CVector3f::Dot(delta, edgeNormal) < 0.f) {
      break;
    }
  }

  if (i == numPoints) {
    const CVector3f delta = polyPoints[0] - point;
    const float distance = CVector3f::Dot(delta, normal);
    const float distanceSq = distance * distance;
    if (distanceSq < Data()->GetBestDistanceSquared()) {
      found = true;
      Data()->SetBestDistanceSquared(distanceSq);
      Data()->SetBestPoint(point + normal * distance);
    }
  } else {
    bool projected = false;
    for (i = 0; i < numPoints; ++i) {
      CVector3f delta = point - polyPoints[(i + 1) % numPoints];
      CVector3f edge = polyPoints[(i + 1) % numPoints] - polyPoints[i];
      const CVector3f edgeNormal = CVector3f::Cross(edge, normal);
      if (CVector3f::Dot(delta, edgeNormal) < 0.f && CVector3f::Dot(delta, edge) <= 0.f) {
        delta += edge;
        if (CVector3f::Dot(delta, edge) >= 0.f) {
          projected = true;
          edge.Normalize();
          delta -= CVector3f::Dot(delta, edge) * edge;
          const float distanceSq = delta.MagSquared();
          if (distanceSq < Data()->GetBestDistanceSquared()) {
            found = true;
            Data()->SetBestDistanceSquared(distanceSq);
            Data()->SetBestPoint(point - delta);
          }
          break;
        }
      }
    }
    if (!projected && !excludePolyPoints) {
      for (i = 0; i < numPoints; ++i) {
        const CVector3f delta = point - polyPoints[i];
        const float distanceSq = delta.MagSquared();
        if (distanceSq < Data()->GetBestDistanceSquared()) {
          found = true;
          Data()->SetBestDistanceSquared(distanceSq);
          Data()->SetBestPoint(polyPoints[i]);
        }
      }
    }
  }
  return found;
}

bool CPFRegion::FindBestPoint(rstl::vector< CVector3f >& polyPoints, const CVector3f& point,
                              uint flags, float paddingSq) {
  int i;
  bool found = false;
  Data()->SetBestDistanceSquared(paddingSq);
  if (!(flags & 2)) {
    for (i = 0; i < GetNumNodes(); ++i) {
      const CPFNode& node = GetNode(i);
      const CPFNode& nextNode = GetNode((i + 1) % GetNumNodes());
      polyPoints.clear();
      polyPoints.push_back(node.GetPos());
      polyPoints.push_back(node.GetPos());
      polyPoints.back()[kDZ] += GetHeight();
      polyPoints.push_back(nextNode.GetPos());
      polyPoints.back()[kDZ] += GetHeight();
      polyPoints.push_back(nextNode.GetPos());
      found |= FindClosestPointOnPolygon(polyPoints, node.GetNormal(), point, true);
    }
  }
  polyPoints.clear();
  for (i = 0; i < GetNumNodes(); ++i) {
    polyPoints.push_back(GetNode(i).GetPos());
  }
  found |= FindClosestPointOnPolygon(polyPoints, GetNormal(), point, false);
  if (!(flags & 2)) {
    polyPoints.clear();
    for (i = GetNumNodes() - 1; i >= 0; --i) {
      polyPoints.push_back(GetNode(i).GetPos());
      polyPoints.back()[kDZ] += GetHeight();
    }
    found |= FindClosestPointOnPolygon(polyPoints, -GetNormal(), point, false);
  }
  return found;
}

void CPFRegion::SetLinkTo(int index) {
  for (int i = 0; i < GetNumLinks(); ++i) {
    if (GetLink(i)->GetRegion() == index) {
      Data()->SetPathLink(i);
      return;
    }
  }
}

void CPFRegion::DropToGround(CVector3f& point) const {
  point[kDZ] -= CVector3f::Dot(point - GetNode(0).GetPos(), GetNormal()) / GetNormal()[kDZ];
}

CVector3f CPFRegion::GetLinkMidPoint(const CPFLink& link) const {
  const CPFNode& node = GetNode(link.GetNode());
  const CPFNode& nextNode = GetNode((link.GetNode() + 1) % GetNumNodes());
  return (node.GetPos() + nextNode.GetPos()) * 0.5f;
}

CVector3f CPFRegion::FitThroughLink2d(const CVector3f& source, const CPFLink& link,
                                      const CVector3f& destination, float radius) const {
  const CPFNode& node = GetNode(link.GetNode());
  const CPFNode& nextNode = GetNode((link.GetNode() + 1) % GetNumNodes());
  const CVector3f edge = nextNode.GetPos() - node.GetPos();
  float t = 0.5f;
  if (radius < 0.5f * link.Get2dWidth()) {
    CVector2f edge2d = edge.DropZ();
    edge2d *= link.GetOO2dWidth();
    const CVector3f sourceDelta = source - node.GetPos();
    const float sourceDistance = CVector3f::Dot(sourceDelta, node.GetNormal());
    const float sourceAlong = CVector2f::Dot(edge2d, sourceDelta.DropZ());
    const CVector3f destinationDelta = destination - node.GetPos();
    const float destinationDistance = -CVector3f::Dot(destinationDelta, node.GetNormal());
    const float destinationAlong = CVector2f::Dot(edge2d, destinationDelta.DropZ());
    const float distance = sourceDistance + destinationDistance;
    if (distance > FLT_EPSILON) {
      t = (1.f / distance) *
          (destinationDistance * sourceAlong + sourceDistance * destinationAlong);
      t = CMath::Clamp(radius, t, link.Get2dWidth() - radius);
      t *= link.GetOO2dWidth();
    }
  }
  return node.GetPos() + edge * t;
}

CVector3f CPFRegion::FitThroughLink3d(const CVector3f& source, const CPFLink& link, float height,
                                      const CVector3f& destination, const float radius,
                                      float halfHeight) const {
  const CPFNode& node = GetNode(link.GetNode());
  const CPFNode& nextNode = GetNode((link.GetNode() + 1) % GetNumNodes());
  const CVector3f edge = nextNode.GetPos() - node.GetPos();
  const float sourceDistance = CVector3f::Dot(source - node.GetPos(), node.GetNormal());
  const float destinationDistance = CVector3f::Dot(node.GetPos() - destination, node.GetNormal());
  const float distance = sourceDistance + destinationDistance;
  float t = 0.5f;
  if (radius < 0.5f * link.Get2dWidth()) {
    CVector2f edge2d = edge.DropZ();
    edge2d *= link.GetOO2dWidth();
    const CVector3f& sourceDelta = source - node.GetPos();
    const float sourceAlong = CVector2f::Dot(edge2d, sourceDelta.DropZ());
    const CVector3f& destinationDelta = destination - node.GetPos();
    const float destinationAlong = CVector2f::Dot(edge2d, destinationDelta.DropZ());
    if (distance > FLT_EPSILON) {
      // The original discards this horizontal interpolation value.
      float t = (1.f / distance) *
                (destinationDistance * sourceAlong + sourceDistance * destinationAlong);
      const float maxT = link.Get2dWidth() - radius;
      t = CMath::Clamp(radius, t, maxT);
      t *= link.GetOO2dWidth();
    }
  }
  CVector3f result = node.GetPos() + edge * t;
  if (halfHeight < 0.5f * height) {
    const float minZ = halfHeight + result.GetZ();
    float z = 0.5f * (source.GetZ() + destination.GetZ());
    if (distance > FLT_EPSILON) {
      z = (1.f / distance) *
          (destinationDistance * source.GetZ() + sourceDistance * destination.GetZ());
    }
    result[kDZ] = CMath::Clamp(minZ, z, height + result.GetZ() - halfHeight);
  } else {
    result[kDZ] = 0.5f * (source.GetZ() + destination.GetZ());
  }
  return result;
}
