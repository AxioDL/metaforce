#include "CWarWasp.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "TCastTo.hpp"

namespace urde::MP1
{
CWarWasp::CWarWasp(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor,
                   CPatterned::EColliderType collider, const CDamageInfo& dInfo1, const CActorParameters& actorParms,
                   CAssetId weapon, const CDamageInfo& dInfo2, CAssetId particle, u32 w3)
: CPatterned(ECharacter::WarWasp, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer, collider,
             EBodyType::Flyer, actorParms, false)
{
}

void CWarWasp::Accept(IVisitor& visitor) { visitor.Visit(this); }
}
