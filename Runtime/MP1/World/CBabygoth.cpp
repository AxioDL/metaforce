#include "CBabygoth.hpp"

namespace urde::MP1 {
CBabygothData::CBabygothData(CInputStream& in)
: x0_(in.readFloatBig())
, x4_(in.readFloatBig())
, x8_(in)
, xc_(in)
, x28_(in)
, x44_(in)
, x48_(in)
, x4c_(in)
, x68_(in)
, xd0_(in)
, x138_(in)
, x13c_(in)
, x140_(in.readFloatBig())
, x144_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x148_(in)
, x14c_(in)
, x150_(in)
, x154_(in)
, x158_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x15c_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x160_(in.readFloatBig())
, x164_(in.readFloatBig())
, x168_(in.readFloatBig())
, x16c_(in)
, x170_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x174_(in) {}

CBabygoth::CBabygoth(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                     const CBabygothData& babyData)
: CPatterned(ECharacter::Babygoth, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Medium) {}
} // namespace urde::MP1
