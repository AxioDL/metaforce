#ifndef _CSCRIPTSPIDERBALLWAYPOINT
#define _CSCRIPTSPIDERBALLWAYPOINT

#include "MetroidPrime/CActor.hpp"

class CScriptSpiderBallWaypoint : public CActor {
public:
  enum ECheckActiveWaypoint {
    kCAW_Check,
    kCAW_SkipCheck,
  };
  CScriptSpiderBallWaypoint(TUniqueId, const rstl::string&, const CEntityInfo&,
                            const CTransform4f& xf, const bool active, uint w1);
  ~CScriptSpiderBallWaypoint();

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  void ClearWaypoints();
  void BuildWaypointListAndBounds(CStateManager& mgr);
  void AddPreviousWaypoint(TUniqueId uid);
  void AddPointToTouchBounds(const CVector3f& point);
  TUniqueId NextWaypoint(const CStateManager& mgr, ECheckActiveWaypoint check) const;
  TUniqueId PreviousWaypoint(const CStateManager& mgr, ECheckActiveWaypoint check) const;
  void GetClosestPointAlongWaypoints(CStateManager& mgr, const CVector3f& ballPos,
                                     float maxPointToBallDist,
                                     const CScriptSpiderBallWaypoint** closestWaypoint,
                                     CVector3f& closestPoint, CVector3f& deltaBetweenPoints,
                                     float deltaBetweenInterpPoints,
                                     CVector3f& interpDeltaBetweenPoints) const;

  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Render(const CStateManager& mgr) const override;

  rstl::optional_object< CAABox > GetTouchBounds() const override;

private:
  uint xe8_;
  rstl::vector< TUniqueId > xec_waypoints;
  rstl::optional_object< CAABox > xfc_aabox;
};

#endif // _CSCRIPTSPIDERBALLWAYPOINT
