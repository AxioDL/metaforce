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
    else if (msg == EScriptObjectMessage::Arrived)
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
        TUniqueId uid = curWaypoint->NextWaypoint(mgr, ECheckActiveWaypoint::SkipCheck);
        while (uid != kInvalidUniqueId)
        {
            curWaypoint = static_cast<CScriptSpiderBallWaypoint*>(mgr.ObjectById(uid));
            uid = curWaypoint->NextWaypoint(mgr, ECheckActiveWaypoint::SkipCheck);
        }

        curWaypoint->AccumulateBounds(x34_transform.origin);
    }
}

void CScriptSpiderBallWaypoint::AddPreviousWaypoint(TUniqueId uid)
{
    xec_waypoints.push_back(uid);
}

TUniqueId CScriptSpiderBallWaypoint::PreviousWaypoint(const CStateManager& mgr, ECheckActiveWaypoint checkActive) const
{
    for (TUniqueId id : xec_waypoints)
    {
        if (const CEntity* ent = mgr.GetObjectById(id))
        {
            if (checkActive == ECheckActiveWaypoint::SkipCheck)
                return id;
            else if (ent->GetActive())
                return id;
        }
    }

    return kInvalidUniqueId;
}

TUniqueId CScriptSpiderBallWaypoint::NextWaypoint(const CStateManager& mgr, ECheckActiveWaypoint checkActive) const
{
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next)
        {
            TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
            if (uid != kInvalidUniqueId)
            {
                if (const CEntity* ent = mgr.GetObjectById(uid))
                {
                    if (checkActive == ECheckActiveWaypoint::SkipCheck)
                        return ent->GetUniqueId();
                    else if (ent->GetActive())
                        return ent->GetUniqueId();
                }
            }
        }
    }

    return kInvalidUniqueId;
}

void CScriptSpiderBallWaypoint::GetClosestPointAlongWaypoints(CStateManager& mgr, const zeus::CVector3f& ballPos,
    float maxPointToBallDist, const CScriptSpiderBallWaypoint*& closestWaypoint, zeus::CVector3f& closestPoint,
    zeus::CVector3f& deltaBetweenPoints, float deltaBetweenInterpDist, zeus::CVector3f& interpDeltaBetweenPoints) const
{
    const CScriptSpiderBallWaypoint* wp = this;
    while (wp->PreviousWaypoint(mgr, ECheckActiveWaypoint::SkipCheck) != kInvalidUniqueId)
        wp = static_cast<const CScriptSpiderBallWaypoint*>(
            mgr.GetObjectById(wp->PreviousWaypoint(mgr, ECheckActiveWaypoint::SkipCheck)));
    float minPointToBallDistSq = maxPointToBallDist * maxPointToBallDist;
    float deltaBetweenInterpDistSq = deltaBetweenInterpDist * deltaBetweenInterpDist;
    zeus::CVector3f lastPoint = wp->GetTranslation();
    zeus::CVector3f lastDelta;
    bool computeDelta = wp->GetActive();
    while (true)
    {
        if (wp->NextWaypoint(mgr, ECheckActiveWaypoint::Check) != kInvalidUniqueId)
        {
            if (computeDelta)
            {
                const CScriptSpiderBallWaypoint* prevWp = wp;
                wp = static_cast<const CScriptSpiderBallWaypoint*>(
                    mgr.GetObjectById(wp->NextWaypoint(mgr, ECheckActiveWaypoint::Check)));
                zeus::CVector3f thisDelta = wp->GetTranslation() - lastPoint;
                zeus::CVector3f lastPointToBall = ballPos - lastPoint;
                if (prevWp->PreviousWaypoint(mgr, ECheckActiveWaypoint::Check) == kInvalidUniqueId)
                    lastDelta = thisDelta;
                float pointToBallDistSq = lastPointToBall.magSquared();
                if (pointToBallDistSq < minPointToBallDistSq)
                {
                    minPointToBallDistSq = pointToBallDistSq;
                    closestPoint = lastPoint;
                    deltaBetweenPoints = thisDelta;
                    interpDeltaBetweenPoints = (thisDelta.normalized() + lastDelta.normalized()) * 0.5f;
                    closestWaypoint = wp;
                }
                float projectedT = lastPointToBall.dot(thisDelta);
                if (projectedT >= 0.f)
                {
                    float normT = projectedT / thisDelta.magSquared();
                    if (normT < 1.f)
                    {
                        zeus::CVector3f projectedPoint = zeus::CVector3f::lerp(lastPoint, wp->GetTranslation(), normT);
                        float projToBallDistSq = (ballPos - projectedPoint).magSquared();
                        if (projToBallDistSq < minPointToBallDistSq)
                        {
                            minPointToBallDistSq = projToBallDistSq;
                            closestPoint = projectedPoint;
                            deltaBetweenPoints = thisDelta;
                            interpDeltaBetweenPoints = deltaBetweenPoints;
                            closestWaypoint = wp;
                            float lastToProjDist = (lastPoint - projectedPoint).magnitude();
                            if (lastToProjDist < deltaBetweenInterpDistSq)
                            {
                                interpDeltaBetweenPoints =
                                    zeus::CVector3f::lerp(0.5f * (thisDelta.normalized() + lastDelta.normalized()),
                                        thisDelta.normalized(), lastToProjDist / deltaBetweenInterpDist);
                            }
                            else if (wp->NextWaypoint(mgr, ECheckActiveWaypoint::Check) != kInvalidUniqueId)
                            {
                                lastToProjDist = (projectedPoint - wp->GetTranslation()).magnitude();
                                if (lastToProjDist < deltaBetweenInterpDist)
                                {
                                    float t = lastToProjDist / deltaBetweenInterpDist;
                                    interpDeltaBetweenPoints =
                                        zeus::CVector3f::lerp(((static_cast<const CScriptSpiderBallWaypoint*>(
                                        mgr.GetObjectById(wp->NextWaypoint(mgr, ECheckActiveWaypoint::Check)))->
                                        GetTranslation() - wp->GetTranslation()).normalized() +
                                        thisDelta.normalized()) * 0.5f, thisDelta.normalized(), t);
                                }
                            }
                        }
                    }
                }
                lastDelta = thisDelta;
                lastPoint = wp->GetTranslation();
                computeDelta = true;
            }
            else
            {
                wp = static_cast<const CScriptSpiderBallWaypoint*>(
                    mgr.GetObjectById(wp->NextWaypoint(mgr, ECheckActiveWaypoint::Check)));
                lastPoint = wp->GetTranslation();
                computeDelta = true;
            }
        }
        else
        {
            if (wp->NextWaypoint(mgr, ECheckActiveWaypoint::SkipCheck) != kInvalidUniqueId)
            {
                wp = static_cast<const CScriptSpiderBallWaypoint*>(
                    mgr.GetObjectById(wp->NextWaypoint(mgr, ECheckActiveWaypoint::SkipCheck)));
                computeDelta = false;
            }
            else
            {
                break;
            }
        }
    }

    if ((ballPos - lastPoint).magSquared() < minPointToBallDistSq)
    {
        closestPoint = lastPoint;
        if (wp->PreviousWaypoint(mgr, ECheckActiveWaypoint::Check) != kInvalidUniqueId)
        {
            wp = static_cast<const CScriptSpiderBallWaypoint*>(
                mgr.GetObjectById(wp->PreviousWaypoint(mgr, ECheckActiveWaypoint::SkipCheck)));
            deltaBetweenPoints = lastPoint - wp->GetTranslation();
            interpDeltaBetweenPoints = deltaBetweenPoints;
        }
        closestWaypoint = wp;
    }
}
}
