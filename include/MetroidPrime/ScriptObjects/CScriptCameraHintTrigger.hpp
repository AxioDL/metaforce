#ifndef _CSCRIPTCAMERAHINTTRIGGER
#define _CSCRIPTCAMERAHINTTRIGGER

#include "MetroidPrime/CActor.hpp"

#include "Collision/COBBox.hpp"

class CScriptCameraHintTrigger : public CActor {
  COBBox xe8_obb;
  CVector3f x124_scale;
  bool x130_24_deactivateOnEnter : 1;
  bool x130_25_deactivateOnExit : 1;
  bool x130_26_playerInside : 1;
  bool x130_27_playerWasInside : 1;

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
