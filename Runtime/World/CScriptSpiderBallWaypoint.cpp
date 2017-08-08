#include "CScriptSpiderBallWaypoint.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptSpiderBallWaypoint::CScriptSpiderBallWaypoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                                     const zeus::CTransform& xf, bool active, u32 w1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Unknown),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_(w1)
{

}

void CScriptSpiderBallWaypoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptSpiderBallWaypoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CActor::AcceptScriptMsg(msg, uid, mgr);

    if (msg == EScriptObjectMessage::InitializedInArea)
        BuildWaypointListAndBounds(mgr);
    else if (msg == EScriptObjectMessage::UNKM2)
        SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
}

void CScriptSpiderBallWaypoint::AccumulateBounds(const zeus::CVector3f& v)
{
    if (!xfc_aabox)
        xfc_aabox.emplace(v, v);
    xfc_aabox->accumulateBounds(v);
}

void CScriptSpiderBallWaypoint::BuildWaypointListAndBounds(CStateManager& mgr)
{
    u32 validConnections = 0;
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next)
        {
            TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
            if (uid != kInvalidUniqueId)
            {
                static_cast<CScriptSpiderBallWaypoint*>(mgr.ObjectById(uid))->AddPreviousWaypoint(GetUniqueId());
                ++validConnections;
            }
        }
    }

    if (validConnections == 0)
        AccumulateBounds(x34_transform.origin);
    else
    {
        CScriptSpiderBallWaypoint* curWaypoint = this;
        TUniqueId uid = curWaypoint->NextWaypoint(mgr, ECheckActiveWaypoint::Yes);
        while (uid != kInvalidUniqueId)
        {
            curWaypoint = static_cast<CScriptSpiderBallWaypoint*>(mgr.ObjectById(uid));
            uid = curWaypoint->NextWaypoint(mgr, ECheckActiveWaypoint::Yes);
        }

        curWaypoint->AccumulateBounds(x34_transform.origin);
    }
}

void CScriptSpiderBallWaypoint::AddPreviousWaypoint(TUniqueId uid)
{
    xec_waypoints.push_back(uid);
}

TUniqueId CScriptSpiderBallWaypoint::NextWaypoint(const CStateManager& mgr, ECheckActiveWaypoint checkActive)
{
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next)
        {
            TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
            if (uid != kInvalidUniqueId)
            {
                const CEntity* ent = mgr.GetObjectById(uid);
                if (ent && checkActive == ECheckActiveWaypoint::Yes && ent->GetActive())
                    return ent->GetUniqueId();
            }
        }
    }

    return kInvalidUniqueId;
}
}
