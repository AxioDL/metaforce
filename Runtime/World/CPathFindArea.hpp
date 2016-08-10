#ifndef CPATHFINDAREA_HPP
#define CPATHFINDAREA_HPP

#include "IObj.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include "CPathFindOpenList.hpp"

namespace urde
{
class CVParamTransfer;

class CPFBitSet
{
public:
    CPFBitSet() = default;
    void Clear();
    void Add(s32);
    bool Test(s32);
    void Rmv(s32);
};

class CPFArea
{
    float x0_ = FLT_MAX;
    zeus::CVector3f x4_;
    std::vector<zeus::CVector3f> x10_;
    u32 x20_ = 0;
    u32 x24_ = 0;
    u32 x28_ = 0;
    u32 x2c_ = 0;
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
    u8* x13c_ = nullptr;
    u32 x140_ = 0;
    u8* x144_ = nullptr;
    u32 x148_ = 0;
    u8* x14c_ = nullptr;
    u32 x150_ = 0;
    u8* x154_ = nullptr;
    u32 x158_ = 0;
    u32 x15c_ = 0;
    u32 x160_ = 0;
    u32 x164_ = 0;
    u32 x168_ = 0;
    u32 x16c_ = 0;
    u32 x170_ = 0;
    u32 x174_ = 0;
    std::vector<CPFRegionData> x178_;
    zeus::CTransform x188_;
public:
    CPFArea(const std::unique_ptr<u8[]>&& buf, int len);
};


std::unique_ptr<IObj> FPathFindAreaFactory(const SObjectTag& /*tag*/, const std::unique_ptr<u8[]>& buf, const CVParamTransfer& xfer);
}

#endif // CPATHFINDAREA_HPP
