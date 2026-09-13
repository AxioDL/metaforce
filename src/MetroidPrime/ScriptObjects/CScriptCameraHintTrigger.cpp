#include "MetroidPrime/ScriptObjects/CScriptCameraHintTrigger.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CScriptCameraHintTrigger::CScriptCameraHintTrigger(TUniqueId uid, const bool active,
                                                   const rstl::string& name,
                                                   const CEntityInfo& info, const CVector3f& scale,
                                                   const CTransform4f& xf, bool deactivateOnEnter,
                                                   bool deactivateOnExit)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Trigger),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_obb(xf, scale)
, x124_scale(scale)
, x130_24_deactivateOnEnter(deactivateOnEnter)
, x130_25_deactivateOnExit(deactivateOnExit)
, x130_26_playerInside(false)
, x130_27_playerWasInside(false) {}

ENTITY_ACCEPT_IMPL(CScriptCameraHintTrigger)

rstl::optional_object< CAABox > CScriptCameraHintTrigger::GetTouchBounds() const {
  return xe8_obb.CalculateAABox(CTransform4f::Identity());
}

void CScriptCameraHintTrigger::Touch(CActor& other, CStateManager& mgr) {
  if (TCastToConstPtr< CPlayer >(other)) {
    rstl::optional_object< CAABox > bounds = other.GetTouchBounds();
    if (bounds.valid()) {
      COBBox otherObb = COBBox::FromAABox(*bounds, CTransform4f::Identity());
      x130_26_playerInside = xe8_obb.OBBIntersectsBox(otherObb);
    }
  }
}

void CScriptCameraHintTrigger::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (x130_26_playerInside && !x130_27_playerWasInside) {
    x130_27_playerWasInside = true;
    SendScriptMsgs(kSS_Entered, mgr, kSM_None);
    if (x130_24_deactivateOnEnter) {
      mgr.DeliverScriptMsg(this, kInvalidUniqueId, kSM_Deactivate);
    }
  }

  if (!x130_26_playerInside && x130_27_playerWasInside) {
    x130_27_playerWasInside = false;
    SendScriptMsgs(kSS_Exited, mgr, kSM_None);
    if (x130_25_deactivateOnExit) {
      mgr.DeliverScriptMsg(this, kInvalidUniqueId, kSM_Deactivate);
    }
  }

  if (x130_26_playerInside) {
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
  }

  x130_26_playerInside = false;
}

CScriptCameraHintTrigger::~CScriptCameraHintTrigger() {}
