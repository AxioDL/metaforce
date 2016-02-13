#ifndef __PSHAG_CINGAMETWEAKMANAGERBASE_HPP__
#define __PSHAG_CINGAMETWEAKMANAGERBASE_HPP__

#include <string>

namespace pshag
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

#endif // __PSHAG_CINGAMETWEAKMANAGERBASE_HPP__
