#ifndef _RSTL_CONSTRUCT
#define _RSTL_CONSTRUCT

#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

namespace rstl {
template < typename T >
static inline void construct(void* dest, const T& src) {
  new (dest) T(src);
}

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
template < typename T >
static inline void destroy(T* const in) {
#else
template < typename T >
static inline void destroy(T* in) {
#endif
  in->~T();
}

template < typename It >
static inline void destroy(It begin, It end) {
  It cur = begin;
  for (; cur != end; ++cur) {
    destroy(&*cur);
  }
}

template < typename It, typename T >
static inline T uninitialized_copy(It begin, It end, T out) {
  T tmp = out;
  It cur = begin;
  for (; cur != end; ++tmp, ++cur) {
    construct(tmp, *cur);
  }

  return tmp;
}

template < typename S, typename D >
static inline D uninitialized_copy_n(S src, int n, D dest) {
  S it = src;
  D cur = dest;
  for (int i = 0; i < n; ++cur, ++i, ++it) {
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
