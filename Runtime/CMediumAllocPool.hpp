#pragma once
#include <list>

#include <GCNTypes.hpp>

namespace metaforce {
class CMediumAllocPool {
  struct SMediumAllocPuddle {
    u8 x0_;
    void* x4_;
    void* x8_;
    void* xc_;
    u32 x10_;
    u32 x14_numBlocks;
    u32 x18_numAllocs;
    u32 x1c_numEntries;
    u8 x20_;
  };
  std::list<SMediumAllocPuddle> x0_puddles;
  std::list<SMediumAllocPuddle>::iterator x18_lastAlloc;
public:
  //CMediumAllocPool();

  u32 GetTotalEntries() const;
  u32 GetNumBlocksAvailable() const;
  u32 GetNumAllocs() const;

};
}
