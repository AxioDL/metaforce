#ifndef __URDE_CGAMEALLOCATOR_HPP__
#define __URDE_CGAMEALLOCATOR_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CGameAllocator
{
    struct SAllocationDescription
    {
        std::unique_ptr<u8[]> memptr;
        size_t allocSize = 0;
        ptrdiff_t freeOffset = 0;
    };

    struct SChunkDescription
    {
        u32 magic = 0xE8E8E8E8;
        SAllocationDescription* parent;
        size_t len = 0;
        u32 sentinal = 0xEFEFEFEF;
    };

    static std::vector<SAllocationDescription> m_allocations;

public:

    static u8* Alloc(size_t len);
    static void Free(u8* ptr);
};
}

#endif
