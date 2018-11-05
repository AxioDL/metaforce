#include "CPuddleToadGamma.hpp"

namespace urde::MP1
{

CPuddleToadGamma::CPuddleToadGamma(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                                   const CActorParameters& aParms, float f1, float f2, float f3,
                                   const zeus::CVector3f& v1,
                                   float f4, float f5, float f6, const CDamageInfo& dInfo1, const CDamageInfo& dInfo2)
: CPatterned(ECharacter::PuddleToad, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::Zero, EBodyType::Restricted, aParms, EKnockBackVariant::Two)
{

}

}
