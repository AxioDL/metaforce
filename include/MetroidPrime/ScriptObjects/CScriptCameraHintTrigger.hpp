#ifndef _CSCRIPTCAMERAHINTTRIGGER
#define _CSCRIPTCAMERAHINTTRIGGER

#include "MetroidPrime/CActor.hpp"

#include "Collision/COBBox.hpp"

class CScriptCameraHintTrigger : public CActor {
  COBBox mObb;
  CVector3f mScale;
  bool mDeactivateOnEnter : 1;
  bool mDeactivateOnExit : 1;
  bool mPlayerInside : 1;
  bool mPlayerWasInside : 1;

public:
  CScriptCameraHintTrigger(TUniqueId uid, bool active, const rstl::string& name,
                           const CEntityInfo& info, const CVector3f& scale, const CTransform4f& xf,
                           bool deactivateOnEnter, const bool deactivateOnExit);
  ~CScriptCameraHintTrigger();
  
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& other, CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
};

#endif // _CSCRIPTCAMERAHINTTRIGGER
