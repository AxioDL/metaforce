// #include "dolphin/os/OSArena.h"
#include "dolphin/types.h"
#include "stddef.h"
#include <Kyoto/Alloc/CGameAllocator.hpp>

#include <Kyoto/Alloc/CCallStack.hpp>
#include <Kyoto/Alloc/CMediumAllocPool.hpp>
#include <Kyoto/Alloc/CSmallAllocPool.hpp>
#include <Kyoto/Basics/COsContext.hpp>
#include <Kyoto/Basics/CStopwatch.hpp>

#include <stdint.h>

/* Here just to make sure the data section matches */
static const char* string_NULL = "<NULL>";
static const char* string_SOURCE_MODULE_UNLOADED = "<SOURCE MODULE UNLOADED>";
static const char* string_ = "";
static int gAllocatorTime = 0;

template < typename U1, typename U2 >
static inline U1 T_round_up(U2 val, int align) {
  return (val + (align - 1)) & ~(align - 1);
}

CGameAllocator::SGameMemInfo* CGameAllocator::GetMemInfoFromBlockPtr(const void* ptr) const {
  return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(ptr) -
                                           sizeof(SGameMemInfo));
}

CGameAllocator::CGameAllocator()
: x4_(0)
, mHeapSize(0)
, mFirst(nullptr)
, mLast(nullptr)
, x54_(0)
, mOomCallback(nullptr)
, mOomTarget(nullptr)
, mSmallAllocPool(nullptr)
, mSmallAllocMainData(nullptr)
, mSmallAllocBookKeeping(nullptr)
, x6c_(false)
, x70_(0)
, mMediumPool(nullptr)
, x80_(0)
, x84_(0)
, x88_(0)
, x8c_(0)
, mHeapSize2(0)
, x94_(0)
, x98_(0)
, x9c_(0)
, xa0_(0)
, xa4_(0)
, xa8_(0)
, xac_(0)
, xb0_(0)
, mPhysicalAddr(nullptr)
, mFakeStatics(0)
, xbc_(0) {}

CGameAllocator::~CGameAllocator() {
  if (mMediumPool) {
    mMediumPool->ClearPuddles();
    FreeNormalAllocation(mMediumPool);
    mMediumPool = nullptr;
  }
}

bool CGameAllocator::Initialize(COsContext& ctx) {
  mHeapSize = ctx.GetBaseFreeRam() - 2 * sizeof(SGameMemInfo);
  mFirst = static_cast< SGameMemInfo* >(OSAllocFromArenaLo(mHeapSize, sizeof(SGameMemInfo)));
  mPhysicalAddr = reinterpret_cast< void* >(
      reinterpret_cast< intptr_t >(mFirst) -
      (reinterpret_cast< uintptr_t >(mFirst) & kAllocatorPointerTopNybbleMask));
  OSGetArenaLo();
  mLast =
      reinterpret_cast< SGameMemInfo* >(reinterpret_cast< char* >(mFirst - 1) + mHeapSize);

  const SGameMemInfo& head = SGameMemInfo(
      nullptr, mLast, mLast, mHeapSize - sizeof(SGameMemInfo) * 2, "MemHead", "MemHead");
  *mFirst = head;
  const SGameMemInfo& tail = SGameMemInfo(mFirst, nullptr, nullptr, 0, "MemTail", "MemTail");
  *mLast = tail;
  for (uint i = 0; i < 16; i++) {
    mBins[i] = nullptr;
  }

  AddFreeEntryToFreeList(mFirst);
  x80_ = 0;
  x84_ = 0;
  x88_ = 0;
  x8c_ = 0;
  mHeapSize2 = mHeapSize;
  x94_ = 0;
  x98_ = 0;
  x9c_ = 0;
  xa0_ = 0;
  xa4_ = 0;
  xa8_ = 0;
  x4_ = 1;

  mSmallAllocMainData = Alloc(0xb0000, kHI_None, kSC_Unk1, kTP_Heap,
                                 CCallStack(0xffffffff, "SmallAllocMainData   ", " - Ignore"));

  mSmallAllocBookKeeping = Alloc(0x16000, kHI_None, kSC_Unk1, kTP_Heap,
                                    CCallStack(0xffffffff, "SmallAllocBookKeeping", " - Ignore"));

  mSmallAllocPool = new (Alloc(0x20, kHI_None, kSC_Unk1, kTP_Heap,
                                  CCallStack(0xffffffff, "SmallAllocClass      ", " - Ignore")))
      CSmallAllocPool(0x2c000, mSmallAllocMainData, mSmallAllocBookKeeping);

  mMediumPool =
      new (Alloc(0x1c, kHI_None, kSC_Unk1, kTP_Heap,
                 CCallStack(0xffffffff, "MediumAllocClass      ", " - Ignore"))) CMediumAllocPool();

  uint mediumSize = CMediumAllocPool::GetAllocMemoryRequired(0x1000);
  mediumSize += CMediumAllocPool::GetBookKeepingMemoryRequired(0x1000);
  x78_ = Alloc(mediumSize, kHI_None, kSC_Unk1, kTP_Heap,
               CCallStack(0xffffffff, "MediumAllocMainData   ", " - Ignore"));
  xbc_ = 0xc6000;
  x84_ -= 4;
  return true;
}

