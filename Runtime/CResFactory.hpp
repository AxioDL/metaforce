#ifndef __RETRO_CRESFACTORY_HPP__
#define __RETRO_CRESFACTORY_HPP__

#include "IFactory.hpp"

namespace Retro
{

class CResFactory : public IFactory
{
public:
    void Build(const SObjectTag&, const CVParamTransfer&);
    void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**);
    void CancelBuild(const SObjectTag&);
    bool CanBuild(const SObjectTag&);
    const SObjectTag& GetResourceIdByName(const char*) const;
};

}

#endif // __RETRO_CRESFACTORY_HPP__
