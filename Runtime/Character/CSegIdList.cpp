#include "Runtime/Character/CSegIdList.hpp"

namespace metaforce {

CSegIdList::CSegIdList(CInputStream& in) {
  u32 count = in.ReadLong();
  x0_list.reserve(count);
  for (u32 i = 0; i < count; ++i)
    x0_list.emplace_back(in);
}

} // namespace metaforce
