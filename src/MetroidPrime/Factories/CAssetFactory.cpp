#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"

#include "Kyoto/Animation/CAnimCharacterSet.hpp"

IObjFactory::~IObjFactory() {}

rstl::auto_ptr< IObj >
CCharacterFactoryBuilder::CDummyFactory::Build(const SObjectTag& tag,
                                               const CVParamTransfer& params) {
  CAssetId id = tag.GetId();
  TToken< CAnimCharacterSet > ancs = gpSimplePool->GetObj(SObjectTag('ANCS', id));
  return CFactoryFnReturn(rs_new CCharacterFactory(*gpSimplePool, **ancs, id)).GetObjForTransfer();
}

void CCharacterFactoryBuilder::CDummyFactory::BuildAsync(const SObjectTag& tag,
                                                       const CVParamTransfer& params,
                                                       IObj** out) {
  *out = Build(tag, params).release();
}

void CCharacterFactoryBuilder::CDummyFactory::CancelBuild(const SObjectTag&) {}

CCharacterFactoryBuilder::CCharacterFactoryBuilder() : mDummyStore(mDummyFactory) {}

CCharacterFactoryBuilder::~CCharacterFactoryBuilder() {}

TToken< CCharacterFactory > CCharacterFactoryBuilder::GetFactory(const CAnimRes& res) {
  return mDummyStore.GetObj(SObjectTag('ANCS', res.GetId()), CVParamTransfer::Null());
}
