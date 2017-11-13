#include "CScriptSwitch.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptSwitch::CScriptSwitch(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active, bool opened,
                             bool closeOnOpened)
: CEntity(uid, info, active, name), x34_opened(opened), x35_closeOnOpened(closeOnOpened)
{
}

void CScriptSwitch::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptSwitch::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr)
{
    if (GetActive())
    {
        if (msg == EScriptObjectMessage::Open)
            x34_opened = true;
        else if (msg == EScriptObjectMessage::Close)
            x34_opened = false;
        else if (msg == EScriptObjectMessage::SetToZero)
        {
            if (x34_opened)
            {
                SendScriptMsgs(EScriptObjectState::Open, mgr, EScriptObjectMessage::None);
                if (x35_closeOnOpened)
                    x34_opened = false;
            }
            else
                SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);
        }
    }

    CEntity::AcceptScriptMsg(msg, objId, mgr);
}
}
