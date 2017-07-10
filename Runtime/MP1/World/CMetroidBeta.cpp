#include "CMetroidBeta.hpp"
#include "World/ScriptLoader.hpp"

namespace urde
{
namespace MP1
{

CMetroidBeta::CMetroidBeta(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                           const CActorParameters& aParms, const CMetroidData& metroidData)
: CPatterned(ECharacter::MetroidBeta, uid, name, EFlavorType::One, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms, 2)
{
}

}
}
