#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Alloc/CMemorySys.hpp"

#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/CGameAllocator.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"

#include "dolphin/os.h"

static CGameAllocator gGameAllocator;
IAllocator* CMemory::mpAllocator = &gGameAllocator;
bool CMemory::mInitialized;
uint gLeakCount = 0;
uint gLeakBytes = 0;

CMemorySys::CMemorySys(COsContext& ctx, IAllocator& allocator) {
  CMemory::Startup(ctx);
  CMemory::SetAllocator(ctx, allocator);
}

CMemorySys::~CMemorySys() { CMemory::Shutdown(); }

IAllocator& CMemorySys::GetGameAllocator() { return gGameAllocator; }

void CMemory::Startup(COsContext& ctx) { mInitialized = mpAllocator->Initialize(ctx); }

void CMemory::SetAllocator(COsContext& ctx, IAllocator& allocator) {
  if (mpAllocator != &allocator) {
    if (mpAllocator != nullptr) {
      mpAllocator->ReleaseAll();
    }
    mpAllocator = &allocator;
    mpAllocator->Initialize(ctx);
  }
}

static bool cmemory_enum_alloc_cb(const IAllocator::SAllocInfo& info, const void* ptr) {
  if (info.mIsAllocated && info.x9_ == 0) {
    ++gLeakCount;
    gLeakBytes += info.mLen;
  }
  return true;
}

void CMemory::Shutdown() {
  CMemory::mInitialized = false;

  if (mpAllocator->GetMetrics().x8_ != 0) {
    gLeakCount = 0;
    gLeakBytes = 0;
    mpAllocator->EnumAllocations((IAllocator::FEnumAllocationsCb)cmemory_enum_alloc_cb, nullptr,
                                 false);
  }
  mpAllocator->Shutdown();
}

void* CMemory::Alloc(size_t len, IAllocator::EHint hint, IAllocator::EScope scope,
                     IAllocator::EType type, const CCallStack& callstack) {
  volatile bool enabled = OSDisableInterrupts();
  void* ret = mpAllocator->Alloc(len, hint, scope, type, callstack);
  if (ret == nullptr) {
    rs_debugger_printf("Alloc failed - Size: %d", len);
  }

  OSRestoreInterrupts(enabled);
  return ret;
}

void CMemory::Free(const void* ptr) {
  volatile bool enabled = OSDisableInterrupts();
  if (ptr != nullptr) {
    mpAllocator->Free(ptr);
  }
  OSRestoreInterrupts(enabled);
}

void CMemory::SetOutOfMemoryCallback(IAllocator::FOutOfMemoryCb cb, const void* context) {
  mpAllocator->SetOutOfMemoryCallback(cb, context);
}

void CMemory::OffsetFakeStatics(int offset) { mpAllocator->OffsetFakeStatics(offset); }

void* operator new(size_t sz, const char* fileAndLine, const char* type) {
  return CMemory::Alloc(sz, IAllocator::kHI_None, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                        CCallStack(-1, fileAndLine, type));
}
void* operator new[](size_t sz, const char* fileAndLine, const char* type) {
  return CMemory::Alloc(sz, IAllocator::kHI_None, IAllocator::kSC_Unk1, IAllocator::kTP_Array,
                        CCallStack(-1, fileAndLine, type));
}
