#ifndef _RSTL_AUTO_PTR
#define _RSTL_AUTO_PTR

#include "rstl/RstlVersions.h"
#include "types.h"

#include "rstl/allocator.hpp"

namespace rstl {
template < typename T >
class auto_ptr {
  mutable bool mHas;
  T* mItem;

public:
  auto_ptr() : mHas(false), mItem(nullptr) {}
  auto_ptr(T* ptr) : mHas(ptr != nullptr), mItem(ptr) {}
  ~auto_ptr() {
    if (mHas) {
#if defined(TARGET_PC)
      pointer_deleter< T >::destroy(mItem);
#else
      delete mItem;
#endif
    }
  }
  // TODO check
  auto_ptr(const auto_ptr& other) : mHas(other.mHas), mItem(other.mItem) {
    other.mHas = false;
  }
#if RSTL_VERSION >= RSTL_R3IJ
  template < typename U >
  friend class auto_ptr;

  template < typename U >
  auto_ptr(const auto_ptr< U >& other) : mHas(other.mHas), mItem(other.mItem) {
    other.mHas = false;
  }
#endif

  auto_ptr& operator=(const auto_ptr& other) {
    if (&other != this) {
      if (mHas) {
#if defined(TARGET_PC)
        pointer_deleter< T >::destroy(mItem);
#else
        delete mItem;
#endif
      }
      mHas = other.mHas;
      mItem = other.mItem;
      other.mHas = false;
    }
    return *this;
  }
  T* get() { return mItem; }
  T* get() const { return mItem; }
  bool owner() const { return mHas; }
  T* operator->() const { return mItem; }
  T& operator*() const { return *mItem; }
  T* release() const {
#if RSTL_VERSION >= RSTL_R3IJ
    if (mHas) {
      mHas = false;
      return mItem;
    }
    return nullptr;
#else
    mHas = false;
    return mItem;
#endif
  }
  bool null() const { return mItem == nullptr; }
  void reset() {
    mHas = false;
    mItem = nullptr;
  }
};
} // namespace rstl

#endif // _RSTL_AUTO_PTR
