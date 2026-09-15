#include "Kyoto/PVS/CPVSVisOctree.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "rstl/auto_ptr.hpp"
#include <Kyoto/PVS/CPVSVisSet.hpp>
#include <Kyoto/Streams/CMemoryInStream.hpp>
#include <WorldFormat/CPVSAreaSet.hpp>

CPVSAreaSet::CPVSAreaSet(int numFeatures, int numLights, int num2ndLights, int numActors,
                         int leafSize, int lightIndexCount, const char* w7, const char* w8,
                         const char* w9)
: x0_(numFeatures)
, x4_(numLights)
, x8_(num2ndLights)
, xc_(numActors)
, x10_leafSize(leafSize)
, x14_(lightIndexCount)
, x18_(w7)
, x1c_lightLeaves(w8)
, x20_octree(CPVSVisOctree::MakePVSVisOctree(w9, 68)) {}

rstl::auto_ptr< CPVSAreaSet > CPVSAreaSet::MakeAreaSet(const char* data, int len) {
  CMemoryInStream in(data, len);
  const int numFeatures = in.ReadLong();
  const int numLights = in.ReadLong();
  const int num2ndLights = in.ReadLong();
  const int numActors = in.ReadLong();
  const int leafSize = in.ReadLong();
  const int lightIndexCount = in.ReadLong();

  data += in.GetReadPosition();
  const char* const entityIndexEnd = data + numActors * 4;
  const char* const octreeData = entityIndexEnd + lightIndexCount * leafSize;

  return rstl::auto_ptr< CPVSAreaSet >(rs_new CPVSAreaSet(numFeatures, numLights, num2ndLights,
                                                       numActors, leafSize, lightIndexCount, data,
                                                       entityIndexEnd, octreeData));
}

CPVSVisOctree& CPVSAreaSet::GetVisOctree() const { return x20_octree; }

CPVSVisSet CPVSAreaSet::GetLightSet(int lightIdx) const {
  rstl::auto_ptr< const char > leaf(x1c_lightLeaves + x10_leafSize * lightIdx);
  leaf.release();
  return CPVSVisSet(x20_octree.GetNumObjects(), x20_octree.GetNumLights(),
                    rstl::auto_ptr< const char >(leaf));
}

int CPVSAreaSet::GetEntityIdByIndex(uint idx) const {
  return CBasics::SwapBytes(reinterpret_cast< const int* >(x18_)[idx]);
}
