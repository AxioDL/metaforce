#pragma once

#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CCollisionEdge {
  u16 x0_index1 = -1;
  u16 x2_index2 = -1;

public:
  constexpr CCollisionEdge() noexcept = default;
  explicit CCollisionEdge(CInputStream&);
  constexpr CCollisionEdge(u16 v0, u16 v1) noexcept : x0_index1(v0), x2_index2(v1) {}

  [[nodiscard]] constexpr u16 GetVertIndex1() const noexcept { return x0_index1; }
  [[nodiscard]] constexpr u16 GetVertIndex2() const noexcept { return x2_index2; }

  constexpr void swapBig() noexcept {
    x0_index1 = hecl::SBig(x0_index1);
    x2_index2 = hecl::SBig(x2_index2);
  }
};
} // namespace metaforce
