#include "CScriptMemoryRelay.hpp"
#include "CStateManager.hpp"
#include "CRelayTracker.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptMemoryRelay::CScriptMemoryRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool defaultActive,
                                       bool skipSendActive, bool ignoreMessages)
: CEntity(uid, info, true, name)
, x34_24_defaultActive(defaultActive)
, x34_25_skipSendActive(skipSendActive)
, x34_26_ignoreMessages(ignoreMessages) {}

void CScriptMemoryRelay::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptMemoryRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  if (x34_26_ignoreMessages)
    return;

  if (msg == EScriptObjectMessage::Deactivate) {
    stateMgr.GetRelayTracker()->RemoveRelay(xc_editorId);
    return;
  } else if (msg == EScriptObjectMessage::Activate) {
    stateMgr.GetRelayTracker()->AddRelay(xc_editorId);
    if (!x34_25_skipSendActive)
      SendScriptMsgs(EScriptObjectState::Active, stateMgr, EScriptObjectMessage::None);

    return;
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

} // namespace urde
