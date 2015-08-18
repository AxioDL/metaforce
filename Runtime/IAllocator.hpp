#ifndef __RETRO_IALLOCATOR_HPP__
#define __RETRO_IALLOCATOR_HPP__

#include "COsContext.hpp"

namespace Retro
{

class IAllocator
{
public:
    virtual bool Initialize(COsContext&)=0;
    virtual void Shutdown()=0;
};

}

#endif // __RETRO_IALLOCATOR_HPP__
