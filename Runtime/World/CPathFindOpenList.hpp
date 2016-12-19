#ifndef __URDE_CPATHFINDOPENLIST_HPP__
#define __URDE_CPATHFINDOPENLIST_HPP__

#include "RetroTypes.hpp"
#include "CPathFindRegion.hpp"

namespace urde
{
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
}

#endif // __URDE_CPATHFINDOPENLIST_HPP__
