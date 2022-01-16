#pragma once
#include "GCNTypes.hpp"

namespace metaforce {
class CSmallAllocPool {
  void* x0_mainData;
  void* x4_bookKeeping;
  uint32_t x8_numBlocks;
  void* xc_cachedBookKeepingOffset = nullptr;
  int32_t x10_ = -1;
  uint32_t x14_ = -1;
  uint32_t x18_numBlocksAvailable = 0;
  uint32_t x1c_numAllocs = 0;

  [[nodiscard]] void* FindFree(uint32_t len);
public:
  CSmallAllocPool(uint32_t len, void* mainData, void* bookKeeping);

  [[nodiscard]] void* Alloc(uint32_t size);
  [[nodiscard]] bool Free(void* buf);

  [[nodiscard]] uint32_t GetNumUsedBlocks() const {
    uint32_t usedBlocks = 0;
    for (uint32_t i = 0; i < x8_numBlocks / 2;) {
      uint8_t p = static_cast<uint8_t*>(x4_bookKeeping)[i];
      // Don't check the header directly, make sure we extract the block count, as it's actually possible to allocate a
      // 0 block buffer, Retro worked around this in retail by not allowing CGameAllocator to allocate buffers >= 0x39
      // bytes
      if (((p >> 4) & 0xf) != 0) {
        uint32_t numBlocks = (p >> 4) & 0xf;
        usedBlocks += numBlocks;
        // skip over blocks used by this allocation
        i += numBlocks / 2;
      } else {
        ++i;
      }
    }
    return usedBlocks;
  }

  [[nodiscard]] uint32_t GetNumUnusedBlocks() const {
    uint32_t unusedBlocks = 0;
    for (uint32_t i = 0; i < x8_numBlocks / 2;) {
      uint8_t p = static_cast<uint8_t*>(x4_bookKeeping)[i];
      // Don't check the header directly, make sure we extract the block count, as it's actually possible to allocate a
      // 0 block buffer, Retro worked around this in retail by not allowing CGameAllocator to allocate buffers >= 0x39
      // bytes
      if (((p >> 4) & 0xf) != 0) {
        // skip over blocks used by this allocation
        i += ((p >> 4) & 0xf) / 2;
      } else {
        ++i;
        unusedBlocks += 2;
      }
    }
    return unusedBlocks;
  }

  [[nodiscard]] u32 GetNumBlocksAvailable() const { return x18_numBlocksAvailable; }
  [[nodiscard]] u32 GetTotalEntries() const { return x8_numBlocks - x18_numBlocksAvailable; }
  [[nodiscard]] u32 GetNumAllocs() const { return x1c_numAllocs; }
};
} // namespace metaforce