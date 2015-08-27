#ifndef __RETRO_CGAMEALLOCATOR_HPP__
#define __RETRO_CGAMEALLOCATOR_HPP__

#include "IAllocator.hpp"

namespace Retro
{
class CCallStack;
class CSmallAllocPool;
class CMediumAllocPool;

class CGameAllocator : public IAllocator
{
public:
    static const u32 skSentinel = 0xEFEFEFEF;
    static const u32 skCanary   = 0xEAEAEAEA;
    static const u32 skMinBlockSize;
    static const u32 skMaxBlockSize;
    static const char* skMemHead;
    static const char* skMemTail;
    struct alignas(32) SGameMemInfo
    {
        u32           x0_sentinel;
        u32           x4_size;
        const char**  x8_fileAndLine;
        const char**  xc_type;
        SGameMemInfo* x10_prev;
        SGameMemInfo* x14_next;
        SGameMemInfo* x18_ctx;
        char          x1c_canary[0]; // don't eat this
    };

private:
    u32           x4_heapSize;
    SGameMemInfo* x8_infoHead = nullptr;
    SGameMemInfo* xc_infoTail = nullptr;
    SGameMemInfo* x10_bins[0x10] ={nullptr};
    TOutOfMemoryCallback x58_oomCb = nullptr;
    void* x5c_oomCtx = nullptr;
    CSmallAllocPool*  x60_smallAllocPool;
    CMediumAllocPool* x70_mediumAllocPool;
    u32 xbc_fakeStaticOff = 0;
public:
    CGameAllocator();
    SGameMemInfo* FindFreeBlock(u32);
    SGameMemInfo* FindFreeBlockFromTopOfHeap(u32);
    u32 FixupAllocPtrs(SGameMemInfo*, u32, u32, EHint, const CCallStack&);
    void UpdateAllocDebugStats(u32, u32, u32);
    void FreeNormalAllocation(void* ptr);
    u32 GetFreeBinEntryForSize(u32);
    void AddFreeEntryToFreeList(SGameMemInfo*);
    void RemoveFreeEntryFromFreeList(SGameMemInfo*);
    void DumpAllocations() const;
    u32 GetLargestFreeChunk() const;
    SGameMemInfo* GetMemInfoFromBlockPtr(void* ptr);

    bool Initialize();
    void Shutdown();
    void* Alloc(size_t, EHint, EScope, EType, const CCallStack&);
    void Free(void*);
    void ReleaseAll();
    void* AllocSecondary(size_t, EHint, EScope, EType, const CCallStack&);
    void FreeSecondary(void*);
    void ReleaseAllSecondary();
    void SetOutOfMemoryCallback(const TOutOfMemoryCallback, void*);
    int EnumAllocations(const TAllocationVisitCallback, void*, bool) const;
    SAllocInfo GetAllocInfo(void*) const;
    void OffsetFakeStatics(int);
    SMetrics GetMetrics() const;
};

class CSmallAllocPool
{
    void* x0_poolStart;
    void* x4_poolEnd;
    u32   x8_size;
    u32   xc_unknown;
    u32   x10_initial1;
    u32   x14_initial2;
public:
    CSmallAllocPool(u32 size, void* begin, void* end)
        : x8_size(size), x0_poolStart(begin), x4_poolEnd(end),
          xc_unknown(0), x10_initial1((u32)~0UL), x14_initial2((u32)~0UL)
    {
        u32 tmpSize = (x8_size >> 1);
        memset(x0_poolStart, 0, tmpSize);
    }

    CGameAllocator::SGameMemInfo* Alloc(u32 size) { return nullptr; }
    void  Free(const void* buf);
};

struct SMediumAllocPuddle
{
};

class CMediumAllocPool : public std::list<SMediumAllocPuddle>
{
private:
    SMediumAllocPuddle* x4_puddleStart;
    SMediumAllocPuddle* x8_puddleEnd;
public:
    u32 GetNumBlocksAvailable();
    u32 GetTotalEntries();
    u32 GetNumAllocs();
    void AddPuddle(u32 size, void* start, void* end, u32) {}
    void ClearPuddles() {}
    CGameAllocator::SGameMemInfo* Alloc(u32 size) { return nullptr; }
    void Free(void* ptr) {}
    void* FindFree();
};
}

#endif // __RETRO_CGAMEALLOCATOR_HPP__
