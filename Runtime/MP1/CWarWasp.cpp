#include "CWarWasp.hpp"

namespace urde
{
namespace MP1
{
CWarWasp::CWarWasp(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor,
                   CPatterned::EColliderType collider, const CDamageInfo& dInfo1, const CActorParameters& actorParms,
                   ResId weapon, const CDamageInfo& dInfo2, ResId particle, u32 w3)
    : CPatterned(EUnknown::ThirtyNine, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer, collider, EBodyType::Three,
                 actorParms, false)
{

}
}

}
