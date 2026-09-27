
#include "rstl/RstlVersions.h"
#include "rstl/allocator.hpp"

#if defined(__MWERKS__) && (RSTL_VERSION >= RSTL_GM8E_02)
void* rstl::rmemory_allocator::allocate(int size) {
  return size == 0 ? nullptr : rs_new uchar[size];
}
#endif
