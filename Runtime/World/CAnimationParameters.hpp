#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Streams/CInputStream.hpp"

namespace metaforce {

class CAnimationParameters {
  CAssetId x0_ancs;
  u32 x4_charIdx = -1;
  u32 x8_defaultAnim = -1;

public:
  CAnimationParameters() = default;
  CAnimationParameters(CAssetId ancs, u32 charIdx, u32 defaultAnim)
  : x0_ancs(ancs), x4_charIdx(charIdx), x8_defaultAnim(defaultAnim) {}
  explicit CAnimationParameters(CInputStream& in)
  : x0_ancs(in.Get<CAssetId>()), x4_charIdx(in.ReadLong()), x8_defaultAnim(in.ReadLong()) {}

  CAssetId GetACSFile() const { return x0_ancs; }
  u32 GetCharacter() const { return x4_charIdx; }
  u32 GetInitialAnimation() const { return x8_defaultAnim; }
  void SetCharacter(u32 charIdx) { x4_charIdx = charIdx; }
};

} // namespace metaforce
