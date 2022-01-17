#pragma once
#include <list>

#include "Runtime/GCNTypes.hpp"


namespace metaforce {
struct SMediumAllocPuddle {
  bool x0_hasBuffer; // was an auto_ptr
  void* x4_mainData; // ""
  void* x8_bookKeeping;
  void* xc_cachedBookKeepingAddr = nullptr;
  s32 x10_ = -1;
  u32 x14_numBlocks;
  u32 x18_numAllocs = 0;
  u32 x1c_numEntries;
  bool x20_24_canErase : 1;

  void* FindFreeEntry(u32 blockCount);
public:
  SMediumAllocPuddle(u32 numBlocks, void* ptr, bool canErase);
  void* FindFree(u32 blockCount);

  void Free(void* ptr);
};

class CMediumAllocPool {
  std::list<SMediumAllocPuddle> x0_puddles;
  std::list<SMediumAllocPuddle>::iterator x18_lastPuddle;
public:
  CMediumAllocPool();

  void* Alloc(u32 len);
  bool Free(void* ptr);
  void AddPuddle(u32 numBlocks, void* ptr, bool v);
  u32 GetTotalEntries() const;
  u32 GetNumBlocksAvailable() const;
  u32 GetNumAllocs() const;
  bool HasPuddles() const { return !x0_puddles.empty(); }
};
}
