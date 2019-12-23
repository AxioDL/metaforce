#include "Runtime/CUnknownPlayerState2.hpp"

namespace urde {
CUnknownPlayerState2::CUnknownPlayerState2(int i1, int i2) : xa8_(i2) {
  x98_ = ((u8)(i1 << 6) & 0x40) | (x98_ & 0x3f);
}
} // namespace urde
