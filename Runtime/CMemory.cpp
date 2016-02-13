#include "CMemory.hpp"
#include "CGameAllocator.hpp"
#include "CCallStack.hpp"
#include <LogVisor/LogVisor.hpp>

namespace pshag
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
    if (!pshag::g_memoryAllocatorReady)
        return malloc(sz);
    pshag::CCallStack cs("?\?(?\?)", "UnknownType");
    return pshag::CMemory::Alloc(sz,
                                 pshag::IAllocator::EHint::None,
                                 pshag::IAllocator::EScope::Default,
                                 pshag::IAllocator::EType::Primitive,
                                 cs);
}

void* operator new(std::size_t sz,
                   const char* funcName, const char* typeName)
{
    if (!pshag::g_memoryAllocatorReady)
        return malloc(sz);
    pshag::CCallStack cs(funcName, typeName);
    return pshag::CMemory::Alloc(sz,
                                 pshag::IAllocator::EHint::None,
                                 pshag::IAllocator::EScope::Default,
                                 pshag::IAllocator::EType::Primitive,
                                 cs);
}

void operator delete(void* ptr) noexcept
{
    if (!pshag::g_memoryAllocatorReady)
    {
        free(ptr);
        return;
    }
    pshag::CMemory::Free(ptr);
}

void* operator new[](std::size_t sz)
{
    if (!pshag::g_memoryAllocatorReady)
        return malloc(sz);
    pshag::CCallStack cs("?\?(?\?)", "UnknownType");
    return pshag::CMemory::Alloc(sz,
                                 pshag::IAllocator::EHint::None,
                                 pshag::IAllocator::EScope::Default,
                                 pshag::IAllocator::EType::Array,
                                 cs);
}

void* operator new[](std::size_t sz,
                     const char* funcName, const char* typeName)
{
    if (!pshag::g_memoryAllocatorReady)
        return malloc(sz);
    pshag::CCallStack cs(funcName, typeName);
    return pshag::CMemory::Alloc(sz,
                                 pshag::IAllocator::EHint::None,
                                 pshag::IAllocator::EScope::Default,
                                 pshag::IAllocator::EType::Array,
                                 cs);
}

void operator delete[](void* ptr) noexcept
{
    if (!pshag::g_memoryAllocatorReady)
    {
        free(ptr);
        return;
    }
    pshag::CMemory::Free(ptr);
}
