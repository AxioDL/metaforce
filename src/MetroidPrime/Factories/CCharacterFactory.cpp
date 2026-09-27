#include "MetroidPrime/Factories/CCharacterFactory.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimationDatabaseGame.hpp"
#include "MetroidPrime/CTransitionDatabaseGame.hpp"

#include "Kyoto/Animation/CAnimCharacterSet.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "rstl/algorithm.hpp"

inline CAnimationManager::CAnimationManager(const TToken< CAnimationDatabase >& animDB,
                                          const CAnimSysContext& sysCtx)
: mAnimDB(animDB)
, mSysCtx(sysCtx) {}

inline CTransitionManager::CTransitionManager(const CAnimSysContext& context) : mContext(context) {}

rstl::auto_ptr< IObj > CCharacterFactory::CDummyFactory::Build(const SObjectTag& tag,
                                                               const CVParamTransfer& params) {
  const CVParamTransfer paramCopy(params);
  const CCharacterInfo& charInfo =
      **static_cast< const TObjOwnerParam< const CCharacterInfo* const >& >(*paramCopy);
  switch (tag.GetType()) {
  case 0:
    return CFactoryFnReturn(
               rs_new CSkinnedModel(
                   gpSimplePool->GetObj(SObjectTag('CMDL', charInfo.GetModelId())),
                   gpSimplePool->GetObj(SObjectTag('CSKR', charInfo.GetSkinRulesId())),
                   gpSimplePool->GetObj(SObjectTag('CINF', charInfo.GetCharLayoutInfoId())),
                   CSkinnedModel::kDO_Owned))
        .GetObjForTransfer();
  case 1:
    return CFactoryFnReturn(
               rs_new CSkinnedModelWithAvgNormals(CSkinnedModel(
                   gpSimplePool->GetObj(SObjectTag('CMDL', charInfo.GetIceModelId())),
                   gpSimplePool->GetObj(SObjectTag('CSKR', charInfo.GetIceSkinRulesId())),
                   gpSimplePool->GetObj(SObjectTag('CINF', charInfo.GetCharLayoutInfoId())),
                   CSkinnedModel::kDO_Owned)))
        .GetObjForTransfer();
  }
  return rstl::auto_ptr< IObj >();
}

void CCharacterFactory::CDummyFactory::BuildAsync(const SObjectTag& tag,
                                                  const CVParamTransfer& params, IObj** out) {
  *out = Build(tag, params).release();
}

void CCharacterFactory::CDummyFactory::CancelBuild(const SObjectTag&) {}

CCharacterFactory::CCharacterFactory(CSimplePool& store, const CAnimCharacterSet& ancs,
                                     CAssetId selfId)
: mCharInfoDB(GetCharacterInfoDB(ancs))
, mCharLayoutInfoDB(GetCharLayoutInfoDB(store, mCharInfoDB))
, mAdditiveInfo(ancs.GetAnimationSet().GetAdditiveAnimInfoList())
, mDefaultAdditiveInfo(ancs.GetAnimationSet().GetDefaultAdditiveAnimInfo())
, mAnimResources(ancs.GetAnimationSet().GetAnimResIdEventResIdList())
, mSelfId(selfId)
, mCacheResPool(mDummyFactory) {
  const CAnimationSet::AnimationList& animations = ancs.GetAnimationSet().GetAnimations();
  const CAnimationSet::TransitionList& transitions = ancs.GetAnimationSet().GetTransitions();
  const CAnimationSet::HalfTransitionList& halfTransitions =
      ancs.GetAnimationSet().GetHalfTransitions();
  const rstl::rc_ptr< IMetaTrans > defaultTrans = ancs.GetAnimationSet().GetDefaultTransition();
  const TToken< CAnimationDatabaseGame > animDB(rs_new CAnimationDatabaseGame(animations));
  const TToken< CTransitionDatabaseGame > transDB(
      rs_new CTransitionDatabaseGame(transitions, halfTransitions, defaultTrans));
  const rstl::ncrc_ptr< CRandom16 > random(rs_new CRandom16(2334));
  mSysContext =
      rstl::ncrc_ptr< CAnimSysContext >(rs_new CAnimSysContext(transDB, random, store));
  mAnimMgr = rs_new CAnimationManager(animDB, *mSysContext);
  mTransMgr = rs_new CTransitionManager(*mSysContext);

  rstl::vector< CPrimitive > primitives;
  animDB.NonConstCopy()->GetAllUniquePrimitives(primitives);
  mAnimSourceDB.reserve(primitives.size());
  rstl::vector< CPrimitive >::const_iterator it = primitives.begin();
  rstl::vector< CPrimitive >::const_iterator primEnd = primitives.end();
  for (; it != primEnd; ++it) {
    const SObjectTag tag('ANIM', it->GetAnimResId());
    mAnimSourceDB.push_back(store.GetObj(tag));
  }
}

