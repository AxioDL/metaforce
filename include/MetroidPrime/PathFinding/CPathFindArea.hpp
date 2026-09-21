#ifndef _CPATHFINDAREA
#define _CPATHFINDAREA

#include "MetroidPrime/PathFinding/CPFBitSet.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/prereserved_vector.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

#if defined(TARGET_PC)
#include <span>
#endif

class CPFOpenList {
public:
  CPFOpenList();
  void Clear() {
    x40_region.Data()->SetOpenMore(&x40_region);
    x40_region.Data()->SetOpenLess(&x40_region);
    x0_bitSet.Clear();
  }
  void Push(CPFRegion* region) {
    x0_bitSet.Add(region->GetIndex());
    CPFRegion* more = x40_region.Data()->GetOpenMore();
    while (more != &x40_region && region->Data()->GetCost() > more->Data()->GetCost()) {
      more = more->Data()->GetOpenMore();
    }
    more->Data()->GetOpenLess()->Data()->SetOpenMore(region);
    region->Data()->SetOpenLess(more->Data()->GetOpenLess());
    more->Data()->SetOpenLess(region);
    region->Data()->SetOpenMore(more);
  }
  CPFRegion* Pop() {
    CPFRegion* region = x40_region.Data()->GetOpenMore();
    CPFRegion* result = nullptr;
    if (region != &x40_region) {
      result = region;
      Pop(region);
    }
    return result;
  }
  void Pop(CPFRegion* region) {
    x0_bitSet.Rmv(region->GetIndex());
    region->Data()->GetOpenMore()->Data()->SetOpenLess(region->Data()->GetOpenLess());
    region->Data()->GetOpenLess()->Data()->SetOpenMore(region->Data()->GetOpenMore());
    region->Data()->SetOpenMore(nullptr);
    region->Data()->SetOpenLess(nullptr);
  }
  bool Test(CPFRegion* region) {
    if (x0_bitSet.Test(region->GetIndex())) {
      return true;
    }
    return false;
  }

private:
  CPFBitSet x0_bitSet;
  CPFRegion x40_region;
  CPFRegionData x90_regionData;
};
CHECK_SIZEOF(CPFOpenList, 0xc0)

class CPFAreaOctree {
  friend class CPFArea;

public:
#if !defined(TARGET_PC)
  void Fixup(CPFArea& area);
#endif
  uint GetChildIndex(const CVector3f& point) const;
  prereserved_vector< CPFRegion* >* GetRegionList(const CVector3f& point);
  void GetRegionListList(rstl::reserved_vector< prereserved_vector< CPFRegion* >*, 32 >& lists,
                         const CVector3f& point, float padding);
  bool IsPointInsidePaddedAABox(const CVector3f& point, float padding) const {
    return point[kDX] >= x4_bounds.GetMinPoint()[kDX] - padding &&
           point[kDX] <= x4_bounds.GetMaxPoint()[kDX] + padding &&
           point[kDY] >= x4_bounds.GetMinPoint()[kDY] - padding &&
           point[kDY] <= x4_bounds.GetMaxPoint()[kDY] + padding &&
           point[kDZ] >= x4_bounds.GetMinPoint()[kDZ] - padding &&
           point[kDZ] <= x4_bounds.GetMaxPoint()[kDZ] + padding;
  }

private:
  bool x0_isLeaf;
  CAABox x4_bounds;
  CVector3f x1c_center;
  CPFAreaOctree* x28_children[8];
  prereserved_vector< CPFRegion* > x48_regions;
};
CHECK_SIZEOF(CPFAreaOctree, 0x50)

class CPFAreaVersion {
public:
  CPFAreaVersion() {}
  ~CPFAreaVersion() {}
};
CHECK_SIZEOF(CPFAreaVersion, 1)

class CPFArea {
public:
  CPFArea(const rstl::auto_ptr< uchar >& data, int size);
#if defined(TARGET_PC)
  CPFArea(const CPFArea&) = delete;
  CPFArea& operator=(const CPFArea&) = delete;
#endif

