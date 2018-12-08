#pragma once

#include "CActor.hpp"

namespace urde {
class CScriptSpiderBallWaypoint : public CActor {
  enum class ECheckActiveWaypoint { Check, SkipCheck };
  u32 xe8_;
  std::vector<TUniqueId> xec_waypoints;
  std::experimental::optional<zeus::CAABox> xfc_aabox;

public:
  CScriptSpiderBallWaypoint(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool, u32);
  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Render(const CStateManager& mgr) const { CActor::Render(mgr); }
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}
  std::experimental::optional<zeus::CAABox> GetTouchBounds() const { return xfc_aabox; }
  void AccumulateBounds(const zeus::CVector3f& v);
  void BuildWaypointListAndBounds(CStateManager& mgr);
  void AddPreviousWaypoint(TUniqueId uid);
  TUniqueId PreviousWaypoint(const CStateManager& mgr, ECheckActiveWaypoint checkActive) const;
  TUniqueId NextWaypoint(const CStateManager& mgr, ECheckActiveWaypoint checkActive) const;
  void GetClosestPointAlongWaypoints(CStateManager& mgr, const zeus::CVector3f& ballPos, float maxPointToBallDist,
                                     const CScriptSpiderBallWaypoint*& closestWaypoint, zeus::CVector3f& closestPoint,
                                     zeus::CVector3f& deltaBetweenPoints, float deltaBetweenInterpDist,
                                     zeus::CVector3f& interpDeltaBetweenPoints) const;
};
} // namespace urde
