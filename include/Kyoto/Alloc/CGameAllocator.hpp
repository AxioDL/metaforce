#ifndef _CGAMEALLOCATOR
#define _CGAMEALLOCATOR

#include <Kyoto/Alloc/IAllocator.hpp>

#include <Kyoto/Alloc/CMediumAllocPool.hpp>
#include <types.h>

class CSmallAllocPool;
class CMediumAllocPool;
class COsContext;
class CGameAllocator : public IAllocator {
public:
  class SGameMemInfo {
    friend class CGameAllocator;

  public:
    SGameMemInfo(SGameMemInfo* prev, SGameMemInfo* next, SGameMemInfo* nextFree, size_t len,
                 const char* fileAndLine, const char* type)
    : mPriorGuard(0xefefefef)
    , mLen(len)
    , mFileAndLine(fileAndLine)
    , mType(type)
    , mPrev(prev)
    , mNext(next)
    , mNextFree(nextFree)
    , mPostGuard(0xeaeaeaea) {}

    SGameMemInfo* GetPrev() const {
      return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mPrev) &
                                               ~(kAllocatorPointerBits - 1));
    }
    void SetPrev(SGameMemInfo* prev) {
      void* ptr = mPrev;
      mPrev = prev;
      mPrev = reinterpret_cast< SGameMemInfo* >(
          (reinterpret_cast< uintptr_t >(ptr) & (kAllocatorPointerBits - 1)) |
          (reinterpret_cast< uintptr_t >(mPrev) & ~(kAllocatorPointerBits - 1)));
    }
    SGameMemInfo* GetNext() const {
      return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mNext) &
                                               ~(kAllocatorPointerBits - 1));
    }
    void SetNext(SGameMemInfo* next) {
      void* ptr = mNext;
      mNext = next;
      mNext = reinterpret_cast< SGameMemInfo* >(
          (reinterpret_cast< uintptr_t >(ptr) & (kAllocatorPointerBits - 1)) |
          (reinterpret_cast< uintptr_t >(mNext) & ~(kAllocatorPointerBits - 1)));
    }
    uint GetPrevMaskedFlags() const {
      return reinterpret_cast< uintptr_t >(mPrev) & (kAllocatorPointerBits - 1);
    }
    void SetPrevMaskedFlags(const uint flags) {
      mPrev = reinterpret_cast< SGameMemInfo* >(
          (reinterpret_cast< uintptr_t >(mPrev) & ~(kAllocatorPointerBits - 1)) | flags);
    }
    void SetAllocated(const bool allocated) {
      if (allocated) {
        SetPrevMaskedFlags((GetPrevMaskedFlags() & ~1) | 1);
      } else {
        mPrev = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mPrev) & ~1);
      }
    }
    uint GetNextMaskedFlags();
    void SetTopOfHeapAllocated(const bool topOfHeap) {
      const uint flags = GetPrevMaskedFlags();
      SGameMemInfo* prev = GetPrev();
      uint topFlag = 0;
      if (topOfHeap) {
        topFlag = 2;
      }
      mPrev = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(prev) |
                                                   (topFlag | (flags & ~2)));
    }
    size_t GetLength() const { return mLen; }
    void SetLength(const size_t len) { mLen = len; }
    SGameMemInfo* GetNextFree() const {
      return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mNextFree) &
                                               ~(kAllocatorPointerBits - 1));
    }
    void SetNextFree(SGameMemInfo* info) {
      void* ptr = mNextFree;
      mNextFree = info;
      mNextFree = reinterpret_cast< SGameMemInfo* >(
          (reinterpret_cast< uintptr_t >(ptr) & (kAllocatorPointerBits - 1)) |
          (reinterpret_cast< uintptr_t >(mNextFree) & ~(kAllocatorPointerBits - 1)));
    }

    bool IsAllocated() const { return reinterpret_cast< uintptr_t >(mPrev) & 1; }
    void SetNotAllocated() {
      void* ptr = mPrev;
      mPrev = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(ptr) & ~1);
    }

    bool IsPostGuardIntact() const { return mPostGuard == kAllocatorPostGuard; }
    bool IsPriorGuardIntact() const { return mPriorGuard == kAllocatorPriorGuard; }

  private:
    size_t mPriorGuard;
    size_t mLen;
    const char* mFileAndLine;
    const char* mType;
    SGameMemInfo* mPrev;
    SGameMemInfo* mNext;
    SGameMemInfo* mNextFree;
    size_t mPostGuard;
  };

  SGameMemInfo* GetMemInfoFromBlockPtr(const void* ptr) const;
  CGameAllocator();
  ~CGameAllocator();

  bool Initialize(COsContext& ctx) override;
  void Shutdown() override;
  void* Alloc(size_t size, EHint hint, EScope scope, EType type, const CCallStack& cs) override;
  SGameMemInfo* FindFreeBlock(uint);
  SGameMemInfo* FindFreeBlockFromTopOfHeap(uint);
  uint FixupAllocPtrs(SGameMemInfo*, const uint, uint, EHint, const CCallStack&);
  void UpdateAllocDebugStats(unsigned int, unsigned int, unsigned int);
  bool Free(const void* ptr) override;
  bool FreeNormalAllocation(const void* ptr);
  void ReleaseAll() override;
  void* AllocSecondary(size_t size, EHint hint, EScope scope, EType type,
                       const CCallStack& cs) override;
  bool FreeSecondary(const void* ptr) override;
  void ReleaseAllSecondary() override;
  void SetOutOfMemoryCallback(FOutOfMemoryCb cb, const void* target) override;
  int EnumAllocations(FEnumAllocationsCb func, const void* ptr, bool b) const override;
  SAllocInfo GetAllocInfo(const void* ptr) const override;
  SMetrics GetMetrics() const override;
  void OffsetFakeStatics(int offset) override;

  static uint GetFreeBinEntryForSize(uint);
  void AddFreeEntryToFreeList(SGameMemInfo*);
  void RemoveFreeEntryFromFreeList(SGameMemInfo*);
  void DumpAllocations() const;
  size_t GetLargestFreeChunk() const;

private:
  SGameMemInfo** GetBinPtr(uint bin) { return &mBins[bin]; }
  uchar x4_;
  uchar x5_;
  uchar x6_;
  uchar x7_;
  uint mHeapSize;
  SGameMemInfo* mFirst;
  SGameMemInfo* mLast;
  SGameMemInfo* mBins[16];
  uint x54_;
  FOutOfMemoryCb mOomCallback;
  const void* mOomTarget;
  CSmallAllocPool* mSmallAllocPool;
  void* mSmallAllocMainData;
  void* mSmallAllocBookKeeping;
  bool x6c_;
  int x70_;
  CMediumAllocPool* mMediumPool;
  void* x78_;
  bool x7c_;
  uint x80_;
  uint x84_;
  uint x88_;
  uint x8c_;
  uint mHeapSize2;
  uint x94_;
  uint x98_;
  uint x9c_;
  uint xa0_;
  uint xa4_;
  uint xa8_;
  uint xac_;
  mutable uint xb0_;
  void* mPhysicalAddr;
  uint mFakeStatics;
  uint xbc_;
};

#endif // _CGAMEALLOCATOR
