#include "Runtime/MP1/World/CChozoGhost.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/CRandom16.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path
namespace urde::MP1 {
CChozoGhost::CBehaveChance::CBehaveChance(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_(in.readFloatBig())
, x8_(in.readFloatBig())
, xc_(in.readFloatBig())
, x10_(in.readFloatBig())
, x14_(in.readFloatBig())
, x18_(x0_propertyCount <= 5 ? 0.5f : in.readFloatBig() * .01f)
, x1c_(x0_propertyCount <= 6 ? 2 : in.readUint32Big()) {
  float f2 = 1.f / (x10_ + xc_ + x4_ + x8_);
  x4_ *= f2;
  x8_ *= f2;
  xc_ *= f2;
  x10_ *= f2;
}

u32 CChozoGhost::CBehaveChance::GetBehave(EBehaveType type, CStateManager& mgr) const {
  float dVar5 = x4_;
  float dVar4 = x8_;
  float dVar3 = xc_;
  if (type == EBehaveType::Zero) {
    float dVar2 = dVar5 / 3.f;
    dVar5 = 0.f;
    dVar4 += dVar2;
    dVar3 += dVar2;
  } else if (type == EBehaveType::One) {
    float dVar2 = dVar4 / 3.f;
    dVar4 = 0.f;
    dVar5 += dVar2;
    dVar3 += dVar2;
  } else if (type == EBehaveType::Two) {
    float dVar2 = dVar3 / 3.f;
    dVar3 = 0.f;
    dVar5 += dVar2;
    dVar4 += dVar2;
  } else if (type == EBehaveType::Three) {
    float dVar2 = x10_ / 3.f;
    dVar5 += dVar2;
    dVar4 += dVar2;
    dVar3 += dVar2;
  }

  float rnd = mgr.GetActiveRandom()->Float();
  if (dVar5 > rnd)
    return 0;
  else if (dVar4 > (rnd - dVar5))
    return 1;
  else if (dVar3 > (rnd - dVar5) - dVar4)
    return 2;
  return 3;
}

CChozoGhost::CChozoGhost(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, float f1,
                         float f2, float f3, float f4, CAssetId wpsc1, const CDamageInfo& dInfo1, CAssetId wpsc2,
                         const CDamageInfo& dInfo2, const CBehaveChance& chance1, const CBehaveChance& chance2,
                         const CBehaveChance& chance3, u16 sId1, float f5, u16 sId2, u16 sId3, u32 w1, float f6, u32 w2,
                         float f7, CAssetId partId, s16 sId4, float f8, float f9, u32 w3, u32 w4)
: CPatterned(ECharacter::ChozoGhost, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::Zero, EBodyType::BiPedal, actParms, EKnockBackVariant::Medium)
, x568_(f1)
, x56c_(f2)
, x570_(f3)
, x574_(f4)
, x578_(wpsc1, dInfo1)
, x5a0_(wpsc2, dInfo2)
, x5c8_(chance1)
, x5e8_(chance2)
, x608_(chance3)
, x628_(sId1)
, x62c_(f5)
, x630_(sId2)
, x632_(sId3)
, x634_(f6)
, x638_(f7)
, x63c_(w2)
, x650_(sId4)
, x654_(f8)
, x658_(f9)
, x65c_(w3)
, x660_(w4)
, x664_24_(w1)
, x664_25_(w1)
, x664_26_(false)
, x664_27_(false)
, x664_28_(false)
, x664_29_(false)
, x664_30_(false)
, x664_31_(false)
, x665_24_(true)
, x665_25_(false)
, x665_26_(false)
, x665_27_(false)
, x665_28_(false)
, x665_29_(false)
, x68c_boneTracking(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None) {}
} // namespace urde::MP1