#include "CSpacePirate.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "TCastTo.hpp"

namespace urde
{
namespace MP1
{
CSpacePirate::CSpacePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CActorParameters& aParams, const CPatternedInfo& pInfo, CInputStream& in,
                           u32 propCount)
    : CPatterned(ECharacter::SpacePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo, EMovementType::Ground,
                 EColliderType::One, EBodyType::BiPedal, aParams, true)
{
}

void CSpacePirate::Accept(IVisitor &visitor)
{
    visitor.Visit(this);
}

}
}
