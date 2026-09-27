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
               (static_cast< const uchar* >(ptr) - static_cast< uchar* >(mMainData)) /
               kAllocatorPointerSize) < mNumBlocks;
  }

  uint GetIndexFromPtr(const void* ptr) const {
    return (static_cast< const uchar* >(ptr) - static_cast< const uchar* >(mMainData)) /
           kAllocatorPointerSize;
  }
  intptr_t GetEntryValue(const uint idx) const {
    return *(static_cast< uchar* >(mBookKeeping) + idx);
  }
  uchar* GetPtrFromIndex(const uint idx) const {
    return static_cast< uchar* >(mMainData) + (idx * (kAllocatorPointerSize * 2));
  }

  uint GetNumBlocksAvailable() const { return mNumBlocksAvailable; }
  uint GetTotalEntries() const { return mNumBlocks; }
  uint GetAllocatedSize() const { return mNumBlocks - mNumBlocksAvailable; }
  uint GetNumAllocs() const { return mNumAllocs; }

private:
  void* mMainData;
  void* mBookKeeping;
  int mNumBlocks;
  void* mCachedBookKeepingOffset;
  int x10_;
  int x14_;
  uint mNumBlocksAvailable;
  uint mNumAllocs;
};

#endif // _CSMALLALLOCPOOL
