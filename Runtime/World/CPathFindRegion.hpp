#pragma once

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"

namespace urde {
class CPFArea;
class CPFLink;
class CPFRegionData;

class CPFNode {
  zeus::CVector3f x0_position;
  zeus::CVector3f xc_normal;

public:
  CPFNode(CMemoryInStream& in);
  const zeus::CVector3f& GetPos() const { return x0_position; }
  const zeus::CVector3f& GetNormal() const { return xc_normal; }
};

class CPFLink {
  u32 x0_node;
  u32 x4_region;
  float x8_2dWidth;
  float xc_oo2dWidth;

public:
  CPFLink(CMemoryInStream& in);
  u32 GetNode() const { return x0_node; }
  u32 GetRegion() const { return x4_region; }
  float Get2dWidth() const { return x8_2dWidth; }
  float GetOO2dWidth() const { return xc_oo2dWidth; }
};

class CPFRegion {
  u32 x0_numNodes = 0;
  CPFNode* x4_startNode = nullptr;
  u32 x8_numLinks = 0;
  CPFLink* xc_startLink = nullptr;
  u32 x10_flags = 0;
  float x14_height = 0.f;
  zeus::CVector3f x18_normal;
  u32 x24_regionIdx = 0;
  zeus::CVector3f x28_centroid;
  zeus::CAABox x34_aabb;
  CPFRegionData* x4c_regionData;

public:
  CPFRegion() = default;
  CPFRegion(CMemoryInStream& in);
  void SetData(CPFRegionData* data) { x4c_regionData = data; }
  CPFRegionData* Data() const { return x4c_regionData; }
  u32 GetIndex() const { return x24_regionIdx; }
  float GetHeight() const { return x14_height; }
  const CPFLink* GetPathLink() const;
  u32 GetNumLinks() const { return x8_numLinks; }
  u32 GetFlags() const { return x10_flags; }
  const CPFLink* GetLink(u32 i) const { return xc_startLink + i; }
  void SetCentroid(const zeus::CVector3f& c) { x28_centroid = c; }
  const zeus::CVector3f& GetCentroid() const { return x28_centroid; }
  void Fixup(CPFArea& area, u32& maxRegionNodes);
  bool IsPointInside(const zeus::CVector3f& point) const;
  const zeus::CVector3f& GetNormal() const { return x18_normal; }
  u32 GetNumNodes() const { return x0_numNodes; }
  const CPFNode* GetNode(u32 i) const { return x4_startNode + i; }
  float PointHeight(const zeus::CVector3f& point) const;
  bool FindClosestPointOnPolygon(const std::vector<zeus::CVector3f>&, const zeus::CVector3f&, const zeus::CVector3f&,
                                 bool);
  bool FindBestPoint(std::vector<zeus::CVector3f>& polyPoints, const zeus::CVector3f& point, u32 flags,
                     float paddingSq);
  void SetLinkTo(s32 idx);
  void DropToGround(zeus::CVector3f& point) const;
  zeus::CVector3f GetLinkMidPoint(const CPFLink& link) const;
  zeus::CVector3f FitThroughLink2d(const zeus::CVector3f& p1, const CPFLink& link, const zeus::CVector3f& p2,
                                   float chRadius) const;
  zeus::CVector3f FitThroughLink3d(const zeus::CVector3f& p1, const CPFLink& link, float regionHeight,
                                   const zeus::CVector3f& p2, float chRadius, float chHalfHeight) const;
  bool IsPointInsidePaddedAABox(const zeus::CVector3f& point, float padding) const;
};

class CPFRegionData {
  float x0_bestPointDistSq = 0.f;
  zeus::CVector3f x4_bestPoint;
  s32 x10_cookie = -1;
  float x14_cost = 0.f;
  float x18_g = 0.f;
  float x1c_h = 0.f;
  CPFRegion* x20_parent = nullptr;
  CPFRegion* x24_openLess = nullptr;
  CPFRegion* x28_openMore = nullptr;
  s32 x2c_pathLink = 0;

public:
  CPFRegionData() = default;
  void SetOpenLess(CPFRegion* r) { x24_openLess = r; }
  void SetOpenMore(CPFRegion* r) { x28_openMore = r; }
  CPFRegion* GetOpenLess() const { return x24_openLess; }
  CPFRegion* GetOpenMore() const { return x28_openMore; }
  float GetCost() const { return x14_cost; }
  float GetG() const { return x18_g; }
  s32 GetPathLink() const { return x2c_pathLink; }
  void SetPathLink(s32 l) { x2c_pathLink = l; }
  CPFRegion* GetParent() const { return x20_parent; }
  void SetBestPoint(const zeus::CVector3f& bestPoint) { x4_bestPoint = bestPoint; }
  const zeus::CVector3f& GetBestPoint() const { return x4_bestPoint; }
  void SetBestPointDistanceSquared(float distSq) { x0_bestPointDistSq = distSq; }
  float GetBestPointDistanceSquared() const { return x0_bestPointDistSq; }
  void SetCookie(s32 c) { x10_cookie = c; }
  s32 GetCookie() const { return x10_cookie; }

  void Setup(CPFRegion* parent, float g, float h) {
    x20_parent = parent;
    x18_g = g;
    x1c_h = h;
    x14_cost = x18_g + x1c_h;
  }
  void Setup(CPFRegion* parent, float g) {
    x20_parent = parent;
    x18_g = g;
    x14_cost = x18_g + x1c_h;
  }
};
} // namespace urde
