#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"

CScriptSpiderBallWaypoint::CScriptSpiderBallWaypoint(TUniqueId uid, const rstl::string& name,
                                                     const CEntityInfo& info,
                                                     const CTransform4f& xf, const bool active,
                                                     uint w1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_(w1) {}

CScriptSpiderBallWaypoint::~CScriptSpiderBallWaypoint() {}

void CScriptSpiderBallWaypoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                                CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_InitializedInArea:
    BuildWaypointListAndBounds(mgr);
    break;
  case kSM_Arrived: {
    if (GetActive()) {
      SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
    }
    break;
  }
  default:
    break;
  }
}

void CScriptSpiderBallWaypoint::ClearWaypoints() {
  xfc_aabox.clear();
  xec_waypoints.clear();
}

void CScriptSpiderBallWaypoint::BuildWaypointListAndBounds(CStateManager& mgr) {
  rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
  int validConnections = 0;

  for (; it != GetConnectionList().end(); ++it) {
    if (it->x0_state == kSS_Arrived && it->x4_msg == kSM_Next) {
      TUniqueId uid = mgr.GetIdForScript(it->x8_objId);
      if (uid != kInvalidUniqueId) {
        CScriptSpiderBallWaypoint* wp =
            static_cast< CScriptSpiderBallWaypoint* >(mgr.ObjectById(uid));
        wp->AddPreviousWaypoint(GetUniqueId());
        ++validConnections;
      }
    }
  }

  if (validConnections == 0) {
    AddPointToTouchBounds(GetTranslation());
    return;
  }

  CScriptSpiderBallWaypoint* curWaypoint = this;
  while (curWaypoint->NextWaypoint(mgr, kCAW_SkipCheck) != kInvalidUniqueId) {
    curWaypoint = static_cast< CScriptSpiderBallWaypoint* >(
        mgr.ObjectById(curWaypoint->NextWaypoint(mgr, kCAW_SkipCheck)));
  }

  curWaypoint->AddPointToTouchBounds(GetTranslation());
}

void CScriptSpiderBallWaypoint::AddPointToTouchBounds(const CVector3f& point) {
  if (!xfc_aabox) {
    xfc_aabox = CAABox(point, point);
  } else {
    xfc_aabox->AccumulateBounds(point);
  }
}

TUniqueId CScriptSpiderBallWaypoint::NextWaypoint(const CStateManager& mgr,
                                                  ECheckActiveWaypoint check) const {
  rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();

  for (; it != GetConnectionList().end(); ++it) {
    if (it->x0_state == kSS_Arrived && it->x4_msg == kSM_Next) {
      TUniqueId uid = mgr.GetIdForScript(it->x8_objId);
      if (const CScriptSpiderBallWaypoint* wp =
              static_cast< const CScriptSpiderBallWaypoint* >(mgr.GetObjectById(uid))) {
        if (check == kCAW_SkipCheck) {
          return uid;
        } else if (wp->GetActive()) {
          return uid;
        }
      }
    }
  }
  return kInvalidUniqueId;
}

TUniqueId CScriptSpiderBallWaypoint::PreviousWaypoint(const CStateManager& mgr,
                                                      ECheckActiveWaypoint check) const {
  for (int i = 0; i < xec_waypoints.size(); ++i) {
    if (const CScriptSpiderBallWaypoint* wp =
            static_cast< const CScriptSpiderBallWaypoint* >(mgr.GetObjectById(xec_waypoints[i]))) {
      if (check == kCAW_SkipCheck) {
        return xec_waypoints[i];
      } else if (wp->GetActive()) {
        return xec_waypoints[i];
      }
    }
  }

  return kInvalidUniqueId;
}

void CScriptSpiderBallWaypoint::AddPreviousWaypoint(TUniqueId uid) {
  if (xec_waypoints.size() == xec_waypoints.capacity()) {
    xec_waypoints.reserve(xec_waypoints.capacity() == 0 ? 4 : xec_waypoints.capacity() * 2);
  }
  xec_waypoints.push_back(uid);
}

