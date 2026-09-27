#ifndef _RSTL_OBJECT_OWNER
#define _RSTL_OBJECT_OWNER

#include "rstl/auto_ptr.hpp"
#include "rstl/ownership_transfer.hpp"

namespace rstl {
template < typename T >
class deep_clone_disabled {};

template < typename T >
class call_deep_clone {};

template < typename T, typename Clone = deep_clone_disabled< T > >
class object_owner {
  T* mPtr;
  Clone mClone;

  object_owner(const object_owner&);
  object_owner& operator=(const object_owner&);

public:
  template < typename U >
  explicit object_owner(U* ptr) : mPtr(ptr) {}

  explicit object_owner(const auto_ptr< T >& ptr) : mPtr(ptr.release()) {}
  explicit object_owner(const ownership_transfer< T >& ptr) : mPtr(ptr.take_ownership()) {}
  ~object_owner() { delete mPtr; }

  T& operator*() { return *mPtr; }
  const T& operator*() const { return *mPtr; }
  T* operator->() { return mPtr; }
  const T* operator->() const { return mPtr; }
};
} // namespace rstl

#endif // _RSTL_OBJECT_OWNER
