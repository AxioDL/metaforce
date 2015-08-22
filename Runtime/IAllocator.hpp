#ifndef __RETRO_IALLOCATOR_HPP__
#define __RETRO_IALLOCATOR_HPP__

namespace Retro
{
class COsContext;

class IAllocator
{
public:
    virtual bool Initialize(COsContext&)=0;
    virtual void Shutdown()=0;
};

}

#endif // __RETRO_IALLOCATOR_HPP__
