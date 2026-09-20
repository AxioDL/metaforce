#include "Kyoto/Alloc/CCircularBuffer.hpp"

CCircularBuffer::CCircularBuffer(void* buf, const int len, const EOwnership ownership)
: x0_ptr(static_cast< char* >(buf)), x8_bufferLen(len), xc_(0), x10_nextFreeAddr(0), x14_(-1) {
  if (ownership == kOS_NotOwned) {
    (void)x0_ptr.release();
  }
}

bool CCircularBuffer::IsWrappedMemory(const int offset, const int len) {
  if (x14_ > -1 && x14_ >= offset && x14_ < (offset + len)) {
    return true;
  }

  return false;
}

void* CCircularBuffer::Alloc(const int len) {
  if ((x8_bufferLen - x10_nextFreeAddr) >= len && !IsWrappedMemory(x10_nextFreeAddr, len)) {
    const int offset = x10_nextFreeAddr;
    uchar* ptr = reinterpret_cast< uchar* >(x0_ptr.get());
    x10_nextFreeAddr = offset + len;
    return ptr + offset;
  }
  if (xc_ >= len && !IsWrappedMemory(0, len)) {
    uint r3 = xc_;
    xc_ = 0;
    x10_nextFreeAddr = len;
    x14_ = r3;
    return x0_ptr.get();
  }

  return nullptr;
}

void CCircularBuffer::Free(void* ptr, const int len) {
  if (x14_ > -1) {
    if (ptr == x0_ptr.get()) {
      x14_ = -1;
      xc_ = len;
    } else {
      x14_ += len;
    }
  } else {
    xc_ += len;
  }

  if (x14_ == -1 && xc_ == x10_nextFreeAddr) {
    x10_nextFreeAddr = 0;
    xc_ = 0;
  }
}

int CCircularBuffer::GetAllocatedAmount() const {
  const int tmp = x14_;
  int res = x10_nextFreeAddr - xc_;
  if (tmp != -1) {
    res += x8_bufferLen - tmp;
  }

  return res;
}
