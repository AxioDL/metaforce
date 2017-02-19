#include "CPVSAreaSet.hpp"

namespace urde
{

CPVSAreaSet::CPVSAreaSet(const u8* data, u32 len)
{
    CMemoryInStream r(data, len);
    x0_numFeatures = r.readUint32Big();
    x4_numLights = r.readUint32Big();
    x8_c = r.readUint32Big();
    xc_numActors = r.readUint32Big();
    x10_leafSize = r.readUint32Big();
    x14_lightIndexCount = r.readUint32Big();
    x18_entityIndex = data + r.position();
    x1c_lightLeaves = x18_entityIndex + xc_numActors * 4;
    const u8* octreeData = x1c_lightLeaves + x14_lightIndexCount * x10_leafSize;
    x20_octree = CPVSVisOctree::MakePVSVisOctree(octreeData);
}

}
