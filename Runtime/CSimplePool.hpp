#ifndef __RETRO_CSIMPLEPOOL_HPP__
#define __RETRO_CSIMPLEPOOL_HPP__

#include "IObjectStore.hpp"

namespace Retro
{
class IFactory;

class CSimplePool : public IObjectStore
{
public:
    CSimplePool(IFactory&)
    {
    }
};

}

#endif // __RETRO_CSIMPLEPOOL_HPP__
