#include "CScriptMemoryRelay.hpp"
#include "CStateManager.hpp"
#include "CRelayTracker.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptMemoryRelay::CScriptMemoryRelay(TUniqueId uid, const std::string& name, const CEntityInfo& info, bool b1, bool b2, bool b3)
    : CEntity(uid, info, true, name),
      x34_24_(b1),
      x34_25_skipSendActive(b2),
      x34_26_ignoreMessages(b3)
{
}

void CScriptMemoryRelay::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptMemoryRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr)
{
    if (x34_26_ignoreMessages)
        return;

    if (msg == EScriptObjectMessage::Deactivate)
    {
        stateMgr.GetRelayTracker()->RemoveRelay(xc_editorId);
        return;
    }
    else if (msg == EScriptObjectMessage::Activate)
    {
        stateMgr.GetRelayTracker()->AddRelay(xc_editorId);
        if (!x34_25_skipSendActive)
            SendScriptMsgs(EScriptObjectState::Active, stateMgr, EScriptObjectMessage::None);

        return;
    }

    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

}
