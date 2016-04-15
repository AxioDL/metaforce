#ifndef __URDE_CMEMORY_HPP__
#define __URDE_CMEMORY_HPP__

#if 0

#include "IAllocator.hpp"

namespace urde
{
class COsContext;

class CMemory
{
public:
    static void Startup();
    static void Shutdown();
    static void SetAllocator(IAllocator&);
    static void OffsetFakeStatics(int);
    static void SetOutOfMemoryCallback(const IAllocator::TOutOfMemoryCallback, void*);
    static void Free(void*);
    static void* Alloc(size_t, IAllocator::EHint, IAllocator::EScope, IAllocator::EType, const CCallStack&);
};

class CMemorySys
{
public:
    CMemorySys(IAllocator&);
    ~CMemorySys();
    static IAllocator& GetGameAllocator();
};

}

/* Custom new funcs */
void* operator new(std::size_t sz, const char* funcName, const char* typeName);
void* operator new[](std::size_t sz, const char* funcName, const char* typeName);

#endif

#endif // __URDE_CMEMORY_HPP__