rstl::auto_ptr< CAnimData >
CCharacterFactory::CreateCharacter(int charIdx, bool loop,
                                   const TLockedToken< CCharacterFactory >& factory,
                                   int defaultAnim) const {
  const CCharacterInfo& charInfo = mCharInfoDB[charIdx];
  const SObjectTag modelTag(0, charInfo.GetModelId());
  TToken< CSkinnedModel > skinnedModel = mCacheResPool.GetObj(
      modelTag, CVParamTransfer(rs_new TObjOwnerParam< const CCharacterInfo* const >(&charInfo)));
  const CAssetId iceModelId = charInfo.GetIceModelId();
  const CAssetId iceSkinId = charInfo.GetIceSkinRulesId();
  const SObjectTag iceTag(1, iceModelId);
  rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > > iceModel;
  if (iceModelId != 0 && iceSkinId != 0) {
    iceModel = TLockedToken< CSkinnedModelWithAvgNormals >(mCacheResPool.GetObj(
        iceTag, CVParamTransfer(rs_new TObjOwnerParam< const CCharacterInfo* const >(&charInfo))));
  }
  CAnimData* animData = rs_new CAnimData(mSelfId, charInfo, defaultAnim, charIdx, loop,
                                         mCharLayoutInfoDB[charIdx], skinnedModel, iceModel,
                                         mSysContext, mAnimMgr, mTransMgr, factory);
  return animData;
}

const CCharacterInfo& CCharacterFactory::GetCharInfo(int charIdx) const {
  return mCharInfoDB[charIdx];
}

rstl::vector< CCharacterInfo >
CCharacterFactory::GetCharacterInfoDB(const CAnimCharacterSet& ancs) {
  const rstl::vector< rstl::pair< int, CCharacterInfo > >& chars =
      ancs.GetCharacterSet().GetCharacterList();
  AUTO(it, chars.begin());
  AUTO(end, chars.end());
  rstl::vector< CCharacterInfo > result;
  result.reserve(chars.size());
  for (; it != end; ++it) {
    result.push_back(it->second);
  }
  return result;
}

rstl::vector< TToken< CCharLayoutInfo > >
CCharacterFactory::GetCharLayoutInfoDB(CSimplePool& store,
                                       const rstl::vector< CCharacterInfo >& chars) {
  rstl::vector< TToken< CCharLayoutInfo > > result;
  const uint count = chars.size();
  result.reserve(count);
  for (uint i = 0; i < count; ++i) {
    TToken< CCharLayoutInfo > layout =
        store.GetObj(SObjectTag('CINF', chars[i].GetCharLayoutInfoId()));
    result.push_back(layout);
  }
  return result;
}

int CCharacterFactory::GetEventResourceIdForAnimResourceId(int id) const {
  AUTO(cmp, (rstl::default_pair_sorter_finder< rstl::vector< rstl::pair< int, int > > >()));
  AUTO(it, rstl::binary_find(mAnimResources.begin(), mAnimResources.end(), id, cmp));
  if (it != mAnimResources.end()) {
    return it->second;
  }
  return -1;
}
