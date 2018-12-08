#include "CPVSAreaSet.hpp"

namespace urde {

CPVSAreaSet::CPVSAreaSet(const u8* data, u32 len) {
  CMemoryInStream r(data, len);
  x0_numFeatures = r.readUint32Big();
  x4_numLights = r.readUint32Big();
  x8_num2ndLights = r.readUint32Big();
  xc_numActors = r.readUint32Big();
  x10_leafSize = r.readUint32Big();
  x14_lightIndexCount = r.readUint32Big();
  x18_entityIndex.reserve(xc_numActors);
  for (int i = 0; i < xc_numActors; ++i)
    x18_entityIndex.push_back(r.readUint32Big());
  x1c_lightLeaves = data + r.position();
  const u8* octreeData = x1c_lightLeaves + x14_lightIndexCount * x10_leafSize;
  x20_octree = CPVSVisOctree::MakePVSVisOctree(octreeData);
}

} // namespace urde
