#ifndef __URDE_CASSETFACTORY_HPP__
#define __URDE_CASSETFACTORY_HPP__

#include "IFactory.hpp"
#include "IObj.hpp"
#include "CToken.hpp"

namespace urde
{
class CCharacterFactory;
class CAnimRes;

class CCharacterFactoryBuilder : public IFactory
{
public:
    std::unique_ptr<IObj> Build(const SObjectTag&, const CVParamTransfer&) {return std::unique_ptr<IObj>();}
    void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) {}
    void CancelBuild(const SObjectTag&) {}
    bool CanBuild(const SObjectTag&) {return false;}
    const SObjectTag* GetResourceIdByName(const char*) const {return nullptr;}
    FourCC GetResourceTypeById(ResId id) const {return {};}
    TToken<CCharacterFactory> GetFactory(const CAnimRes& res);
};

}

#endif // __URDE_CASSETFACTORY_HPP__
