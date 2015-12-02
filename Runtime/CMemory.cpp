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

void CMemory::Startup()
{
    g_memoryAllocatorReady = g_memoryAllocator->Initialize();
}

void CMemory::Shutdown()
{
    g_memoryAllocator->Shutdown();
    g_memoryAllocatorReady = false;
}

void CMemory::SetAllocator(IAllocator& alloc)
{
    if (&alloc != g_memoryAllocator)
    {
        g_memoryAllocator = &alloc;
        g_memoryAllocator->Initialize();
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

CMemorySys::CMemorySys(IAllocator& alloc)
{
    CMemory::Startup();
    CMemory::SetAllocator(alloc);
}

CMemorySys::~CMemorySys()
{
    CMemory::Shutdown();
}

IAllocator& CMemorySys::GetGameAllocator() {return g_gameAllocator;}

}

void* operator new(std::size_t sz)
{
    if (!Retro::g_memoryAllocatorReady)
        return malloc(sz);
    Retro::CCallStack cs("?\?(?\?)", "UnknownType");
    return Retro::CMemory::Alloc(sz,
                                 Retro::IAllocator::EHint::None,
                                 Retro::IAllocator::EScope::Default,
                                 Retro::IAllocator::EType::Primitive,
                                 cs);
}

void* operator new(std::size_t sz,
                   const char* funcName, const char* typeName)
{
    if (!Retro::g_memoryAllocatorReady)
        return malloc(sz);
    Retro::CCallStack cs(funcName, typeName);
    return Retro::CMemory::Alloc(sz,
                                 Retro::IAllocator::EHint::None,
                                 Retro::IAllocator::EScope::Default,
                                 Retro::IAllocator::EType::Primitive,
                                 cs);
}

void operator delete(void* ptr) noexcept
{
    if (!Retro::g_memoryAllocatorReady)
    {
        free(ptr);
        return;
    }
    Retro::CMemory::Free(ptr);
}

void* operator new[](std::size_t sz)
{
    if (!Retro::g_memoryAllocatorReady)
        return malloc(sz);
    Retro::CCallStack cs("?\?(?\?)", "UnknownType");
    return Retro::CMemory::Alloc(sz,
                                 Retro::IAllocator::EHint::None,
                                 Retro::IAllocator::EScope::Default,
                                 Retro::IAllocator::EType::Array,
                                 cs);
}

void* operator new[](std::size_t sz,
                     const char* funcName, const char* typeName)
{
    if (!Retro::g_memoryAllocatorReady)
        return malloc(sz);
    Retro::CCallStack cs(funcName, typeName);
    return Retro::CMemory::Alloc(sz,
                                 Retro::IAllocator::EHint::None,
                                 Retro::IAllocator::EScope::Default,
                                 Retro::IAllocator::EType::Array,
                                 cs);
}

void operator delete[](void* ptr) noexcept
{
    if (!Retro::g_memoryAllocatorReady)
    {
        free(ptr);
        return;
    }
    Retro::CMemory::Free(ptr);
}
