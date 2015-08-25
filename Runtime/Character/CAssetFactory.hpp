#ifndef __RETRO_CASSETFACTORY_HPP__
#define __RETRO_CASSETFACTORY_HPP__

#include "../IFactory.hpp"

namespace Retro
{
class CCharacterFactoryBuilder : public IFactory
{
public:
    std::unique_ptr<IObj> Build(const SObjectTag&, const CVParamTransfer&) {}
    void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) {}
    void CancelBuild(const SObjectTag&) {}
    bool CanBuild(const SObjectTag&) {}
    u32 GetResourceIdByName(const char*) const {}
};
}

#endif // __RETRO_CASSETFACTORY_HPP__
