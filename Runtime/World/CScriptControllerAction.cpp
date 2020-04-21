#include "Runtime/World/CScriptControllerAction.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Input/ControlMapper.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptControllerAction::CScriptControllerAction(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                                 bool active, ControlMapper::ECommands command, bool mapScreenResponse,
                                                 u32 w1, bool deactivateOnClose)
: CEntity(uid, info, active, name)
, x34_command(command)
, x38_mapScreenSubaction(w1)
, x3c_24_mapScreenResponse(mapScreenResponse)
, x3c_25_deactivateOnClose(deactivateOnClose) {}

void CScriptControllerAction::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptControllerAction::Think(float, CStateManager& stateMgr) {
  bool oldPressed = x3c_26_pressed;
  if (x3c_24_mapScreenResponse) {
    if (x38_mapScreenSubaction == 0)
      x3c_26_pressed = stateMgr.GetInMapScreen();
  } else {
    x3c_26_pressed = ControlMapper::GetDigitalInput(x34_command, stateMgr.GetFinalInput());
  }

  if (GetActive() && x3c_26_pressed != oldPressed) {
    if (x3c_26_pressed) {
      SendScriptMsgs(EScriptObjectState::Open, stateMgr, EScriptObjectMessage::None);
    } else {
      SendScriptMsgs(EScriptObjectState::Closed, stateMgr, EScriptObjectMessage::None);
      if (x3c_25_deactivateOnClose) {
        SetActive(false);
        SendScriptMsgs(EScriptObjectState::Inactive, stateMgr, EScriptObjectMessage::None);
      }
    }
  }
}

} // namespace urde
