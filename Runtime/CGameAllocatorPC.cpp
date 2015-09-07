#include "CGameAllocator.hpp"
#include "CCallStack.hpp"
#include <cstddef>

#define ALLOC_ROUNDUP(x) (((x) + (32 - 1)) & ~(32 - 1))
#define ALLOC_ROUNDDOWN(x) ((x) & ~(32 - 1))

namespace Retro
{
LogVisor::LogModule AllocLog = LogVisor::LogModule("CGameAllocatorPC");

const u32 CGameAllocator::skMinBlockSize = sizeof(CGameAllocator::SGameMemInfo);
const u32 CGameAllocator::skMaxBlockSize = (2 * 1024 * 1024);
const char* CGameAllocator::skMemHead = "MemHead\0";
const char* CGameAllocator::skMemTail = "MemTail\0";

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlock(u32 size)
{
    u32 binIdx = GetFreeBinEntryForSize(size);
    SGameMemInfo** bin = &x10_bins[binIdx];
    SGameMemInfo* blockr31 = nullptr;
    SGameMemInfo* blockr7 = nullptr;
    u32 prevIdx = 0;

    u32 unk1 = 0x1000;

    while (!blockr31)
    {
        if (binIdx >= 0x10)
            break;
        SGameMemInfo* r9 = (*bin)->x14_next;
        SGameMemInfo* r10 = nullptr;
        while (r9)
        {
            SGameMemInfo* r0 = r9->x10_prev;
            if (!r0)
            {
                r10 = r9;
                r9 = r9->x18_ctx;
                continue;
            }

            u32 r0_u = r9->x4_size;
            if (r0_u < size)
            {
                r10 = r9;
                r9 = r9->x18_ctx;
                continue;
            }

            r0 = r9->x14_next;
            if (!r0)
            {
                r10 = r9;
                r9 = r9->x18_ctx;
                continue;
            }

            u32 r4 = (size - r0_u);
            if (r4 >= unk1)
            {
                r10 = r9;
                r9 = r9->x18_ctx;
                continue;
            }
            unk1 = r4;
            blockr31 = r9;
            blockr7 = r10;
            prevIdx = binIdx;

            if (r4 < 0x20)
                break;
        }
        binIdx++;
        bin++;
    }

    if (!blockr31 || !blockr7)
        return nullptr;

    SGameMemInfo* ret = blockr7->x18_ctx;
    blockr7->x18_ctx = blockr31->x18_ctx;
    return ret;
}

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlockFromTopOfHeap(u32 size)
{
    SGameMemInfo** bins = &x10_bins[0];
    SGameMemInfo* bin = nullptr;
    u32 i = 0;
    while (i < 0x10)
    {
        if (bins[i] && bins[i]->x4_size >= size)
        {
            bin = bins[i];
            break;
        }
        i++;
    }

    if (!bin)
        return nullptr;

    RemoveFreeEntryFromFreeList(bin);
    return bin;
}

u32 CGameAllocator::FixupAllocPtrs(SGameMemInfo* memInfo, u32 size, u32 roundedSize, EHint hint, const CCallStack& cs)
{
    roundedSize += 32; // sizeof(SGameMemInfo);
    switch(hint)
    {
        case HintNone:
        {
        }
            break;
        case HintLarge:
            break;
        case HintTopOfHeap:
            break;
    }

    return 32;
}

void CGameAllocator::UpdateAllocDebugStats(u32, u32, u32)
{
}

void CGameAllocator::FreeNormalAllocation(void* ptr)
{
    SGameMemInfo* memInfo = GetMemInfoFromBlockPtr(ptr);
    SGameMemInfo* next = memInfo->x14_next;
    if (next)
    {
        u32 size = next->x4_size;
        next->x10_prev = next->x14_next;
    }
}

u32 CGameAllocator::GetFreeBinEntryForSize(u32 size)
{
    u32 curBlock = skMinBlockSize;
    u32 bin = 0;

    while (curBlock < skMaxBlockSize)
    {
        if (size > curBlock)
        {
            curBlock <<= 1;
            bin++;
        }
        else
            return bin;
    }

    return 0xF;
}

void CGameAllocator::AddFreeEntryToFreeList(SGameMemInfo* memInfo)
{
    SGameMemInfo*& bin = x10_bins[GetFreeBinEntryForSize(memInfo->x4_size)];
    memInfo->x18_ctx = bin;
    bin = memInfo;
}

void CGameAllocator::RemoveFreeEntryFromFreeList(SGameMemInfo* memInfo)
{
    SGameMemInfo*& bin = x10_bins[GetFreeBinEntryForSize(memInfo->x4_size)];
    SGameMemInfo* next = bin->x14_next;
    SGameMemInfo* prev = nullptr;
    while (next)
    {
        if (prev)
        {
            prev = next->x18_ctx;
            bin = prev;
        }

        next->x18_ctx = bin->x18_ctx;

        if (bin == memInfo)
            bin = memInfo->x18_ctx;

        next = next->x14_next;
    }
}

void CGameAllocator::DumpAllocations() const
{
}

u32 CGameAllocator::GetLargestFreeChunk() const
{
    SGameMemInfo* memInfo = xc_infoTail;
    u32 ret = 0;
    while(memInfo)
    {
        SGameMemInfo* tmp = memInfo->x10_prev;
        if (!tmp || memInfo->x4_size <= ret)
        {
            memInfo = memInfo->x18_ctx;
            continue;
        }
        ret = memInfo->x4_size;
        memInfo = memInfo->x18_ctx;
    }

    return ret;
}

CGameAllocator::SGameMemInfo* CGameAllocator::GetMemInfoFromBlockPtr(void* ptr)
{
    return reinterpret_cast<SGameMemInfo*>(reinterpret_cast<u8*>(ptr) - sizeof(SGameMemInfo));
}

bool CGameAllocator::Initialize()
{
    x4_heapSize = (0x0116d280);
    x54_heap = (u8*)_mm_malloc(x4_heapSize, sizeof(SGameMemInfo));
    if (!x54_heap)
    {
        AllocLog.report(LogVisor::FatalError, _S("Failed allocate memory, unable to continue!"));
        return false;
    }

    x8_infoHead = (SGameMemInfo*)(x54_heap);
    x8_infoHead->x0_sentinel = skSentinel;
    x8_infoHead->x4_size = x4_heapSize;
    x8_infoHead->x8_fileAndLine = &skMemHead;
    x8_infoHead->xc_type = &skMemHead;
    x8_infoHead->x10_prev = nullptr;
    x8_infoHead->x14_next = (SGameMemInfo*)(((u8*)x54_heap) + x4_heapSize - sizeof(SGameMemInfo));
    x8_infoHead->x18_ctx = x8_infoHead->x14_next;
    memset(x8_infoHead->x1c_canary, skCanary, sizeof(SGameMemInfo) - offsetof(SGameMemInfo, x1c_canary));
    xc_infoTail = x8_infoHead->x14_next;
    xc_infoTail->x0_sentinel = skSentinel;
    xc_infoTail->x4_size = 0;
    xc_infoTail->x8_fileAndLine = &skMemTail;
    xc_infoTail->xc_type = &skMemTail;
    xc_infoTail->x10_prev = x8_infoHead;
    xc_infoTail->x14_next = nullptr;
    xc_infoTail->x18_ctx = x8_infoHead;
    memset(xc_infoTail->x1c_canary, skCanary, sizeof(SGameMemInfo) - offsetof(SGameMemInfo, x1c_canary));

    AddFreeEntryToFreeList(x8_infoHead);

    Alloc((18 * 1024 * 1024), HintNone, ScopeDefault, TypePrimitive, CCallStack(__FILE__, "Test"));

    return true;
}

void CGameAllocator::Shutdown()
{
    ReleaseAll();
    _mm_free((void*)x54_heap);
    x54_heap = nullptr;
    x4_heapSize = 0;
}

void* CGameAllocator::Alloc(size_t size, EHint hint, EScope scope, EType type, const CCallStack& cs)
{
    SGameMemInfo* ret = nullptr;
    u32 roundedSize = ALLOC_ROUNDUP(size);
    switch(hint)
    {
        case HintNone:
        {
            if (x60_smallAllocPool && size <= 0x38)
                ret = x60_smallAllocPool->Alloc(size);

            if (ret)
                break;
        }
        case HintLarge:
        {
            if (x70_mediumAllocPool && size <= 0x400)
                ret = x70_mediumAllocPool->Alloc(size);

            if (ret)
                break;
        }
        /*case HintTopOfHeap:
            ret = FindFreeBlockFromTopOfHeap(roundedSize);*/
            break;
    }

    if (!ret)
        ret = FindFreeBlock(size);

    if (!ret)
    {
        AllocLog.report(LogVisor::FatalError, "Unable to acquire memory!");
        return nullptr;
    }

    const char* fileAndLine = cs.GetFileAndLineText();
    ret->x8_fileAndLine = &fileAndLine;
    const char* typeText = cs.GetTypeText();
    ret->xc_type = &typeText;
    u32 offset = FixupAllocPtrs(ret, size, roundedSize, hint, cs);

    return (void*)((u8*)(ret) + offset);
}

void CGameAllocator::Free(void* buf)
{
}

void CGameAllocator::ReleaseAll()
{
    if (x70_mediumAllocPool)
    {
        x70_mediumAllocPool->ClearPuddles();
        FreeNormalAllocation((void*)x70_mediumAllocPool);
        x70_mediumAllocPool = nullptr;
    }

    SGameMemInfo* node = xc_infoTail;
    while (node)
    {
        u8* prev = (u8*)node->x10_prev;
        if (prev)
        {
            prev += 32; //sizeof(SGameMemInfo);
            FreeNormalAllocation((void*)prev);
        }

        node = node->x14_next;
    }

    xc_infoTail = nullptr;
}

void* CGameAllocator::AllocSecondary(size_t size, EHint hint, EScope scope, EType type, const CCallStack& callstack)
{
    return Alloc(size, hint, scope, type, callstack);
}

void CGameAllocator::FreeSecondary(void* ptr)
{
    Free(ptr);
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

void CGameAllocator::OffsetFakeStatics(int offset)
{
    xbc_fakeStaticOff += offset;
}

CGameAllocator::SMetrics CGameAllocator::GetMetrics() const
{
}

}
