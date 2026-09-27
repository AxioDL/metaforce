#ifndef _RSTL_ALLOCATOR_AUTO_PTR
#define _RSTL_ALLOCATOR_AUTO_PTR

#include "types.h"

namespace rstl {
template < typename T, typename Alloc >
struct allocator_auto_ptr {
  allocator_auto_ptr(T* ptr, Alloc* alloc) : mPtr(ptr), mAlloc(alloc) {}
  ~allocator_auto_ptr() {
    if (mPtr != nullptr) {
      mAlloc->deallocate(mPtr);
      mPtr = nullptr;
    }
  }

  T* release() const {
    T* ret = mPtr;
    const_cast< allocator_auto_ptr* >(this)->mPtr = nullptr;
    return ret;
  }

private:
  T* mPtr;
  Alloc* mAlloc;
};
} // namespace rstl

#endif // _RSTL_ALLOCATOR_AUTO_PTR
