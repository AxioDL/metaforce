#include "Runtime/CScriptMailbox.hpp"

#include "Runtime/CSaveWorld.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CWorld.hpp"

#include <algorithm>

namespace metaforce {

CScriptMailbox::CScriptMailbox(CBitStreamReader& in, const CSaveWorld& saveWorld) {
  const u32 relayCount = saveWorld.GetRelayCount();
  if (saveWorld.GetRelayCount()) {
    std::vector<bool> relayStates(saveWorld.GetRelayCount());
    for (u32 i = 0; i < relayCount; ++i) {
      relayStates[i] = in.ReadEncoded(1);
    }

    for (u32 i = 0; i < relayCount; ++i) {
      if (!relayStates[i]) {
        continue;
        }
        x0_relays.push_back(saveWorld.GetRelayEditorId(i));
    }
  }
}

bool CScriptMailbox::HasMsg(TEditorId id) const {
  return std::find(x0_relays.cbegin(), x0_relays.cend(), id) != x0_relays.cend();
}

void CScriptMailbox::AddMsg(TEditorId id) {
  if (HasMsg(id)) {
    return;
  }

  x0_relays.push_back(id);
}

void CScriptMailbox::RemoveMsg(TEditorId id) {
  if (!HasMsg(id)) {
    return;
  }

  std::erase(x0_relays, id);
}

void CScriptMailbox::SendMsgs(TAreaId areaId, CStateManager& stateMgr) {
  const CWorld* world = stateMgr.GetWorld();
  u32 relayCount = world->GetRelayCount();

  bool hasActiveRelays = false;
  for (u32 i = 0; i < relayCount; ++i) {
    const CWorld::CRelay& relay = world->GetRelay(i);
    if (relay.GetTargetId().AreaNum() != areaId)
      continue;

    if (!HasMsg(relay.GetRelayId()))
      continue;

    stateMgr.SendScriptMsg(kInvalidUniqueId, relay.GetTargetId(), EScriptObjectMessage(relay.GetMessage()),
                           EScriptObjectState::Any);
    if (relay.GetActive())
      hasActiveRelays = true;
  }

  if (!hasActiveRelays)
    return;

  for (u32 i = 0; i < relayCount; ++i) {
    const CWorld::CRelay& relay = world->GetRelay(i);
    if (relay.GetTargetId().AreaNum() != areaId)
      continue;

    if (!HasMsg(relay.GetRelayId()) || !relay.GetActive())
      continue;

    RemoveMsg(relay.GetRelayId());
  }
}

void CScriptMailbox::PutTo(CBitStreamWriter& out, const CSaveWorld& saveWorld) {
  const u32 relayCount = saveWorld.GetRelayCount();
  std::vector<bool> relays(relayCount);

  for (const TEditorId& id : x0_relays) {
    const s32 idx = saveWorld.GetRelayIndex(id);
    if (idx >= 0) {
      relays[idx] = true;
    }
  }

  for (u32 i = 0; i < relayCount; ++i) {
    out.WriteEncoded(u32(relays[i]), 1);
  }
}

} // namespace metaforce
