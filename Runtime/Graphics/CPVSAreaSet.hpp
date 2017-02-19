#ifndef __URDE_CPVSAREASET_HPP__
#define __URDE_CPVSAREASET_HPP__

#include "RetroTypes.hpp"
#include "CPVSVisOctree.hpp"

namespace urde
{

class CPVSAreaSet
{
    u32 x0_numFeatures;
    u32 x4_numLights;
    u32 x8_c;
    u32 xc_numActors;
    u32 x10_leafSize;
    u32 x14_lightIndexCount;
    const u8* x18_entityIndex;
    const u8* x1c_lightLeaves;
    CPVSVisOctree x20_octree;

public:
    CPVSAreaSet(const u8* data, u32 len);
    u32 GetNumFeatures() const { return x0_numFeatures; }
    u32 GetNumActors() const { return xc_numActors; }
    u32 GetEntityIdByIndex(int idx) const { return x18_entityIndex[idx]; }
    const CPVSVisOctree& GetVisOctree() { return x20_octree; }
};

}

#endif // __URDE_CPVSAREASET_HPP__
