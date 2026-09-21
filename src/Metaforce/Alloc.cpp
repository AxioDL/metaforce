#include "Kyoto/Alloc/CMemory.hpp"

#include <limits>

bool CMemory::mInitialized;

void CMemory::Startup(COsContext& ctx) { mInitialized = true; }

void CMemory::Shutdown() { mInitialized = false; }

void* CMemory::Alloc(size_t len, IAllocator::EHint hint, IAllocator::EScope scope,
                     IAllocator::EType type, const CCallStack& callstack) {
  if (hint & IAllocator::kHI_RoundUpLen) {
    if (len > std::numeric_limits<size_t>::max() - 31) {
      throw std::bad_array_new_length();
    }
    len = (len + 31) & ~size_t{31};
  }
  return new uchar[len];
}

void CMemory::Free(const void* ptr) {
  operator delete[](const_cast< void* >(ptr));
}

void CMemory::SetOutOfMemoryCallback(IAllocator::FOutOfMemoryCb cb, const void* context) {}

void CMemory::OffsetFakeStatics(int offset) {}
