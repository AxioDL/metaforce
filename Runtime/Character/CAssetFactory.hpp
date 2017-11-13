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
        CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&, CObjectReference* selfRef);
        void BuildAsync(const SObjectTag&, const CVParamTransfer&, std::unique_ptr<IObj>*, CObjectReference* selfRef);
        void CancelBuild(const SObjectTag&);
        bool CanBuild(const SObjectTag&);
        const SObjectTag* GetResourceIdByName(std::string_view) const;
        FourCC GetResourceTypeById(CAssetId id) const;

        void EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const;
        void EnumerateNamedResources(const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const;

        u32 ResourceSize(const urde::SObjectTag& tag);
        std::shared_ptr<IDvdRequest> LoadResourceAsync(const urde::SObjectTag& tag, void* target);
        std::shared_ptr<IDvdRequest> LoadResourcePartAsync(const urde::SObjectTag& tag, u32 off, u32 size, void* target);
        std::unique_ptr<u8[]> LoadResourceSync(const urde::SObjectTag& tag);
        std::unique_ptr<u8[]> LoadNewResourcePartSync(const urde::SObjectTag& tag, u32 off, u32 size);
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
