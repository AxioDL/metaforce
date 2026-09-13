#include "MetroidPrime/ScriptObjects/CScriptControllerAction.hpp"

#include "MetroidPrime/CStateManager.hpp"

CScriptControllerAction::CScriptControllerAction(TUniqueId uid, const rstl::string& name,
                                                 const CEntityInfo& info, const bool active,
                                                 const ECommands command,
                                                 const bool mapScreenResponse, const uint w1,
                                                 const bool deactivateOnClose)
: CEntity(uid, info, active, name)
, x34_command(command)
, x38_mapScreenSubaction(w1)
, x3c_24_mapScreenResponse(mapScreenResponse)
, x3c_25_deactivateOnClose(deactivateOnClose)
, x3c_26_pressed(false) {}

ENTITY_ACCEPT_IMPL(CScriptControllerAction)

void CScriptControllerAction::Think(float dt, CStateManager& stateMgr) {
  bool oldPressed = x3c_26_pressed;
  if (x3c_24_mapScreenResponse) {
    switch (x38_mapScreenSubaction) {
    case 0:
      if (stateMgr.GetInMapScreen()) {
        x3c_26_pressed = true;
      } else {
        x3c_26_pressed = false;
      }
      break;
    default:
      break;
    }
  } else {
    if (ControlMapper::GetDigitalInput(GetCommand(x34_command), stateMgr.GetFinalInput())) {
      x3c_26_pressed = true;
    } else {
      x3c_26_pressed = false;
    }
  }

  if (GetActive() && x3c_26_pressed != oldPressed) {
    if (x3c_26_pressed) {
      SendScriptMsgs(kSS_Open, stateMgr, kSM_None);
    } else {
      SendScriptMsgs(kSS_Closed, stateMgr, kSM_None);
      if (x3c_25_deactivateOnClose) {
        SetActive(false);
        SendScriptMsgs(kSS_Inactive, stateMgr, kSM_None);
      }
    }
  }
}

ControlMapper::ECommands CScriptControllerAction::GetCommand(const ECommands cmd) {
  return static_cast< ControlMapper::ECommands >(cmd);
}
