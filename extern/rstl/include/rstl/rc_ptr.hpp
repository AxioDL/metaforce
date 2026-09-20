#ifndef _RSTL_RC_PTR
#define _RSTL_RC_PTR

#include "types.h"
#include "rstl/allocator.hpp"
namespace rstl {
class CRefData {
public:
  CRefData(const void* ptr) : x0_ptr(ptr), x4_refCount(1) {}
  CRefData(const void* ptr, int refCount) : x0_ptr(ptr), x4_refCount(refCount) {}
  ~CRefData() {}

  void* GetPtr() const { return const_cast< void* >(x0_ptr); }
  int GetRefCount() const { return x4_refCount; }
  int AddRef() { return ++x4_refCount; }
  int DelRef() { return --x4_refCount; }

  const void* x0_ptr;
  int x4_refCount;

  static CRefData sNull;
};

template < typename T >
class rc_ptr {
public:
  rc_ptr() : x0_refData(&CRefData::sNull) { x0_refData->AddRef(); }
  rc_ptr(const T* ptr) : x0_refData(rs_new CRefData(ptr)) {}
  rc_ptr(const rc_ptr& other) : x0_refData(other.x0_refData) { x0_refData->AddRef(); }
  ~rc_ptr() { ReleaseData(); }
  rc_ptr& operator=(const rc_ptr& other) {
    if (x0_refData != other.x0_refData) {
      ReleaseData();
      x0_refData = other.x0_refData;
      x0_refData->AddRef();
    }
    return *this;
  }
  T* GetPtr() const { return static_cast< T* >(x0_refData->GetPtr()); }
  bool IsNull() const { return GetPtr() == nullptr; }
  template < typename U >
  void Assign(const U* ptr) {
    const T* base = ptr;
    ReleaseData();
    x0_refData = rs_new CRefData(base);
  }
  void ReleaseData();
  void reset() {
    ReleaseData();
    x0_refData = &CRefData::sNull;
    x0_refData->AddRef();
  }
  T* operator->() const { return GetPtr(); }
  T& operator*() const { return *GetPtr(); }
  operator bool() const { return GetPtr() != nullptr; }

private:
  CRefData* x0_refData;
};

template < typename T >
void rc_ptr< T >::ReleaseData() {
  if (x0_refData->DelRef() <= 0) {
    T* const ptr = GetPtr();
    delete ptr;
    delete x0_refData;
  }
}

template < typename T >
class ncrc_ptr : public rc_ptr< T > {
public:
  ncrc_ptr() {}
  ncrc_ptr(T* ptr) : rc_ptr< T >(ptr) {}
  ncrc_ptr(const rc_ptr< T >& other) : rc_ptr< T >(other) {}
  ncrc_ptr& operator=(const rc_ptr< T >& other) {
    rc_ptr< T >::operator=(other);
    return *this;
  }
  template < typename U >
  ncrc_ptr& operator=(const U* ptr) {
    rc_ptr< T >::Assign(ptr);
    return *this;
  }
};

template < typename T >
bool operator==(const rc_ptr< T >& left, const rc_ptr< T >& right) {
  return left.GetPtr() == right.GetPtr();
}

} // namespace rstl

#endif // _RSTL_RC_PTR
