#include "Runtime/Character/CPrimitive.hpp"

namespace metaforce {

CPrimitive::CPrimitive(CInputStream& in) {
  x0_animId = in.readUint32Big();
  x4_animIdx = in.readUint32Big();
  x8_animName = in.readString();
}

} // namespace metaforce
