#ifndef __PSHAG_CGAMEALLOCATOR_HPP__
#define __PSHAG_CGAMEALLOCATOR_HPP__

#include "IAllocator.hpp"

namespace urde
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
        size_t        x4_size;
        const char**  x8_fileAndLine;
        const char**  xc_type;
        SGameMemInfo* x10_prev;
        SGameMemInfo* x14_next;
        SGameMemInfo* x18_ctx;
        char          x1c_canary[0]; // don't eat this
    };

private:
    bool                 x4_isInitialized    = false; // The game stores a byte here
    u32                  x8_heapSize         = 0;
    SGameMemInfo*        xc_infoHead         = nullptr;
    SGameMemInfo*        x10_infoTail        = nullptr;
    SGameMemInfo*        x14_bins[0x10]         = {nullptr};
    void*                x54_heap               = nullptr;
    TOutOfMemoryCallback x58_oomCb              = nullptr;
    void*                x5c_oomCtx             = nullptr;
    CSmallAllocPool*     x60_smallAllocPool     = nullptr;
    void*                x64_smallAllocMainData = nullptr;
    void*                x68_smallAllocBookKeeping = nullptr;
    u32                  x6c_unknown         = 0;
    u32                  x70_unknown          = 0;
    CMediumAllocPool*    x74_mediumAllocPool  = nullptr;
    void*                x78_mediumAllocMainData= 0;
    u32                  x7c_unknown         = 0;
    u32                  x80_unknown         = 0;
    u32                  x84_unknown         = 0;
    u32                  x88_unknown         = 0;
    u32                  x8c_unknown         = 0;
    u32                  x90_unknown         = 0;
    u32                  x94_unknown         = 0;
    u32                  x98_unknown         = 0;
    u32                  x9c_unknown         = 0;
    u32                  xa0_unknown         = 0;
    u32                  xa4_unknown         = 0;
    u32                  xa8_unknown         = 0;
    u32                  xac_unknown         = 0;
    u32                  xb0_unknown         = 0;
    u32                  xb4_unknown         = 0;
    u32                  xb8_fakeStaticOff   = 0;
    u32                  xbc_unknown         = 0;
public:
    SGameMemInfo* FindFreeBlock(u32);
    SGameMemInfo* FindFreeBlockFromTopOfHeap(u32);
    u32 FixupAllocPtrs(SGameMemInfo*, size_t, size_t, EHint, const CCallStack&);
    void UpdateAllocDebugStats(u32, u32, u32);
    bool FreeNormalAllocation(void* ptr);
    static u32 GetFreeBinEntryForSize(size_t);
    void AddFreeEntryToFreeList(SGameMemInfo*);
    void RemoveFreeEntryFromFreeList(SGameMemInfo*);
    u32 DumpAllocations() const;
    u32 GetLargestFreeChunk() const;
    SGameMemInfo* GetMemInfoFromBlockPtr(void* ptr) const;

    bool Initialize();
    void Shutdown();
    void* Alloc(size_t, EHint, EScope, EType, const CCallStack&);
    bool Free(void*);
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
    void* xc_unknown;
    u32   x10_initial1;
    u32   x14_initial2;
    u32   x18_unkSize;
    void* x1c_unkPtr;

public:
    CSmallAllocPool(u32 size, void* begin, void* end)
        : x8_size(size), x0_poolStart(begin), x4_poolEnd(end),
          xc_unknown(nullptr), x10_initial1((u32)~0UL), x14_initial2((u32)~0UL),
          x18_unkSize(size), x1c_unkPtr(nullptr)
    { memset(x0_poolStart, 0, (x8_size >> 1)); }

    CGameAllocator::SGameMemInfo* Alloc(u32 size)
    {
        u32 r3 = 1;
        if (size < 4)
        {
            u32 mask = (1 << 31);
            if (r3 )
            {

            }
        }
        return nullptr;
    }

    void* FindFree(int size)
    {
        return nullptr;
    }

    bool  Free(const void* buf)
    {
        return false;
    }
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
    bool Free(void* ptr) { return false; }
    void* FindFree();
};
}

#endif // __PSHAG_CGAMEALLOCATOR_HPP__
