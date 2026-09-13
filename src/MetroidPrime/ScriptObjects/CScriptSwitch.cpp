#include "MetroidPrime/ScriptObjects/CScriptSwitch.hpp"

CScriptSwitch::CScriptSwitch(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const bool active, const bool opened, const bool closeOnOpened)
: CEntity(uid, info, active, name) {
  mOpened = opened;
  mCloseOnOpened = closeOnOpened;
}

ENTITY_ACCEPT_IMPL(CScriptSwitch)

void CScriptSwitch::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  if (GetActive()) {
    switch (msg) {
    case kSM_Open:
      mOpened = true;
      break;
    case kSM_Close:
      mOpened = false;
      break;
    case kSM_SetToZero: {
      if (mOpened) {
        SendScriptMsgs(kSS_Open, mgr, kSM_None);
        if (mCloseOnOpened)
          mOpened = false;
      } else {
        SendScriptMsgs(kSS_Closed, mgr, kSM_None);
      }
      break;
    }
    }
  }
  CEntity::AcceptScriptMsg(msg, objId, mgr);
}

CScriptSwitch::~CScriptSwitch() {}
