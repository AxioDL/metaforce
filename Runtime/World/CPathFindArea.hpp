#ifndef CPATHFINDAREA_HPP
#define CPATHFINDAREA_HPP

#include "IObj.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CVParamTransfer;

class CPFRegion
{
    friend class CPFOpenList;
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
};

class CPFRegionData
{
    friend class CPFOpenList;
    float x0_ = 0.f;
    zeus::CVector3f x4_;
    s32 x10_ = -1;
    zeus::CVector3f x14_;
    s32 x20_ = 0;
    s32 x24_ = 0;
    s32 x28_ = 0;
    s32 x2c_ = 0;
};

class CPFOpenList
{
    friend class CPathFindArea;
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
    std::vector<CPFRegion> x40_region;
    std::vector<CPFRegionData> x90_regionData;
public:
    CPFOpenList()
    {
    }
};

class CPathFindArea
{
    zeus::CVector3f x0_;
    float xc_ = 0.f;
    std::vector<zeus::CVector3f> x10_;
    bool x30_ = false;
    std::vector<CPFOpenList> x78_;
    u8* x13c_data;
    zeus::CTransform x188_;
public:
    CPathFindArea(const std::unique_ptr<u8[]>&& buf, int len);
};


std::unique_ptr<IObj> FPathFindAreaFactory(const SObjectTag& /*tag*/, const std::unique_ptr<u8[]>& buf, const CVParamTransfer& xfer);
}

#endif // CPATHFINDAREA_HPP
