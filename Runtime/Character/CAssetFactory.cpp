#include "CAssetFactory.hpp"
#include "CAnimCharacterSet.hpp"
#include "CCharacterFactory.hpp"
#include "GameGlobalObjects.hpp"
#include "CModelData.hpp"
#include "Character/CCharLayoutInfo.hpp"

namespace urde
{

CFactoryFnReturn CCharacterFactoryBuilder::CDummyFactory::Build(const SObjectTag& tag,
                                                                const CVParamTransfer&,
                                                                CObjectReference* selfRef)
{
    TLockedToken<CAnimCharacterSet> ancs =
        g_SimplePool->GetObj({SBIG('ANCS'), tag.id});
    return TToken<CCharacterFactory>::GetIObjObjectFor(
        std::make_unique<CCharacterFactory>(*g_SimplePool, *ancs.GetObj(), tag.id));
}

void CCharacterFactoryBuilder::CDummyFactory::BuildAsync(const SObjectTag& tag,
                                                         const CVParamTransfer& parms,
                                                         std::unique_ptr<IObj>* objOut,
                                                         CObjectReference* selfRef)
{
    *objOut = Build(tag, parms, selfRef);
}

void CCharacterFactoryBuilder::CDummyFactory::CancelBuild(const SObjectTag&)
{
}

bool CCharacterFactoryBuilder::CDummyFactory::CanBuild(const SObjectTag&)
{
    return true;
}

const SObjectTag* CCharacterFactoryBuilder::CDummyFactory::GetResourceIdByName(const char*) const
{
    return nullptr;
}

FourCC CCharacterFactoryBuilder::CDummyFactory::GetResourceTypeById(CAssetId id) const
{
    return {};
}

void CCharacterFactoryBuilder::CDummyFactory::EnumerateResources(
        const std::function<bool(const SObjectTag&)>& lambda) const
{
}

void CCharacterFactoryBuilder::CDummyFactory::EnumerateNamedResources(
        const std::function<bool(const std::string&, const SObjectTag&)>& lambda) const
{
}

u32 CCharacterFactoryBuilder::CDummyFactory::ResourceSize(const urde::SObjectTag& tag)
{
    return 0;
}

bool CCharacterFactoryBuilder::CDummyFactory::LoadResourceAsync(const urde::SObjectTag& tag,
                                                                std::unique_ptr<u8[]>& target)
{
    return false;
}

bool CCharacterFactoryBuilder::CDummyFactory::LoadResourcePartAsync(const urde::SObjectTag& tag,
                                                                    u32 size, u32 off,
                                                                    std::unique_ptr<u8[]>& target)
{
    return false;
}

std::unique_ptr<u8[]> CCharacterFactoryBuilder::CDummyFactory::LoadResourceSync(const urde::SObjectTag& tag)
{
    return {};
}

std::unique_ptr<u8[]> CCharacterFactoryBuilder::CDummyFactory::LoadResourcePartSync(const urde::SObjectTag& tag,
                                                                                    u32 size, u32 off)
{
    return {};
}

CCharacterFactoryBuilder::CCharacterFactoryBuilder() : x4_dummyStore(x0_dummyFactory) {}

TToken<CCharacterFactory> CCharacterFactoryBuilder::GetFactory(const CAnimRes& res)
{
    return x4_dummyStore.GetObj({SBIG('ANCS'), res.GetId()});
}

}
