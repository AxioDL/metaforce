#include "Runtime/Graphics/CPVSVisOctree.hpp"
#include "Runtime/Streams/IOStreams.hpp"

#include <array>

namespace metaforce {

CPVSVisOctree CPVSVisOctree::MakePVSVisOctree(const u8* data) {
  CMemoryInStream r(data, 68);
  const zeus::CAABox aabb = r.Get<zeus::CAABox>();
  const u32 numObjects = r.ReadLong();
  const u32 numLights = r.ReadLong();
  r.ReadLong();
  return CPVSVisOctree(aabb, numObjects, numLights, data + r.GetReadPosition());
}

CPVSVisOctree::CPVSVisOctree(const zeus::CAABox& aabb, u32 numObjects, u32 numLights, const u8* c)
: x0_aabb(aabb)
, x18_numObjects(numObjects)
, x1c_numLights(numLights)
, x20_bufferFlag(c != nullptr)
, x24_octreeData(c)
, x2c_searchAabb(x0_aabb) {
  x20_bufferFlag = false;
}

u32 CPVSVisOctree::GetNumChildren(u8 byte) const {
  static constexpr std::array<u32, 8> numChildTable{0, 2, 2, 4, 2, 4, 4, 8};
  return numChildTable[byte & 0x7];
}

u32 CPVSVisOctree::GetChildIndex(const u8*, const zeus::CVector3f&) const { return 0; }

s32 CPVSVisOctree::IterateSearch(u8 nodeData, const zeus::CVector3f& tp) const {
  if (!(nodeData & 0x7))
    return -1; // Leaf node

  zeus::CVector3f newMin = x2c_searchAabb.center();
  zeus::CVector3f newMax;
  std::array<bool, 3> highFlags{};

  if (tp.x() > newMin.x()) {
    newMax.x() = x2c_searchAabb.max.x();
    highFlags[0] = true;
  } else {
    newMax.x() = float(newMin.x());
    newMin.x() = float(x2c_searchAabb.min.x());
    highFlags[0] = false;
  }

  if (tp.y() > newMin.y()) {
    newMax.y() = float(x2c_searchAabb.max.y());
    highFlags[1] = true;
  } else {
    newMax.y() = float(newMin.y());
    newMin.y() = float(x2c_searchAabb.min.y());
    highFlags[1] = false;
  }

  if (tp.z() > newMin.z()) {
    newMax.z() = float(x2c_searchAabb.max.z());
    highFlags[2] = true;
  } else {
    newMax.z() = float(newMin.z());
    newMin.z() = float(x2c_searchAabb.min.z());
    highFlags[2] = false;
  }

  std::array<u8, 2> axisCounts{1, 1};
  if (nodeData & 0x1)
    axisCounts[0] = 2;
  if (nodeData & 0x2)
    axisCounts[1] = 2;

  zeus::CAABox& newSearch = const_cast<zeus::CAABox&>(x2c_searchAabb);
  if (nodeData & 0x1) {
    newSearch.min.x() = float(newMin.x());
    newSearch.max.x() = float(newMax.x());
  }
  if (nodeData & 0x2) {
    newSearch.min.y() = float(newMin.y());
    newSearch.max.y() = float(newMax.y());
  }
  if (nodeData & 0x4) {
    newSearch.min.z() = float(newMin.z());
    newSearch.max.z() = float(newMax.z());
  }

  // Branch node - calculate next relative pointer
  return highFlags[0] * bool(nodeData & 0x1) + highFlags[1] * axisCounts[0] * bool(nodeData & 0x2) +
         highFlags[2] * axisCounts[0] * axisCounts[1] * bool(nodeData & 0x4);
}

} // namespace metaforce
