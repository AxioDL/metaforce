#ifndef __URDE_CPVSVISOCTREE_HPP__
#define __URDE_CPVSVISOCTREE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "CPVSVisSet.hpp"

namespace urde
{

class CPVSVisOctree
{
    zeus::CAABox x0_aabb;
    u32 x18_totalBits;
    u32 x1c_lightBits;
    bool x20_bufferFlag;
    const u8* x24_octreeData;
    zeus::CAABox x2c_searchAabb;
public:
    static CPVSVisOctree MakePVSVisOctree(const u8* data);
    CPVSVisOctree() = default;
    CPVSVisOctree(const zeus::CAABox& aabb, u32 a, u32 b, const u8* c);
    u32 GetNumChildren(u8 byte) const;
    u32 GetChildIndex(const u8*, const zeus::CVector3f&) const;
    const zeus::CAABox& GetBounds() const { return x0_aabb; }
    const u8* GetOctreeData() const { return x24_octreeData; }

    u32 GetTotalBits() const { return x18_totalBits; }
    u32 GetLightBits() const { return x1c_lightBits; }
    void ResetSearch() const { const_cast<CPVSVisOctree&>(*this).x2c_searchAabb = x0_aabb; }
    s32 IterateSearch(u8 nodeData, const zeus::CVector3f& tp) const;
};

}

#endif // __URDE_CPVSVISOCTREE_HPP__
