#include "Runtime/World/CPathFindRegion.hpp"

#include "Runtime/World/CPathFindArea.hpp"

namespace urde {

CPFNode::CPFNode(CMemoryInStream& in) {
  x0_position.readBig(in);
  xc_normal.readBig(in);
}

CPFLink::CPFLink(CMemoryInStream& in) {
  x0_node = in.readUint32Big();
  x4_region = in.readUint32Big();
  x8_2dWidth = in.readFloatBig();
  xc_oo2dWidth = in.readFloatBig();
}

CPFRegion::CPFRegion(CMemoryInStream& in) {
  x0_numNodes = in.readUint32Big();
  x4_startNode = reinterpret_cast<CPFNode*>(in.readUint32Big());
  x8_numLinks = in.readUint32Big();
  xc_startLink = reinterpret_cast<CPFLink*>(in.readUint32Big());
  x10_flags = in.readUint32Big();
  x14_height = in.readFloatBig();
  x18_normal.readBig(in);
  x24_regionIdx = in.readUint32Big();
  x28_centroid.readBig(in);
  x34_aabb.readBoundingBoxBig(in);
  x4c_regionData = reinterpret_cast<CPFRegionData*>(in.readUint32Big());
}

const CPFLink* CPFRegion::GetPathLink() const { return &xc_startLink[x4c_regionData->GetPathLink()]; }

void CPFRegion::Fixup(CPFArea& area, u32& maxRegionNodes) {
  if (x0_numNodes)
    x4_startNode = &area.x140_nodes[reinterpret_cast<uintptr_t>(x4_startNode)];
  else
    x4_startNode = nullptr;
  if (x8_numLinks)
    xc_startLink = &area.x148_links[reinterpret_cast<uintptr_t>(xc_startLink)];
  else
    xc_startLink = nullptr;
  x4c_regionData = &area.x178_regionDatas[x24_regionIdx];
  if (x0_numNodes > maxRegionNodes)
    maxRegionNodes = x0_numNodes;
}

bool CPFRegion::IsPointInside(const zeus::CVector3f& point) const {
  if (!x34_aabb.pointInside(point))
    return false;
  u32 i;
  for (i = 0; i < x0_numNodes; ++i) {
    CPFNode& node = x4_startNode[i];
    if ((point - node.GetPos()).dot(node.GetNormal()) < 0.f)
      break;
  }
  if (i != x0_numNodes)
    return false;
  zeus::CVector3f nodeToPoint = point - x4_startNode->GetPos();
  return nodeToPoint.dot(x18_normal) >= 0.f &&
         (nodeToPoint - x14_height * zeus::skUp).dot(x18_normal) <= 0.f;
}

float CPFRegion::PointHeight(const zeus::CVector3f& point) const {
  return (point - x4_startNode->GetPos()).dot(x18_normal);
}

bool CPFRegion::FindClosestPointOnPolygon(const std::vector<zeus::CVector3f>& polyPoints, const zeus::CVector3f& normal,
                                          const zeus::CVector3f& point, bool excludePolyPoints) const {
  bool found = false;
  size_t i;
  for (i = 0; i < polyPoints.size(); ++i) {
    const zeus::CVector3f& p0 = polyPoints[i];
    const zeus::CVector3f& p1 = polyPoints[(i + 1) % polyPoints.size()];
    if ((p1 - p0).cross(normal).dot(point - p0) < 0.f)
      break;
  }

  if (i == polyPoints.size()) {
    float distToPoly = (polyPoints.front() - point).dot(normal);
    float distToPolySq = distToPoly * distToPoly;
    if (distToPolySq < x4c_regionData->GetBestPointDistanceSquared()) {
      found = true;
      x4c_regionData->SetBestPointDistanceSquared(distToPolySq);
      x4c_regionData->SetBestPoint(normal * distToPoly + point);
    }
  } else {
    bool projected = false;
    for (i = 0; i < polyPoints.size(); ++i) {
      const zeus::CVector3f& p0 = polyPoints[i];
      const zeus::CVector3f& p1 = polyPoints[(i + 1) % polyPoints.size()];
      zeus::CVector3f p0ToP1 = p1 - p0;
      zeus::CVector3f p1ToPoint = point - p1;
      zeus::CVector3f sum = p1ToPoint + p0ToP1;
      if (p0ToP1.cross(normal).dot(p1ToPoint) < 0.f && p0ToP1.dot(p1ToPoint) <= 0.f && sum.dot(p0ToP1) >= 0.f) {
        projected = true;
        p0ToP1.normalize();
        sum -= p0ToP1.dot(sum) * p0ToP1;
        float distSq = sum.magSquared();
        if (distSq < x4c_regionData->GetBestPointDistanceSquared()) {
          found = true;
          x4c_regionData->SetBestPointDistanceSquared(distSq);
          x4c_regionData->SetBestPoint(point - sum);
        }
        break;
      }
    }

    if (!projected && !excludePolyPoints) {
      for (i = 0; i < polyPoints.size(); ++i) {
        const zeus::CVector3f& p0 = polyPoints[i];
        float distSq = (point - p0).magSquared();
        if (distSq < x4c_regionData->GetBestPointDistanceSquared()) {
          found = true;
          x4c_regionData->SetBestPointDistanceSquared(distSq);
          x4c_regionData->SetBestPoint(p0);
        }
      }
    }
  }
  return found;
}

bool CPFRegion::FindBestPoint(std::vector<zeus::CVector3f>& polyPoints, const zeus::CVector3f& point, u32 flags,
                              float paddingSq) const {
  bool found = false;
  bool isFlyer = (flags & 0x2) != 0;
  x4c_regionData->SetBestPointDistanceSquared(paddingSq);
  if (!isFlyer) {
    for (u32 i = 0; i < x0_numNodes; ++i) {
      CPFNode& node = x4_startNode[i];
      CPFNode& nextNode = x4_startNode[(i + 1) % x0_numNodes];
      polyPoints.clear();
      polyPoints.push_back(node.GetPos());
      polyPoints.push_back(node.GetPos());
      polyPoints.back().z() += x14_height;
      polyPoints.push_back(nextNode.GetPos());
      polyPoints.back().z() += x14_height;
      polyPoints.push_back(nextNode.GetPos());
      found |= FindClosestPointOnPolygon(polyPoints, node.GetNormal(), point, true);
    }
  }

  polyPoints.clear();
  for (u32 i = 0; i < x0_numNodes; ++i) {
    CPFNode& node = x4_startNode[i];
    polyPoints.push_back(node.GetPos());
  }
  found |= FindClosestPointOnPolygon(polyPoints, x18_normal, point, false);

  if (!isFlyer) {
    polyPoints.clear();
    for (int i = int(x0_numNodes) - 1; i >= 0; --i) {
      CPFNode& node = x4_startNode[i];
      polyPoints.push_back(node.GetPos());
      polyPoints.back().z() += x14_height;
    }
    found |= FindClosestPointOnPolygon(polyPoints, -x18_normal, point, false);
  }

  return found;
}

void CPFRegion::SetLinkTo(s32 idx) {
  if (x8_numLinks <= 0)
    return;
  for (u32 i = 0; i < x8_numLinks; ++i)
    if (xc_startLink[i].GetRegion() == idx) {
      Data()->SetPathLink(i);
      return;
    }
}

void CPFRegion::DropToGround(zeus::CVector3f& point) const {
  point.z() -= (point - x4_startNode->GetPos()).dot(x18_normal) / x18_normal.z();
}

zeus::CVector3f CPFRegion::GetLinkMidPoint(const CPFLink& link) const {
  const CPFNode& node = x4_startNode[link.GetNode()];
  const CPFNode& nextNode = x4_startNode[(link.GetNode() + 1) % x0_numNodes];
  return (node.GetPos() + nextNode.GetPos()) * 0.5f;
}

zeus::CVector3f CPFRegion::FitThroughLink2d(const zeus::CVector3f& p1, const CPFLink& link, const zeus::CVector3f& p2,
                                            float chRadius) const {
  CPFNode& node = x4_startNode[link.GetNode()];
  CPFNode& nextNode = x4_startNode[(link.GetNode() + 1) % x0_numNodes];
  zeus::CVector3f nodeDelta = nextNode.GetPos() - node.GetPos();
  float t = 0.5f;
  if (chRadius < 0.5f * link.Get2dWidth()) {
    zeus::CVector2f delta2d = nodeDelta.toVec2f();
    delta2d *= link.GetOO2dWidth();
    zeus::CVector3f nodeToP1 = p1 - node.GetPos();
    float f27 = nodeToP1.dot(node.GetNormal());
    float f31 = delta2d.dot(nodeToP1.toVec2f());
    zeus::CVector3f nodeToP2 = p2 - node.GetPos();
    float f26 = -nodeToP2.dot(node.GetNormal());
    float f1b = delta2d.dot(nodeToP2.toVec2f());
    float f3 = f27 + f26;
    if (f3 > FLT_EPSILON)
      t = zeus::clamp(chRadius, 1.f / f3 * (f26 * f31 + f27 * f1b), link.Get2dWidth() - chRadius) * link.GetOO2dWidth();
  }
  return nodeDelta * t + node.GetPos();
}

zeus::CVector3f CPFRegion::FitThroughLink3d(const zeus::CVector3f& p1, const CPFLink& link, float regionHeight,
                                            const zeus::CVector3f& p2, float chRadius, float chHalfHeight) const {
  CPFNode& node = x4_startNode[link.GetNode()];
  CPFNode& nextNode = x4_startNode[(link.GetNode() + 1) % x0_numNodes];
  zeus::CVector3f nodeDelta = nextNode.GetPos() - node.GetPos();
  float f25 = (p1 - node.GetPos()).dot(node.GetNormal());
  float f24 = (node.GetPos() - p2).dot(node.GetNormal());
  float f23 = f25 + f24;
#if 0
  if (chRadius < 0.5f * link.Get2dWidth()) {
    zeus::CVector2f delta2d(nodeDelta.x, nodeDelta.y);
    delta2d *= link.GetOO2dWidth();
    zeus::CVector3f nodeToP1 = p1 - node.GetPos();
    float f29 = delta2d.dot(zeus::CVector2f(nodeToP1.y, nodeToP1.y));
    zeus::CVector3f nodeToP2 = p2 - node.GetPos();
    float f1b = delta2d.dot(zeus::CVector2f(nodeToP2.y, nodeToP2.y));
    if (f23 > FLT_EPSILON) {
      zeus::clamp(chRadius, 1.f / f23 * f24 * f29 + f25 * f1b, link.Get2dWidth() - chRadius) *
          link.GetOO2dWidth();
    }
  }
#endif
  zeus::CVector3f midPoint = nodeDelta * 0.5f + node.GetPos();
  float z;
  if (chHalfHeight < 0.5f * regionHeight) {
    float minZ = chHalfHeight + midPoint.z();
    z = 0.5f * (p1.z() + p2.z());
    if (f23 > FLT_EPSILON)
      z = (f24 * p1.z() + f25 * p2.z()) / f23;
    z = zeus::clamp(minZ, z, regionHeight + midPoint.z() - chHalfHeight);
  } else {
    z = (p1.z() + p2.z()) * 0.5f;
  }
  return {midPoint.x(), midPoint.y(), z};
}

bool CPFRegion::IsPointInsidePaddedAABox(const zeus::CVector3f& point, float padding) const {
  return point.x() >= x34_aabb.min.x() - padding && point.x() <= x34_aabb.max.x() + padding &&
         point.y() >= x34_aabb.min.y() - padding && point.y() <= x34_aabb.max.y() + padding &&
         point.z() >= x34_aabb.min.z() - padding && point.z() <= x34_aabb.max.z() + padding;
}

} // namespace urde
