#include "Runtime/MP1/World/CPhazonHealingNodule.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPatternedInfo.hpp"

namespace urde::MP1 {
CPhazonHealingNodule::CPhazonHealingNodule(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                           const zeus::CTransform& xf, CModelData&& mData,
                                           const CActorParameters& actParams, const CPatternedInfo& pInfo,
                                           CAssetId particleDescId, std::string str)
: CPatterned(ECharacter::PhazonHealingNodule, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Restricted, actParams, EKnockBackVariant::Medium)
, x570_(g_SimplePool->GetObj(SObjectTag{SBIG('ELSC'), particleDescId}))
, x580_(pInfo.GetHealthInfo())
, x58c_(std::move(str)) {
  const CMaterialFilter& filter = GetMaterialFilter();
  CMaterialList include = filter.GetIncludeList();
  CMaterialList exclude = filter.GetExcludeList();
  exclude.Add(EMaterialTypes::Character);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
}
} // namespace urde::MP1
