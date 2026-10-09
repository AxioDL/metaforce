#ifndef _RSTL_MATH
#define _RSTL_MATH

#include "rstl/RstlVersions.h"
#include "types.h"

namespace rstl {
#if RSTL_VERSION >= RSTL_R3IJ
template < typename T >
inline T min_val(T a, T b) {
  return (b < a) ? b : a;
}

template < typename T >
inline T max_val(T a, T b) {
  return (a < b) ? b : a;
}
#else
template < typename T >
inline const T& min_val(const T& a, const T& b) {
  return (b < a) ? b : a;
}

template < typename T >
inline const T& max_val(const T& a, const T& b) {
  return (a < b) ? b : a;
}
#endif

} // namespace rstl

#endif // _RSTL_MATH
