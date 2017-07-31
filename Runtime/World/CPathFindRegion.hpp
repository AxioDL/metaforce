#ifndef __URDE_CPATHFINDREGION_HPP__
#define __URDE_CPATHFINDREGION_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CPFArea;
class CPFLink;
class CPFRegionData;
class CPFRegion
{
    u32 x0_ = 0;
    u32 x4_ = 0;
    u32 x8_ = 0;
    u32 xc_ = 0;
    u32 x10_ = 0;
    float x14_ = 0.f;
    zeus::CVector3f x18_;
    u32 x24_ = 0;
    zeus::CVector3f x28_;
    zeus::CAABox x34_;
    u32 x4c_;
public:
    CPFRegion() = default;
    void SetData(CPFRegionData*) {}
    CPFRegionData* Data() const;
    void GetIndex() const;
    float GetHeight() const;
    void GetPathLink() const;
    void GetNumLinks() const;
    void GetFlags() const;
    void GetLink(s32) const;
    void SetCentroid(const zeus::CVector3f&);
    zeus::CVector3f GetCentroid() const;
    void Fixup(CPFArea&, s32&);
    bool IsPointInside(const zeus::CVector3f&);
    zeus::CVector3f GetNormal();
    s32 GetNumNodes() const;
    void GetNode(s32) const;
    void PointHeight(const zeus::CVector3f&);
    void FindClosestPointOnPolygon(const std::vector<zeus::CVector3f>&, const zeus::CVector3f&, const zeus::CVector3f&,
                                   bool);
    void FindBestPoint(std::vector<zeus::CVector3f>&, const zeus::CVector3f&, u32, float);
    void SetLinkTo(s32);
    void DropToGround(zeus::CVector3f&) const;
    void GetLinkMidPoint(const CPFLink&);
    void FitThroughLink2d(const zeus::CVector3f&, const CPFLink&, const zeus::CVector3f&, float) const;
    void FitThroughLink3d(const zeus::CVector3f&, const CPFLink&, float, const zeus::CVector3f&, float, float) const;
    void IsPointInsidePaddedAABox(const zeus::CVector3f&, float) const;
};

class CPFRegionData
{
    float x0_ = 0.f;
    zeus::CVector3f x4_;
    s32 x10_ = -1;
    zeus::CVector3f x14_;
    s32 x20_ = 0;
    CPFRegion* x24_openLess = nullptr;
    CPFRegion* x28_openMore = nullptr;
    s32 x2c_ = 0;

public:
    CPFRegionData() = default;
    void SetOpenLess(CPFRegion*);
    void SetOpenMore(CPFRegion*);
    CPFRegion* GetOpenLess();
    CPFRegion* GetOpenMore();
    void GetCost();
    void GetPathLink() const;
    void GetParent() const;
    void Setup(CPFRegion*, float, float);
    void SetBestPoint(const zeus::CVector3f&);
    void SetBestPointDistanceSquared(float);
    float GetBestPointDistanceSquared() const;
    void SetPathLink(s32);
    void SetCookie(s32);
    zeus::CVector3f GetBestPoint() const;
    void GetCookie() const;
};
}

#endif // __URDE_CPATHFINDREGION_HPP__
