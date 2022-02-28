#pragma once

#include "Runtime/Streams/COutputStream.hpp"
#include "Runtime/rstl.hpp"
#include <vector>
namespace metaforce {
template <class T, size_t N>
void write_reserved_vector(const rstl::reserved_vector<T, N>& v, COutputStream& out) {
  out.Put(v.size());
  for (const auto& t : v) {
    out.Put(t);
  }
}

template <class T>
void write_vector(const std::vector<T>& v, COutputStream& out) {
  out.Put(v.size());
  for (const auto& t : v) {
    out.Put(t);
  }
}
} // namespace metaforce