#pragma once

#include "Runtime/GCNTypes.hpp"

namespace urde {

class CVisorParameters {
public:
  u8 x0_mask : 4 = 0xf;
  bool x0_4_b1 : 1 = false;
  bool x0_5_scanPassthrough : 1 = false;
  CVisorParameters() = default;
  CVisorParameters(u8 mask, bool b1, bool scanPassthrough)
  : x0_mask(mask), x0_4_b1(b1), x0_5_scanPassthrough(scanPassthrough) {}
  u8 GetMask() const { return x0_mask; }
};

} // namespace urde
