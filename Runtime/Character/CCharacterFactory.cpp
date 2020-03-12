#include "Runtime/Character/CCharacterFactory.hpp"

#include "Runtime/CRandom16.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CAdditiveAnimPlayback.hpp"
#include "Runtime/Character/CAnimCharacterSet.hpp"
#include "Runtime/Character/CAnimData.hpp"
#include "Runtime/Character/CAnimationDatabaseGame.hpp"
#include "Runtime/Character/CAnimationManager.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CParticleGenInfo.hpp"
#include "Runtime/Character/CPrimitive.hpp"
#include "Runtime/Character/CTransitionDatabaseGame.hpp"
#include "Runtime/Character/CTransitionManager.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"

namespace urde {

CFactoryFnReturn CCharacterFactory::CDummyFactory::Build(const SObjectTag& tag, const CVParamTransfer& params,
                                                         CObjectReference* selfRef) {

  const CCharacterInfo& charInfo = *params.GetOwnedObj<const CCharacterInfo*>();

  switch (tag.type.toUint32() & 0x1) {
  case 0:
    return TToken<CSkinnedModel>::GetIObjObjectFor(
        std::make_unique<CSkinnedModel>(*g_SimplePool, charInfo.GetModelId(), charInfo.GetSkinRulesId(),
                                        charInfo.GetCharLayoutInfoId(), 0, tag.type.toUint32() >> 16));
  case 1:
    return TToken<CSkinnedModel>::GetIObjObjectFor(
        std::make_unique<CMorphableSkinnedModel>(*g_SimplePool, charInfo.GetIceModelId(), charInfo.GetIceSkinRulesId(),
                                                 charInfo.GetCharLayoutInfoId(), 0, tag.type.toUint32() >> 16));
  default:
    break;
  }

  return {};
}

void CCharacterFactory::CDummyFactory::BuildAsync(const SObjectTag& tag, const CVParamTransfer& parms,
                                                  std::unique_ptr<IObj>* objOut, CObjectReference* selfRef) {
  *objOut = Build(tag, parms, selfRef);
}

void CCharacterFactory::CDummyFactory::CancelBuild(const SObjectTag&) {}

bool CCharacterFactory::CDummyFactory::CanBuild(const SObjectTag&) { return true; }

const SObjectTag* CCharacterFactory::CDummyFactory::GetResourceIdByName(std::string_view) const { return nullptr; }

FourCC CCharacterFactory::CDummyFactory::GetResourceTypeById(CAssetId id) const { return {}; }

void CCharacterFactory::CDummyFactory::EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const {}
void CCharacterFactory::CDummyFactory::EnumerateNamedResources(
    const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const {}

u32 CCharacterFactory::CDummyFactory::ResourceSize(const urde::SObjectTag& tag) { return 0; }

std::shared_ptr<IDvdRequest> CCharacterFactory::CDummyFactory::LoadResourceAsync(const urde::SObjectTag& tag,
                                                                                 void* target) {
  return {};
}

std::shared_ptr<IDvdRequest> CCharacterFactory::CDummyFactory::LoadResourcePartAsync(const urde::SObjectTag& tag,
                                                                                     u32 off, u32 size, void* target) {
  return {};
}

std::unique_ptr<u8[]> CCharacterFactory::CDummyFactory::LoadResourceSync(const urde::SObjectTag& tag) { return {}; }

std::unique_ptr<u8[]> CCharacterFactory::CDummyFactory::LoadNewResourcePartSync(const urde::SObjectTag& tag, u32 off,
                                                                                u32 size) {
  return {};
}

std::unique_ptr<CAnimData> CCharacterFactory::CreateCharacter(int charIdx, bool loop,
                                                              const TLockedToken<CCharacterFactory>& factory,
                                                              int defaultAnim, int drawInsts) const {
  const CCharacterInfo& charInfo = x4_charInfoDB[charIdx];
  const CVParamTransfer charParm(new TObjOwnerParam<const CCharacterInfo*>(&charInfo));

  TToken<CSkinnedModel> skinnedModel = const_cast<CCharacterFactory*>(this)->x70_cacheResPool.GetObj(
      {FourCC(drawInsts << 16), charInfo.GetModelId()}, charParm);

  std::optional<TToken<CMorphableSkinnedModel>> iceModel;
  if (charInfo.GetIceModelId().IsValid() && charInfo.GetIceSkinRulesId().IsValid()) {
    iceModel.emplace(const_cast<CCharacterFactory*>(this)->x70_cacheResPool.GetObj(
        {FourCC((drawInsts << 16) | 1), charInfo.GetIceModelId()}, charParm));
  }

  return std::make_unique<CAnimData>(x68_selfId, charInfo, defaultAnim, charIdx, loop, x14_charLayoutInfoDB[charIdx],
                                     std::move(skinnedModel), iceModel, x24_sysContext, x28_animMgr, x2c_transMgr,
                                     factory, drawInsts);
}

CAssetId CCharacterFactory::GetEventResourceIdForAnimResourceId(CAssetId id) const {
  auto search = std::find_if(x58_animResources.cbegin(), x58_animResources.cend(),
                             [&](const std::pair<CAssetId, CAssetId>& elem) -> bool { return id == elem.first; });
  if (search == x58_animResources.cend())
    return CAssetId();
  return search->second;
}

const CAdditiveAnimationInfo& CCharacterFactory::FindAdditiveInfo(s32 idx) const {
  auto search = rstl::binary_find(x40_additiveInfo.cbegin(), x40_additiveInfo.cend(), idx,
                                  [](const auto& anim) { return anim.first; });

  if (search == x40_additiveInfo.cend())
    return x50_defaultAdditiveInfo;
  return search->second;
}

bool CCharacterFactory::HasAdditiveInfo(s32 idx) const {
  auto search = rstl::binary_find(x40_additiveInfo.cbegin(), x40_additiveInfo.cend(), idx,
                                  [](const auto& anim) { return anim.first; });
  return search != x40_additiveInfo.cend();
}

std::vector<CCharacterInfo> CCharacterFactory::GetCharacterInfoDB(const CAnimCharacterSet& ancs) {
  std::vector<CCharacterInfo> ret;
  const std::map<u32, CCharacterInfo>& charInfoMap = ancs.GetCharacterSet().GetCharacterInfoMap();
  ret.reserve(charInfoMap.size());
  for (const auto& charInfo : charInfoMap)
    ret.push_back(charInfo.second);
  return ret;
}

std::vector<TLockedToken<CCharLayoutInfo>>
CCharacterFactory::GetCharLayoutInfoDB(CSimplePool& store, const std::vector<CCharacterInfo>& chars) {
  std::vector<TLockedToken<CCharLayoutInfo>> ret;
  ret.reserve(chars.size());
  for (const CCharacterInfo& charInfo : chars)
    ret.push_back(store.GetObj({SBIG('CINF'), charInfo.GetCharLayoutInfoId()}));
  return ret;
}

CCharacterFactory::CCharacterFactory(CSimplePool& store, const CAnimCharacterSet& ancs, CAssetId selfId)
: x4_charInfoDB(GetCharacterInfoDB(ancs))
, x14_charLayoutInfoDB(GetCharLayoutInfoDB(store, x4_charInfoDB))
, x24_sysContext(std::make_shared<CAnimSysContext>(
      TToken<CTransitionDatabaseGame>(std::make_unique<CTransitionDatabaseGame>(
          ancs.GetAnimationSet().GetTransitions(), ancs.GetAnimationSet().GetHalfTransitions(),
          ancs.GetAnimationSet().GetDefaultTransition())),
      2334, store))
, x28_animMgr(std::make_shared<CAnimationManager>(
      TToken<CAnimationDatabaseGame>(std::make_unique<CAnimationDatabaseGame>(ancs.GetAnimationSet().GetAnimations())),
      *x24_sysContext))
, x2c_transMgr(std::make_shared<CTransitionManager>(*x24_sysContext))
, x40_additiveInfo(ancs.GetAnimationSet().GetAdditiveInfo())
, x50_defaultAdditiveInfo(ancs.GetAnimationSet().GetDefaultAdditiveInfo())
, x58_animResources(ancs.GetAnimationSet().GetAnimResIds())
, x68_selfId(selfId)
, x70_cacheResPool(x6c_dummyFactory) {
  std::vector<CPrimitive> primitives;
  x28_animMgr->GetAnimationDatabase()->GetAllUniquePrimitives(primitives);
  x30_animSourceDB.reserve(primitives.size());
  for (const CPrimitive& prim : primitives)
    x30_animSourceDB.push_back(store.GetObj({SBIG('ANIM'), prim.GetAnimResId()}));
}

} // namespace urde
