#include "CMediumAllocPool.hpp"
#include <bit>

namespace metaforce {
namespace {
CMediumAllocPool* gMediumAllocPtr = nullptr;
void InitBookKeeping(void* bookKeeping, u32 count) {
  u32 roundedLen = count & 0xFFFF;
  u8 header = 0;
  if (roundedLen > 3) {
    header = (count >> 8) | 0x80;
    u8* bookKeepingIter = static_cast<u8*>(bookKeeping);
    bookKeepingIter[0] = header;
    bookKeepingIter[1] = static_cast<u8>(count);
    bookKeepingIter[roundedLen - 2] = static_cast<u8>(count);
    bookKeepingIter[roundedLen - 1] = header;
    return;
  }

  if (roundedLen == 3) {
    header = 0x60;
  } else if (roundedLen == 2) {
    header = 0x40;
  } else {
    header = 0x20;
  }

  static_cast<u8*>(bookKeeping)[0] = header;
  if ((count & 0xFFFF) >= 2) {
    static_cast<u8*>(bookKeeping)[(count & 0xFFFF) - 1] = header | 0x80;
  }
}

u32 GetBlockOffset(u8* startPtr, u8* endPtr) {
  u32 numBlocks = (endPtr - startPtr) < 2 ? 0 : *startPtr;

  numBlocks += (*startPtr & 0x7f) * 0x100;
  u32 flags = numBlocks & 0x6000;
  if (flags == 0) {
    return numBlocks;
  }

  if (flags == 0x6000) {
    numBlocks = 3;
  } else {
    numBlocks = (std::countl_zero<u32>(0x4000 - flags) / 32) + 1;
  }

  return numBlocks & 0xFFFF;
}
} // namespace

SMediumAllocPuddle::SMediumAllocPuddle(u32 numBlocks, void* ptr, bool canErase)
: x0_hasBuffer(ptr != nullptr)
, x4_mainData(ptr)
, x8_bookKeeping(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + numBlocks * 32))
, x14_numBlocks(numBlocks)
, x1c_numEntries(numBlocks)
, x20_24_canErase(canErase) {
  InitBookKeeping(x8_bookKeeping, numBlocks & 0xFFFF);
}

void* SMediumAllocPuddle::FindFreeEntry(u32 blockCount) {
  if (blockCount > x14_numBlocks)
    return nullptr;

  u8* pbVar3 = reinterpret_cast<u8*>(x8_bookKeeping);
  u8* pbVar4 = reinterpret_cast<u8*>(xc_cachedBookKeepingAddr);
  if (xc_cachedBookKeepingAddr == nullptr) {
    pbVar4 = pbVar3;
  }
  u8* ptr2 = pbVar3 + x1c_numEntries;
  u8* ptr1 = pbVar4;

  u8* pbVar2 = nullptr;
  while (pbVar2 != pbVar4) {
    if (((*ptr1) & 0x80) == 0 || ptr1 == ptr2) {
      pbVar2 = pbVar3;
      if (ptr1 != ptr2) {
        pbVar2 = ptr1 + *ptr1;
      }
    } else {
      u32 uVar1 = GetBlockOffset(ptr1, ptr2) & 0xFFFF;
      if (blockCount <= uVar1) {
        uVar1 = (uVar1 - blockCount) & 0xFFFF;
        if (uVar1 != 0) {
          InitBookKeeping(ptr1 + blockCount, uVar1);
        }
        xc_cachedBookKeepingAddr = ptr1;
        return ptr1;
      }

      pbVar2 = ptr1 + uVar1;
      if (pbVar2 == pbVar4) {
        return nullptr;
      }

      if (pbVar2 == ptr2) {
        pbVar2 = reinterpret_cast<u8*>(pbVar3);
      }
    }
    ptr1 = pbVar2;
  }
  return nullptr;
}

void* SMediumAllocPuddle::FindFree(u32 blockCount) {
  u8* ptr = reinterpret_cast<u8*>(FindFreeEntry(blockCount));

  if (ptr == nullptr) {
    return nullptr;
  }

  u8* bookKeepingPtr = reinterpret_cast<u8*>(x8_bookKeeping);
  u8* mainDataPtr = reinterpret_cast<u8*>(x4_mainData);

  mainDataPtr = mainDataPtr + (ptr - bookKeepingPtr) * 32;
  *(ptr + blockCount - 1) = static_cast<u8>(blockCount);
  x14_numBlocks -= blockCount;
  ++x18_numAllocs;
  return mainDataPtr;
}

