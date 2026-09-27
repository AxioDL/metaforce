#include "MetroidPrime/ScriptObjects/CScriptControllerAction.hpp"

#include "MetroidPrime/CStateManager.hpp"

CScriptControllerAction::CScriptControllerAction(TUniqueId uid, const rstl::string& name,
                                                 const CEntityInfo& info, const bool active,
                                                 const ECommands command,
                                                 const bool mapScreenResponse, const uint w1,
                                                 const bool deactivateOnClose)
: CEntity(uid, info, active, name)
, mCommand(command)
, mMapScreenSubaction(w1)
, mMapScreenResponse(mapScreenResponse)
, mDeactivateOnClose(deactivateOnClose)
, mPressed(false) {}

ENTITY_ACCEPT_IMPL(CScriptControllerAction)

void CScriptControllerAction::Think(float dt, CStateManager& stateMgr) {
  bool oldPressed = mPressed;
  if (mMapScreenResponse) {
    switch (mMapScreenSubaction) {
    case 0:
      if (stateMgr.GetInMapScreen()) {
        mPressed = true;
      } else {
        mPressed = false;
      }
      break;
    default:
      break;
    }
  } else {
    if (ControlMapper::GetDigitalInput(GetCommand(mCommand), stateMgr.GetFinalInput())) {
      mPressed = true;
    } else {
      mPressed = false;
    }
  }

  if (GetActive() && mPressed != oldPressed) {
    if (mPressed) {
      SendScriptMsgs(kSS_Open, stateMgr, kSM_None);
    } else {
      SendScriptMsgs(kSS_Closed, stateMgr, kSM_None);
      if (mDeactivateOnClose) {
        SetActive(false);
        SendScriptMsgs(kSS_Inactive, stateMgr, kSM_None);
      }
    }
  }
}

ControlMapper::ECommands CScriptControllerAction::GetCommand(const ECommands cmd) {
  return static_cast< ControlMapper::ECommands >(cmd);
}
