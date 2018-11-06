#include "CNewIntroBoss.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "TCastTo.hpp"

namespace urde::MP1
{

CNewIntroBoss::CNewIntroBoss(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                             const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                             const CActorParameters& actParms, float, u32, const CDamageInfo& dInfo,
                             u32, u32, u32, u32)
: CPatterned(ECharacter::NewIntroBoss, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
{
}

void CNewIntroBoss::Accept(IVisitor &visitor)
{
    visitor.Visit(this);
}

}
