#pragma once

#include "RetroTypes.hpp"

namespace urde {
class CCollisionEdge {
  u16 x0_index1 = -1;
  u16 x2_index2 = -1;

public:
  CCollisionEdge() = default;
  CCollisionEdge(CInputStream&);

  u16 GetVertIndex1() const { return x0_index1; }
  u16 GetVertIndex2() const { return x2_index2; }

  void swapBig() {
    x0_index1 = hecl::SBig(x0_index1);
    x2_index2 = hecl::SBig(x2_index2);
  }
};
} // namespace urde
