#ifndef _ALLOCATORCOMMON
#define _ALLOCATORCOMMON

#include <stddef.h> // For size_t

static const int kAllocatorPointerSize = sizeof(void*);
static const int kAllocatorPointerBits = kAllocatorPointerSize * 8;

template < size_t PointerSize >
struct PatternExpander;

template <>
struct PatternExpander< 4 > {
  typedef unsigned int Type;
  typedef size_t MaskType;
  static const Type Multiplier = 0x01010101U;
  static const MaskType TopNybbleMask = 0xF0000000U;
};

template <>
struct PatternExpander< 8 > {
  typedef unsigned long long Type;
  static const Type Multiplier = 0x0101010101010101ULL;
  enum { TopNybbleShift = 60 };
};

#define EXPAND_PATTERN(byte_val)                                                                   \
  (static_cast< PatternExpander< kAllocatorPointerSize >::Type >(byte_val) *                       \
   PatternExpander< kAllocatorPointerSize >::Multiplier)

static const intptr_t kAllocatorPostGuard = EXPAND_PATTERN(0xEA);
static const intptr_t kAllocatorPriorGuard = EXPAND_PATTERN(0xEF);
static const intptr_t kAllocatorPointerTopNybbleMask = PatternExpander< sizeof(void*) >::TopNybbleMask;
#endif // _ALLOCATORCOMMON
