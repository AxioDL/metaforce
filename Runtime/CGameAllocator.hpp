#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "Runtime/IAllocator.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/CSmallAllocPool.hpp"
#include "Runtime/CMediumAllocPool.hpp"

namespace metaforce {
class CGameAllocator : public IAllocator {
public:
private:
  class SGameMemInfo {
  public:
    u32 x0_priorGuard = 0xefefefef;
    size_t x4_len = 0;
    const char* x8_fileAndLine;
    const char* xc_type;
    void* x10_prev = nullptr;
    void* x14_next = nullptr;
    void* x18_ = nullptr;
    u32 x1c_postGuard = 0xeaeaeaea;
  public:
    void* GetPrev() { return x10_prev; }
    void SetPrev(void* prev) { x10_prev = prev; }
    void* GetNext() { return x14_next; }
    void SetNext(void* next) { x14_next = next; }
    u32 GetPrevMaskedFlags();
    u32 GetNextMaskedFlags();
    void SetTopOfHeapAllocated(bool topOfHeap);
  };

  u8 x4_;
  u8 x5_;
  u8 x6_;
  u8 x7_;
  u32 x8_heapSize;
  SGameMemInfo* xc_first;
  SGameMemInfo* x10_last;
  std::array<SGameMemInfo*, 16> x14_bins;
  u32 x54_;
  FOutOfMemoryCb x58_oomCallBack;
  void* x5c_oomTarget;
  std::unique_ptr<CSmallAllocPool> x60_smallPool;
  void* x64_smallAllocBookKeeping;
  void* x68_smallAllocMainData;
  bool x6c_;
  u32 x70_;
  std::unique_ptr<CMediumAllocPool> x74_mediumPool;
  void* x78_;
  bool x7c_;
  u32 x80_;
  u32 x84_;
  u32 x88_;
  u32 x8c_;
  u32 x90_heapSize2;
  u32 x94_;
  u32 x98_;
  u32 x9c_;
  u32 xa0_;
  u32 xa4_;
  u32 xa8_;
  u32 xac_;
  u32 xb0_;
  u32 xb4_;
  u32 xb8_fakeStatics = 0;
  u32 xbc_;

  static u32 GetFreeBinEntryForSize(size_t len);
  SGameMemInfo* GetMemInfoFromBlockPtr(void* ptr) {
    return reinterpret_cast<SGameMemInfo*>(reinterpret_cast<void*>(intptr_t(ptr) - sizeof(SGameMemInfo)));
  }

public:
  bool Initialize() override; // const COsContext& ctx);
  void* Alloc(size_t size) override;
  bool Free(void* ptr) override;
  void ReleaseAll() override;
  void* AllocSecondary(size_t size) override;
  bool FreeSecondary(void* ptr) override;
  void ReleaseAllSecondary() override;
  void SetOutOfMemoryCallback(FOutOfMemoryCb cb, void* target) override;
  s32 EnumAllocations(IAllocator::FEnumAllocationsCb cb, const void* ptr, bool b) override;
  SAllocInfo GetAllocInfo(void* ptr) override;
  void OffsetFakeStatics(s32 offset) override;
  SMetrics GetMetrics() override;
  void AllocSecondary();
  void FreeSecondary();
  void* Alloc();
};
} // namespace metaforce
