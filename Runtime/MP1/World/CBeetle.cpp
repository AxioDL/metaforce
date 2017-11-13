#include "CBeetle.hpp"
#include "World/CDamageInfo.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "TCastTo.hpp"

namespace urde
{
namespace MP1
{

CBeetle::CBeetle(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
                 const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor, CBeetle::EEntranceType, const CDamageInfo&,
                 const CDamageVulnerability&, const zeus::CVector3f&, float, float, float, const CDamageVulnerability&,
                 const CActorParameters& aParams, const rstl::optional_object<CStaticRes>)
    : CPatterned(ECharacter::Beetle, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Ground,
                 EColliderType::One, EBodyType::BiPedal, aParams, bool(flavor))
{

}

void CBeetle::Accept(IVisitor &visitor)
{
    visitor.Visit(this);
}

}
}
