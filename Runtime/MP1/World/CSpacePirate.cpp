#include "CSpacePirate.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "TCastTo.hpp"

namespace urde::MP1
{
CSpacePirate::CSpacePirateData::CSpacePirateData(urde::CInputStream& in, u32 propCount)
: x0_(in.readFloatBig()), x4_(in.readFloatBig()), x8_(in.readFloatBig()), xc_(in.readFloatBig())
, x10_(in.readFloatBig()), x14_(in.readFloatBig()), x18_(in.readUint32Big()), x1c_(in.readBool()), x20_(in)
, x48_(CSfxManager::TranslateSFXID(in.readUint32Big())), x4c_(in), x68_(in.readFloatBig()), x6c_(in)
, x94_(in.readFloatBig()), x98_(CSfxManager::TranslateSFXID(in.readUint32Big())), x9c_(in.readFloatBig())
, xa0_(in.readFloatBig()), xa4_(CSfxManager::TranslateSFXID(in.readUint32Big())), xa8_(in.readFloatBig())
, xac_(in.readUint32Big()), xb0_(in.readFloatBig()), xb4_(in.readFloatBig()), xb8_(in.readFloatBig())
, xbc_(in.readFloatBig()), xc0_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xc2_(CSfxManager::TranslateSFXID(in.readUint32Big())), xc4_(propCount > 35 ? in.readFloatBig() : 0.2f)
, xc8_(propCount > 36 ? in.readFloatBig() : 8.f)
{

}

CSpacePirate::CSpacePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CActorParameters& aParams, const CPatternedInfo& pInfo,
                           CInputStream& in, u32 propCount)
: CPatterned(ECharacter::SpacePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo, EMovementType::Ground,
                 EColliderType::One, EBodyType::BiPedal, aParams, true)
, x568_(in, propCount)
{
}

void CSpacePirate::Accept(IVisitor &visitor)
{
    visitor.Visit(this);
}

}
