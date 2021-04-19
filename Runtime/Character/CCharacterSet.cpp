#include "Runtime/Character/CCharacterSet.hpp"

namespace metaforce {

CCharacterSet::CCharacterSet(CInputStream& in) : x0_version(in.readUint16Big()) {
  u32 charCount = in.readUint32Big();
  for (u32 i = 0; i < charCount; ++i) {
    u32 id = in.readUint32Big();
    x4_characters.emplace(id, in);
  }
}

} // namespace metaforce
