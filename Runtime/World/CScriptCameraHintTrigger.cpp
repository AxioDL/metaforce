#include "Runtime/World/CScriptCameraHintTrigger.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptCameraHintTrigger::CScriptCameraHintTrigger(TUniqueId uid, bool active, std::string_view name,
                                                   const CEntityInfo& info, const zeus::CVector3f& scale,
                                                   const zeus::CTransform& xf, bool deactivateOnEnter,
                                                   bool deactivateOnExit)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Trigger),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_obb(xf, scale)
, x124_scale(scale)
, x130_24_deactivateOnEnter(deactivateOnEnter)
, x130_25_deactivateOnExit(deactivateOnExit) {}

void CScriptCameraHintTrigger::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptCameraHintTrigger::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (x130_26_playerInside && !x130_27_playerWasInside) {
    x130_27_playerWasInside = true;
    SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);
    if (x130_24_deactivateOnEnter)
      mgr.SendScriptMsg(this, kInvalidUniqueId, EScriptObjectMessage::Deactivate);
  }

  if (!x130_26_playerInside && x130_27_playerWasInside) {
    x130_27_playerWasInside = false;
    SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
    if (x130_25_deactivateOnExit)
      mgr.SendScriptMsg(this, kInvalidUniqueId, EScriptObjectMessage::Deactivate);
  }

  if (x130_26_playerInside)
    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);

  x130_26_playerInside = false;
}

void CScriptCameraHintTrigger::Touch(CActor& other, CStateManager& mgr) {
  if (TCastToPtr<CPlayer>(other)) {
    if (auto tb = other.GetTouchBounds())
      x130_26_playerInside = xe8_obb.OBBIntersectsBox(zeus::COBBox::FromAABox(*tb, {}));
  }
}

std::optional<zeus::CAABox> CScriptCameraHintTrigger::GetTouchBounds() const {
  return {xe8_obb.calculateAABox()};
}

} // namespace urde
