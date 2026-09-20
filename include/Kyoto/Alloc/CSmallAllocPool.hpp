#ifndef _CSMALLALLOCPOOL
#define _CSMALLALLOCPOOL

#include <types.h>

#include <Kyoto/Alloc/AllocatorCommon.hpp>

class CSmallAllocPool {
public:
  CSmallAllocPool(uint len, void* mainData, void* bookKeeping);
  void* FindFree(int len);
  void* Alloc(uint size);
  bool Free(const void* ptr);

  bool PtrWithinPool(const void* ptr) const {
    return static_cast< uint >(
               (static_cast< const uchar* >(ptr) - static_cast< uchar* >(x0_mainData)) /
               kAllocatorPointerSize) < x8_numBlocks;
  }

  uint GetIndexFromPtr(const void* ptr) const {
    return (static_cast< const uchar* >(ptr) - static_cast< const uchar* >(x0_mainData)) /
           kAllocatorPointerSize;
  }
  intptr_t GetEntryValue(const uint idx) const {
    return *(static_cast< uchar* >(x4_bookKeeping) + idx);
  }
  uchar* GetPtrFromIndex(const uint idx) const {
    return static_cast< uchar* >(x0_mainData) + (idx * (kAllocatorPointerSize * 2));
  }

  uint GetNumBlocksAvailable() const { return x18_numBlocksAvailable; }
  uint GetTotalEntries() const { return x8_numBlocks; }
  uint GetAllocatedSize() const { return x8_numBlocks - x18_numBlocksAvailable; }
  uint GetNumAllocs() const { return x1c_numAllocs; }

private:
  void* x0_mainData;
  void* x4_bookKeeping;
  int x8_numBlocks;
  void* xc_cachedBookKeepingOffset;
  int x10_;
  int x14_;
  uint x18_numBlocksAvailable;
  uint x1c_numAllocs;
};

#endif // _CSMALLALLOCPOOL
