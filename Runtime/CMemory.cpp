#include "CMemory.hpp"
#include "CGameAllocator.hpp"

namespace Retro
{

static CGameAllocator GAME_ALLOCATOR;
static IAllocator* MEMORY_ALLOCATOR = &GAME_ALLOCATOR;
static bool MEMORY_ALLOCATOR_READY = false;

void CMemory::Startup(COsContext& cos)
{
    MEMORY_ALLOCATOR_READY = MEMORY_ALLOCATOR->Initialize(cos);
}

void CMemory::Shutdown()
{
    MEMORY_ALLOCATOR->Shutdown();
    MEMORY_ALLOCATOR_READY = false;
}

void CMemory::SetAllocator(COsContext& cos, IAllocator& alloc)
{
    if (&alloc != MEMORY_ALLOCATOR)
    {
        MEMORY_ALLOCATOR = &alloc;
        MEMORY_ALLOCATOR->Initialize(cos);
    }
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

IAllocator& CMemorySys::GetGameAllocator() {return GAME_ALLOCATOR;}

}
