#pragma once

#include <string>

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {

class CPrimitive {
  CAssetId x0_animId;
  u32 x4_animIdx;
  std::string x8_animName;

public:
  explicit CPrimitive(CInputStream& in);
  CAssetId GetAnimResId() const { return x0_animId; }
  u32 GetAnimDbIdx() const { return x4_animIdx; }
  std::string_view GetName() const { return x8_animName; }
  bool operator<(const CPrimitive& other) const { return x8_animName < other.x8_animName; }
};

} // namespace metaforce
