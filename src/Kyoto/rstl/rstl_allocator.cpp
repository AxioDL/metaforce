#include "GameVersions.h"

#include "rstl/allocator.hpp"

#if defined(__MWERKS__) && (VERSION >= VERSION_GM8P_00)
void* rstl::rmemory_allocator::allocate(int size) {
  return size == 0 ? nullptr : rs_new uchar[size];
}
#endif
