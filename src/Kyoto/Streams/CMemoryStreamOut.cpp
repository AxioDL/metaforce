#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

#include <string.h>

CMemoryStreamOut::CMemoryStreamOut(void* buffer, size_t len, EOwnerShip ownerShip, int blockLen)
: COutputStream(blockLen)
, mOutPtr(buffer)
, mOutLength(len)
, mCurrentPosition(0)
, mBufferOwned(ownerShip == kOS_Owned) {}

CMemoryStreamOut::~CMemoryStreamOut() {
  COutputStream::Flush();

  if (mBufferOwned) {
    delete[] static_cast< uchar* >(mOutPtr);
  }
}


void CMemoryStreamOut::Write(const void* ptr, size_t len) {
  len = (mOutLength - mCurrentPosition) < len ? (mOutLength - mCurrentPosition) : len;
  if (len != 0) {
    memcpy(static_cast<uchar*>(mOutPtr) + mCurrentPosition, ptr, len);
    mCurrentPosition += len;
  }
}
