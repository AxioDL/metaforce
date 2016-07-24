#include "CRelayTracker.hpp"
#include "CStateManager.hpp"
#include "CSaveWorld.hpp"
#include "World/CWorld.hpp"
#include <algorithm>

namespace urde
{

CRelayTracker::CRelayTracker(CBitStreamReader& in, const CSaveWorld& saveworld)
{
    u32 relayCount = saveworld.GetRelayCount();
    if (saveworld.GetRelayCount())
    {
        std::vector<bool> relayStates(saveworld.GetRelayCount());
        for (u32 i=0 ; i<relayCount ; ++i)
            relayStates[i] = in.ReadEncoded(1);

        for (u32 i=0 ; i<relayCount ; ++i)
        {
            if (!relayStates[i])
                continue;
            x0_relayStates.push_back(saveworld.GetRelayEditorId(i));
        }
    }
}

bool CRelayTracker::HasRelay(TEditorId id)
{
    return std::find(x0_relayStates.begin(), x0_relayStates.end(), id) != x0_relayStates.end();
}

void CRelayTracker::AddRelay(TEditorId id)
{
    if (std::find(x0_relayStates.begin(), x0_relayStates.end(), id) == x0_relayStates.end())
        x0_relayStates.push_back(id);
}

void CRelayTracker::RemoveRelay(TEditorId id)
{
    if (std::find(x0_relayStates.begin(), x0_relayStates.end(), id) != x0_relayStates.end())
        x0_relayStates.erase(std::remove(x0_relayStates.begin(), x0_relayStates.end(), id), x0_relayStates.end());
}

void CRelayTracker::SendMsgs(const TAreaId& areaId, CStateManager& stateMgr)
{
    const CWorld* world = stateMgr.GetWorld();
    u32 relayCount = world->GetRelayCount();

    bool hasInactiveRelays = false;
    for (u32 i=0 ; i<relayCount ; ++i)
    {
        const CWorld::CRelay& relay = world->GetRelay(i);
        if (((relay.GetRelayId() >> 16) & 0x3FF) != areaId)
            continue;

        if (!HasRelay(relay.GetRelayId()))
            continue;

        stateMgr.SendScriptMsg(kInvalidUniqueId, relay.GetTargetId(), EScriptObjectMessage(relay.GetMessage()),
                               EScriptObjectState::Any);
        if (!relay.GetActive())
            hasInactiveRelays = true;
    }

    if (!hasInactiveRelays)
        return;

    for (u32 i=0 ; i<relayCount ; ++i)
    {
        const CWorld::CRelay& relay = world->GetRelay(i);
        if (((relay.GetRelayId() >> 16) & 0x3FF) != areaId)
            continue;

        if (!HasRelay(relay.GetRelayId()))
            continue;

        RemoveRelay(relay.GetRelayId());
    }
}

void CRelayTracker::PutTo(CBitStreamWriter& out, const CSaveWorld& saveworld)
{
    u32 relayCount = saveworld.GetRelayCount();
    std::vector<bool> relays(relayCount);

    for (const TEditorId& id : x0_relayStates)
        relays[saveworld.GetRelayIndex(id)] = true;

    for (u32 i=0 ; i<relayCount ; ++i)
        out.WriteEncoded(u32(relays[i]), 1);
}

}
