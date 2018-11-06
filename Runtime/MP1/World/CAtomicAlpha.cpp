#include "CAtomicAlpha.hpp"

namespace urde::MP1
{
CAtomicAlpha::CAtomicAlpha(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                           CAssetId wpsc, const CDamageInfo& dInfo, float f1, float f2, float f3, CAssetId cmdl,
                           bool b1, bool b2)
    : CPatterned(ECharacter::AtomicAlpha, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
                 EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, actParms, EKnockBackVariant::Medium)
{

}
}
