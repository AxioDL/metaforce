#ifndef __URDE_CRELAYTRACKER_HPP__
#define __URDE_CRELAYTRACKER_HPP__

#include "IOStreams.hpp"
#include "World/ScriptObjectSupport.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CStateManager;
class CSaveWorld;
#if 0
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
#endif


class CRelayTracker
{
    std::vector<TEditorId> x0_relayStates;
public:
    CRelayTracker() = default;
    CRelayTracker(CBitStreamReader&, const CSaveWorld&);

    bool HasRelay(TEditorId);
    void AddRelay(TEditorId);
    void RemoveRelay(TEditorId);
    void SendMsgs(const TAreaId&, CStateManager&);
    void PutTo(CBitStreamWriter&, const CSaveWorld&);
};

}

#endif // __URDE_CRELAYTRACKER_HPP__
