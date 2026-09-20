#ifndef _RSTL_MATH
#define _RSTL_MATH

#include "types.h"

namespace rstl {
template < typename T >
inline const T& min_val(const T& a, const T& b) {
  return (b < a) ? b : a;
}

template < typename T >
inline const T& max_val(const T& a, const T& b) {
  return (a < b) ? b : a;
}
} // namespace rstl

#endif // _RSTL_MATH