  const CTransform4f& GetTransform() const { return x188_transform; }
  void SetTransform(const CTransform4f& transform) { x188_transform = transform; }
  CVector3f GetClosestPoint() const { return x4_closestPoint; }
  int GetNumRegions() const { return x150_regions.size(); }
  CPFRegion& GetRegion(int index) { return x150_regions[index]; }
  const CPFNode& GetNode(int index) const { return x140_nodes[index]; }
  CPFLink& GetLink(int index) { return x148_links[index]; }
  CPFRegionData& GetRegionData(int index) { return x178_regionData[index]; }
  CPFAreaOctree& GetOctree(int index) { return x158_octree[index]; }
  CPFRegion*& GetOctreeRegionPtrs(int index) { return x160_octreeRegions[index]; }
  prereserved_vector< CPFRegion* >* GetOctreeRegionList(const CVector3f& point);
  CPFOpenList& OpenList() { return x78_openList; }
  CPFBitSet& ClosedSet() { return x38_closedSet; }
  int FindRegions(rstl::reserved_vector< CPFRegion*, 4 >& regions, const CVector3f& point,
                  uint flags, uint indexMask);
  CPFRegion* FindClosestRegion(const CVector3f& point, uint flags, uint indexMask, float padding);
  CVector3f FindClosestReachablePoint(rstl::reserved_vector< CPFRegion*, 4 >& regions,
                                      const CVector3f& point, uint flags, uint indexMask);
  bool PathExists(const CPFRegion* source, const CPFRegion* destination, uint flags) const;

private:
  float x0_bestPointDistSq;
  CVector3f x4_closestPoint;
  rstl::vector< CVector3f > x10_polyPoints;
  prereserved_vector< CPFRegion* >* x20_cachedRegionList;
  CVector3f x24_cachedRegionListPoint;
  bool x30_hasCachedRegionList;
  int x34_regionFindCookie;
  CPFBitSet x38_closedSet;
  CPFOpenList x78_openList;
  CPFAreaVersion x138_version;
#if defined(TARGET_PC)
  rstl::vector< CPFNode > x140_nodes;
  rstl::vector< CPFLink > x148_links;
  rstl::vector< CPFRegion > x150_regions;
  rstl::vector< CPFAreaOctree > x158_octree;
  rstl::vector< CPFRegion* > x160_octreeRegions;
  rstl::vector< uint > x168_connectionsGround;
  rstl::vector< uint > x170_connectionsFlyers;
#else
  rstl::single_ptr< uchar > x13c_data;
  prereserved_vector< CPFNode > x140_nodes;
  prereserved_vector< CPFLink > x148_links;
  prereserved_vector< CPFRegion > x150_regions;
  prereserved_vector< CPFAreaOctree > x158_octree;
  prereserved_vector< CPFRegion* > x160_octreeRegions;
  prereserved_vector< uint > x168_connectionsGround;
  prereserved_vector< uint > x170_connectionsFlyers;
#endif
  rstl::vector< CPFRegionData > x178_regionData;
  CTransform4f x188_transform;

#if defined(TARGET_PC)
  void ReadData(std::span< const uchar > data);
#endif
};
CHECK_SIZEOF(CPFArea, 0x1b8)

#if !defined(TARGET_PC)
inline void CPFAreaOctree::Fixup(CPFArea& area) {
  x0_isLeaf = *reinterpret_cast< const int* >(this) != 0;
  if (x0_isLeaf) {
    if (x48_regions.size() != 0) {
      x48_regions.set_data(
          &area.GetOctreeRegionPtrs(reinterpret_cast< intptr_t >(&x48_regions[0])));
    }
  } else {
    for (int i = 0; i < 8; ++i) {
      intptr_t index = reinterpret_cast< intptr_t >(x28_children[i]);
      x28_children[i] = index >= 0 ? &area.GetOctree(index) : nullptr;
    }
  }
}
#endif

#endif // _CPATHFINDAREA
