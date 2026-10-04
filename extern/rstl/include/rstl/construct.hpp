#ifndef _RSTL_CONSTRUCT
#define _RSTL_CONSTRUCT

#include "rstl/RstlVersions.h"
#include "rstl/iterator.hpp"
#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

namespace rstl {
template < typename T >
struct is_trivially_destructible {
  enum { value = false };
};

template < typename T >
static inline void construct(void* dest, const T& src) {
  new (dest) T(src);
}

#if RSTL_VERSION >= RSTL_R3ME_00
template < >
inline void construct< uint >(void* dest, const uint& src) {
  *static_cast< uint* >(dest) = src;
}

template < >
struct is_trivially_destructible< uint > {
  enum { value = true };
};

template < typename T >
inline void construct(void* dest, T* const& src) {
  *static_cast< T** >(dest) = src;
}

template < >
inline void construct< bool >(void* dest, const bool& src) {
  *static_cast< bool* >(dest) = src;
}

template < >
inline void construct< float >(void* dest, const float& src) {
  *static_cast< float* >(dest) = src;
}
#endif

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
  if (is_trivially_destructible< T >::value) {
    return;
  }
  for (; begin != end; ++begin) {
    destroy(begin);
  }
}

template < typename It >
static inline void destroy(It begin, It end) {
  if (is_trivially_destructible< typename iterator_traits< It >::value_type >::value) {
    return;
  }
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
  if (is_trivially_destructible< typename iterator_traits< It >::value_type >::value) {
    return;
  }
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
#if RSTL_VERSION >= RSTL_R3ME_00
  int remaining = n;
  S it = src;
  D cur = dest;
  for (; remaining != 0; --remaining, ++it, ++cur) {
    construct(&*cur, *it);
  }
  return cur;
#else
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
#endif
}

template < typename D, typename S >
static inline void uninitialized_fill_n(D dest, int n, const S& value) {
#if RSTL_VERSION >= RSTL_R3ME_00
  for (int i = 0; i < n; ++i, ++dest) {
    void* ptr = &*dest;
    new (ptr) S(value);
  }
#else
  D cur = dest;
  for (int i = 0; i < n; ++i, ++cur) {
    void* ptr = &*cur;
    new (ptr) S(value);
  }
#endif
}
} // namespace rstl

#endif // _RSTL_CONSTRUCT
