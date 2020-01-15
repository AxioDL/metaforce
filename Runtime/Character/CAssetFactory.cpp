#include "Runtime/Character/CAssetFactory.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CAnimCharacterSet.hpp"
#include "Runtime/Character/CCharacterFactory.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CModelData.hpp"

namespace urde {

CFactoryFnReturn CCharacterFactoryBuilder::CDummyFactory::Build(const SObjectTag& tag, const CVParamTransfer&,
                                                                CObjectReference* selfRef) {
  TLockedToken<CAnimCharacterSet> ancs = g_SimplePool->GetObj({SBIG('ANCS'), tag.id});
  return TToken<CCharacterFactory>::GetIObjObjectFor(
      std::make_unique<CCharacterFactory>(*g_SimplePool, *ancs.GetObj(), tag.id));
}

void CCharacterFactoryBuilder::CDummyFactory::BuildAsync(const SObjectTag& tag, const CVParamTransfer& parms,
                                                         std::unique_ptr<IObj>* objOut, CObjectReference* selfRef) {
  *objOut = Build(tag, parms, selfRef);
}

void CCharacterFactoryBuilder::CDummyFactory::CancelBuild(const SObjectTag&) {}

bool CCharacterFactoryBuilder::CDummyFactory::CanBuild(const SObjectTag&) { return true; }

const SObjectTag* CCharacterFactoryBuilder::CDummyFactory::GetResourceIdByName(std::string_view) const {
  return nullptr;
}

FourCC CCharacterFactoryBuilder::CDummyFactory::GetResourceTypeById(CAssetId id) const { return {}; }

void CCharacterFactoryBuilder::CDummyFactory::EnumerateResources(
    const std::function<bool(const SObjectTag&)>& lambda) const {}

void CCharacterFactoryBuilder::CDummyFactory::EnumerateNamedResources(
    const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const {}

u32 CCharacterFactoryBuilder::CDummyFactory::ResourceSize(const urde::SObjectTag& tag) { return 0; }

std::shared_ptr<IDvdRequest> CCharacterFactoryBuilder::CDummyFactory::LoadResourceAsync(const urde::SObjectTag& tag,
                                                                                        void* target) {
  return {};
}

std::shared_ptr<IDvdRequest> CCharacterFactoryBuilder::CDummyFactory::LoadResourcePartAsync(const urde::SObjectTag& tag,
                                                                                            u32 off, u32 size,
                                                                                            void* target) {
  return {};
}

std::unique_ptr<u8[]> CCharacterFactoryBuilder::CDummyFactory::LoadResourceSync(const urde::SObjectTag& tag) {
  return {};
}

std::unique_ptr<u8[]> CCharacterFactoryBuilder::CDummyFactory::LoadNewResourcePartSync(const urde::SObjectTag& tag,
                                                                                       u32 off, u32 size) {
  return {};
}

CCharacterFactoryBuilder::CCharacterFactoryBuilder() : x4_dummyStore(x0_dummyFactory) {}

TToken<CCharacterFactory> CCharacterFactoryBuilder::GetFactory(const CAnimRes& res) {
  return x4_dummyStore.GetObj({SBIG('ANCS'), res.GetId()});
}

} // namespace urde