void CGameAllocator::Shutdown() {
  ReleaseAll();
  x4_ = 0;
  x54_ = 0;
}

void* CGameAllocator::Alloc(size_t size, const EHint hint, const EScope scope, const EType type,
                            const CCallStack& callstack) {
  uint tmp = 0;
  void* buf = nullptr;
  const OSTick startTick = OSGetTick();

  if (hint & kHI_RoundUpLen) {
    size = T_round_up< size_t, size_t >(size, 32);
  }

  bool bVar1 = size <= 56 && !(hint & (kHI_RoundUpLen | kHI_TopOfHeap)) && mSmallAllocPool;

  if (bVar1 && x70_ > 0) {
    bVar1 = false;
    --x70_;
  }

  if (bVar1) {
    buf = mSmallAllocPool->Alloc(size);
    tmp = mSmallAllocPool->GetAllocatedSize();
    if (xac_ < tmp) {
      xac_ = tmp;
      static int sLastSmallAllocSize = 0;
      if (sLastSmallAllocSize + 128 < tmp) {
        sLastSmallAllocSize = tmp;
      }
    }

    if (buf != nullptr) {
      gAllocatorTime += (OSGetTick() - startTick);
      return buf;
    }
    x70_ = 25;
    x6c_ = true;
  }

  if (mMediumPool && size <= 0x400 && !(hint & kHI_TopOfHeap)) {
    if (!mMediumPool->HasPuddles()) {
      buf = nullptr;
      mMediumPool->AddPuddle(0x1000, x78_, false);
      x78_ = buf;
    }

    buf = mMediumPool->Alloc(size);

    if (buf == nullptr) {
      void* puddlePtr = Alloc(CMediumAllocPool::GetAllocMemoryRequired(0x1000) +
                                  CMediumAllocPool::GetBookKeepingMemoryRequired(0x1000),
                              kHI_None, kSC_Unk1, kTP_Heap,
                              CCallStack(-1, "MediumAllocMainData   ", " - Ignore"));
      mMediumPool->AddPuddle(0x1000, puddlePtr, true);
      buf = mMediumPool->Alloc(size);
    }

    if (buf != nullptr) {
      gAllocatorTime += OSGetTick() - startTick;
      return buf;
    }

    if (!x7c_) {
      mMediumPool->GetTotalEntries();
      mMediumPool->GetNumAllocs();
      mMediumPool->GetNumBlocksAvailable();
    }
    x7c_ = true;
  }

  const bool topOfHeap = (hint & kHI_TopOfHeap) != 0;
  uint roundedSize = T_round_up< uint, size_t >(size, 32);
  SGameMemInfo* info = nullptr;

  if (topOfHeap) {
    info = FindFreeBlockFromTopOfHeap(roundedSize);
  } else {
    info = FindFreeBlock(roundedSize);
  }

  if (info == nullptr) {
    void* mediumBuf = nullptr;
    if (mOomCallback) {
      mOomCallback(mOomTarget, size);

      static bool bTriedCallback = false;
      if (!bTriedCallback) {
        bTriedCallback = true;
        mediumBuf = Alloc(size, hint, scope, type, callstack);
        bTriedCallback = false;
      } else {
        return nullptr;
      }
    }
    if (mediumBuf == nullptr) {
      (void)callstack.GetFileAndLineText();
      (void)callstack.GetTypeText();
      DumpAllocations();
      return nullptr;
    }
    return mediumBuf;
  }

  tmp = FixupAllocPtrs(info, size, roundedSize, hint, callstack);
  if (topOfHeap && !info->IsAllocated()) {
    info = info->GetNext();
  }

  UpdateAllocDebugStats(size, roundedSize, tmp);
  gAllocatorTime += OSGetTick() - startTick;
  return ++info;
}

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlock(uint len) {
  uint delta;
  CGameAllocator::SGameMemInfo* ret = nullptr;
  uint binIndex = GetFreeBinEntryForSize(len);

  uint chosenBin = 0;
  SGameMemInfo* previous = nullptr;
  uint bestDelta = 0x10000000;

  for (; binIndex < 16 && !ret; ++binIndex) {
    SGameMemInfo* candidate = mBins[binIndex];
    SGameMemInfo* last = nullptr;
    for (; candidate; last = candidate, candidate = candidate->GetNextFree()) {
      if (!candidate->IsAllocated() && candidate->mLen >= len) {
        delta = candidate->mLen - len;
        if (delta < bestDelta && candidate->GetNext()) {
          ret = candidate;
          previous = last;
          bestDelta = delta;
          chosenBin = binIndex;
          if (delta < sizeof(SGameMemInfo)) {
            break;
          }
        }
      }
    }
  }

  if (ret) {
    if (previous == NULL) {
      mBins[chosenBin] = ret->GetNextFree();
    } else {
      previous->SetNextFree(ret->GetNextFree());
    }
  }
  return ret;
}

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlockFromTopOfHeap(uint size) {
  SGameMemInfo* iter = mLast;
  SGameMemInfo* ret = nullptr;

  while (iter != nullptr) {
    if (!iter->IsAllocated() && iter->GetLength() >= size) {
      ret = iter;
      break;
    }
    iter = iter->GetPrev();
  }

  RemoveFreeEntryFromFreeList(ret);
  return ret;
}

