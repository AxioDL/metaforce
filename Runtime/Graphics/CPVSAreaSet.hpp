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
    u32 x8_num2ndLights;
    u32 xc_numActors;
    u32 x10_leafSize;
    u32 x14_lightIndexCount;
    std::vector<u32> x18_entityIndex;
    const u8* x1c_lightLeaves;
    CPVSVisOctree x20_octree;

    CPVSVisSet _GetLightSet(u32 lightIdx) const
    {
        CPVSVisSet ret;
        ret.SetFromMemory(x20_octree.GetNumObjects(), x20_octree.GetNumLights(),
                          x1c_lightLeaves + x10_leafSize * lightIdx);
        return ret;
    }

public:
    CPVSAreaSet(const u8* data, u32 len);
    u32 GetNumFeatures() const { return x0_numFeatures; }
    u32 GetNumActors() const { return xc_numActors; }
    u32 Get1stLightIndex(u32 lightIdx) const { return x0_numFeatures + x8_num2ndLights + lightIdx; }
    u32 Get2ndLightIndex(u32 lightIdx) const { return x0_numFeatures + lightIdx; }
    bool Has2ndLayerLights() const { return x8_num2ndLights != 0; }
    u32 GetEntityIdByIndex(int idx) const { return x18_entityIndex[idx]; }
    const CPVSVisOctree& GetVisOctree() const { return x20_octree; }
    CPVSVisSet Get1stLightSet(u32 lightIdx) const { return _GetLightSet(x8_num2ndLights + lightIdx); }
    CPVSVisSet Get2ndLightSet(u32 lightIdx) const { return _GetLightSet(lightIdx); }
};

}

#endif // __URDE_CPVSAREASET_HPP__
