#pragma once

#include <vector>

#include "Runtime/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/ScriptObjectSupport.hpp"

namespace urde {
class CSaveWorld;
class CStateManager;

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

class CRelayTracker {
  std::vector<TEditorId> x0_relayStates;

public:
  CRelayTracker() = default;
  CRelayTracker(CBitStreamReader&, const CSaveWorld&);

  bool HasRelay(TEditorId);
  void AddRelay(TEditorId);
  void RemoveRelay(TEditorId);
  void SendMsgs(TAreaId, CStateManager&);
  void PutTo(CBitStreamWriter&, const CSaveWorld&);
};

} // namespace urde
