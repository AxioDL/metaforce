#ifndef _RSTL_RMEMORY_ALLOCATOR
#define _RSTL_RMEMORY_ALLOCATOR

#include "GameVersions.h"
#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

namespace rstl {
struct rmemory_allocator {
  rmemory_allocator() {}
  rmemory_allocator(const rmemory_allocator&) {}

#if defined(__MWERKS__) && (VERSION >= VERSION_GM8P_00)
  static void* allocate(int size);
#endif

  template < typename T >
  static void allocate(T*& out, int count) {
#ifdef __MWERKS__
    int size = count * sizeof(T);
#else
    if (count < 0 || static_cast< size_t >(count) > static_cast< size_t >(-1) / sizeof(T)) {
      throw std::bad_alloc();
    }
    const size_t size = static_cast< size_t >(count) * sizeof(T);
#if defined(__cpp_aligned_new)
    if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      out = size == 0 ? nullptr
                      : static_cast< T* >(::operator new[](size, std::align_val_t(alignof(T))));
      return;
    }
#endif
#endif
#if defined(__MWERKS__) && (VERSION >= VERSION_GM8P_00)
    out = reinterpret_cast< T* >(allocate(size));
#else
    if (size == 0) {
      out = nullptr;
    } else {
      out = reinterpret_cast< T* >(rs_new uchar[size]);
    }
#endif
  }

  template < typename T >
  static void deallocate(T* ptr) {
#if !defined(__MWERKS__) && defined(__cpp_aligned_new)
    if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      ::operator delete[](ptr, std::align_val_t(alignof(T)));
      return;
    }
#endif
    if (ptr != nullptr) {
      delete[] reinterpret_cast< uchar* >(ptr);
    }
  }
};

struct aligned_allocator {
  aligned_allocator() {}
  aligned_allocator(const aligned_allocator&) {}

  template < typename T >
  static void allocate(T*& out, int count) {
    const int size = count * sizeof(T);
    out = size == 0 ? nullptr
                    : static_cast< T* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  }

  template < typename T >
  static void deallocate(T* ptr) {
    delete[] reinterpret_cast< uchar* >(ptr);
  }
};
} // namespace rstl

#endif // _RSTL_RMEMORY_ALLOCATOR
