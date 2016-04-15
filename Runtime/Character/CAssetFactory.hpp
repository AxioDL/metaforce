#ifndef __URDE_CASSETFACTORY_HPP__
#define __URDE_CASSETFACTORY_HPP__

#include "IFactory.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "CSimplePool.hpp"

namespace urde
{
class CCharacterFactory;
class CAnimRes;

class CCharacterFactoryBuilder
{
public:
    class CDummyFactory : public IFactory
    {
    public:
        CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&);
        void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**);
        void CancelBuild(const SObjectTag&);
        bool CanBuild(const SObjectTag&);
        const SObjectTag* GetResourceIdByName(const char*) const;
        FourCC GetResourceTypeById(ResId id) const;
    };

private:
    CDummyFactory x0_dummyFactory;
    CSimplePool x4_dummyStore;

public:
    CCharacterFactoryBuilder();
    TToken<CCharacterFactory> GetFactory(const CAnimRes& res);
};

}

#endif // __URDE_CASSETFACTORY_HPP__
