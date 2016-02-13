#ifndef __PSHAG_CASSETFACTORY_HPP__
#define __PSHAG_CASSETFACTORY_HPP__

#include "../IFactory.hpp"
#include "../IObj.hpp"

namespace pshag
{
class CCharacterFactoryBuilder : public IFactory
{
public:
    std::unique_ptr<IObj> Build(const SObjectTag&, const CVParamTransfer&) {return std::unique_ptr<IObj>();}
    void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) {}
    void CancelBuild(const SObjectTag&) {}
    bool CanBuild(const SObjectTag&) {return false;}
    const SObjectTag* GetResourceIdByName(const char*) const {return nullptr;}
};
}

#endif // __PSHAG_CASSETFACTORY_HPP__
