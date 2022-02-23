#include "Runtime/Graphics/CPVSAreaSet.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce {

CPVSAreaSet::CPVSAreaSet(const u8* data, u32 len) {
  CMemoryInStream r(data, len);
  x0_numFeatures = r.ReadLong();
  x4_numLights = r.ReadLong();
  x8_num2ndLights = r.ReadLong();
  xc_numActors = r.ReadLong();
  x10_leafSize = r.ReadLong();
  x14_lightIndexCount = r.ReadLong();
  x18_entityIndex.reserve(xc_numActors);
  for (u32 i = 0; i < xc_numActors; ++i)
    x18_entityIndex.push_back(r.ReadLong());
  x1c_lightLeaves = data + r.GetReadPosition();
  const u8* octreeData = x1c_lightLeaves + x14_lightIndexCount * x10_leafSize;
  x20_octree = CPVSVisOctree::MakePVSVisOctree(octreeData);
}

} // namespace metaforce
