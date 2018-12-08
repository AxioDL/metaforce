#include "CScriptCounter.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde {

CScriptCounter::CScriptCounter(TUniqueId uid, std::string_view name, const CEntityInfo& info, s32 initial, s32 max,
                               bool autoReset, bool active)
: CEntity(uid, info, active, name)
, x34_initial(initial)
, x38_current(initial)
, x3c_max(max)
, x40_autoReset(autoReset) {}

void CScriptCounter::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptCounter::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  switch (msg) {
  case EScriptObjectMessage::SetToZero:
    if (GetActive()) {
      x38_current = 0;
      SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);

      if (x40_autoReset)
        x38_current = x34_initial;
    }
    break;
  case EScriptObjectMessage::SetToMax:
    if (GetActive()) {
      x38_current = x3c_max;
      SendScriptMsgs(EScriptObjectState::MaxReached, stateMgr, EScriptObjectMessage::None);

      if (x40_autoReset)
        x38_current = x34_initial;
    }
    break;
  case EScriptObjectMessage::Decrement:
    if (GetActive() && x38_current > 0) {
      --x38_current;
      if (x38_current == 0) {
        SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
        if (x40_autoReset)
          x38_current = x34_initial;
      }
    }
    break;
  case EScriptObjectMessage::Increment:
    if (GetActive() && x38_current < x3c_max) {
      ++x38_current;
      if (x38_current >= x3c_max) {
        SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
        if (x40_autoReset)
          x38_current = x34_initial;
      }
    }
    break;
  case EScriptObjectMessage::Reset:
    if (GetActive())
      x38_current = x34_initial;
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

} // namespace urde