void SMediumAllocPuddle::Free(void* ptr) {
  u32 uVar3 = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(x4_mainData);
  u32 uVar4 = (reinterpret_cast<u8*>(x8_bookKeeping) + uVar3)[0];
  x14_numBlocks += uVar4;
  --x18_numAllocs;
  u8* bookKeepingStart = reinterpret_cast<u8*>(x8_bookKeeping);
  u8* pvVar2 = reinterpret_cast<u8*>(xc_cachedBookKeepingAddr);
  u8* pvVar5 = bookKeepingStart + uVar3;
  u8* ptr2 = bookKeepingStart + x1c_numEntries;

  uVar3 = uVar4;
  u8* bookKeepingPtr = pvVar5;
  if (bookKeepingStart < pvVar5) {
    u8 bVar1 = *(pvVar5 - 1);
    if ((bVar1 & 0x80) != 0) {
      if ((bVar1 & 0x60) == 0) {
        uVar3 = *(pvVar5 - 2) + (bVar1 & 0x7f) * 0x100;
      } else if ((bVar1 & 0x60) == 0x60) {
        uVar3 = 3;
      } else {
        uVar3 = (std::countl_zero<u32>(0x40 - (bVar1 & 0x60)) / 32) + 1;
      }

      bookKeepingPtr = pvVar5 - (uVar3 & 0xFFFF);
      uVar3 = (uVar4 + (uVar3 & 0xFFFF)) & 0xFFFF;
    }
  }

  u8* ptr1 = pvVar5 + uVar4;
  if ((ptr1 < ptr2) && (*ptr1 & 0x80) != 0) {
    uVar4 = GetBlockOffset(ptr1, ptr2);
    uVar3 += uVar4 & 0xFFFF;
  }

  InitBookKeeping(bookKeepingPtr, uVar3);
  if (pvVar2 == pvVar5) {
    if (bookKeepingPtr == bookKeepingStart) {
      xc_cachedBookKeepingAddr = nullptr;
    } else {
      xc_cachedBookKeepingAddr = reinterpret_cast<void*>(bookKeepingPtr - (bookKeepingPtr - 1));
    }
  }
}

CMediumAllocPool::CMediumAllocPool() {
  x18_lastPuddle = x0_puddles.begin();
  gMediumAllocPtr = this;
}

void* CMediumAllocPool::Alloc(u32 len) {
  u32 blockCount = 1;
  if (len > 16) {
    blockCount = (len + 16) / 32;
  }
  SMediumAllocPuddle& puddle = *x18_lastPuddle;
  void* free = puddle.FindFree(blockCount);

  if (free == nullptr) {
    for (auto it = x0_puddles.begin(); it != x0_puddles.end(); ++it) {
      if (it != x18_lastPuddle) {
        free = it->FindFreeEntry(blockCount);
        if (free != nullptr) {
          x18_lastPuddle = it;
        }
      }
    }
  }

  return free;
}

bool CMediumAllocPool::Free(void* ptr) {
  auto node = x0_puddles.begin();
  while (true) {
    if (node == x0_puddles.end()) {
      return true;
    }

    if ((reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(node->x4_mainData)) < node->x1c_numEntries / 32)
      break;

    ++node;
  }

  node->Free(ptr);
  if (node->x18_numAllocs == 0 && node->x20_24_canErase) {
    if (x18_lastPuddle == node) {
      x18_lastPuddle = x0_puddles.begin();
    }

    x0_puddles.erase(node);
  }
  return true;
}

void CMediumAllocPool::AddPuddle(u32 numBlocks, void* ptr, bool v) {
  x0_puddles.emplace_back(numBlocks, ptr, v);
  x18_lastPuddle = x0_puddles.end();
  x18_lastPuddle = --x18_lastPuddle;
}

u32 CMediumAllocPool::GetTotalEntries() const {
  u32 ret = 0;
  for (const auto& puddle : x0_puddles) {
    ret += puddle.x1c_numEntries;
  }

  return ret;
}

u32 CMediumAllocPool::GetNumBlocksAvailable() const {
  u32 ret = 0;
  for (const auto& puddle : x0_puddles) {
    ret += puddle.x14_numBlocks;
  }

  return ret;
}

u32 CMediumAllocPool::GetNumAllocs() const {
  u32 ret = 0;
  for (const auto& puddle : x0_puddles) {
    ret += puddle.x18_numAllocs;
  }

  return ret;
}
} // namespace metaforce
