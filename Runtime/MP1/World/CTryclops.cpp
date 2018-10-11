#include "CTryclops.hpp"

namespace urde::MP1
{
CTryclops::CTryclops(urde::TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
    urde::CModelData&& mData, const urde::CPatternedInfo& pInfo, const urde::CActorParameters& actParms, float, float, float, float)
: CPatterned(ECharacter::Tryclops, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
    EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, 0)
{}
}