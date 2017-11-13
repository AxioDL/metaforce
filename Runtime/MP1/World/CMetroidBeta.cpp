#include "CMetroidBeta.hpp"
#include "World/ScriptLoader.hpp"

namespace urde
{
namespace MP1
{

CMetroidBeta::CMetroidBeta(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                           const CActorParameters& aParms, const CMetroidData& metroidData)
: CPatterned(ECharacter::MetroidBeta, uid, name, EFlavorType::One, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms, 2)
{
}

void CMetroidBeta::RenderHitGunEffect() const
{

}

void CMetroidBeta::RenderHitBallEffect() const
{

}

}
}
