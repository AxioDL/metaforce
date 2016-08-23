#include "CNewIntroBoss.hpp"
#include "Character/CCharLayoutInfo.hpp"

namespace urde
{
namespace MP1
{

CNewIntroBoss::CNewIntroBoss(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                             const CActorParameters& actParms, float, u32, const CDamageInfo& dInfo,
                             u32, u32, u32, u32)
: CPatterned(EUnknown::TwentyThree, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Two, actParms, true)
{
}

}
}
