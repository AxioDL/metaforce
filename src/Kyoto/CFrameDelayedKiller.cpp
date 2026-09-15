#include "Kyoto/CFrameDelayedKiller.hpp"

#include "Kyoto/Particles/IElement.hpp"

#include <dolphin/gx/GXManage.h>
#include <rstl/list.hpp>

#if NONMATCHING
#include <stdint.h>
#endif

static uint sCurList = 0;
static rstl::list< void* > sFrameDelayedList[2];

#if defined(__MWERKS__) && (VERSION < VERSION_GM8P_00 || VERSION == VERSION_GM8E_02)
#pragma force_active on
CFrameDelayedKiller::Stats CFrameDelayedKiller::mUnusedStats = {0, 0, 0, 0, 0, 0};
#pragma force_active reset
#endif

void CFrameDelayedKiller::Initialize() { StallAndFlushAllAllocations(); }

void CFrameDelayedKiller::ShutDown() { StallAndFlushAllAllocations(); }

void CFrameDelayedKiller::FlushAllAllocations() {
  for (int i = 0; i < 2; ++i) {
    FlushAllocationsForFrame();
  }
}

void CFrameDelayedKiller::StallAndFlushAllAllocations() {
  GXDrawDone();
  FlushAllAllocations();
}

void CFrameDelayedKiller::ScheduleDeletion(const EWhichFrame thisFrame, void* victim) {
  uint index = thisFrame == true ? sCurList : sCurList ^ 1;

  sFrameDelayedList[index].push_back(victim);
}

void CFrameDelayedKiller::FlushAllocationsForFrame() {
  sCurList ^= 1;
  rstl::list< void* >& list = sFrameDelayedList[sCurList];
  for (rstl::list< void* >::iterator t = list.begin(); t != list.end(); ++t) {
    CMemory::Free(*t);
  }

  list.clear();
}

CElementAllocationChunk::CElementAllocationChunk()
: x0_capacity(256)
, x4_allocatedWords(0)
, x8_allocationCount(0) {}

bool CElementAllocationChunk::CanAllocate(uint size) const {
  return x0_capacity > x4_allocatedWords + (size + 3) / 4;
}

bool CElementAllocationChunk::Contains(const void* ptr) const {
#if NONMATCHING
  return reinterpret_cast< uintptr_t >(ptr) - reinterpret_cast< uintptr_t >(xc_data) <
         sizeof(xc_data);
#else
  int offset = static_cast< const char* >(ptr) - reinterpret_cast< const char* >(xc_data);
  int index = offset / 4;
  return x0_capacity > index;
#endif
}

void* CElementAllocationChunk::Allocate(uint size) {
  void* ptr = &xc_data[x4_allocatedWords];
  x4_allocatedWords += (size + 3) / 4;
  ++x8_allocationCount;
  return ptr;
}

void CElementAllocationChunk::Free(void*) { --x8_allocationCount; }

void CElementAllocationChunk::Rewind(uint size) {
  uint words = (size + 3) / 4;
  if (words > x4_allocatedWords) {
    x4_allocatedWords = 0;
  } else {
    x4_allocatedWords -= words;
  }
}

uint CElementAllocationChunk::GetAllocatedSize() const { return x4_allocatedWords * 4; }

uint CElementAllocationChunk::GetAllocationCount() const { return x8_allocationCount; }

void* IElement::operator new(size_t sz, const char* fileAndLine, const char* type) {
  return CElementAllocator::Alloc(sz, fileAndLine, type);
}

void IElement::operator delete(void* ptr, const size_t sz) { CElementAllocator::Free(ptr, sz); }
