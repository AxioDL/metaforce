#ifndef CPATHFINDAREA_HPP
#define CPATHFINDAREA_HPP

#include "IObj.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include "IFactory.hpp"
#include "CPathFindRegion.hpp"

namespace urde
{
class CVParamTransfer;
class CObjectReference;

class CPFBitSet
{
public:
    CPFBitSet() = default;
    void Clear();
    void Add(s32);
    bool Test(s32);
    void Rmv(s32);
};

class CPFAreaOctree
{
    u32 x0_isLeaf;
    zeus::CVector3f x4_points[3];
    CPFAreaOctree* x28_children[8];
    u32 x48_regionCount;
    CPFRegion** x4c_regions;
public:
    CPFAreaOctree(CMemoryInStream& in);
    void Fixup(CPFArea& area);
    void GetChildIndex(const zeus::CVector3f&) const;
    void GetRegionList(const zeus::CVector3f&) const;
    //void GetRegionListList(rstl::reserved_vector<rstl::prereserved_vector<CPFRegion>, 32>, const zeus::CVector3f&, float);
    bool IsPointInPaddedAABox(const zeus::CVector3f&, float);
    void Render();
};

class CPFOpenList
{
    friend class CPFArea;
    u32 x0_ = 0;
    u32 x4_ = 0;
    u32 x8_ = 0;
    u32 xc_ = 0;
    u32 x10_ = 0;
    u32 x14_ = 0;
    u32 x18_ = 0;
    u32 x1c_ = 0;
    u32 x20_ = 0;
    u32 x24_ = 0;
    u32 x28_ = 0;
    u32 x2c_ = 0;
    u32 x30_ = 0;
    u32 x34_ = 0;
    u32 x38_ = 0;
    u32 x3c_ = 0;
    CPFRegion x40_region;
    CPFRegionData x90_regionData;

public:
    CPFOpenList();

    void Clear();
    void Push(CPFRegion*);
    void Pop();
    void Pop(CPFRegion*);
    void Test(CPFRegion*);
};

class CPFArea
{
    friend class CPFRegion;
    friend class CPFAreaOctree;

    float x0_ = FLT_MAX;
    zeus::CVector3f x4_;
    std::vector<zeus::CVector3f> x10_;
    u32 x20_ = 0;
    zeus::CVector3f x24_;
    bool x30_ = false;
    u32 x34_ = 0;
    u32 x38_ = 0;
    u32 x3c_ = 0;
    u32 x40_ = 0;
    u32 x44_ = 0;
    u32 x48_ = 0;
    u32 x4c_ = 0;
    u32 x50_ = 0;
    u32 x54_ = 0;
    u32 x58_ = 0;
    u32 x5c_ = 0;
    u32 x60_ = 0;
    u32 x64_ = 0;
    u32 x68_ = 0;
    u32 x6c_ = 0;
    u32 x70_ = 0;
    u32 x74_ = 0;
    CPFOpenList x78_;
    u32 x138_;
    //std::unique_ptr<u8[]> x13c_data;
    std::vector<CPFNode> x140_nodes; // x140: count, x144: ptr
    std::vector<CPFLink> x148_links; // x148: count, x14c: ptr
    std::vector<CPFRegion> x150_regions; // x150: count, x154: ptr
    std::vector<CPFAreaOctree> x158_octree; // x158: count, x15c: ptr
    std::vector<CPFRegion*> x160_octreeRegionLookup; // x160: count, x164: ptr
    std::vector<u32> x168_connectionsA; // x168: word_count, x16c: ptr
    std::vector<u32> x170_connectionsB; // x170: word_count, x174: ptr
    std::vector<CPFRegionData> x178_regionDatas;
    zeus::CTransform x188_transform;
public:
    CPFArea(std::unique_ptr<u8[]>&& buf, u32 len);

    void SetTransform(const zeus::CTransform& xf) { x188_transform = xf; }
    const zeus::CTransform& GetTransform() const { return x188_transform; }
    const CPFRegion& GetRegion(s32 i) const { return x150_regions[i]; }
    void GetClosestPoint() const;
    void OpenList();
    void ClosedSet();
    const CPFRegionData& GetRegionData(s32 i) const { return x178_regionDatas[i]; }
    const CPFLink& GetLink(s32 i) const { return x148_links[i]; }
    const CPFNode& GetNode(s32 i) const { return x140_nodes[i]; }
    const CPFAreaOctree& GetOctree(s32 i) const { return x158_octree[i]; }
    const CPFRegion* GetOctreeRegionPtrs(s32 i) const { return x160_octreeRegionLookup[i]; }
    void GetOctreeRegionList(const zeus::CVector3f&);
    void FindRegions(rstl::reserved_vector<CPFRegion, 4>&, const zeus::CVector3f&, u32);
    void FindClosestRegion(const zeus::CVector3f&, u32, float);
    void FindClosestReachablePoint(rstl::reserved_vector<CPFRegion, 4>&, const zeus::CVector3f&, u32);
};


CFactoryFnReturn FPathFindAreaFactory(const urde::SObjectTag& tag,
                                      std::unique_ptr<u8[]>&& in, u32 len,
                                      const urde::CVParamTransfer& vparms,
                                      CObjectReference* selfRef);
}

#endif // CPATHFINDAREA_HPP
