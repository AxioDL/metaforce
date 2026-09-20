#ifndef _RSTL_ALLOCATOR_AUTO_PTR
#define _RSTL_ALLOCATOR_AUTO_PTR

#include "types.h"

namespace rstl {
template < typename T, typename Alloc >
struct allocator_auto_ptr {
  allocator_auto_ptr(T* ptr, Alloc* alloc) : x0_ptr(ptr), x4_alloc(alloc) {}
  ~allocator_auto_ptr() {
    if (x0_ptr != nullptr) {
      x4_alloc->deallocate(x0_ptr);
      x0_ptr = nullptr;
    }
  }

  T* release() const {
    T* ret = x0_ptr;
    const_cast< allocator_auto_ptr* >(this)->x0_ptr = nullptr;
    return ret;
  }

private:
  T* x0_ptr;
  Alloc* x4_alloc;
};
} // namespace rstl

#endif // _RSTL_ALLOCATOR_AUTO_PTR
