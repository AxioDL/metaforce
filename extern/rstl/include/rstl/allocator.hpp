#ifndef _RSTL_RMEMORY_ALLOCATOR
#define _RSTL_RMEMORY_ALLOCATOR

#include "rstl/RstlVersions.h"
#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

#if defined(TARGET_PC)
#include <new>
#include <type_traits>
#endif

namespace rstl {

struct rmemory_allocator {
  rmemory_allocator() {}
  rmemory_allocator(const rmemory_allocator&) {}

#if defined(__MWERKS__) && (RSTL_VERSION >= RSTL_GM8P_00) && RSTL_VERSION != RSTL_GM8E_02
  static void* allocate(int size);
#endif

  template < typename T >
  static void allocate(T*& out, int count) {
#if defined(__MWERKS__) || defined(CLANGD)
    int size = count * sizeof(T);
#else
    if (count < 0 || static_cast< size_t >(count) > static_cast< size_t >(-1) / sizeof(T)) {
      throw std::bad_alloc();
    }
    const size_t size = static_cast< size_t >(count) * sizeof(T);
#if defined(__cpp_aligned_new)
    if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      out = size == 0 ? nullptr
                      : static_cast< T* >(operator new[](
                            size, static_cast< std::align_val_t >(alignof(T))));
      return;
    }
#endif
#endif
#if defined(__MWERKS__) && (RSTL_VERSION >= RSTL_GM8P_00) && RSTL_VERSION != RSTL_GM8E_02
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
#if defined(__MWERKS__) || defined(CLANGD)
    const int size = count * sizeof(T);
    out = size == 0 ? nullptr : static_cast< T* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
#else
    if (count < 0 || static_cast< size_t >(count) > (static_cast< size_t >(-1) - 31) / sizeof(T)) {
      throw std::bad_array_new_length();
    }
    const size_t size = (static_cast< size_t >(count) * sizeof(T) + 31) & ~size_t{31};
    constexpr size_t alignment = alignof(T) > 32 ? alignof(T) : 32;
    out = size == 0
              ? nullptr
              : static_cast< T* >(operator new[](size, static_cast< std::align_val_t >(alignment)));
#endif
  }

  template < typename T >
  static void deallocate(T* ptr) {
#if defined(__MWERKS__) || defined(CLANGD)
    delete[] reinterpret_cast< uchar* >(ptr);
#else
    constexpr size_t alignment = alignof(T) > 32 ? alignof(T) : 32;
    ::operator delete[](ptr, static_cast< std::align_val_t >(alignment));
#endif
  }
};

#if defined(TARGET_PC)
template < typename T >
struct pointer_deleter {
  static void destroy(T* ptr) {
    using Value = std::remove_cv_t< T >;
    // These types are used with `operator new[]` in game code, so we have to match the destructor
    if constexpr (std::is_same_v< Value, char > || std::is_same_v< Value, unsigned char > ||
                  std::is_same_v< Value, unsigned short > ||
                  std::is_same_v< Value, unsigned int > || std::is_same_v< Value, float >) {
      operator delete[](const_cast< void* >(static_cast< const volatile void* >(ptr)));
    } else {
      delete ptr;
    }
  }
};
#endif
} // namespace rstl

#endif // _RSTL_RMEMORY_ALLOCATOR
