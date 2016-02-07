#include "CGameAllocator.hpp"
#include "CCallStack.hpp"
#include <new>
#include <cstddef>

#define ALLOC_ROUNDUP(x) (((x) + (32 - 1)) & ~(32 - 1))
#define ALLOC_ROUNDDOWN(x) ((x) & ~(32 - 1))

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define ROTATE_RIGHT(x, n) (((x) >> (n)) | ((32-(n))) << (x))

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
    SGameMemInfo** binsStart = &x14_bins[binIdx];
    SGameMemInfo* ret = nullptr;
    SGameMemInfo* prev = nullptr;
    u32 prevIdx = 0;

    u32 blockSize = 0x1000;

    while (!ret)
    {
        if (binIdx >= 0x10)
            break;
        SGameMemInfo* curBin = *binsStart;
        SGameMemInfo* prevBin = nullptr;
        while (curBin)
        {
            u32 blockDif = (curBin->x4_size - size);
            if (curBin->x10_prev
                    || curBin->x4_size < size
                    || blockSize >= blockDif
                    || !curBin->x14_next)
            {
                prevBin = curBin;
                curBin = curBin->x18_ctx;
                continue;
            }

            blockSize = blockDif;
            ret = curBin;
            prev = prevBin;
            prevIdx = binIdx;

            if (blockDif < 0x20)
                break;
        }
        binIdx++;
        binsStart++;
    }

    if (!ret)
        return nullptr;
    if (!prev)
        return ret;

    prev->x18_ctx = ret->x18_ctx;
    return ret;
}

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlockFromTopOfHeap(u32 size)
{
    SGameMemInfo* bin = x10_infoTail;
    SGameMemInfo* ret = nullptr;
    while (bin)
    {
        SGameMemInfo* tmp = bin->x10_prev;
        if (tmp && tmp->x4_size >= size)
        {
            ret = tmp;
            break;
        }

        bin = tmp;
    }

    RemoveFreeEntryFromFreeList(ret);
    return ret;
}

u32 CGameAllocator::FixupAllocPtrs(SGameMemInfo* memInfo, u32 size, u32 roundedSize, EHint hint, const CCallStack& cs)
{
    return 0;
}

void CGameAllocator::UpdateAllocDebugStats(u32, u32, u32)
{
}

bool CGameAllocator::FreeNormalAllocation(void* ptr)
{
    SGameMemInfo* block = GetMemInfoFromBlockPtr(ptr);
    SGameMemInfo* next = block->x14_next;
    SGameMemInfo* prev = block->x10_prev;
    SGameMemInfo* tmp = block;

    u32 offset = 0;
    u32 oldSize = block->x4_size;
    u32 newSize = 0;

    if (block->x14_next)
        newSize = (block->x14_next - block) - 0x20;

    block->x4_size = newSize;
    if (prev && prev->x10_prev)
    {
        RemoveFreeEntryFromFreeList(prev);

        prev->x14_next = reinterpret_cast<SGameMemInfo*>((size_t)next | (((size_t)prev->x14_next) & 0x1F)); // replace with sizeof(SGameMemInfo) - 1;
        if (next)
            next->x10_prev = reinterpret_cast<SGameMemInfo*>((size_t)prev | (((size_t)next->x10_prev) & 0x1F)); // replace with sizeof(SGameMemInfo) - 1;

        prev->x4_size += tmp->x4_size + 0x20;
        tmp = prev;
    }

    if (next && next->x14_next)
    {
        RemoveFreeEntryFromFreeList(next);

        tmp->x14_next = reinterpret_cast<SGameMemInfo*>((size_t)next->x14_next | (((size_t)tmp->x14_next) & 0x1F));
        SGameMemInfo* tmpNext = tmp->x14_next;
        if (tmpNext)
            tmpNext->x10_prev = reinterpret_cast<SGameMemInfo*>((size_t)tmpNext->x10_prev | (((size_t)tmpNext->x10_prev) & 0x1F));

        offset += 0x20;
        tmp->x4_size += next->x4_size + 0x20;
    }


    AddFreeEntryToFreeList(tmp);

    x84_unknown--;
    x88_unknown = oldSize - x88_unknown;
    x8c_unknown -= newSize + offset;
    x90_unknown += newSize + offset;
    if (oldSize <= 0x38)
        xa8_unknown--;

    return true;
}

