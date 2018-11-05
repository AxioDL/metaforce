#include "CMetroidBeta.hpp"
#include "World/ScriptLoader.hpp"

namespace urde::MP1
{

CMetroidBetaData::CMetroidBetaData(CInputStream& in)
: x0_(in)
, x68_(in)
, xd0_(in.readFloatBig())
, xd4_(in.readFloatBig())
, xd8_(in.readFloatBig())
, xdc_(in.readFloatBig())
, xe0_(in.readFloatBig())
, xe4_(in.readFloatBig())
, xe8_(in.readFloatBig())
, xec_(in.readFloatBig())
, xf0_(in.readFloatBig())
, xf4_(in)
, xf8_(in)
, xfc_(in)
, x100_(in)
, x104_(in)
, x108_24_(in.readBool())
{
}

CMetroidBeta::CMetroidBeta(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                           const CActorParameters& aParms, const CMetroidBetaData& metroidData)
: CPatterned(ECharacter::MetroidBeta, uid, name, EFlavorType::One, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms, EKnockBackVariant::Two)
{
}

void CMetroidBeta::RenderHitGunEffect() const
{

}

void CMetroidBeta::RenderHitBallEffect() const
{

}
}
