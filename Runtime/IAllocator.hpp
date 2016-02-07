#ifndef __PSHAG_IALLOCATOR_HPP__
#define __PSHAG_IALLOCATOR_HPP__

#include "RetroTypes.hpp"

namespace pshag
{
class COsContext;
class CCallStack;

class IAllocator
{
public:
    virtual ~IAllocator() {}
    enum class EHint
    {
        None = 0x0,
        TopOfHeap = 0x1,
        Large = 0x2
    };
    enum class EScope
    {
        None = 0,
        Default = 1
    };
    enum class EType
    {
        Primitive = 0,
        Array = 1
    };
    struct SAllocInfo
    {
        void* x0_memInfo;
        u32   x4_size;
        u8    x8_unk1; // Retro puts memInfo's prev pointer here o.o
        u8    x9_unk2; // this is always set to 0
        const char**    xc_fileAndLine;
        const char**    x10_type;
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
    virtual bool Free(void*)=0;
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

#endif // __PSHAG_IALLOCATOR_HPP__
