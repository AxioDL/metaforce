#ifndef __PSHAG_CSCRIPTMAILBOX_HPP__
#define __PSHAG_CSCRIPTMAILBOX_HPP__

#include "IOStreams.hpp"
#include "ScriptObjectSupport.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CStateManager;
struct CMailMessage
{
    TEditorId x0_id;
    EScriptObjectMessage x4_msg;
    bool x8_;
    CMailMessage(TEditorId id, EScriptObjectMessage msg, bool flag) : x0_id(id), x4_msg(msg), x8_(flag) {}
    CMailMessage(const CMailMessage& other) : x0_id(other.x0_id), x4_msg(other.x4_msg), x8_(other.x8_) {}

    bool operator==(const CMailMessage& other) const
    { return (x0_id == other.x0_id && x4_msg == other.x4_msg); }
};

class CScriptMailbox
{
    rstl::reserved_vector<CMailMessage, 1024> x0_messages;
public:
    CScriptMailbox() = default;
    CScriptMailbox(CBitStreamReader&);

    void AddMsg(TEditorId, EScriptObjectMessage, bool);
    void RemoveMsg(TEditorId, EScriptObjectMessage, bool);
    void SendScriptMsgs(const TAreaId&, CStateManager&);
    void PutTo(CBitStreamWriter&);
};

}

#endif // __PSHAG_CSCRIPTMAILBOX_HPP__
