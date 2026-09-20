#ifndef _IALLOCATOR
#define _IALLOCATOR

#include "types.h"
#include <stddef.h>
#include "Kyoto/Alloc/AllocatorCommon.hpp"

class COsContext;
class CCallStack;

class IAllocator {
public:
  enum EHint {
    kHI_None = 0,
    kHI_TopOfHeap = (1 << 0),
    kHI_RoundUpLen = (1 << 1),
  };

  enum EScope {
    kSC_Unk0,
    kSC_Unk1,
  };

  enum EType {
    kTP_Heap,
    kTP_Array,
  };

  struct SMetrics {
    uint x0_heapSize;
    uint x4_;
    uint x8_;
    uint xc_;
    uint x10_;
    uint x14_heapSize2; // Remaining heap size?
    uint x18_;
    uint x1c_;
    uint x20_;
    uint x24_;
    uint x28_;
    uint x2c_smallNumAllocs;
    uint x30_smallAllocatedSize;
    uint x34_smallRemainingSize;
    uint x38_mediumNumAllocs;
    uint x3c_mediumAllocatedSize;
    uint x40_mediumBlocksAvailable;
    uint x44_;
    uint x48_;
    uint x4c_;
    uint x50_mediumTotalAllocated;
    uint x54_fakeStatics;
    SMetrics(uint heapSize, uint unk1, uint unk2, uint unk3, uint unk4, uint heapSize2, uint unk5,
             uint unk6, uint unk7, uint unk8, uint unk9, uint smallAllocNumAllocs,
             uint smallAllocAllocatedSize, uint smallAllocRemainingSize, uint mediumAllocNumAllocs,
             uint mediumAllocAllocatedSize, uint mediumAllocBlocksAvailable, uint unk10, uint unk11,
             uint unk12, uint mediumAllocTotalAllocated, uint fakeStatics);
    SMetrics(const SMetrics& other);
  };

  struct SAllocInfo {
    const void* x0_infoPtr;
    uint x4_len;
    bool x8_isAllocated;
    bool x9_;
    const char* xc_fileAndLine;
    const char* x10_type;

    SAllocInfo(const void* ptr, uint len, bool isAllocated, bool b2, const char* fileAndLine,
               const char* type)
    : x0_infoPtr(ptr)
    , x4_len(len)
    , x8_isAllocated(isAllocated)
    , x9_(b2)
    , xc_fileAndLine(fileAndLine)
    , x10_type(type) {}
  };

  typedef const bool (*FOutOfMemoryCb)(const void*, uint);
  typedef const bool (*FEnumAllocationsCb)(const SAllocInfo& info, const void* ptr);
  
  virtual ~IAllocator();

  virtual bool Initialize(COsContext& ctx) = 0;
  virtual void Shutdown() = 0;
  virtual void* Alloc(size_t size, EHint hint, EScope scope, EType type, const CCallStack& cs) = 0;
  virtual bool Free(const void* ptr) = 0;
  virtual void ReleaseAll() = 0;
  virtual void* AllocSecondary(size_t size, EHint hint, EScope scope, EType type,
                               const CCallStack& cs) = 0;
  virtual bool FreeSecondary(const void* ptr) = 0;
  virtual void ReleaseAllSecondary() = 0;
  virtual void SetOutOfMemoryCallback(FOutOfMemoryCb cb, const void* data) = 0;
  virtual int EnumAllocations(FEnumAllocationsCb func, const void* ptr, bool b) const = 0;
  virtual SAllocInfo GetAllocInfo(const void* ptr) const = 0;
  virtual void OffsetFakeStatics(int offset) = 0;
  virtual SMetrics GetMetrics() const = 0;
};

#endif // _IALLOCATOR
