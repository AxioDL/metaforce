#ifndef _CSCRIPTCAMERAWAYPOINT
#define _CSCRIPTCAMERAWAYPOINT

#include "MetroidPrime/CActor.hpp"

class CScriptCameraWaypoint : public CActor {
  float mHfov;
  uint xec_;

public:
  CScriptCameraWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                        const CTransform4f& xf, const bool active, float hfov, uint);
  ~CScriptCameraWaypoint();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;

  TUniqueId NextWaypoint(CStateManager& mgr);
  float GetHFov() const { return mHfov; }
};

#endif // _CSCRIPTCAMERAWAYPOINT
