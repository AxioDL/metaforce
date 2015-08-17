#ifndef __RETRO_CGAMEALLOCATOR_HPP__
#define __RETRO_CGAMEALLOCATOR_HPP__

#include "IAllocator.hpp"

namespace Retro
{
namespace Common
{

class CGameAllocator : public IAllocator
{
public:
    bool Initialize(COsContext&);
    void Shutdown();
};

}
}

#endif // __RETRO_CGAMEALLOCATOR_HPP__
