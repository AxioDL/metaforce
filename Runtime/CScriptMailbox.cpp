#include "CScriptMailbox.hpp"
#include "CStateManager.hpp"

namespace urde
{

CScriptMailbox::CScriptMailbox(CBitStreamReader&)
{
}

void CScriptMailbox::AddMsg(TEditorId id, EScriptObjectMessage msg, bool flag)
{
    /* TODO: Verify this behavior */
    CMailMessage mail{id, msg, flag};
    auto it = std::find(x0_messages.begin(), x0_messages.end(), mail);

    if (it != x0_messages.end())
        *it = mail;
    else
        x0_messages.push_back(mail);
}

void CScriptMailbox::RemoveMsg(TEditorId id, EScriptObjectMessage msg, bool flag)
{
    CMailMessage mail{id, msg, flag};
    auto it = std::find(x0_messages.begin(), x0_messages.end(), mail);
    if (it != x0_messages.end())
        x0_messages.erase(it);
}

void CScriptMailbox::SendMsgs(const TAreaId& areaId, CStateManager& stateMgr)
{
}

void CScriptMailbox::PutTo(CBitStreamWriter&)
{

}

}
