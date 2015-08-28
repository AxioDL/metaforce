#include "CGameAllocator.hpp"

namespace Retro
{

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlock(u32)
{
}
CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlockFromTopOfHeap(u32)
{
}
u32 CGameAllocator::FixupAllocPtrs(SGameMemInfo*, u32, u32, EHint, const CCallStack&)
{
}
void CGameAllocator::UpdateAllocDebugStats(u32, u32, u32)
{
}
bool CGameAllocator::FreeNormalAllocation(void*)
{
}
u32 CGameAllocator::GetFreeBinEntryForSize(u32)
{
}
void CGameAllocator::AddFreeEntryToFreeList(SGameMemInfo*)
{
}
void CGameAllocator::RemoveFreeEntryFromFreeList(SGameMemInfo*)
{
}
void CGameAllocator::DumpAllocations() const
{
}
u32 CGameAllocator::GetLargestFreeChunk() const
{
}
CGameAllocator::SGameMemInfo* CGameAllocator::GetMemInfoFromBlockPtr(void* ptr)
{
    return reinterpret_cast<SGameMemInfo*>(reinterpret_cast<u8*>(ptr) - 32);
}

bool CGameAllocator::Initialize()
{
}
void CGameAllocator::Shutdown()
{
}
void* CGameAllocator::Alloc(size_t, EHint, EScope, EType, const CCallStack&)
{
}
void CGameAllocator::Free(void*)
{
}
void CGameAllocator::ReleaseAll()
{
}
void* CGameAllocator::AllocSecondary(size_t, EHint, EScope, EType, const CCallStack&)
{
}
void CGameAllocator::FreeSecondary(void*)
{
}
void CGameAllocator::ReleaseAllSecondary()
{
}
void CGameAllocator::SetOutOfMemoryCallback(const TOutOfMemoryCallback cb, void* ctx)
{
    x58_oomCb = cb;
    x5c_oomCtx = ctx;
}
int CGameAllocator::EnumAllocations(const TAllocationVisitCallback, void*, bool) const
{
}
CGameAllocator::SAllocInfo CGameAllocator::GetAllocInfo(void*) const
{
}
void CGameAllocator::OffsetFakeStatics(int)
{
}
CGameAllocator::SMetrics CGameAllocator::GetMetrics() const
{
}

}
