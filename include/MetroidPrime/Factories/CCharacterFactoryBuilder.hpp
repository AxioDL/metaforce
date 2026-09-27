#ifndef _CCHARACTERFACTORYBUILDER
#define _CCHARACTERFACTORYBUILDER

#include "types.h"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/IFactory.hpp"
#include "Kyoto/TToken.hpp"

class CAnimRes;
class CCharacterFactory;

class CCharacterFactoryBuilder {
public:
  class CDummyFactory : public IFactory {
  public:
    rstl::auto_ptr< IObj > Build(const SObjectTag& tag, const CVParamTransfer& params) override;
    void BuildAsync(const SObjectTag& tag, const CVParamTransfer& params, IObj** out) override;
    void CancelBuild(const SObjectTag& tag) override;
    bool CanBuild(const SObjectTag&) override { return true; }
    const SObjectTag* GetResourceIdByName(const char*) const override { return nullptr; }
  };

  CCharacterFactoryBuilder();
  ~CCharacterFactoryBuilder();
  TToken< CCharacterFactory > GetFactory(const CAnimRes& res);

private:
  CDummyFactory mDummyFactory;
  CSimplePool mDummyStore;
};

NESTED_CHECK_SIZEOF(CCharacterFactoryBuilder, CDummyFactory, 0x4)
CHECK_SIZEOF(CCharacterFactoryBuilder, 0x24)

extern CCharacterFactoryBuilder* gpCharacterFactoryBuilder;

#endif // _CCHARACTERFACTORYBUILDER
