#include "MetroidPrime/ScriptObjects/CScriptCameraHintTrigger.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CScriptCameraHintTrigger::CScriptCameraHintTrigger(TUniqueId uid, const bool active,
                                                   const rstl::string& name,
                                                   const CEntityInfo& info, const CVector3f& scale,
                                                   const CTransform4f& xf, bool deactivateOnEnter,
                                                   bool deactivateOnExit)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Trigger),
         CActorParameters::None(), kInvalidUniqueId)
, mObb(xf, scale)
, mScale(scale)
, mDeactivateOnEnter(deactivateOnEnter)
, mDeactivateOnExit(deactivateOnExit)
, mPlayerInside(false)
, mPlayerWasInside(false) {}

ENTITY_ACCEPT_IMPL(CScriptCameraHintTrigger)

rstl::optional_object< CAABox > CScriptCameraHintTrigger::GetTouchBounds() const {
  return mObb.CalculateAABox(CTransform4f::Identity());
}

void CScriptCameraHintTrigger::Touch(CActor& other, CStateManager& mgr) {
  if (TCastToConstPtr< CPlayer >(other)) {
    rstl::optional_object< CAABox > bounds = other.GetTouchBounds();
    if (bounds.valid()) {
      COBBox otherObb = COBBox::FromAABox(*bounds, CTransform4f::Identity());
      mPlayerInside = mObb.OBBIntersectsBox(otherObb);
    }
  }
}

void CScriptCameraHintTrigger::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (mPlayerInside && !mPlayerWasInside) {
    mPlayerWasInside = true;
    SendScriptMsgs(kSS_Entered, mgr, kSM_None);
    if (mDeactivateOnEnter) {
      mgr.DeliverScriptMsg(this, kInvalidUniqueId, kSM_Deactivate);
    }
  }

  if (!mPlayerInside && mPlayerWasInside) {
    mPlayerWasInside = false;
    SendScriptMsgs(kSS_Exited, mgr, kSM_None);
    if (mDeactivateOnExit) {
      mgr.DeliverScriptMsg(this, kInvalidUniqueId, kSM_Deactivate);
    }
  }

  if (mPlayerInside) {
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
  }

  mPlayerInside = false;
}

CScriptCameraHintTrigger::~CScriptCameraHintTrigger() {}
