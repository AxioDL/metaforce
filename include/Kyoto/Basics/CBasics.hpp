#ifndef _CBASICS
#define _CBASICS

#include "types.h"

namespace CBasics {
bool Init();
char* Stringize(const char* fmt, ...);
// Inferred name for Trilogy's cache-aware copy routine next to CBasics::Init.
void CopyMem(void* dest, const void* src, uint size);
void ZeroMem(void* dest, uint size);

#if TARGET_LITTLE_ENDIAN
namespace detail {
template < typename T >
inline T SwapBytes(T value) {
  uchar* bytes = reinterpret_cast< uchar* >(&value);
  for (uint i = 0; i < sizeof(T) / 2; ++i) {
    const uchar byte = bytes[i];
    bytes[i] = bytes[sizeof(T) - 1 - i];
    bytes[sizeof(T) - 1 - i] = byte;
  }
  return value;
}
} // namespace detail
#endif

// Convert between serialized big-endian values and host byte order.
inline const int SwapBytes(int x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const uint SwapBytes(uint x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const float SwapBytes(float x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const short SwapBytes(const short x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const ushort SwapBytes(const ushort x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const u64 SwapBytes(u64 x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

} // namespace CBasics

#endif // _CBASICS
