#pragma once
#include "Runtime/Streams/CInputStream.hpp"
#include "Runtime/rstl.hpp"

namespace metaforce {
template <class T, size_t N>
void read_reserved_vector(rstl::reserved_vector<T, N>& v, CInputStream& in) {
  u32 count = in.ReadLong();
  v.resize(count);
  for (u32 i = 0; i < count; ++i) {
    v[i] = in.Get<T>();
  }
}
} // namespace metaforce
