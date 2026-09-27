#include "MetroidPrime/ScriptObjects/CScriptMemoryRelay.hpp"

#include "MetroidPrime/CScriptMailbox.hpp"

CScriptMemoryRelay::CScriptMemoryRelay(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, bool defaultActive,
                                       bool skipSendActive, bool ignoreMessages)
: CEntity(uid, info, true, name)
, mDefaultActive(defaultActive)
, mSkipSendActive(skipSendActive)
, mIgnoreMessages(ignoreMessages) {}

CScriptMemoryRelay::~CScriptMemoryRelay() {}

void CScriptMemoryRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  if (mIgnoreMessages) {
    return;
  }

  switch (msg) {
    case kSM_Activate:
      stateMgr.Mailbox()->AddMsg(GetEditorId());
      if (!mSkipSendActive) {
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
