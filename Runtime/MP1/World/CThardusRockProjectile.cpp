#include "CThardusRockProjectile.hpp"

namespace urde
{

CThardusRockProjectile::CThardusRockProjectile(
        TUniqueId uid, std::string_view name, const CEntityInfo& info,
        const zeus::CTransform& xf, CModelData&& modelData, const CActorParameters& aParms,
        const CPatternedInfo& patternedInfo, const std::vector<CModelData>& mDataVec, u32)
: CPatterned(ECharacter::ThardusRockProjectile, uid, name, EFlavorType::Zero, info, xf, std::move(modelData),
             patternedInfo, EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms, EKnockBackVariant::One)
{

}

}
