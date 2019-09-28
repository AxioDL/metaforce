#pragma once

#include "Runtime/RetroTypes.hpp"
#include <zeus/CVector3f.hpp>

namespace urde {
class CPVSVisOctree;

enum class EPVSVisSetState { EndOfTree, NodeFound, OutOfBounds };

class CPVSVisSet {
  EPVSVisSetState x0_state;
  u32 x4_numBits;
  u32 x8_numLights;
  // bool xc_; Used to be part of auto_ptr
  const u8* x10_ptr;

public:
  void Reset(EPVSVisSetState state);
  EPVSVisSetState GetState() const { return x0_state; }
  EPVSVisSetState GetVisible(u32 idx) const;
  void SetFromMemory(u32 numBits, u32 numLights, const u8* leafPtr);
  void SetTestPoint(const CPVSVisOctree& octree, const zeus::CVector3f&);
};

} // namespace urde