uint CGameAllocator::FixupAllocPtrs(SGameMemInfo* info, const uint len, uint roundedLen, EHint hint,
                                    const CCallStack& cs) {

  const bool topOfHeap = (hint & kHI_TopOfHeap) != 0;
  uint ret = 0;
  const size_t blockLength = info->mLen;
  if (blockLength == roundedLen + sizeof(SGameMemInfo)) {
    ret = sizeof(SGameMemInfo);
    roundedLen += sizeof(SGameMemInfo);
  }

  SGameMemInfo* newPtr = info;
  if (blockLength != roundedLen) {
    SGameMemInfo* newInfo;

    SGameMemInfo* infoNext = info->GetNext();
    if (topOfHeap) {
      newInfo =
          reinterpret_cast< SGameMemInfo* >(reinterpret_cast< char* >(infoNext) - roundedLen) - 1;
      const SGameMemInfo& block = SGameMemInfo(info, infoNext, nullptr, len, "", "");
      *newInfo = block;
      info->mLen -= roundedLen + sizeof(SGameMemInfo);
      AddFreeEntryToFreeList(info);
      newPtr = newInfo;
    } else {
      uint offset = roundedLen + sizeof(SGameMemInfo);
      newInfo = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< char* >(info) + offset);
      const SGameMemInfo& block =
          SGameMemInfo(info, infoNext, info->GetNextFree(),
                       info->mLen - roundedLen - sizeof(SGameMemInfo), "", "");
      *newInfo = block;
      AddFreeEntryToFreeList(newInfo);
    }
    newPtr->mFileAndLine = cs.GetFileAndLineText();
    newPtr->mType = cs.GetTypeText();
    ret = sizeof(SGameMemInfo);

    infoNext->SetPrev(newInfo);
    info->SetNext(newInfo);
  } else {
    info->mFileAndLine = cs.GetFileAndLineText();
    info->mType = cs.GetTypeText();
  }

  newPtr->SetTopOfHeapAllocated(topOfHeap);
  newPtr->SetAllocated(true);
  newPtr->mLen = len;
  return ret;
}

