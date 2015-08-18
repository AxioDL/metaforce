#ifndef __RETRO_CCHARACTERFACTORY_HPP__
#define __RETRO_CCHARACTERFACTORY_HPP__

#include "../IFactory.hpp"

namespace Retro
{
namespace Common
{

class CCharacterFactory : public IFactory
{
public:
    class CDummyFactory : public IFactory
    {
    };
};

}
}

#endif // __RETRO_CCHARACTERFACTORY_HPP__
