#pragma once
#include "Runtime/GCNTypes.hpp"

namespace metaforce {
class CCircularBuffer {
public:
  enum class EOwnership { Unowned, Owned };

private:
  bool x0_canDelete;
  u8* x4_ptr;
  s32 x8_bufferLen;
  s32 xc_ = 0;
  s32 x10_nextFreeAddr = 0;
  s32 x14_ = -1;

public:
  CCircularBuffer(void* buf, s32 len, EOwnership ownership = EOwnership::Owned);
  ~CCircularBuffer();
  s32 GetAllocatedAmount() const;
  bool IsWrappedMemory(s32 offset, s32 len);
  void* Alloc(s32 len);
  void Free(void* ptr, s32 r5);
};
} // namespace metaforce