void CGameAllocator::UpdateAllocDebugStats(uint len, uint roundedLen, uint offset) {
  ++x84_;
  ++x80_;
  x88_ += len;
  x8c_ += roundedLen + offset;
  mHeapSize2 -= roundedLen + offset;

  if (x84_ > x94_) {
    x94_ = x84_;
  }

  if (x8c_ > x98_) {
    x98_ = x8c_;
  }

  if (len < x9c_) {
    x9c_ = len;
  }

  if (len > xa0_) {
    xa0_ = len;
  }
  xa4_ = (len + xa4_ * (x80_ - 1)) / x80_;
  if (len > 56) {
    return;
  }

  ++xa8_;
}

bool CGameAllocator::Free(const void* ptr) {
  if (ptr == nullptr) {
    return true;
  }

  if (mSmallAllocPool && mSmallAllocPool->PtrWithinPool(ptr)) {
    return mSmallAllocPool->Free(ptr);
  }

  if (mMediumPool) {
    int tmp = mMediumPool->Free(ptr);
    if (tmp != 1) {
      return tmp > 0;
    }
  }
  return FreeNormalAllocation(ptr);
}

bool CGameAllocator::FreeNormalAllocation(const void* ptr) {
  SGameMemInfo* info = GetMemInfoFromBlockPtr(ptr);
  size_t newLen = 0;
  const size_t infoLen = info->mLen;
  SGameMemInfo* k = info->GetNext();
  size_t len = 0;
  if (k) {
    len = reinterpret_cast< size_t >(k) - reinterpret_cast< size_t >(info) - sizeof(SGameMemInfo);
  }
  info->SetLength(len);

  SGameMemInfo* prev = info->GetPrev();
  SGameMemInfo* next = info->GetNext();

  if (prev && !prev->IsAllocated()) {
    RemoveFreeEntryFromFreeList(prev);
    prev->SetNext(next);
    if (next) {
      next->SetPrev(prev);
    }
    newLen = sizeof(SGameMemInfo);
    prev->mLen += info->mLen + sizeof(SGameMemInfo);
    info = prev;
  }

  if (next && !next->IsAllocated() && next->GetNext()) {
    RemoveFreeEntryFromFreeList(next);
    info->SetNext(next->GetNext());
    if (info->GetNext()) {
      info->GetNext()->SetPrev(info);
    }
    newLen += sizeof(SGameMemInfo);
    info->mLen += next->mLen + sizeof(SGameMemInfo);
    info->SetAllocated(false);
  } else {
    info->SetAllocated(false);
  }
  AddFreeEntryToFreeList(info);

  x84_ -= 1;
  x88_ -= infoLen;
  x8c_ -= (len + newLen);
  mHeapSize2 += (len + newLen);
  if (infoLen <= 56) {
    xa8_ -= 1;
  }

  return true;
};

void CGameAllocator::ReleaseAll() {
  if (mMediumPool) {
    mMediumPool->ClearPuddles();
    FreeNormalAllocation(mMediumPool);
    mMediumPool = nullptr;
  }

  SGameMemInfo* iter = mFirst;
  while (iter != nullptr) {
    SGameMemInfo* next = iter->GetNext();
    if (iter->IsAllocated()) {
      FreeNormalAllocation(((uchar*)iter) + sizeof(SGameMemInfo));
    }
    iter = next;
  }

  mFirst = nullptr;
  mLast = nullptr;
};

void* CGameAllocator::AllocSecondary(size_t size, EHint hint, EScope scope, EType type,
                                     const CCallStack& callstack) {
  return Alloc(size, hint, scope, type, callstack);
};

bool CGameAllocator::FreeSecondary(const void* ptr) { return Free(ptr); };

void CGameAllocator::ReleaseAllSecondary() {};

void CGameAllocator::SetOutOfMemoryCallback(FOutOfMemoryCb cb, const void* target) {
  mOomCallback = cb;
  mOomTarget = target;
};

IAllocator::SAllocInfo CGameAllocator::GetAllocInfo(const void* ptr) const {
  const SGameMemInfo* info = GetMemInfoFromBlockPtr(ptr);

  return SAllocInfo(info, info->GetLength(), info->IsAllocated(), false, info->mFileAndLine,
                    info->mType);
};

