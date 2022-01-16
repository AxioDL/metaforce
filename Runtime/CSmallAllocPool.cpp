#include "CSmallAllocPool.hpp"

#include <cstring>

namespace metaforce {
CSmallAllocPool::CSmallAllocPool(u32 len, void* mainData, void* bookKeeping)
: x0_mainData(mainData), x4_bookKeeping(bookKeeping), x8_numBlocks(len), x18_numBlocksAvailable(len) {
  memset(x4_bookKeeping, 0, len / 2);
}

void* CSmallAllocPool::FindFree(u32 len) {
  if (xc_cachedBookKeepingOffset == nullptr) {
    xc_cachedBookKeepingOffset = x4_bookKeeping;
  }

  auto* curKeepingOffset = static_cast<u8*>(xc_cachedBookKeepingOffset);
  auto* bookKeepingPtr = static_cast<u8*>(x4_bookKeeping);
  auto* bookKeepingEndPtr = bookKeepingPtr + (x8_numBlocks >> 1);
  auto* curKeepingIter = curKeepingOffset;
  while (true) {
    u8* tmpIter = nullptr;
    if (static_cast<u8*>(curKeepingIter)[0] == 0 && curKeepingIter != bookKeepingEndPtr) {
      tmpIter = curKeepingIter;
      do {
        ++tmpIter;
        if (tmpIter == curKeepingOffset || tmpIter == bookKeepingEndPtr || tmpIter == curKeepingIter + len / 2) {
          break;
        }
      } while (static_cast<u8*>(tmpIter)[0] == 0);

      if (tmpIter == curKeepingIter + len / 2) {
        if (tmpIter == bookKeepingEndPtr) {
          xc_cachedBookKeepingOffset = bookKeepingPtr;
        } else {
          xc_cachedBookKeepingOffset = curKeepingIter;
        }
        return curKeepingIter;
      }

      if (tmpIter == curKeepingOffset) {
        return nullptr;
      }
      if (tmpIter == bookKeepingEndPtr) {
        tmpIter = bookKeepingPtr;
      }
    } else {
      tmpIter = bookKeepingPtr;
      if (curKeepingIter != bookKeepingEndPtr) {
        u32 tmp = static_cast<u8*>(curKeepingIter)[0];
        tmpIter = curKeepingIter + (tmp >> 5);
      }
    }
    curKeepingIter = tmpIter;
    if (tmpIter == curKeepingOffset) {
      return nullptr;
    }
  }
}

void* CSmallAllocPool::Alloc(uint32_t size) {
  uint32_t len = 1;
  if (size > 3) {
    len = (size + 3) / 4;
  }
  if ((len & 1) != 0) {
    len += 1;
  }

  auto* freePtr = static_cast<uint8_t*>(FindFree(len));
  if (freePtr == nullptr) {
    return nullptr;
  }

  auto* bookKeepingStart = static_cast<uint8_t*>(x4_bookKeeping);
  uint32_t blockCount = (len - 2) / 2;
  auto* bufPtr = static_cast<uint8_t*>(x0_mainData);

  static_cast<uint8_t*>(freePtr)[0] = (len << 4) | 0xf;
  uint8_t* freePtrIter = freePtr + 1;
  if (blockCount != 0) {
    uint32_t uVar5 = blockCount >> 3;
    if (uVar5 != 0) {
      do {
        static_cast<uint8_t*>(freePtrIter)[0] = 0xff;
        static_cast<uint8_t*>(freePtrIter)[1] = 0xff;
        static_cast<uint8_t*>(freePtrIter)[2] = 0xff;
        static_cast<uint8_t*>(freePtrIter)[3] = 0xff;
        static_cast<uint8_t*>(freePtrIter)[4] = 0xff;
        static_cast<uint8_t*>(freePtrIter)[5] = 0xff;
        static_cast<uint8_t*>(freePtrIter)[6] = 0xff;
        static_cast<uint8_t*>(freePtrIter)[7] = 0xff;
        freePtrIter += 8;
      } while (--uVar5 != 0u);

      blockCount &= 7;
    }

    if (blockCount != 0) {
      do {
        static_cast<uint8_t*>(freePtrIter)[0] = 0xff;
        ++freePtrIter;
      } while (--blockCount != 0u);
    }
  }

  x18_numBlocksAvailable = x18_numBlocksAvailable - len;
  ++x1c_numAllocs;

  return bufPtr + ((freePtr - bookKeepingStart) * 8);
}

bool CSmallAllocPool::Free(void* buf) {
  u32 offset = static_cast<u8*>(buf) - static_cast<u8*>(x0_mainData);
  u32 block = (offset >> 2) + static_cast<u32>(static_cast<s32>(offset < 0 && ((offset & 3) != 0u)) != 0);
  u32 numBlocks = (static_cast<u8*>(x4_bookKeeping)[0] + (block >> 1)) >> (~-(block & 1) & 4) & 0xf;
  x18_numBlocksAvailable += numBlocks;
  --x1c_numAllocs;
  x14_ = block;
  if (block == x10_) {
    x10_ = -1;
  }
  u8* puVar1 = static_cast<u8*>(x4_bookKeeping) + (block >> 1);
  for (; numBlocks != 0; numBlocks -= 2) {
    static_cast<u8*>(puVar1)[0] = 0;
    ++puVar1;
  }

  return true;
}

} // namespace metaforce