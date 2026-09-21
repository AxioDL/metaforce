#ifndef _CPATHFINDREGION
#define _CPATHFINDREGION

#include <Kyoto/Math/CAABox.hpp>
#include <Kyoto/Math/CVector3f.hpp>
#include <rstl/vector.hpp>

class CPFArea;
class CPFNode {
  friend class CPFArea;

public:
  const CVector3f& GetPos() const { return x0_position; }
  const CVector3f& GetNormal() const { return xc_normal; }

private:
  CVector3f x0_position;
  CVector3f xc_normal;
};
CHECK_SIZEOF(CPFNode, 0x18)

class CPFLink {
  friend class CPFArea;

public:
  int GetNode() const { return x0_node; }
  int GetRegion() const { return x4_region; }
  float Get2dWidth() const { return x8_2dWidth; }
  float GetOO2dWidth() const { return xc_oo2dWidth; }

private:
  int x0_node;
  int x4_region;
  float x8_2dWidth;
  float xc_oo2dWidth;
};
CHECK_SIZEOF(CPFLink, 0x10)

class CPFRegion;
class CPFRegionData {
public:
  CPFRegionData();

  void SetOpenLess(CPFRegion* region) { x24_openLess = region; }
  CPFRegion* GetOpenLess() { return x24_openLess; }

  void SetOpenMore(CPFRegion* region) { x28_openMore = region; }
  CPFRegion* GetOpenMore() { return x28_openMore; }

  float GetCost() { return x14_cost; }

  CPFRegion* GetParent() { return x20_parent; }
  void Setup(CPFRegion* region, float g) {
    x20_parent = region;
    x18_g = g;
    x14_cost = x18_g + x1c_h;
  }
  void Setup(CPFRegion* region, float g, float h) {
    x20_parent = region;
    x18_g = g;
    x1c_h = h;
    x14_cost = x18_g + x1c_h;
  }
  float GetG() { return x18_g; }
  int GetPathLink() const { return x2c_parentLink; }
  void SetPathLink(int link) { x2c_parentLink = link; }

  void SetBestPoint(const CVector3f& point) { x4_bestPoint = point; }
  const CVector3f& GetBestPoint() const { return x4_bestPoint; }
  void SetBestDistanceSquared(float distance) { x0_bestPointDistSq = distance; }
  float GetBestDistanceSquared() const { return x0_bestPointDistSq; }

  void SetCookie(int cookie) { x10_cookie = cookie; }
  int GetCookie() const { return x10_cookie; }

private:
  float x0_bestPointDistSq;
  CVector3f x4_bestPoint;
  int x10_cookie;
  float x14_cost;
  float x18_g;
  float x1c_h;
  CPFRegion* x20_parent;
  CPFRegion* x24_openLess;
  CPFRegion* x28_openMore;
  int x2c_parentLink;
};
CHECK_SIZEOF(CPFRegionData, 0x30)

class CPFRegion {
  friend class CPFArea;

public:
  CPFRegion();

#if !defined(TARGET_PC)
  void Fixup(CPFArea& area, int& numNodes);
#endif
  void SetData(CPFRegionData* data) { x4c_data = data; }
  CPFRegionData* Data() const { return x4c_data; }
  int GetIndex() const { return x24_regionIdx; }
  uint GetFlags() const { return x10_flags; }
  int GetNumLinks() const { return x8_numLinks; }
  const CPFLink* GetLink(int index) const { return &xc_startLink[index]; }
  const CPFLink* GetPathLink() const { return &xc_startLink[x4c_data->GetPathLink()]; }
  const CVector3f& GetCentroid() const { return x28_centroid; }
  void SetCentroid(const CVector3f& point) { x28_centroid = point; }
  float GetHeight() const { return x14_height; }
  int GetNumNodes() const { return x0_numNodes; }
  const CPFNode& GetNode(int index) const { return x4_startNode[index]; }
  const CVector3f& GetNormal() const { return x18_normal; }
  bool IsPointInside(const CVector3f& point) const;
  bool IsPointInsidePaddedAABox(const CVector3f& point, float padding) const {
    return point[kDX] >= x34_bounds.GetMinPoint()[kDX] - padding &&
           point[kDX] <= x34_bounds.GetMaxPoint()[kDX] + padding &&
           point[kDY] >= x34_bounds.GetMinPoint()[kDY] - padding &&
           point[kDY] <= x34_bounds.GetMaxPoint()[kDY] + padding &&
           point[kDZ] >= x34_bounds.GetMinPoint()[kDZ] - padding &&
           point[kDZ] <= x34_bounds.GetMaxPoint()[kDZ] + padding;
  }
  float PointHeight(const CVector3f& point) const;
  bool FindClosestPointOnPolygon(const rstl::vector< CVector3f >& polyPoints,
                                 const CVector3f& normal, const CVector3f& point,
                                 bool excludePolyPoints);
  bool FindBestPoint(rstl::vector< CVector3f >& polyPoints, const CVector3f& point, uint flags,
                     float paddingSq);
  void SetLinkTo(int index);
  void DropToGround(CVector3f& point) const;
  CVector3f GetLinkMidPoint(const CPFLink& link) const;
  CVector3f FitThroughLink2d(const CVector3f& source, const CPFLink& link,
                             const CVector3f& destination, float radius) const;
  CVector3f FitThroughLink3d(const CVector3f& source, const CPFLink& link, float height,
                             const CVector3f& destination, const float radius,
                             float halfHeight) const;

private:
  int x0_numNodes;
  const CPFNode* x4_startNode;
  int x8_numLinks;
  CPFLink* xc_startLink;
  uint x10_flags;
  float x14_height;
  CVector3f x18_normal;
  int x24_regionIdx;
  CVector3f x28_centroid;
  CAABox x34_bounds;
  CPFRegionData* x4c_data;
};
CHECK_SIZEOF(CPFRegion, 0x50)

#endif // _CPATHFINDREGION
