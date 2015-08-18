#ifndef __RETRO_CINGAMETWEAKMANAGER_HPP__
#define __RETRO_CINGAMETWEAKMANAGER_HPP__

#include <string>

namespace Retro
{
namespace Common
{

class CInGameTweakManager
{
public:
    bool ReadFromMemoryCard(const std::string& name)
    {
        return true;
    }
};

}
}

#endif // __RETRO_CINGAMETWEAKMANAGER_HPP__
