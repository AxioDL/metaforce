#ifndef __RETRO_CASSETFACTORY_HPP__
#define __RETRO_CASSETFACTORY_HPP__

#include "../IFactory.hpp"

namespace Retro
{

class CCharacterFactoryBuilder : public IFactory
{
public:
    class CDummyFactory : public IFactory
    {
    };
};

}

#endif // __RETRO_CASSETFACTORY_HPP__
