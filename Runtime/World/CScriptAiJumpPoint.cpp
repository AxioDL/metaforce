#include "CScriptAiJumpPoint.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "CScriptWaypoint.hpp"

namespace urde
{
CScriptAiJumpPoint::CScriptAiJumpPoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                       zeus::CTransform& xf, bool active, float f1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Zero),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_(f1)
{
    xec_.emplace(xf.origin, xf.origin);
}

void CScriptAiJumpPoint::Think(float dt, CStateManager&)
{
    if (x110_timeRemaining <= 0)
        return;

    x110_timeRemaining -= dt;
}

void CScriptAiJumpPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr)
{
    AcceptScriptMsg(msg, other, mgr);

    if (msg != EScriptObjectMessage::InternalMessage13)
        return;

    for (SConnection& conn : x20_conns)
    {
        if (conn.x0_state != EScriptObjectState::Arrived || conn.x4_msg != EScriptObjectMessage::Next)
            continue;

        const CScriptWaypoint* wpnt =
            static_cast<const CScriptWaypoint*>(mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)));
        if (wpnt)
        {
            x10c_currentWaypoint = wpnt->GetUniqueId();
            const CScriptWaypoint* nextWpnt = wpnt->NextWaypoint(mgr);
            if (nextWpnt)
                x10e_nextWaypoint = nextWpnt->GetUniqueId();
        }
    }
}

rstl::optional_object<zeus::CAABox> CScriptAiJumpPoint::GetTouchBounds() const { return xec_; }

bool CScriptAiJumpPoint::GetInUse(TUniqueId uid) const
{
    if (x108_24 || x110_timeRemaining > 0.f || x10a_occupant != kInvalidUniqueId || uid != kInvalidUniqueId ||
        uid != x10a_occupant)
        return true;

    return false;
}
}
