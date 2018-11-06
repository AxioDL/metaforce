#include "CEyeball.hpp"

namespace urde::MP1
{
CEyeball::CEyeball(TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, float, float,
                   CAssetId, const CDamageInfo&, CAssetId, CAssetId, CAssetId, CAssetId, u32, u32, u32, u32, u32,
                   const CActorParameters& actParms, bool)
: CPatterned(ECharacter::EyeBall, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
                 EColliderType::Zero, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
{

}
}