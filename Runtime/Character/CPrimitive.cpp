#include "Runtime/Character/CPrimitive.hpp"

namespace metaforce {

CPrimitive::CPrimitive(CInputStream& in) {
  x0_animId = in.Get<CAssetId>();
  x4_animIdx = in.ReadLong();
  x8_animName = in.Get<std::string>();
}

} // namespace metaforce
