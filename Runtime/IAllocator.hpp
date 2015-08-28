#ifndef __RETRO_IALLOCATOR_HPP__
#define __RETRO_IALLOCATOR_HPP__

#include "RetroTypes.hpp"

namespace Retro
{
class COsContext;
class CCallStack;

class IAllocator
{
public:
    virtual ~IAllocator() {}
    enum EHint
    {
        HintNone = 0x0,
        HintTopOfHeap = 0x1,
        HintLarge = 0x2
    };
    enum EScope
    {
        ScopeNone = 0,
        ScopeDefault = 1
    };
    enum EType
    {
        TypePrimitive = 0,
        TypeArray = 1
    };
    struct SAllocInfo
    {
    };
    struct SMetrics
    {
        u32 a;
        u32 b;
        u32 c;
        u32 d;
        u32 e;
        u32 f;
        u32 g;
        u32 h;
        u32 i;
        u32 j;
        u32 k;
        u32 l;
        u32 m;
        u32 n;
        u32 o;
        u32 p;
        u32 q;
        u32 r;
        u32 s;
        u32 t;
        u32 u;
        u32 v;
    };
    typedef bool(*TOutOfMemoryCallback)(void*, u32);
    typedef bool(*TAllocationVisitCallback)(const SAllocInfo&, void*);

    virtual bool Initialize()=0;
    virtual void Shutdown()=0;
    virtual void* Alloc(size_t, EHint, EScope, EType, const CCallStack&)=0;
    virtual void Free(void*)=0;
    virtual void ReleaseAll()=0;
    virtual void* AllocSecondary(size_t, EHint, EScope, EType, const CCallStack&)=0;
    virtual void FreeSecondary(void*)=0;
    virtual void ReleaseAllSecondary()=0;
    virtual void SetOutOfMemoryCallback(const TOutOfMemoryCallback, void*)=0;
    virtual int EnumAllocations(const TAllocationVisitCallback, void*, bool) const=0;
    virtual SAllocInfo GetAllocInfo(void*) const=0;
    virtual void OffsetFakeStatics(int)=0;
    virtual SMetrics GetMetrics() const=0;
};

}

#endif // __RETRO_IALLOCATOR_HPP__