void CScriptSpiderBallWaypoint::GetClosestPointAlongWaypoints(
    CStateManager& mgr, const CVector3f& ballPos, float maxPointToBallDist,
    const CScriptSpiderBallWaypoint** closestWaypoint, CVector3f& closestPoint,
    CVector3f& deltaBetweenPoints, float deltaBetweenInterpPoints,
    CVector3f& interpDeltaBetweenPoints) const {
  const CScriptSpiderBallWaypoint* wp = this;
  while (wp->PreviousWaypoint(mgr, kCAW_SkipCheck) != kInvalidUniqueId) {
    wp = static_cast< const CScriptSpiderBallWaypoint* >(
        mgr.GetObjectById(wp->PreviousWaypoint(mgr, kCAW_SkipCheck)));
  }

  float minPointToBallDistSq = maxPointToBallDist * maxPointToBallDist;
  const float deltaBetweenInterpDistSq = deltaBetweenInterpPoints * deltaBetweenInterpPoints;
  CVector3f lastPoint = wp->GetTranslation();
  CVector3f lastDelta = CVector3f::Zero();
  bool computeDelta = wp->GetActive();
  bool done = false;

  while (!done) {
    if (wp->NextWaypoint(mgr, kCAW_Check) != kInvalidUniqueId) {
      if (computeDelta) {
        const CScriptSpiderBallWaypoint* prevWp = wp;
        wp = static_cast< const CScriptSpiderBallWaypoint* >(
            mgr.GetObjectById(wp->NextWaypoint(mgr, kCAW_Check)));

        const CVector3f nextPoint = wp->GetTranslation();
        const CVector3f lastPointToBall = ballPos - lastPoint;
        const CVector3f thisDelta = nextPoint - lastPoint;
        if (prevWp->PreviousWaypoint(mgr, kCAW_Check) == kInvalidUniqueId) {
          lastDelta = thisDelta;
        }

        const float pointToBallDistSq = lastPointToBall.MagSquared();
        if (pointToBallDistSq < minPointToBallDistSq) {
          minPointToBallDistSq = pointToBallDistSq;
          closestPoint = lastPoint;
          deltaBetweenPoints = thisDelta;
          interpDeltaBetweenPoints = (lastDelta.AsNormalized() + thisDelta.AsNormalized()) * 0.5f;
          *closestWaypoint = wp;
        }

        const float projectedT = CVector3f::Dot(lastPointToBall, thisDelta);
        if (projectedT >= 0.f) {
          const float normT = projectedT / thisDelta.MagSquared();
          if (normT < 1.f) {
            const CVector3f projectedPoint = CVector3f::Lerp(lastPoint, nextPoint, normT);
            const float projToBallDistSq = (ballPos - projectedPoint).MagSquared();
            if (projToBallDistSq < minPointToBallDistSq) {
              minPointToBallDistSq = projToBallDistSq;
              closestPoint = projectedPoint;
              deltaBetweenPoints = thisDelta;
              *closestWaypoint = wp;
              interpDeltaBetweenPoints = deltaBetweenPoints;
              float lastToProjDist = (lastPoint - projectedPoint).Magnitude();
              if (lastToProjDist < deltaBetweenInterpDistSq) {
                interpDeltaBetweenPoints = CVector3f::Lerp(
                    0.5f * (lastDelta.AsNormalized() + thisDelta.AsNormalized()),
                    thisDelta.AsNormalized(), lastToProjDist / deltaBetweenInterpPoints);
              } else if (wp->NextWaypoint(mgr, kCAW_Check) != kInvalidUniqueId) {
                lastToProjDist = (projectedPoint - nextPoint).Magnitude();
                if (lastToProjDist < deltaBetweenInterpPoints) {
                  const CScriptSpiderBallWaypoint* tmpWp =
                      static_cast< const CScriptSpiderBallWaypoint* >(
                          mgr.GetObjectById(wp->NextWaypoint(mgr, kCAW_Check)));
                  const CVector3f nextDelta = tmpWp->GetTranslation() - nextPoint;
                  interpDeltaBetweenPoints = CVector3f::Lerp(
                      (nextDelta.AsNormalized() + thisDelta.AsNormalized()) * 0.5f,
                      thisDelta.AsNormalized(), lastToProjDist / deltaBetweenInterpPoints);
                }
              }
            }
          }
        }
        lastDelta = thisDelta;
        lastPoint = nextPoint;
        computeDelta = true;
      } else {
        computeDelta = true;
        wp = static_cast< const CScriptSpiderBallWaypoint* >(
            mgr.GetObjectById(wp->NextWaypoint(mgr, kCAW_Check)));
        lastPoint = wp->GetTranslation();
      }
    } else if (wp->NextWaypoint(mgr, kCAW_SkipCheck) != kInvalidUniqueId) {
      wp = static_cast< const CScriptSpiderBallWaypoint* >(
          mgr.GetObjectById(wp->NextWaypoint(mgr, kCAW_SkipCheck)));
      computeDelta = false;
    } else {
      done = true;
    }
  }

  if (!((ballPos - lastPoint).MagSquared() < minPointToBallDistSq)) {
    return;
  }

  closestPoint = lastPoint;

  if (wp->PreviousWaypoint(mgr, kCAW_Check) != kInvalidUniqueId) {
    wp = static_cast< const CScriptSpiderBallWaypoint* >(
        mgr.GetObjectById(wp->PreviousWaypoint(mgr, kCAW_Check)));
    deltaBetweenPoints = lastPoint - wp->GetTranslation();
    interpDeltaBetweenPoints = deltaBetweenPoints;
  }

  *closestWaypoint = wp;
}

void CScriptSpiderBallWaypoint::AddToRenderer(const CFrustumPlanes& frustum,
                                              const CStateManager& mgr) const {}

void CScriptSpiderBallWaypoint::Render(const CStateManager& mgr) const { CActor::Render(mgr); }

ENTITY_ACCEPT_IMPL(CScriptSpiderBallWaypoint)

rstl::optional_object< CAABox > CScriptSpiderBallWaypoint::GetTouchBounds() const {
  return xfc_aabox;
}
