#include "CSpacePirate.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "TCastTo.hpp"

namespace urde
{
namespace MP1
{
CSpacePirate::CSpacePirate(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CActorParameters& aParams, const CPatternedInfo& pInfo, CInputStream& in,
                           u32 propCount)
    : CPatterned(EUnknown::ThirtyFour, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo, EMovementType::Ground,
                 EColliderType::One, EBodyType::One, aParams, true)
{
}

}
}
