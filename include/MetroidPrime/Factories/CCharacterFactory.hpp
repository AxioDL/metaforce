#ifndef _CCHARACTERFACTORY
#define _CCHARACTERFACTORY

#include "types.h"

#include "Kyoto/Animation/CAdditiveAnimationInfo.hpp"
#include "Kyoto/Animation/CAnimationManager.hpp"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/Animation/CTransitionManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/IFactory.hpp"
#include "Kyoto/IObjFactory.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/vector.hpp"

class CAllFormatsAnimSource;
class CAnimCharacterSet;
class CAnimData;
class CAnimationManager;
class CAnimSysContext;
class CCharLayoutInfo;
class CTransitionManager;

class CCharacterFactory : public IObjFactory {
public:
  ~CCharacterFactory() override {}

  class CDummyFactory : public IFactory {
  public:
    rstl::auto_ptr< IObj > Build(const SObjectTag& tag, const CVParamTransfer& params) override;
    void BuildAsync(const SObjectTag& tag, const CVParamTransfer& params, IObj** out) override;
    void CancelBuild(const SObjectTag& tag) override;
    bool CanBuild(const SObjectTag&) override { return true; }
    const SObjectTag* GetResourceIdByName(const char*) const override { return nullptr; }
  };

  CCharacterFactory(CSimplePool& store, const CAnimCharacterSet& ancs, CAssetId selfId);
  int GetEventResourceIdForAnimResourceId(int id) const;
  rstl::auto_ptr< CAnimData > CreateCharacter(int charIdx, bool loop,
                                             const TLockedToken< CCharacterFactory >& factory,
                                             int defaultAnim) const;
  const CCharacterInfo& GetCharInfo(int charIdx) const;
  const rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > >& GetAdditiveAnimInfoList() const {
    return mAdditiveInfo;
  }
  const CAdditiveAnimationInfo& GetDefaultAdditiveAnimInfo() const {
    return mDefaultAdditiveInfo;
  }

  static rstl::vector< CCharacterInfo > GetCharacterInfoDB(const CAnimCharacterSet& ancs);
  static rstl::vector< TToken< CCharLayoutInfo > >
  GetCharLayoutInfoDB(CSimplePool& store, const rstl::vector< CCharacterInfo >& chars);

private:
  rstl::vector< CCharacterInfo > mCharInfoDB;
  rstl::vector< TToken< CCharLayoutInfo > > mCharLayoutInfoDB;
  rstl::ncrc_ptr< CAnimSysContext > mSysContext;
  rstl::rc_ptr< CAnimationManager > mAnimMgr;
  rstl::rc_ptr< CTransitionManager > mTransMgr;
  rstl::vector< TCachedToken< CAllFormatsAnimSource > > mAnimSourceDB;
  rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > > mAdditiveInfo;
  CAdditiveAnimationInfo mDefaultAdditiveInfo;
  rstl::vector< rstl::pair< int, int > > mAnimResources;
  CAssetId mSelfId;
  CDummyFactory mDummyFactory;
  mutable CSimplePool mCacheResPool;
};

NESTED_CHECK_SIZEOF(CCharacterFactory, CDummyFactory, 0x4)
CHECK_SIZEOF(CCharacterFactory, 0x90)

#endif // _CCHARACTERFACTORY
