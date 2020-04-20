#include "Runtime/World/CScriptAiJumpPoint.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptAiJumpPoint::CScriptAiJumpPoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                       zeus::CTransform& xf, bool active, float apex)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_apex(apex)
, xec_touchBounds(xf.origin, xf.origin) {}

void CScriptAiJumpPoint::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptAiJumpPoint::Think(float dt, CStateManager&) {
  if (x110_timeRemaining <= 0)
    return;

  x110_timeRemaining -= dt;
}

void CScriptAiJumpPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, other, mgr);

  if (msg != EScriptObjectMessage::InitializedInArea)
    return;

  for (SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Arrived || conn.x4_msg != EScriptObjectMessage::Next)
      continue;

    const CScriptWaypoint* wpnt =
        static_cast<const CScriptWaypoint*>(mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)));
    if (wpnt) {
      x10c_currentWaypoint = wpnt->GetUniqueId();
      x10e_nextWaypoint = wpnt->NextWaypoint(mgr);
    }
  }
}

std::optional<zeus::CAABox> CScriptAiJumpPoint::GetTouchBounds() const { return xec_touchBounds; }

bool CScriptAiJumpPoint::GetInUse(TUniqueId uid) const {
  return x108_24 || x110_timeRemaining > 0.f || x10a_occupant != kInvalidUniqueId || uid != kInvalidUniqueId ||
      uid != x10a_occupant;
}
} // namespace urde
