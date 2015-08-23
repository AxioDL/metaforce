#ifndef __RETRO_CMEMORY_HPP__
#define __RETRO_CMEMORY_HPP__

#include "IAllocator.hpp"

namespace Retro
{
class COsContext;

class CMemory
{
public:
    static void Startup(COsContext&);
    static void Shutdown();
    static void SetAllocator(COsContext&, IAllocator&);
    static void OffsetFakeStatics(int);
    static void SetOutOfMemoryCallback(const IAllocator::TOutOfMemoryCallback, void*);
    static void Free(void*);
    static void* Alloc(u32, IAllocator::EHint, IAllocator::EScope, IAllocator::EType, const CCallStack&);
};

class CMemorySys
{
public:
    CMemorySys(COsContext&, IAllocator&);
    ~CMemorySys();
    static IAllocator& GetGameAllocator();
};

}

#endif // __RETRO_CMEMORY_HPP__
