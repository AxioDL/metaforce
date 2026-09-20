#ifndef _CCIRCULARBUFFER
#define _CCIRCULARBUFFER

#include <types.h>
#include <rstl/auto_ptr.hpp>

class CCircularBuffer {
public:
  enum EOwnership { kOS_Owned, kOS_NotOwned };

  CCircularBuffer(void* buf, int len, EOwnership owned = kOS_NotOwned);
  bool IsWrappedMemory(int offset, int len);
  void* Alloc(int len);
  void Free(void* ptr, int len);
  int GetAllocatedAmount() const;
  void* GetOffsettedMemory(const int offset) {
    return x0_ptr.get() + offset;
  }

private:
  rstl::auto_ptr<char> x0_ptr;
  int x8_bufferLen;
  int xc_;
  int x10_nextFreeAddr;
  int x14_;
};

#endif // _CCIRCULARBUFFER
