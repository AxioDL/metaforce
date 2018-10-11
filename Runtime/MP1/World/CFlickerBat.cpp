#include "CFlickerBat.hpp"

namespace urde::MP1
{

CFlickerBat::CFlickerBat(TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor, const CEntityInfo& info,
                         const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                         EColliderType colType, bool b1, const CActorParameters& actParms, bool b2)
: CPatterned(ECharacter::FlickerBat, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer, colType,
    EBodyType::Pitchable, actParms, 0)
{

}
}