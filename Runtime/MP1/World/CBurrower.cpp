#include "CBurrower.hpp"

namespace urde::MP1
{

CBurrower::CBurrower(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                     CAssetId, CAssetId, CAssetId, const CDamageInfo&, CAssetId, u32, CAssetId)
: CPatterned(ECharacter::Burrower, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
    EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, 0)
{

}
}