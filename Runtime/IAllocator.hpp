#pragma once

#include <athena/Global.hpp>
#include "Runtime/GCNTypes.hpp"

namespace metaforce {
class CCallStack {
  const char* x0_line;
  const char* x4_type;

public:
  CCallStack(int lineNum, const char* lineStr, const char* type) : x0_line(lineStr), x4_type(type) {}
};

enum class EHint {
  Unk = (1 << 0),
  RoundUpLen = (1 << 1),
};
ENABLE_BITWISE_ENUM(EHint);

enum class EScope {

};

enum class EType {

};

class IAllocator {
public:
  using FOutOfMemoryCb = bool (*)(void*, u32);
  struct SMetrics {
    u32 x0_heapSize;
    u32 x4_;
    u32 x8_;
    u32 xc_;
    u32 x10_;
    u32 x14_heapSize2; // Remaining heap size?
    u32 x18_;
    u32 x1c_;
    u32 x20_;
    u32 x24_;
    u32 x28_;
    u32 x2c_smallNumAllocs;
    u32 x30_smallAllocatedSize;
    u32 x34_smallRemainingSize;
    u32 x38_mediumNumAllocs;
    u32 x3c_mediumAllocatedSize;
    u32 x40_mediumBlocksAvailable;
    u32 x44_;
    u32 x48_;
    u32 x4c_;
    u32 x50_mediumTotalAllocated;
    u32 x54_fakeStatics;
    SMetrics(u32 heapSize, u32 unk1, u32 unk2, u32 unk3, u32 unk4, u32 heapSize2, u32 unk5, u32 unk6, u32 unk7,
             u32 unk8, u32 unk9, u32 smallAllocNumAllocs, u32 smallAllocAllocatedSize, u32 smallAllocRemainingSize,
             u32 mediumAllocNumAllocs, u32 mediumAllocAllocatedSize, u32 mediumAllocBlocksAvailable, u32 unk10, u32 unk11, u32 unk12,
             u32 mediumAllocTotalAllocated, u32 fakeStatics);
  };

  struct SAllocInfo {
    void* x0_infoPtr;
    size_t x4_len;
    bool x8_hasPrevious;
    bool x9_;
    const char* xc_fileAndLne;
    const char* x10_type;
  };

  using FEnumAllocationsCb = const bool (*)(const SAllocInfo& info, const void* ptr);
  virtual bool Initialize() = 0; // const COSContext& ctx) = 0;

  virtual void* Alloc(size_t size) = 0;
  virtual bool Free(void* ptr) = 0;
  virtual void ReleaseAll() = 0;
  virtual void* AllocSecondary(size_t size) = 0;
  virtual bool FreeSecondary(void* ptr) = 0;
  virtual void ReleaseAllSecondary() = 0;
  virtual void SetOutOfMemoryCallback(FOutOfMemoryCb cb, void* target) = 0;
  virtual s32 EnumAllocations(FEnumAllocationsCb cb, const void* ptr, bool b) = 0;
  virtual SAllocInfo GetAllocInfo(void* ptr) = 0;
  virtual void OffsetFakeStatics(s32 offset) = 0;
  virtual SMetrics GetMetrics() = 0;
};
}