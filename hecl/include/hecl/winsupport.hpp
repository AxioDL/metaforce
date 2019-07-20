#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include "windows.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#ifndef DEF_INLINE_MEMMEM
#define DEF_INLINE_MEMMEM
inline void* memmem(const void* haystack, size_t hlen, const void* needle, size_t nlen) {
  int needle_first;
  const uint8_t* p = static_cast<const uint8_t*>(haystack);
  size_t plen = hlen;

  if (!nlen)
    return NULL;

  needle_first = *(unsigned char*)needle;

  while (plen >= nlen && (p = static_cast<const uint8_t*>(memchr(p, needle_first, plen - nlen + 1)))) {
    if (!memcmp(p, needle, nlen))
      return (void*)p;

    p++;
    plen = hlen - (p - static_cast<const uint8_t*>(haystack));
  }

  return NULL;
}
#endif