u32 CGameAllocator::GetFreeBinEntryForSize(u32 size)
{
    u32 curBlock = skMinBlockSize;
    u32 bin = 0;

    while (curBlock < skMaxBlockSize)
    {
        if (size > curBlock)
        {
            curBlock *= 2;
            bin++;
        }
        else
            return bin;
    }

    return 0xF;
}

void CGameAllocator::AddFreeEntryToFreeList(SGameMemInfo* memInfo)
{
    u32 binIdx = GetFreeBinEntryForSize(memInfo->x4_size);
    SGameMemInfo*& bin = x14_bins[binIdx];
    memInfo->x18_ctx = bin;
    bin = memInfo;
}

void CGameAllocator::RemoveFreeEntryFromFreeList(SGameMemInfo* memInfo)
{
    SGameMemInfo*& bin = x14_bins[GetFreeBinEntryForSize(memInfo->x4_size)];
    SGameMemInfo* ctx = nullptr;

    while (bin)
    {
        if (bin != memInfo)
        {
            ctx = bin;
            bin = bin->x18_ctx;
            continue;
        }
        if (ctx)
        {
            ctx->x18_ctx = bin->x18_ctx;
            break;
        }

        bin = bin->x18_ctx;
        break;
    }
}

u32 CGameAllocator::DumpAllocations() const
{
    u32 ret = GetLargestFreeChunk();
    SGameMemInfo* node = xc_infoHead;
    int i = 0;
    while (node)
    {
        i++;
        int tmp = (ret << 30);
        ret = (ret >> 31);
        tmp = ret - tmp;
        tmp = ROTATE_LEFT(tmp, 2);
        tmp += ret;
        if (tmp > 0)
            ; // game waits .005 seconds
        node = node->x14_next;
    }

    return ret;
}

u32 CGameAllocator::GetLargestFreeChunk() const
{
    SGameMemInfo* memInfo = x10_infoTail;
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
    return reinterpret_cast<SGameMemInfo*>(reinterpret_cast<u8*>(ptr) - 32 /*sizeof(SGameMemInfo)*/);
}

