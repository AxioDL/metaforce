#include "Runtime/World/CFishCloudModifier.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CFishCloud.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CFishCloudModifier::CFishCloudModifier(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& eInfo,
                                       const zeus::CVector3f& pos, bool isRepulsor, bool swirl, float radius,
                                       float priority)
: CActor(uid, active, name, eInfo, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         {EMaterialTypes::NoStepLogic}, CActorParameters::None(), kInvalidUniqueId)
, xe8_radius(radius)
, xec_priority(priority)
, xf0_isRepulsor(isRepulsor)
, xf1_swirl(swirl) {}

void CFishCloudModifier::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFishCloudModifier::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  if ((msg == EScriptObjectMessage::Activate || msg == EScriptObjectMessage::InitializedInArea) && GetActive())
    AddSelf(mgr);
  else if (msg == EScriptObjectMessage::Deactivate || msg == EScriptObjectMessage::Deleted)
    RemoveSelf(mgr);
}

void CFishCloudModifier::AddSelf(CStateManager& mgr) {
  for (const SConnection& conn : GetConnectionList()) {
    if (conn.x0_state != EScriptObjectState::Modify || conn.x4_msg != EScriptObjectMessage::Follow)
      continue;

    if (TCastToPtr<CFishCloud> fishCloud = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
      if (xf0_isRepulsor)
        fishCloud->AddRepulsor(GetUniqueId(), xf1_swirl, xe8_radius, xec_priority);
      else
        fishCloud->AddAttractor(GetUniqueId(), xf1_swirl, xe8_radius, xec_priority);
    }
  }
}

void CFishCloudModifier::RemoveSelf(CStateManager& mgr) {
  for (const SConnection& conn : GetConnectionList()) {
    if (conn.x0_state != EScriptObjectState::Modify || conn.x4_msg != EScriptObjectMessage::Follow)
      continue;

    if (TCastToPtr<CFishCloud> fishCloud = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
      if (xf0_isRepulsor)
        fishCloud->RemoveRepulsor(GetUniqueId());
      else
        fishCloud->RemoveAttractor(GetUniqueId());
    }
  }
}
} // namespace urde
