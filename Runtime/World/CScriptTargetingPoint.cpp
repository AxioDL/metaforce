#include "World/CScriptTargetingPoint.hpp"
#include "World/CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde {

CScriptTargetingPoint::CScriptTargetingPoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                             const zeus::CTransform& xf, bool active)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId) {}

void CScriptTargetingPoint::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptTargetingPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Deactivate || msg == EScriptObjectMessage::Activate)
    CEntity::SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
}

void CScriptTargetingPoint::Think(float dt, CStateManager&) {
  if (xec_time <= 0.f)
    return;

  xec_time -= dt;
}

bool CScriptTargetingPoint::GetLocked() const { return !x20_conns.empty(); }
} // namespace urde
