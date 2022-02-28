#pragma once

#include "Runtime/Streams/CInputStream.hpp"
#include "Runtime/rstl.hpp"
#include <vector>
namespace metaforce {
template <class T, size_t N>
void read_reserved_vector(rstl::reserved_vector<T, N>& v, CInputStream& in) {
  u32 count = in.ReadLong();
  v.resize(count);
  for (u32 i = 0; i < count; ++i) {
    v[i] = in.Get<T>();
  }
}

template <class T>
void read_vector(std::vector<T>& v, CInputStream& in) {
  u32 count = in.ReadLong();
  v.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    v.emplace_back(in.Get<T>());
  }
}
} // namespace metaforce