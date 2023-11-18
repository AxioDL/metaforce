#include "Runtime/CGameAllocator.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce {
static logvisor::Module Log("metaforce::CGameAllocator");

#pragma GCC diagnostic ignored "-Wclass-memaccess"

std::vector<CGameAllocator::SAllocationDescription> CGameAllocator::m_allocations;

u8* CGameAllocator::Alloc(size_t len) {
  size_t roundedLen = ROUND_UP_64(len + sizeof(SChunkDescription));
  for (SAllocationDescription& alloc : m_allocations) {
    /* We need to supply enough room for allocation information */
    if (alloc.freeOffset + roundedLen < alloc.allocSize) {
      u8* ptr = alloc.memptr.get() + alloc.freeOffset;
      SChunkDescription* chunkInfo = reinterpret_cast<SChunkDescription*>(ptr);
      *chunkInfo = SChunkDescription();
      chunkInfo->parent = &alloc;
      chunkInfo->len = len;
      alloc.freeOffset += roundedLen;
      return ptr + sizeof(SChunkDescription);
    }
  }

  /* 1MiB minimum allocation to prevent constantly allocating small amounts of memory */
  size_t allocSz = len;
  if (allocSz < (1 * 1024 * 1024 * 1024))
    allocSz = 1 * 1024 * 1024 * 1024;

  /* Pad size to allow for allocation information */
  allocSz = ROUND_UP_64(allocSz + sizeof(SChunkDescription));
  auto& alloc = m_allocations.emplace_back();
  alloc.memptr.reset(new u8[allocSz]);
  u8* ptr = alloc.memptr.get();
  alloc.allocSize = allocSz;
  alloc.freeOffset += roundedLen;
  SChunkDescription* chunkInfo = reinterpret_cast<SChunkDescription*>(ptr);
  *chunkInfo = SChunkDescription();
  chunkInfo->parent = &alloc;
  chunkInfo->len = len;
  return ptr + sizeof(SChunkDescription);
}

void CGameAllocator::Free(u8* ptr) {
  SChunkDescription* info = reinterpret_cast<SChunkDescription*>(ptr - sizeof(SChunkDescription));
  if (info->magic != 0xE8E8E8E8 || info->sentinal != 0xEFEFEFEF) {
    Log.report(logvisor::Fatal, FMT_STRING("Invalid chunk description, memory corruption!"));
    return;
  }

  SAllocationDescription& alloc = *info->parent;
  size_t roundedLen = ROUND_UP_32(info->len + sizeof(SChunkDescription));
  alloc.freeOffset -= roundedLen;
  /* Invalidate chunk allocation descriptor */
  memset(info, 0, ROUND_UP_64(info->len + sizeof(SChunkDescription)));
}
} // namespace metaforce
