#ifndef _CSCRIPTAIJUMPPOINT
#define _CSCRIPTAIJUMPPOINT

#include "MetroidPrime/CActor.hpp"

class CScriptAiJumpPoint : public CActor {
private:
  float mApex;
  rstl::optional_object<CAABox> mTouchBounds;
  bool mInUse : 1;
  TUniqueId mOccupant;
  TUniqueId mCurrentWaypoint;
  TUniqueId mNextWaypoint;
  float mTimeRemaining;

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
  TUniqueId GetJumpPoint() const { return mCurrentWaypoint; }
  TUniqueId GetJumpTarget() const { return mNextWaypoint; }
  float GetJumpApex() const { return mApex; }
};

#endif // _CSCRIPTAIJUMPPOINT
