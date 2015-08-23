#include "CMemory.hpp"
#include "CGameAllocator.hpp"
#include "CCallStack.hpp"
#include <LogVisor/LogVisor.hpp>

namespace Retro
{

LogVisor::LogModule Log("CMemory");

static CGameAllocator g_gameAllocator;
static IAllocator* g_memoryAllocator = &g_gameAllocator;
static bool g_memoryAllocatorReady = false;

void CMemory::Startup(COsContext& cos)
{
    g_memoryAllocatorReady = g_memoryAllocator->Initialize(cos);
}

void CMemory::Shutdown()
{
    g_memoryAllocator->Shutdown();
    g_memoryAllocatorReady = false;
}

void CMemory::SetAllocator(COsContext& cos, IAllocator& alloc)
{
    if (&alloc != g_memoryAllocator)
    {
        g_memoryAllocator = &alloc;
        g_memoryAllocator->Initialize(cos);
    }
}

void CMemory::OffsetFakeStatics(int val)
{
    g_memoryAllocator->OffsetFakeStatics(val);
}

void CMemory::SetOutOfMemoryCallback(const IAllocator::TOutOfMemoryCallback cb, void* ctx)
{
    g_memoryAllocator->SetOutOfMemoryCallback(cb, ctx);
}

void CMemory::Free(void* ptr)
{
    g_memoryAllocator->Free(ptr);
}

void* CMemory::Alloc(size_t sz, IAllocator::EHint hint, IAllocator::EScope scope,
                     IAllocator::EType type, const CCallStack& cs)
{
    void* newPtr = g_memoryAllocator->Alloc(sz, hint, scope, type, cs);
    if (!newPtr)
        Log.report(LogVisor::Error, "Alloc Failed! - Size %d", sz);
    return newPtr;
}

CMemorySys::CMemorySys(COsContext& cos, IAllocator& alloc)
{
    CMemory::Startup(cos);
    CMemory::SetAllocator(cos, alloc);
}

CMemorySys::~CMemorySys()
{
    CMemory::Shutdown();
}

IAllocator& CMemorySys::GetGameAllocator() {return g_gameAllocator;}

}

void* operator new(std::size_t)
{
    extern void *bare_new_erroneously_called();
    return bare_new_erroneously_called();
}

void* operator new(std::size_t sz,
                   const char* funcName, const char* typeName)
{
    Retro::CCallStack cs(funcName, typeName);
    return Retro::CMemory::Alloc(sz,
                                 Retro::IAllocator::HintNone,
                                 Retro::IAllocator::ScopeDefault,
                                 Retro::IAllocator::TypePrimitive,
                                 cs);
}

void operator delete(void* ptr) noexcept
{
    Retro::CMemory::Free(ptr);
}

void* operator new[](std::size_t)
{
    extern void *bare_new_array_erroneously_called();
    return bare_new_array_erroneously_called();
}

void* operator new[](std::size_t sz,
                     const char* funcName, const char* typeName)
{
    Retro::CCallStack cs(funcName, typeName);
    return Retro::CMemory::Alloc(sz,
                                 Retro::IAllocator::HintNone,
                                 Retro::IAllocator::ScopeDefault,
                                 Retro::IAllocator::TypeArray,
                                 cs);
}

void operator delete[](void* ptr) noexcept
{
    Retro::CMemory::Free(ptr);
}
