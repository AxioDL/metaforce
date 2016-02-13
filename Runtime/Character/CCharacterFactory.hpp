#ifndef __PSHAG_CCHARACTERFACTORY_HPP__
#define __PSHAG_CCHARACTERFACTORY_HPP__

#include "../IFactory.hpp"

namespace pshag
{

class CCharacterFactory : public IFactory
{
public:
    class CDummyFactory : public IFactory
    {
    };
};

}

#endif // __PSHAG_CCHARACTERFACTORY_HPP__
