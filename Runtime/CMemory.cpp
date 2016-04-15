#if 0

#include "CMemory.hpp"
#include "CGameAllocator.hpp"
#include "CCallStack.hpp"
#include "logvisor/logvisor.hpp"

namespace urde
{

static logvisor::Module Log("CMemory");

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
        Log.report(logvisor::Error, "Alloc Failed! - Size %d", sz);
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
    if (!urde::g_memoryAllocatorReady)
        return malloc(sz);
    urde::CCallStack cs("?\?(?\?)", "UnknownType");
    return urde::CMemory::Alloc(sz,
                                 urde::IAllocator::EHint::None,
                                 urde::IAllocator::EScope::Default,
                                 urde::IAllocator::EType::Primitive,
                                 cs);
}

void* operator new(std::size_t sz,
                   const char* funcName, const char* typeName)
{
    if (!urde::g_memoryAllocatorReady)
        return malloc(sz);
    urde::CCallStack cs(funcName, typeName);
    return urde::CMemory::Alloc(sz,
                                 urde::IAllocator::EHint::None,
                                 urde::IAllocator::EScope::Default,
                                 urde::IAllocator::EType::Primitive,
                                 cs);
}

void operator delete(void* ptr) noexcept
{
    if (!urde::g_memoryAllocatorReady)
    {
        free(ptr);
        return;
    }
    urde::CMemory::Free(ptr);
}

void* operator new[](std::size_t sz)
{
    if (!urde::g_memoryAllocatorReady)
        return malloc(sz);
    urde::CCallStack cs("?\?(?\?)", "UnknownType");
    return urde::CMemory::Alloc(sz,
                                 urde::IAllocator::EHint::None,
                                 urde::IAllocator::EScope::Default,
                                 urde::IAllocator::EType::Array,
                                 cs);
}

void* operator new[](std::size_t sz,
                     const char* funcName, const char* typeName)
{
    if (!urde::g_memoryAllocatorReady)
        return malloc(sz);
    urde::CCallStack cs(funcName, typeName);
    return urde::CMemory::Alloc(sz,
                                 urde::IAllocator::EHint::None,
                                 urde::IAllocator::EScope::Default,
                                 urde::IAllocator::EType::Array,
                                 cs);
}

void operator delete[](void* ptr) noexcept
{
    if (!urde::g_memoryAllocatorReady)
    {
        free(ptr);
        return;
    }
    urde::CMemory::Free(ptr);
}

#endif
