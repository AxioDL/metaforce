#include "CBurrower.hpp"
#include "CStateManager.hpp"

namespace urde::MP1 {

CBurrower::CBurrower(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, CAssetId,
                     CAssetId, CAssetId, const CDamageInfo&, CAssetId, u32, CAssetId)
: CPatterned(ECharacter::Burrower, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Small) {}

void CBurrower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  if (msg == EScriptObjectMessage::Registered)
    x450_bodyController->Activate(mgr);
  else if (msg == EScriptObjectMessage::InitializedInArea) {

  } else if (msg == EScriptObjectMessage::InvulnDamage)
    x6a4_ = 1.f;
}
} // namespace urde::MP1