#ifndef _TONESTATIC
#define _TONESTATIC

#include "stdio.h"
#include "types.h"

template < typename T >
class TOneStatic {
public:
#ifndef __MWERKS__
  // For clangd
  void* operator new(const size_t sz) { return operator new(sz, nullptr, nullptr); }
#endif
  void* operator new(size_t sz, const char* const fileAndLine, const char* const type);
  void operator delete(void* ptr);

private:
  static void* GetAllocSpace();
  static uint& ReferenceCount();
};

template < typename T >
uint& TOneStatic< T >::ReferenceCount() {
  static uint sReferenceCount = 0;
  return sReferenceCount;
}

template < typename T >
void* TOneStatic< T >::GetAllocSpace() {
  ALIGNAS(T) static char sAllocSpace[sizeof(T)];
  return &sAllocSpace;
}

template < typename T >
void* TOneStatic< T >::operator new(size_t sz, const char* const fileAndLine,
                                    const char* const type) {
  ReferenceCount()++;
  return GetAllocSpace();
}

template < typename T >
void TOneStatic< T >::operator delete(void* ptr) {
  ReferenceCount()--;
}

#endif // _TONESTATIC
