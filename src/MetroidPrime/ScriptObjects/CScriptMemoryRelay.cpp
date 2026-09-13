#include "MetroidPrime/ScriptObjects/CScriptMemoryRelay.hpp"

#include "MetroidPrime/CScriptMailbox.hpp"

CScriptMemoryRelay::CScriptMemoryRelay(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, bool defaultActive,
                                       bool skipSendActive, bool ignoreMessages)
: CEntity(uid, info, true, name)
, x34_24_defaultActive(defaultActive)
, x34_25_skipSendActive(skipSendActive)
, x34_26_ignoreMessages(ignoreMessages) {}

CScriptMemoryRelay::~CScriptMemoryRelay() {}

void CScriptMemoryRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  if (x34_26_ignoreMessages) {
    return;
  }

  switch (msg) {
    case kSM_Activate:
      stateMgr.Mailbox()->AddMsg(GetEditorId());
      if (!x34_25_skipSendActive) {
        SendScriptMsgs(kSS_Active, stateMgr, kSM_None);
      }
      break;

    case kSM_Deactivate:
      stateMgr.Mailbox()->RemoveMsg(GetEditorId());
      break;
    
    default:
      CEntity::AcceptScriptMsg(msg, objId, stateMgr);
      break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptMemoryRelay)
