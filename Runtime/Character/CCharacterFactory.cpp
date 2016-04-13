#include "CCharacterFactory.hpp"
#include "CAnimCharacterSet.hpp"
#include "CSimplePool.hpp"
#include "CTransitionDatabaseGame.hpp"
#include "CAnimationDatabaseGame.hpp"
#include "CAnimationManager.hpp"
#include "CTransitionManager.hpp"
#include "CRandom16.hpp"
#include "CPrimitive.hpp"

namespace urde
{

CFactoryFnReturn CCharacterFactory::CDummyFactory::Build(const SObjectTag&, const CVParamTransfer&)
{
    return {};
}

void CCharacterFactory::CDummyFactory::BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**)
{
}

void CCharacterFactory::CDummyFactory::CancelBuild(const SObjectTag&)
{
}

bool CCharacterFactory::CDummyFactory::CanBuild(const SObjectTag&)
{
    return true;
}

const SObjectTag* CCharacterFactory::CDummyFactory::GetResourceIdByName(const char*) const
{
    return nullptr;
}

TResId CCharacterFactory::GetEventResourceIdForAnimResourceId(TResId) const
{
}

std::vector<CCharacterInfo>
CCharacterFactory::GetCharacterInfoDB(const CAnimCharacterSet& ancs)
{
    std::vector<CCharacterInfo> ret;
    const std::map<u32, CCharacterInfo>& charInfoMap = ancs.GetCharacterSet().GetCharacterInfoMap();
    ret.reserve(charInfoMap.size());
    for (const auto& charInfo : charInfoMap)
        ret.push_back(charInfo.second);
    return ret;
}

std::vector<TLockedToken<CCharLayoutInfo>>
CCharacterFactory::GetCharLayoutInfoDB(CSimplePool& store,
                                       const std::vector<CCharacterInfo>& chars)
{
    std::vector<TLockedToken<CCharLayoutInfo>> ret;
    ret.reserve(chars.size());
    for (const CCharacterInfo& charInfo : chars)
        ret.push_back(store.GetObj({SBIG('CINF'), charInfo.GetCharLayoutInfoId()}));
    return ret;
}

CCharacterFactory::CCharacterFactory(CSimplePool& store,
                                     const CAnimCharacterSet& ancs,
                                     TResId selfId)
: x4_charInfoDB(GetCharacterInfoDB(ancs)),
  x14_charLayoutInfoDB(GetCharLayoutInfoDB(store, x4_charInfoDB)),
  x24_sysContext(std::make_shared<CAnimSysContext>(
                 TToken<CTransitionDatabaseGame>(
                 std::make_unique<CTransitionDatabaseGame>(ancs.GetAnimationSet().GetTransitions(),
                                                           ancs.GetAnimationSet().GetHalfTransitions(),
                                                           ancs.GetAnimationSet().GetDefaultTransition())),
                     2334, store)),
  x28_animMgr(std::make_shared<CAnimationManager>(
              TToken<CAnimationDatabaseGame>(
              std::make_unique<CAnimationDatabaseGame>(
              ancs.GetAnimationSet().GetAnimations())), *x24_sysContext)),
  x2c_transMgr(std::make_shared<CTransitionManager>(*x24_sysContext)),
  x40_additiveInfo(ancs.GetAnimationSet().GetAdditiveInfo()),
  x50_defaultAdditiveInfo(ancs.GetAnimationSet().GetDefaultAdditiveInfo()),
  x58_animResources(ancs.GetAnimationSet().GetAnimResIds()),
  x68_selfId(selfId),
  x70_cacheResPool(x6c_dummyFactory)
{
    std::vector<CPrimitive> primitives;
    x28_animMgr->GetAnimationDatabase()->GetAllUniquePrimitives(primitives);
    x30_animSourceDB.reserve(primitives.size());
    for (const CPrimitive& prim : primitives)
        x30_animSourceDB.push_back(store.GetObj({SBIG('ANIM'), prim.GetAnimResId()}));
}

}
