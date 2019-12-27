#include "Runtime/CRelayTracker.hpp"

#include "Runtime/CSaveWorld.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CWorld.hpp"

#include <algorithm>

namespace urde {

CRelayTracker::CRelayTracker(CBitStreamReader& in, const CSaveWorld& saveworld) {
  u32 relayCount = saveworld.GetRelayCount();
  if (saveworld.GetRelayCount()) {
    std::vector<bool> relayStates(saveworld.GetRelayCount());
    for (u32 i = 0; i < relayCount; ++i)
      relayStates[i] = in.ReadEncoded(1);

    for (u32 i = 0; i < relayCount; ++i) {
      if (!relayStates[i])
        continue;
      x0_relayStates.push_back(saveworld.GetRelayEditorId(i));
    }
  }
}

bool CRelayTracker::HasRelay(TEditorId id) {
  return std::find(x0_relayStates.begin(), x0_relayStates.end(), id) != x0_relayStates.end();
}

void CRelayTracker::AddRelay(TEditorId id) {
  if (std::find(x0_relayStates.begin(), x0_relayStates.end(), id) == x0_relayStates.end())
    x0_relayStates.push_back(id);
}

void CRelayTracker::RemoveRelay(TEditorId id) {
  if (std::find(x0_relayStates.begin(), x0_relayStates.end(), id) != x0_relayStates.end())
    x0_relayStates.erase(std::remove(x0_relayStates.begin(), x0_relayStates.end(), id), x0_relayStates.end());
}

void CRelayTracker::SendMsgs(TAreaId areaId, CStateManager& stateMgr) {
  const CWorld* world = stateMgr.GetWorld();
  u32 relayCount = world->GetRelayCount();

  bool hasActiveRelays = false;
  for (u32 i = 0; i < relayCount; ++i) {
    const CWorld::CRelay& relay = world->GetRelay(i);
    if (relay.GetTargetId().AreaNum() != areaId)
      continue;

    if (!HasRelay(relay.GetRelayId()))
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

    if (!HasRelay(relay.GetRelayId()) || !relay.GetActive())
      continue;

    RemoveRelay(relay.GetRelayId());
  }
}

void CRelayTracker::PutTo(CBitStreamWriter& out, const CSaveWorld& saveworld) {
  u32 relayCount = saveworld.GetRelayCount();
  std::vector<bool> relays(relayCount);

  for (const TEditorId& id : x0_relayStates) {
    s32 idx = saveworld.GetRelayIndex(id);
    if (idx >= 0)
      relays[idx] = true;
  }

  for (u32 i = 0; i < relayCount; ++i)
    out.WriteEncoded(u32(relays[i]), 1);
}

} // namespace urde
