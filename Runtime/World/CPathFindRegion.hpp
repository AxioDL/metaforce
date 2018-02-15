#ifndef __URDE_CPATHFINDREGION_HPP__
#define __URDE_CPATHFINDREGION_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CPFArea;
class CPFLink;
class CPFRegionData;

class CPFNode
{
    zeus::CVector3f x0_position;
    zeus::CVector3f xc_normal;
public:
    CPFNode(CMemoryInStream& in);
    const zeus::CVector3f& GetPos() const { return x0_position; }
    const zeus::CVector3f& GetNormal() const { return xc_normal; }
};

class CPFLink
{
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

class CPFRegion
{
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
    void GetPathLink() const {}
    u32 GetNumLinks() const { return x8_numLinks; }
    u32 GetFlags() const { return x10_flags; }
    const CPFLink* GetLink(u32 i) const { return xc_startLink + i; }
    void SetCentroid(const zeus::CVector3f&);
    const zeus::CVector3f& GetCentroid() const { return x28_centroid; }
    void Fixup(CPFArea& area, u32& maxRegionNodes);
    bool IsPointInside(const zeus::CVector3f&);
    const zeus::CVector3f& GetNormal() const { return x18_normal; }
    u32 GetNumNodes() const { return x0_numNodes; }
    const CPFNode* GetNode(u32 i) const { return x4_startNode + i; }
    void PointHeight(const zeus::CVector3f&);
    void FindClosestPointOnPolygon(const std::vector<zeus::CVector3f>&, const zeus::CVector3f&,
                                   const zeus::CVector3f&, bool);
    void FindBestPoint(std::vector<zeus::CVector3f>&, const zeus::CVector3f&, u32, float);
    void SetLinkTo(s32);
    void DropToGround(zeus::CVector3f&) const;
    void GetLinkMidPoint(const CPFLink&);
    zeus::CVector3f FitThroughLink2d(const zeus::CVector3f&, const CPFLink&, const zeus::CVector3f&, float) const;
    zeus::CVector3f FitThroughLink3d(const zeus::CVector3f&, const CPFLink&, float, const zeus::CVector3f&, float, float) const;
    void IsPointInsidePaddedAABox(const zeus::CVector3f&, float) const;
};

class CPFRegionData
{
    float x0_ = 0.f;
    zeus::CVector3f x4_;
    s32 x10_cookie = -1;
    zeus::CVector3f x14_;
    s32 x20_ = 0;
    CPFRegion* x24_openLess = nullptr;
    CPFRegion* x28_openMore = nullptr;
    s32 x2c_pathLink = 0;

public:
    CPFRegionData() = default;
    void SetOpenLess(CPFRegion* r) { x24_openLess = r; }
    void SetOpenMore(CPFRegion* r) { x28_openMore = r; }
    CPFRegion* GetOpenLess() const { return x24_openLess; }
    CPFRegion* GetOpenMore() const { return x28_openMore; }
    void GetCost();
    s32 GetPathLink() const { return x2c_pathLink; }
    void SetPathLink(s32 l) { x2c_pathLink = l; }
    void GetParent() const;
    void Setup(CPFRegion*, float, float);
    void SetBestPoint(const zeus::CVector3f&);
    void SetBestPointDistanceSquared(float);
    float GetBestPointDistanceSquared() const;
    zeus::CVector3f GetBestPoint() const;
    void SetCookie(s32 c) { x10_cookie = c; }
    s32 GetCookie() const { return x10_cookie; }
};
}

#endif // __URDE_CPATHFINDREGION_HPP__
