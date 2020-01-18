#include "Runtime/MP1/World/CMagdolite.hpp"

namespace urde::MP1 {

CMagdolite::CMagdoliteData::CMagdoliteData(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_(in.readUint32Big())
, x8_(in)
, xc_(in.readUint32Big())
, x10_(in.readFloatBig())
, x18_(in.readFloatBig())
, x1c_(in.readFloatBig()) {}

CMagdolite::CMagdolite(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, float f1,
                       float f2, const CDamageInfo& dInfo1, const CDamageInfo& dInfo2,
                       const CDamageVulnerability& dVuln1, const CDamageVulnerability& dVuln2, CAssetId modelId,
                       CAssetId skinId, float f3, float f4, float f5, float f6,
                       const CMagdolite::CMagdoliteData& magData, float f7, float f8, float f9)
: CPatterned(ECharacter::Magdolite, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x568_(f4)
, x56c_(f5)
, x570_(f6)
, x574_(f3)
, x578_(std::cos(zeus::degToRad(f2)))
, x57c_(f1)
, x584_boneTracker(*GetModelData()->GetAnimationData(), "head"sv, zeus::degToRad(f1), zeus::degToRad(90.f),
  EBoneTrackingFlags::ParentIk) {

}

void CMagdolite::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {

  switch (msg) {
  case EScriptObjectMessage::Damage:
  case EScriptObjectMessage::InvulnDamage:
    /* TODO Implement */
    return;
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    AddMaterial(EMaterialTypes::NonSolidDamageable, mgr);
    x584_boneTracker.SetActive(false);
    CreateShadow(false);
    /* TODO Finish */
    break;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}
} // namespace urde::MP1
