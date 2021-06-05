#pragma once

#include <vector>

#include "Runtime/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/ScriptObjectSupport.hpp"

namespace metaforce {
class CWorldSaveGameInfo;
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

class CScriptMailbox {
  std::vector<TEditorId> x0_relays;

public:
  CScriptMailbox() = default;
  CScriptMailbox(CBitStreamReader& in, const CWorldSaveGameInfo& saveWorld);

  bool HasMsg(TEditorId id) const;
  void AddMsg(TEditorId id);
  void RemoveMsg(TEditorId id);
  void SendMsgs(TAreaId areaId, CStateManager& stateMgr);
  void PutTo(CBitStreamWriter& out, const CWorldSaveGameInfo& saveWorld);
};

} // namespace metaforce
