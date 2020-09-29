#include "Runtime/World/CScriptCameraWaypoint.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptCameraWaypoint::CScriptCameraWaypoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                             const zeus::CTransform& xf, bool active, float hfov, u32 w1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_hfov(hfov)
, xec_(w1) {}

void CScriptCameraWaypoint::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptCameraWaypoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  if (GetActive() && msg == EScriptObjectMessage::Arrived)
    SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
}

TUniqueId CScriptCameraWaypoint::GetRandomNextWaypointId(CStateManager& mgr) const {
  std::vector<TUniqueId> candidateIds;
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next) {
      TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
      if (uid == kInvalidUniqueId)
        continue;
      candidateIds.push_back(uid);
    }
  }

  if (candidateIds.empty())
    return kInvalidUniqueId;

  return candidateIds[mgr.GetActiveRandom()->Range(0, s32(candidateIds.size() - 1))];
}

} // namespace urde
