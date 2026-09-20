#include "Kyoto/Alloc/CSmallAllocPool.hpp"
#include <string.h>

CSmallAllocPool::CSmallAllocPool(uint len, void* mainData, void* bookKeeping)
: x0_mainData(mainData)
, x4_bookKeeping(bookKeeping)
, x8_numBlocks(len)
, xc_cachedBookKeepingOffset(NULL)
, x10_(-1)
, x14_(-1)
, x18_numBlocksAvailable(len)
, x1c_numAllocs(0) {
  memset(bookKeeping, 0, len / 2);
}

void* CSmallAllocPool::FindFree(int len) {
  uchar* bookKeepingPtr;
  int size = (int)len / 2;
  if (xc_cachedBookKeepingOffset == nullptr) {
    xc_cachedBookKeepingOffset = x4_bookKeeping;
  }
  uchar* curKeepingOffset = static_cast< uchar* >(xc_cachedBookKeepingOffset);
  bookKeepingPtr = static_cast< uchar* >(x4_bookKeeping);
  uchar* bookKeepingEndPtr = bookKeepingPtr + ((uint)x8_numBlocks >> 1);
  uchar* curKeepingIter = curKeepingOffset;
  while (true) {
    uchar* iter;
    if (static_cast< uchar* >(curKeepingIter)[0] != 0 || curKeepingIter == bookKeepingEndPtr) {
      if (curKeepingIter == bookKeepingEndPtr) {
        curKeepingIter = bookKeepingPtr;
      } else {
        int tmp = static_cast< uchar* >(curKeepingIter)[0];
        int reg = tmp >> 4;
        curKeepingIter += (reg / 2);
      }
    } else {
      uchar* tempIter = curKeepingIter + size;
      iter = curKeepingIter + 1;
      while (iter != curKeepingOffset && iter != bookKeepingEndPtr && iter != tempIter) {
        if (static_cast< uchar* >(iter)[0] == 0) {
          iter++;
        } else {
          break;
        }
      }

      if (iter == curKeepingIter + size) {
        if (iter == bookKeepingEndPtr) {
          xc_cachedBookKeepingOffset = bookKeepingPtr;
        } else {
          xc_cachedBookKeepingOffset = curKeepingIter;
        }
        return curKeepingIter;
      }

      if (iter == curKeepingOffset) {
        return nullptr;
      }
      if (iter == bookKeepingEndPtr) {
        curKeepingIter = bookKeepingPtr;
      } else {
        curKeepingIter = iter;
      }
    }
    if (curKeepingIter == curKeepingOffset) {
      return nullptr;
    }
  };
}

void* CSmallAllocPool::Alloc(uint size) {
  uint len = size >= 4 ? len = (size + 3) / 4 : 1;

  if ((len & 1) != 0) {
    len += 1;
  }

  uchar* freePtr = static_cast< uchar* >(FindFree(len));
  if (freePtr == nullptr) {
    return nullptr;
  }

  int sub = len - 2;
  uchar* bufPtr = GetPtrFromIndex(freePtr - static_cast< uchar* >(x4_bookKeeping));
  *static_cast< uchar* >(freePtr) = (len << 4) | 0xf;
  int blockSize = sub / 2;
  uchar* freePtrIter = freePtr + 1;
  while (blockSize--) {
    *freePtrIter = 0xff;
    ++freePtrIter;
  }

  x18_numBlocksAvailable -= len;
  ++x1c_numAllocs;

  return bufPtr;
}

bool CSmallAllocPool::Free(const void* arg0) {
  int temp_r8 = GetIndexFromPtr(arg0);
  int mask = (temp_r8 & 1) ? 0 : 4;
  size_t temp_r9 = static_cast< size_t >(temp_r8) / 2;
  long temp_r4_2 = GetEntryValue(temp_r9);
  temp_r4_2 = (temp_r4_2 >> mask) & 0xF;
  x18_numBlocksAvailable += temp_r4_2;
  int var_r5 = temp_r4_2;
  x1c_numAllocs -= 1;
  x14_ = temp_r8;
  if (static_cast< size_t >(temp_r8) == static_cast< size_t >(x10_)) {
    x10_ = -1;
  }

  uchar* var_r3 = static_cast< uchar* >(x4_bookKeeping) + temp_r9;
  while (var_r5 != 0) {
    *var_r3 = 0;
    var_r5 -= 2;
    ++var_r3;
  }
  return 1;
}
