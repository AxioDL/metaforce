#include "CBloodFlower.hpp"

namespace urde::MP1 {
CBloodFlower::CBloodFlower(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, CAssetId partId1, CAssetId wpscId1,
                           const CActorParameters& actParms, CAssetId wpscId2, const CDamageInfo& dInfo1,
                           const CDamageInfo& dInfo2, const CDamageInfo& dInfo3, CAssetId partId2, CAssetId partId3,
                           CAssetId partId4, float f1, CAssetId partId5, u32 soundId)
: CPatterned(ECharacter::BloodFlower, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::Restricted, actParms, EKnockBackVariant::Medium) {}
} // namespace urde::MP1