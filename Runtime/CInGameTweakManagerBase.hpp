#ifndef __RETRO_CINGAMETWEAKMANAGERBASE_HPP__
#define __RETRO_CINGAMETWEAKMANAGERBASE_HPP__

#include <string>

namespace Retro
{

class CInGameTweakManagerBase
{
public:
    bool ReadFromMemoryCard(const std::string& name)
    {
        return true;
    }
};

}

#endif // __RETRO_CINGAMETWEAKMANAGERBASE_HPP__
