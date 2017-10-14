#include "CScriptCameraWaypoint.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"

namespace urde
{

CScriptCameraWaypoint::CScriptCameraWaypoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                             const zeus::CTransform& xf, bool active, float f1, u32 w1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Unknown),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_(f1)
, xec_(w1)
{

}

void CScriptCameraWaypoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptCameraWaypoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CActor::AcceptScriptMsg(msg, uid, mgr);
    if (!GetActive() && msg == EScriptObjectMessage::Arrived)
        SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
}

TUniqueId CScriptCameraWaypoint::GetRandomNextWaypointId(CStateManager& mgr) const
{
    std::vector<TUniqueId> candidateIds;
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next)
        {
            TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
            if (uid == kInvalidUniqueId)
                continue;
            candidateIds.push_back(uid);
        }
    }

    if (candidateIds.empty())
        return kInvalidUniqueId;

    return candidateIds[mgr.GetActiveRandom()->Range(0, s32(candidateIds.size() - 1))];
}

}
