#include "Kyoto/Alloc/CCircularBuffer.hpp"

CCircularBuffer::CCircularBuffer(void* buf, const int len, const EOwnership ownership)
: mPtr(static_cast< char* >(buf)), mBufferLen(len), xc_(0), mNextFreeAddr(0), x14_(-1) {
  if (ownership == kOS_NotOwned) {
    (void)mPtr.release();
  }
}

bool CCircularBuffer::IsWrappedMemory(const int offset, const int len) {
  if (x14_ > -1 && x14_ >= offset && x14_ < (offset + len)) {
    return true;
  }

  return false;
}

void* CCircularBuffer::Alloc(const int len) {
  if ((mBufferLen - mNextFreeAddr) >= len && !IsWrappedMemory(mNextFreeAddr, len)) {
    const int offset = mNextFreeAddr;
    uchar* ptr = reinterpret_cast< uchar* >(mPtr.get());
    mNextFreeAddr = offset + len;
    return ptr + offset;
  }
  if (xc_ >= len && !IsWrappedMemory(0, len)) {
    uint r3 = xc_;
    xc_ = 0;
    mNextFreeAddr = len;
    x14_ = r3;
    return mPtr.get();
  }

  return nullptr;
}

void CCircularBuffer::Free(void* ptr, const int len) {
  if (x14_ > -1) {
    if (ptr == mPtr.get()) {
      x14_ = -1;
      xc_ = len;
    } else {
      x14_ += len;
    }
  } else {
    xc_ += len;
  }

  if (x14_ == -1 && xc_ == mNextFreeAddr) {
    mNextFreeAddr = 0;
    xc_ = 0;
  }
}

int CCircularBuffer::GetAllocatedAmount() const {
  const int tmp = x14_;
  int res = mNextFreeAddr - xc_;
  if (tmp != -1) {
    res += mBufferLen - tmp;
  }

  return res;
}
