#include "CChozoGhost.hpp"

namespace urde::MP1
{
CChozoGhost::CBehaveChance::CBehaveChance(CInputStream& in)
    : x0_propertyCount(in.readUint32Big()), x4_(in.readFloatBig()), x8_(in.readFloatBig()), xc_(in.readFloatBig()),
      x10_(in.readFloatBig()), x14_(in.readFloatBig()), x18_(x0_propertyCount <= 5 ? 0.5f : in.readFloatBig() * .01f),
      x1c_(x0_propertyCount <= 6 ? 2 : in.readUint32Big())
{
    float f2 = 1.f / (x10_ + xc_ + x4_ + x8_);
    x4_ *= f2;
    x8_ *= f2;
    xc_ *= f2;
    x10_ *= f2;
}


CChozoGhost::CChozoGhost(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, float f1,
                         float f2, float f3, float f4, CAssetId wpsc1, const CDamageInfo& dInfo1, CAssetId wpsc2,
                         const CDamageInfo& dInfo2, const CBehaveChance& chance1, const CBehaveChance& chance2,
                         const CBehaveChance& chance3, u16 sId1, float f5, u16 sId2, u16 sId3, u32 w1, float f6, u32 w2,
                         float f7, CAssetId partId, s16 sId4, float f8, float f9, u32 w3, u32 w4)
    : CPatterned(ECharacter::ChozoGhost, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
                 EMovementType::Flyer, EColliderType::Zero, EBodyType::BiPedal, actParms, EKnockBackVariant::One)
{

}
}