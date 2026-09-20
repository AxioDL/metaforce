#ifndef _RSTL_SINGLE_PTR
#define _RSTL_SINGLE_PTR

#include "types.h"
#include "rstl/allocator.hpp"

namespace rstl {
template < typename T >
class single_ptr {
  mutable T* x0_ptr;

public:
  single_ptr() : x0_ptr(nullptr) {}
  single_ptr(T* ptr) : x0_ptr(ptr) {}
  single_ptr(const single_ptr& other) : x0_ptr(other.x0_ptr) { other.x0_ptr = nullptr; }
  ~single_ptr() { delete x0_ptr; }
  single_ptr& operator=(single_ptr& other) {
    if (&other == this) {
      return *this;
    }
    delete x0_ptr;
    x0_ptr = other.x0_ptr;
    other.x0_ptr = nullptr;
    return *this;
  }

  single_ptr& operator=(T* const ptr) {
    delete x0_ptr;
    x0_ptr = ptr;
    return *this;
  }

  T* get() const { return x0_ptr; }
  // const T* get() const { return x0_ptr; }
  T* operator->() const { return x0_ptr; }
  T& operator*() { return *x0_ptr; }
  const T& operator*() const { return *x0_ptr; }

  bool null() const { return x0_ptr == nullptr; }
  T* release() {
    T* ptr = x0_ptr;
    x0_ptr = nullptr;
    return ptr;
  }

  // This is certainly not real, but handy to force not-inline
  single_ptr& reset(T* ptr);
};

template < typename T >
single_ptr< T >& single_ptr< T >::reset(T* ptr) {
  return *this = ptr;
}

typedef single_ptr< char > unk_singleptr;
CHECK_SIZEOF(unk_singleptr, 0x4);
} // namespace rstl

#endif // _RSTL_SINGLE_PTR
