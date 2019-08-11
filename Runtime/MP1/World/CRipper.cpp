#include "MP1/World/CRipper.hpp"

namespace urde::MP1 {
CRipper::CRipper(TUniqueId uid, std::string_view name, EFlavorType type, const CEntityInfo& info,
    const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
    const CGrappleParameters& grappleParms)
: CPatterned(ECharacter::Ripper, uid, name, type, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
    EColliderType::One, EBodyType::Flyer, actParms, EKnockBackVariant::Medium) {}
}