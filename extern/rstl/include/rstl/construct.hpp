#ifndef _RSTL_CONSTRUCT
#define _RSTL_CONSTRUCT

#include "rstl/RstlVersions.h"
#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

namespace rstl {
template < typename T >
static inline void construct(void* dest, const T& src) {
  new (dest) T(src);
}

#if RSTL_VERSION >= RSTL_GM8P_00
template < typename T >
static inline void destroy(T* const in) {
#else
template < typename T >
static inline void destroy(T* in) {
#endif
  in->~T();
}

#if RSTL_VERSION >= RSTL_R3IJ
template < typename T >
static inline void destroy(T* begin, T* end) {
  for (; begin != end; ++begin) {
    destroy(begin);
  }
}

template < typename It >
static inline void destroy(It begin, It end) {
  It cur = begin;
  if (begin != end) {
    for (; cur != end; ++cur) {
      destroy(&*cur);
    }
  }
}

#else
template < typename It >
static inline void destroy(It begin, It end) {
  It cur = begin;
  for (; cur != end; ++cur) {
    destroy(&*cur);
  }
}

#endif

template < typename It, typename T >
static inline T uninitialized_copy(It begin, It end, T out) {
  T tmp = out;
  It cur = begin;
#if RSTL_VERSION >= RSTL_R3IJ
  for (; cur != end; ++cur, ++tmp) {
#else
  for (; cur != end; ++tmp, ++cur) {
#endif
    construct(tmp, *cur);
  }

  return tmp;
}

template < typename S, typename D >
static inline D uninitialized_copy_n(S src, int n, D dest) {
  S it = src;
  D cur = dest;
#if RSTL_VERSION >= RSTL_R3IJ
  for (int i = 0; i != n; ++it, ++i, ++cur) {
#else
  for (int i = 0; i < n; ++cur, ++i, ++it) {
#endif
    construct(&*cur, *it);
  }

  return cur;
}

template < typename D, typename S >
static inline void uninitialized_fill_n(D dest, int n, const S& value) {
  D cur = dest;
  for (int i = 0; i < n; ++i, ++cur) {
    void* ptr = &*cur;
    new (ptr) S(value);
  }
}
} // namespace rstl

#endif // _RSTL_CONSTRUCT
