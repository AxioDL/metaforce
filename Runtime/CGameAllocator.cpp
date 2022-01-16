#include "Runtime/CGameAllocator.hpp"
#include <new>

namespace metaforce {
logvisor::Module AllocLog("metaforce::CGameAllocator");

#pragma GCC diagnostic ignored "-Wclass-memaccess"

u32 CGameAllocator::GetFreeBinEntryForSize(size_t len) {
  size_t bin = 0;
  size_t binSize = 32;
  while (true) {
    if (binSize > 0x200000) {
      return 15;
    }
    if (len < binSize) {
      break;
    }
    binSize <<= 1;
    ++bin;
  }
  return bin;
}

bool CGameAllocator::Initialize() { return true; }
void* CGameAllocator::Alloc() { return nullptr; }
bool CGameAllocator::Free(void* ptr) { return 0; }
void CGameAllocator::ReleaseAll() {}
void CGameAllocator::AllocSecondary() {}
void CGameAllocator::FreeSecondary() {}
void CGameAllocator::ReleaseAllSecondary() {}

void CGameAllocator::SetOutOfMemoryCallback(IAllocator::FOutOfMemoryCb cb, void* target) {
  x58_oomCallBack = cb;
  x5c_oomTarget = target;
}

s32 CGameAllocator::EnumAllocations(IAllocator::FEnumAllocationsCb cb, const void* ptr, bool b) {
  s32 ret = 0;
  SAllocInfo info;
  info.x0_infoPtr = xc_first;
  while (true) {
    if (info.x0_infoPtr == nullptr) {
      return ret;
    }

    if (static_cast<SGameMemInfo*>(info.x0_infoPtr)->x1c_postGuard != 0xeaeaeaea) {
      break;
    }
    if (static_cast<SGameMemInfo*>(info.x0_infoPtr)->x0_priorGuard != 0xefefefef) {
      return -1;
    }

    void* next = static_cast<SGameMemInfo*>(info.x0_infoPtr)->GetNext();
    info.x10_type = static_cast<SGameMemInfo*>(info.x0_infoPtr)->xc_type;
    info.x8_hasPrevious = static_cast<SGameMemInfo*>(info.x0_infoPtr)->x10_prev != nullptr;
    info.xc_fileAndLne = static_cast<SGameMemInfo*>(info.x0_infoPtr)->x8_fileAndLine;
    info.x4_len = static_cast<SGameMemInfo*>(info.x0_infoPtr)->x4_len;
    info.x9_ = false;
    (*cb)(info, ptr);

    ++ret;
    info.x0_infoPtr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(next) & ~sizeof(SGameMemInfo));
  }

  return -1;
}

IAllocator::SAllocInfo CGameAllocator::GetAllocInfo(void* ptr) {
  SGameMemInfo* info = GetMemInfoFromBlockPtr(ptr);
  return {.x0_infoPtr = info,
          .x4_len = info->x4_len,
          .x8_hasPrevious = info->x10_prev != nullptr,
          .x9_ = false,
          .xc_fileAndLne = info->x8_fileAndLine,
          .x10_type = info->xc_type};
}

void CGameAllocator::OffsetFakeStatics(s32 offset) { xb8_fakeStatics += offset; }
IAllocator::SMetrics CGameAllocator::GetMetrics() {
  u32 mediumAllocTotalAllocated = x74_mediumPool == nullptr ? 0 : x74_mediumPool->GetTotalEntries();
  u32 mediumAllocBlocksAvailable = x74_mediumPool == nullptr ? 0 : x74_mediumPool->GetNumBlocksAvailable();
  u32 mediumAllocAllocatedSize =
      x74_mediumPool == nullptr ? 0 : x74_mediumPool->GetTotalEntries() - x74_mediumPool->GetNumBlocksAvailable();
  u32 mediumAllocNumAllocs = x74_mediumPool == nullptr ? 0 : x74_mediumPool->GetNumAllocs();

  u32 smallAllocRemainingSize = 0;
  u32 smallAllocAllocatedSize = 0;
  u32 smallAllocNumAllocs = 0;
  if (x60_smallPool == nullptr) {

  } else {
    smallAllocRemainingSize = x60_smallPool->GetNumBlocksAvailable();
    smallAllocAllocatedSize = x60_smallPool->GetTotalEntries();
    smallAllocNumAllocs = x60_smallPool->GetNumAllocs();
  }

  return SMetrics(x8_heapSize, x80_, x84_, x88_, x8c_, x90_heapSize2, x94_, x98_, x9c_, xa0_, xa4_, smallAllocNumAllocs,
                  smallAllocAllocatedSize, smallAllocRemainingSize, mediumAllocNumAllocs, mediumAllocAllocatedSize,
                  mediumAllocBlocksAvailable, x80_ - xb0_, xb4_, xbc_, mediumAllocTotalAllocated, xb8_fakeStatics);
}
} // namespace metaforce