IAllocator::SMetrics CGameAllocator::GetMetrics() const {
  uint mediumAllocTotalAllocated =
      mMediumPool != nullptr ? mMediumPool->GetTotalEntries() * 32 : 0;
  uint mediumAllocBlocksAvailable =
      mMediumPool != nullptr ? mMediumPool->GetNumBlocksAvailable() : 0;
  uint mediumAllocAllocatedSize =
      mMediumPool != nullptr
          ? mMediumPool->GetTotalEntries() - mMediumPool->GetNumBlocksAvailable()
          : 0;
  const uint mediumAllocNumAllocs = mMediumPool != nullptr ? mMediumPool->GetNumAllocs() : 0;
  SMetrics ret(mHeapSize, x80_, x84_, x88_, x8c_, mHeapSize2, x94_, x98_, x9c_, xa0_, xa4_,
               mSmallAllocPool != nullptr ? mSmallAllocPool->GetNumAllocs() : 0,
               mSmallAllocPool != nullptr ? mSmallAllocPool->GetAllocatedSize() : 0,
               mSmallAllocPool != nullptr ? mSmallAllocPool->GetNumBlocksAvailable() : 0,
               mediumAllocNumAllocs, mediumAllocAllocatedSize, mediumAllocBlocksAvailable,
               x80_ - xb0_, reinterpret_cast< uintptr_t >(mPhysicalAddr), xbc_,
               mediumAllocTotalAllocated, mFakeStatics);
  xb0_ = x80_;
  return ret;
};

int CGameAllocator::EnumAllocations(FEnumAllocationsCb func, const void* ptr, bool b) const {

  int i = 0;
  const SGameMemInfo* iter = mFirst;

  while (iter != nullptr) {
    if (!iter->IsPostGuardIntact()) {
      return -1;
    }

    if (!iter->IsPriorGuardIntact()) {
      return -1;
    }

    const SGameMemInfo* next = iter->GetNext();
    SAllocInfo alloc(iter, iter->GetLength(), iter->IsAllocated(), false, iter->mFileAndLine,
                     iter->mType);
    func(alloc, ptr);
    ++i;
    iter = next;
  }

  return i;
};

uint CGameAllocator::GetFreeBinEntryForSize(const uint size) {
  uint maxLen = 0x20;
  uint bin = 0;

  while (maxLen < 0x200000) {
    if (size < maxLen) {
      return bin;
    }

    maxLen <<= 1;
    ++bin;
  }

  return 0xf;
}

void CGameAllocator::AddFreeEntryToFreeList(SGameMemInfo* info) {
  uint bin = GetFreeBinEntryForSize(info->GetLength());
  info->SetNextFree(mBins[bin]);
  mBins[bin] = info;
}

void CGameAllocator::RemoveFreeEntryFromFreeList(SGameMemInfo* memInfo) {
  uint bin = GetFreeBinEntryForSize(memInfo->GetLength());
  SGameMemInfo* curBin = nullptr;
  SGameMemInfo* binIt = mBins[bin];

  while (binIt != nullptr) {
    if (binIt == memInfo) {
      if (curBin == nullptr) {
        mBins[bin] = binIt->GetNextFree();
      } else {
        curBin->SetNextFree(binIt->GetNextFree());
      }
      return;
    }

    curBin = binIt;
    binIt = binIt->GetNextFree();
  }
}

static inline bool DoWait(int v) { return (v % 4) == 0; }

void CGameAllocator::DumpAllocations() const {
  GetLargestFreeChunk();
  uint i = 0;
  SGameMemInfo* iter = mFirst;

  while (iter != nullptr) {
    ++i;

    if (DoWait(i)) {
      CStopwatch::Wait(0.005f);
    }
    iter = iter->GetNext();
  }
}

size_t CGameAllocator::GetLargestFreeChunk() const {
  const SGameMemInfo* iter = mFirst;
  size_t ret = 0;
  while (iter != nullptr) {
    if (!iter->IsAllocated() && iter->GetLength() > ret) {
      ret = iter->GetLength();
    }
    iter = iter->GetNextFree();
  }

  return ret;
}
void CGameAllocator::OffsetFakeStatics(const int offset) { mFakeStatics += offset; }