bool CGameAllocator::Initialize()
{
    x8_heapSize = (0x0119d2c0);
    x8_heapSize &= 0xFFFFFF80;
    x54_heap = (u8*)_mm_malloc(x8_heapSize, sizeof(SGameMemInfo));
    if (!x54_heap)
    {
        AllocLog.report(LogVisor::FatalError, _S("Failed allocate memory, unable to continue!"));
        return false;
    }

    /* Initialize head */
    xc_infoHead = reinterpret_cast<SGameMemInfo*>(x54_heap);
    xc_infoHead->x0_sentinel = skSentinel;
    xc_infoHead->x4_size = 0x0116d280;
    xc_infoHead->x8_fileAndLine = &skMemHead;
    xc_infoHead->xc_type = &skMemHead;
    xc_infoHead->x10_prev = nullptr;
    xc_infoHead->x14_next = reinterpret_cast<SGameMemInfo*>(reinterpret_cast<u8*>(x54_heap) + x8_heapSize - sizeof(SGameMemInfo));
    xc_infoHead->x18_ctx = xc_infoHead->x14_next;
    /* Initialize tail */
    memset(xc_infoHead->x1c_canary, skCanary, sizeof(SGameMemInfo) - offsetof(SGameMemInfo, x1c_canary));
    x10_infoTail = xc_infoHead->x14_next;
    x10_infoTail->x0_sentinel = skSentinel;
    x10_infoTail->x4_size = 0;
    x10_infoTail->x8_fileAndLine = &skMemTail;
    x10_infoTail->xc_type = &skMemTail;
    x10_infoTail->x10_prev = xc_infoHead;
    x10_infoTail->x14_next = nullptr;
    x10_infoTail->x18_ctx = nullptr;
    memset(x10_infoTail->x1c_canary, skCanary, sizeof(SGameMemInfo) - offsetof(SGameMemInfo, x1c_canary));

    x14_bins[0]  = nullptr; x14_bins[1]  = nullptr; x14_bins[2]  = nullptr; x14_bins[3]  = nullptr;
    x14_bins[4]  = nullptr; x14_bins[5]  = nullptr; x14_bins[6]  = nullptr; x14_bins[7]  = nullptr;
    x14_bins[8]  = nullptr; x14_bins[9]  = nullptr; x14_bins[10] = nullptr; x14_bins[11] = nullptr;
    x14_bins[12] = nullptr; x14_bins[13] = nullptr; x14_bins[14] = nullptr; x14_bins[15] = nullptr;

    /* Add head to our list of free bins */
    AddFreeEntryToFreeList(xc_infoHead);

    /* TODO: Figure out what's going on here */
    x80_unknown = 0;
    x88_unknown = 0;
    x8c_unknown = 0;
    x90_unknown = x8_heapSize;
    x94_unknown = 0;
    x98_unknown = 0;
    x9c_unknown = 0;
    xa0_unknown = 0;
    xa4_unknown = 0;
    xa8_unknown = 0;
    x4_isInitialized = 1;

    /* Report our allocation information */
    AllocLog.report(LogVisor::Info, _S("TotalMem: %d Head: %p Tail: %p"), x8_heapSize, xc_infoHead, x10_infoTail);

    /* Allocate memory for CSmallAllocPool */
    x64_smallAllocMainData    = Alloc(0xB,
                                      EHint::None, EScope::Default, EType::Primitive,
                                      CCallStack("SmallAllocMainData   ", " - Ignore"));
    x68_smallAllocBookKeeping = Alloc(0x16000,
                                      EHint::None, EScope::Default, EType::Primitive,
                                      CCallStack("SmallAllocBookKeeping", " - Ignore"));
    void* smallAlloc          = Alloc(sizeof(CSmallAllocPool),
                                      EHint::None, EScope::Default, EType::Primitive,
                                      CCallStack("SmallAllocClass      ", " - Ignore"));
    if (smallAlloc)
        x60_smallAllocPool = new(smallAlloc) CSmallAllocPool(0x2C000, x64_smallAllocMainData, x68_smallAllocBookKeeping);
    else
        x60_smallAllocPool = nullptr;

    /* Allocate memory for CMediumAllocPool */
    void* mediumAlloc = Alloc(sizeof(CMediumAllocPool),
                              EHint::None, EScope::Default, EType::Primitive,
                              CCallStack("MediumAllocClass     ", " - Ignore"));

    if (mediumAlloc)
        x74_mediumAllocPool = new(mediumAlloc) CMediumAllocPool();
    else
        x74_mediumAllocPool = nullptr;

    x78_mediumAllocMainData = Alloc(0x21000,
                                     EHint::None, EScope::Default, EType::Primitive,
                                     CCallStack("MediumAllocMainData  ", " - Ignore"));
    x84_unknown -= 4;
    xbc_fakeStaticOff = 0xC6000;
    return true;
}

void CGameAllocator::Shutdown()
{
    ReleaseAll();
    x54_heap = nullptr;
    x4_isInitialized = false;
}

void* CGameAllocator::Alloc(size_t size, EHint hint, EScope scope, EType type, const CCallStack& cs)
{
    u32 r27 = size;
    EHint r28 = hint;
    EScope r26 = scope;
    EType r25 = type;
    CCallStack r29 = cs;

    return nullptr;
}

bool CGameAllocator::Free(void* buf)
{
    if (buf)
    {
        if (x60_smallAllocPool && x60_smallAllocPool->Free(buf))
            return true;
        if (x74_mediumAllocPool && x74_mediumAllocPool->Free(buf))
            return true;

        return FreeNormalAllocation(buf);
    }

    return false;
}

void CGameAllocator::ReleaseAll()
{
    if (x74_mediumAllocPool)
    {
        x74_mediumAllocPool->ClearPuddles();
        FreeNormalAllocation((void*)x74_mediumAllocPool);
        x74_mediumAllocPool = nullptr;
    }

    SGameMemInfo* node = xc_infoHead;
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

    xc_infoHead = nullptr;
    x10_infoTail = nullptr;
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
    return 0;
}

CGameAllocator::SAllocInfo CGameAllocator::GetAllocInfo(void*) const
{
    return SAllocInfo();
}

void CGameAllocator::OffsetFakeStatics(int offset)
{
    xbc_fakeStaticOff += offset;
}

CGameAllocator::SMetrics CGameAllocator::GetMetrics() const
{
    return SMetrics();
}

}

