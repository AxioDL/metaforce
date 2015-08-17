#ifndef __RETRO_CMEMORY_HPP__
#define __RETRO_CMEMORY_HPP__

#include "IAllocator.hpp"
#include "COsContext.hpp"

namespace Retro
{
namespace Common
{

class CMemory
{
public:
    static void Startup(COsContext&);
    static void Shutdown();
    static void SetAllocator(COsContext&, IAllocator&);
};

class CMemorySys
{
public:
    CMemorySys(COsContext&, IAllocator&);
    ~CMemorySys();
    static IAllocator& GetGameAllocator();
};

}
}

#endif // __RETRO_CMEMORY_HPP__
