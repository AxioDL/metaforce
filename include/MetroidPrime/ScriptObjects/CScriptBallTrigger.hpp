#ifndef _CSCRIPTBALLTRIGGER
#define _CSCRIPTBALLTRIGGER

#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

class CScriptBallTrigger : public CScriptTrigger {
public:
  CScriptBallTrigger(TUniqueId, const rstl::string&, const CEntityInfo&, const CVector3f&,
                     const CVector3f&, bool, float, float, float, CVector3f, bool);
  ~CScriptBallTrigger();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager& mgr) override;
  void InhabitantAdded(CActor&, CStateManager&) override;
  void InhabitantExited(CActor&, CStateManager&) override;

private:
  float mForce;
  float mMinAngle;
  float mMaxDistance;
  CVector3f mForceAngle;
  bool mCanApplyForce : 1;
  bool mStopPlayer : 1;
};

#endif // _CSCRIPTBALLTRIGGER
