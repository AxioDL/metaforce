#include "Runtime/Character/CCharacterSet.hpp"

namespace metaforce {

CCharacterSet::CCharacterSet(CInputStream& in) : x0_version(in.ReadShort()) {
  u32 charCount = in.ReadLong();
  for (u32 i = 0; i < charCount; ++i) {
    u32 id = in.ReadLong();
    x4_characters.emplace(id, in);
  }
}

} // namespace metaforce
