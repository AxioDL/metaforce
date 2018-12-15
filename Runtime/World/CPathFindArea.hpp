#pragma once

#include "IObj.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include "IFactory.hpp"
#include "CPathFindRegion.hpp"

namespace urde {
class CVParamTransfer;
class CObjectReference;

class CPFBitSet {
  u32 x0_bitmap[16] = {};

public:
  void Clear() {
    for (u32& w : x0_bitmap)
      w = 0;
  }
  void Add(s32 i) { x0_bitmap[i / 32] |= (1 << (i % 32)); }
  bool Test(s32 i) const { return (x0_bitmap[i / 32] & (1 << (i % 32))) != 0; }
  void Rmv(s32 i) { x0_bitmap[i / 32] &= ~(1 << (i % 32)); }
};

class CPFAreaOctree {
  u32 x0_isLeaf;
  zeus::CAABox x4_aabb;
  zeus::CVector3f x1c_center;
  CPFAreaOctree* x28_children[8];
  rstl::prereserved_vector<CPFRegion*> x48_regions;

public:
  CPFAreaOctree(CMemoryInStream& in);
  void Fixup(CPFArea& area);
  int GetChildIndex(const zeus::CVector3f& point) const;
  rstl::prereserved_vector<CPFRegion*>* GetRegionList(const zeus::CVector3f& point);
  void GetRegionListList(rstl::reserved_vector<rstl::prereserved_vector<CPFRegion*>*, 32>& listOut,
                         const zeus::CVector3f& point, float padding);
  bool IsPointInsidePaddedAABox(const zeus::CVector3f& point, float padding) const;
  void Render();
};

class CPFOpenList {
  friend class CPFArea;
  CPFBitSet x0_bitSet;
  CPFRegion x40_region;
  CPFRegionData x90_regionData;

public:
  CPFOpenList();
  void Clear();
  void Push(CPFRegion* reg);
  CPFRegion* Pop();
  void Pop(CPFRegion* reg);
  bool Test(CPFRegion* reg);
};

class CPFArea {
  friend class CPFRegion;
  friend class CPFAreaOctree;
  friend class CPathFindSearch;

  float x0_ = FLT_MAX;
  zeus::CVector3f x4_closestPoint;
  std::vector<zeus::CVector3f> x10_tmpPolyPoints;
  rstl::prereserved_vector<CPFRegion*>* x20_cachedRegionList = nullptr;
  zeus::CVector3f x24_cachedRegionListPoint;
  bool x30_hasCachedRegionList = false;
  s32 x34_curCookie = 0;
  CPFBitSet x38_closedSet;
  CPFOpenList x78_openList;
  // u32 x138_;
  // std::unique_ptr<u8[]> x13c_data;
  /* Used to be prereserved_vectors backed by x13c_data
   * This has been changed to meet storage requirements of
   * modern systems */
  std::vector<CPFNode> x140_nodes;                 // x140: count, x144: ptr
  std::vector<CPFLink> x148_links;                 // x148: count, x14c: ptr
  std::vector<CPFRegion> x150_regions;             // x150: count, x154: ptr
  std::vector<CPFAreaOctree> x158_octree;          // x158: count, x15c: ptr
  std::vector<CPFRegion*> x160_octreeRegionLookup; // x160: count, x164: ptr
  std::vector<u32> x168_connectionsGround;         // x168: word_count, x16c: ptr
  std::vector<u32> x170_connectionsFlyers;         // x170: word_count, x174: ptr
  std::vector<CPFRegionData> x178_regionDatas;
  zeus::CTransform x188_transform;

public:
  CPFArea(std::unique_ptr<u8[]>&& buf, u32 len);

  void SetTransform(const zeus::CTransform& xf) { x188_transform = xf; }
  const zeus::CTransform& GetTransform() const { return x188_transform; }
  const CPFRegion& GetRegion(s32 i) const { return x150_regions[i]; }
  const zeus::CVector3f& GetClosestPoint() const { return x4_closestPoint; }
  CPFOpenList& OpenList() { return x78_openList; }
  CPFBitSet& ClosedSet() { return x38_closedSet; }
  const CPFRegionData& GetRegionData(s32 i) const { return x178_regionDatas[i]; }
  const CPFLink& GetLink(s32 i) const { return x148_links[i]; }
  const CPFNode& GetNode(s32 i) const { return x140_nodes[i]; }
  const CPFAreaOctree& GetOctree(s32 i) const { return x158_octree[i]; }
  const CPFRegion* GetOctreeRegionPtrs(s32 i) const { return x160_octreeRegionLookup[i]; }
  rstl::prereserved_vector<CPFRegion*>* GetOctreeRegionList(const zeus::CVector3f& point);
  u32 FindRegions(rstl::reserved_vector<CPFRegion*, 4>& regions, const zeus::CVector3f& point, u32 flags,
                  u32 indexMask);
  CPFRegion* FindClosestRegion(const zeus::CVector3f& point, u32 flags, u32 indexMask, float padding);
  zeus::CVector3f FindClosestReachablePoint(rstl::reserved_vector<CPFRegion*, 4>& regs, const zeus::CVector3f& point,
                                            u32 flags, u32 indexMask);
  bool PathExists(const CPFRegion* r1, const CPFRegion* r2, u32 flags) const;
};

CFactoryFnReturn FPathFindAreaFactory(const urde::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                      const urde::CVParamTransfer& vparms, CObjectReference* selfRef);
} // namespace urde
