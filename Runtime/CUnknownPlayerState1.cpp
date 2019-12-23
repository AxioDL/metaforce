#include "Runtime/CUnknownPlayerState1.hpp"

namespace urde {
CUnknownPlayerState1::CUnknownPlayerState1(float f) : x194_(f) {
  std::fill(std::begin(x4_), std::end(x4_), true);
  std::fill(std::begin(x64_), std::end(x64_), false);
  std::fill(std::begin(x19c_), std::end(x19c_), 0.f);
  std::fill(std::begin(x30c_), std::end(x30c_), 0.f);
}
} // namespace urde
