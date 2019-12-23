#pragma once

#include <array>

namespace urde {

class CUnknownPlayerState1 {
private:

public:
  std::array<bool, 91> x4_;
  std::array<bool, 91> x64_;
  std::array<float, 48> xc4_;
  uint x184_ = 0;
  float x188_ = 0.f;
  int x18c_ = -1;
  int x190_ = -1;
  float x194_;
  std::array<float, 91> x19c_;
  std::array<float, 91> x30c_;

  CUnknownPlayerState1(float f);
};
} // namespace urde
