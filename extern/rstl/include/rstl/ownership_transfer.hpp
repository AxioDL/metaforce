#ifndef _RSTL_OWNERSHIP_TRANSFER
#define _RSTL_OWNERSHIP_TRANSFER

#include "types.h"

namespace rstl {
template < typename T >
class ownership_transfer {
  mutable bool x0_owns;
  mutable T* x4_ptr;

public:
  ownership_transfer() : x0_owns(false), x4_ptr(nullptr) {}
  ownership_transfer(T* ptr) : x0_owns(ptr != nullptr), x4_ptr(ptr) {}
  template < typename U >
  ownership_transfer(U* ptr) : x0_owns(ptr != nullptr), x4_ptr(static_cast< T* >(ptr)) {}
  ownership_transfer(const ownership_transfer& other)
  : x0_owns(other.x0_owns), x4_ptr(other.x4_ptr) {
    other.x0_owns = false;
  }
  ~ownership_transfer() { dec_ref_count(); }

  void operator=(const ownership_transfer& other) {
    if (&other != this) {
      if (x0_owns) {
        delete x4_ptr;
      }
      x0_owns = other.x0_owns;
      x4_ptr = other.x4_ptr;
      other.x0_owns = false;
    }
  }

  void dec_ref_count() {
    if (x0_owns) {
      delete x4_ptr;
    }
  }
  T* take_ownership() const {
    x0_owns = false;
    return x4_ptr;
  }
  T* get() const { return x4_ptr; }
  T* operator->() const { return get(); }
  T& operator*() const { return *get(); }
  bool owns_object() const { return x0_owns; }
};

} // namespace rstl

#endif // _RSTL_OWNERSHIP_TRANSFER
