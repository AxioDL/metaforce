#ifndef _TYPES
#define _TYPES

#include "GameVersions.h"
#include "ByteOrder.hpp"

#ifdef __cplusplus
#include "static_assert.hpp"

// MWCC 1.3.2 rejects template-dependent alignment attributes. Keep its original
// layouts, and enforce the stored type's alignment on modern compilers.
#ifdef __MWERKS__
#define ALIGNAS(N)
#else
#define ALIGNAS(N) alignas(N)
#endif

extern "C" {
#endif

#include <dolphin/types.h>

// Dolphin u32 is unsigned long
typedef unsigned int uint;
typedef signed short sshort;
typedef unsigned short ushort;
typedef signed char schar;
typedef unsigned char uchar;

// Pointer to unknown, to be determined at a later date.
typedef void* unkptr;

#define ARRAY_SIZE(arr) static_cast< int >(sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
}
#endif

#endif // _TYPES
