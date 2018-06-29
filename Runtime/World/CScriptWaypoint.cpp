#include "CScriptWaypoint.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"

namespace urde
{

CScriptWaypoint::CScriptWaypoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, float f1, float f2,
                                 u32 w1, u32 w2, u32 w3, u32 w4, u32 w5, u32 w6, u32 w7)
: CActor(uid, active, name, info, xf, CModelData(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId),
  xe8_speed(f1), xec_(w7), xf0_(f2), xf4_(w1), xf5_(w2), xf6_(w3), xf7_(w4), xf8_(w5), xfa_jumpFlags(w6)
{
    SetUseInSortedLists(false);
    SetCallTouch(false);
}

void CScriptWaypoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptWaypoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    CActor::AcceptScriptMsg(msg, sender, mgr);
    if (GetActive())
        if (msg == EScriptObjectMessage::Arrived)
            SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
}

void CScriptWaypoint::AddToRenderer(const zeus::CFrustum&, const CStateManager&) const
{
    // Empty
}

TUniqueId CScriptWaypoint::FollowWaypoint(CStateManager& mgr) const
{
    for (const SConnection& conn : x20_conns)
        if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Follow)
            return mgr.GetIdForScript(conn.x8_objId);
    return kInvalidUniqueId;
}

TUniqueId CScriptWaypoint::NextWaypoint(CStateManager& mgr) const
{
    rstl::reserved_vector<TUniqueId, 10> ids;
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next)
        {
            TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
            if (id != kInvalidUniqueId)
                if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(id))
                    if (wp->GetActive())
                        ids.push_back(wp->GetUniqueId());
        }
    }

    if (ids.size() == 0)
        return kInvalidUniqueId;

    return ids[int(mgr.GetActiveRandom()->Float() * ids.size() * 0.99f)];
}

}
