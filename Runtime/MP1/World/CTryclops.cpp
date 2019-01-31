#include "CTryclops.hpp"
#include "CStateManager.hpp"
#include "World/CWorld.hpp"
#include "World/CGameArea.hpp"

namespace urde::MP1 {
CTryclops::CTryclops(urde::TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     urde::CModelData&& mData, const urde::CPatternedInfo& pInfo,
                     const urde::CActorParameters& actParms, float, float, float, float)
: CPatterned(ECharacter::Tryclops, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Small) {}

void CTryclops::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
  } else if (msg == EScriptObjectMessage::InitializedInArea) {
    //x568_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
  }
}
} // namespace urde::MP1