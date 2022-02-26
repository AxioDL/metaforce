#include "Runtime/Memory/CCircularBuffer.hpp"

namespace metaforce {
CCircularBuffer::CCircularBuffer(void* buf, s32 len, EOwnership ownership)
: x0_canDelete(buf != nullptr), x4_ptr(reinterpret_cast<u8*>(buf)), x8_bufferLen(len) {
  if (ownership == EOwnership::Owned)
    x0_canDelete = false;
}
CCircularBuffer::~CCircularBuffer() {
  if (x0_canDelete) {
    delete x4_ptr;
  }
}

s32 CCircularBuffer::GetAllocatedAmount() const {
  s32 res = x10_nextFreeAddr - xc_;
  return (x14_ == -1) ? res : res + (x8_bufferLen - x14_);
}

bool CCircularBuffer::IsWrappedMemory(s32 offset, s32 len) {
  return x14_ > -1 && x14_ >= offset && (x14_ < (offset + len));
}

void* CCircularBuffer::Alloc(s32 len) {
  if ((x8_bufferLen - x10_nextFreeAddr) >= len && !IsWrappedMemory(x10_nextFreeAddr, len)) {
    s32 offset = x10_nextFreeAddr;
    x10_nextFreeAddr = offset + len;
    return x4_ptr + offset;
  } else if (xc_ >= len && !IsWrappedMemory(0, len)) {
    u32 r3 = xc_;
    xc_ = 0;
    x10_nextFreeAddr = len;
    x14_ = r3;
    return x4_ptr;
  }

  return nullptr;
}

void CCircularBuffer::Free(void* ptr, s32 r5) {
  if (x14_ <= -1) {
    xc_ += r5;
  } else if (ptr == x4_ptr) {
    x14_ = -1;
    xc_ = r5;
  } else {
    x14_ += r5;
  }

  if (x14_ != -1 || xc_ != x10_nextFreeAddr) {
    return;
  }

  x10_nextFreeAddr = 0;
  xc_ = 0;
}
} // namespace metaforce
