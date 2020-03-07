#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CPVSVisSet.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {

class CPVSVisOctree {
  zeus::CAABox x0_aabb;
  u32 x18_numObjects = 0;
  u32 x1c_numLights = 0;
  bool x20_bufferFlag = false;
  const u8* x24_octreeData = nullptr;
  zeus::CAABox x2c_searchAabb;

public:
  static CPVSVisOctree MakePVSVisOctree(const u8* data);
  CPVSVisOctree() = default;
  CPVSVisOctree(const zeus::CAABox& aabb, u32 numObjects, u32 numLights, const u8* c);
  u32 GetNumChildren(u8 byte) const;
  u32 GetChildIndex(const u8*, const zeus::CVector3f&) const;
  const zeus::CAABox& GetBounds() const { return x0_aabb; }
  const u8* GetOctreeData() const { return x24_octreeData; }

  u32 GetNumObjects() const { return x18_numObjects; }
  u32 GetNumLights() const { return x1c_numLights; }
  void ResetSearch() const { const_cast<CPVSVisOctree&>(*this).x2c_searchAabb = x0_aabb; }
  s32 IterateSearch(u8 nodeData, const zeus::CVector3f& tp) const;
};

} // namespace urde
