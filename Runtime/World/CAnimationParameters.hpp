#pragma once

#include "Runtime/RetroTypes.hpp"

namespace urde {

class CAnimationParameters {
  CAssetId x0_ancs;
  u32 x4_charIdx = -1;
  u32 x8_defaultAnim = -1;

public:
  CAnimationParameters() = default;
  CAnimationParameters(CAssetId ancs, u32 charIdx, u32 defaultAnim)
  : x0_ancs(ancs), x4_charIdx(charIdx), x8_defaultAnim(defaultAnim) {}
  CAnimationParameters(CInputStream& in)
  : x0_ancs(in.readUint32Big()), x4_charIdx(in.readUint32Big()), x8_defaultAnim(in.readUint32Big()) {}

  CAssetId GetACSFile() const { return x0_ancs; }
  u32 GetCharacter() const { return x4_charIdx; }
  u32 GetInitialAnimation() const { return x8_defaultAnim; }
  void SetCharacter(u32 charIdx) { x4_charIdx = charIdx; }
};

} // namespace urde
