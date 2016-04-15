#include "CAssetFactory.hpp"
#include "CAnimCharacterSet.hpp"
#include "CCharacterFactory.hpp"
#include "GameGlobalObjects.hpp"
#include "CModelData.hpp"

namespace urde
{

CFactoryFnReturn CCharacterFactoryBuilder::CDummyFactory::Build(const SObjectTag& tag,
                                                                const CVParamTransfer&)
{
    TLockedToken<CAnimCharacterSet> ancs =
        g_SimplePool->GetObj({SBIG('ANCS'), tag.id});
    return TToken<CCharacterFactory>::GetIObjObjectFor(
        std::make_unique<CCharacterFactory>(*g_SimplePool, *ancs.GetObj(), tag.id));
}

void CCharacterFactoryBuilder::CDummyFactory::BuildAsync(const SObjectTag& tag,
                                                         const CVParamTransfer& parms,
                                                         IObj** objOut)
{
    *objOut = Build(tag, parms).release();
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

FourCC CCharacterFactoryBuilder::CDummyFactory::GetResourceTypeById(ResId id) const
{
    return {};
}

CCharacterFactoryBuilder::CCharacterFactoryBuilder() : x4_dummyStore(x0_dummyFactory) {}

TToken<CCharacterFactory> CCharacterFactoryBuilder::GetFactory(const CAnimRes& res)
{
    return x4_dummyStore.GetObj({SBIG('ANCS'), res.x0_ancsId});
}

}
