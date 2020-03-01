#include "Runtime/MP1/World/CThardusRockProjectile.hpp"

namespace urde::MP1 {
CThardusRockProjectile::CThardusRockProjectile(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                               const zeus::CTransform& xf, CModelData&& modelData,
                                               const CActorParameters& aParms, const CPatternedInfo& patternedInfo,
                                               std::vector<std::unique_ptr<CModelData>>&& mDataVec,
                                               CAssetId stateMachine, float)
: CPatterned(ECharacter::ThardusRockProjectile, uid, name, EFlavorType::Zero, info, xf, std::move(modelData),
             patternedInfo, EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms,
             EKnockBackVariant::Medium)
, x57c_(std::move(mDataVec)) {}

} // namespace urde::MP1
