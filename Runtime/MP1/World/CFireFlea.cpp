#include "CFireFlea.hpp"

namespace urde::MP1
{

CFireFlea::CFireFlea(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, float)
    : CPatterned(ECharacter::FireFlea, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
                 EMovementType::Flyer, EColliderType::One, EBodyType::NewFlyer, actParms, EKnockBackVariant::Zero)
{

}
}