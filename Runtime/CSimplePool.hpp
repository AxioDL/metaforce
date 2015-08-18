#ifndef __RETRO_CSIMPLEPOOL_HPP__
#define __RETRO_CSIMPLEPOOL_HPP__

#include "IFactory.hpp"
#include "CFactoryStore.hpp"

namespace Retro
{

class CSimplePool : public IObjectStore
{
public:
    CSimplePool(IFactory&)
    {
    }
};

}

#endif // __RETRO_CSIMPLEPOOL_HPP__
