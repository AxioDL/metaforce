#ifndef _CSCRIPTAIJUMPPOINT
#define _CSCRIPTAIJUMPPOINT

#include "MetroidPrime/CActor.hpp"

class CScriptAiJumpPoint : public CActor {
private:
  float xe8_apex;
  rstl::optional_object<CAABox> xec_touchBounds;
  bool x108_24_inUse : 1;
  TUniqueId x10a_occupant;
  TUniqueId x10c_currentWaypoint;
  TUniqueId x10e_nextWaypoint;
  float x110_timeRemaining;

public:
  CScriptAiJumpPoint(TUniqueId, const rstl::string&, const CEntityInfo&, const CTransform4f&, bool,
                     float);
  ~CScriptAiJumpPoint();
  
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  bool GetInUse(TUniqueId uid) const;
  TUniqueId GetJumpPoint() const { return x10c_currentWaypoint; }
  TUniqueId GetJumpTarget() const { return x10e_nextWaypoint; }
  float GetJumpApex() const { return xe8_apex; }
};

#endif // _CSCRIPTAIJUMPPOINT
