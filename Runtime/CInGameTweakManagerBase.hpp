#ifndef __URDE_CINGAMETWEAKMANAGERBASE_HPP__
#define __URDE_CINGAMETWEAKMANAGERBASE_HPP__

#include <string>

namespace urde
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

#endif // __URDE_CINGAMETWEAKMANAGERBASE_HPP__
