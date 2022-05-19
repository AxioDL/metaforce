#include "Runtime/World/CScriptCameraBlurKeyframe.hpp"

#include "Runtime/CStateManager.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {
CScriptCameraBlurKeyframe::CScriptCameraBlurKeyframe(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                                     EBlurType type, float amount, u32 unk, float timeIn, float timeOut,
                                                     bool active)
: CEntity(uid, info, active, name)
, x34_type(type)
, x38_amount(amount)
, x3c_(unk)
, x40_timeIn(timeIn)
, x44_timeOut(timeOut) {}

void CScriptCameraBlurKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  switch (msg) {
  case EScriptObjectMessage::Increment:
    if (GetActive()) {
      stateMgr.GetCameraBlurPass(3).SetBlur(x34_type, x38_amount, x40_timeIn, false);
    }
    break;
  case EScriptObjectMessage::Decrement:
    if (GetActive()) {
      stateMgr.GetCameraBlurPass(3).DisableBlur(x44_timeOut);
    }
    break;
  default:
    break;
  }
}

void CScriptCameraBlurKeyframe::Accept(IVisitor& visitor) { visitor.Visit(this); }

} // namespace metaforce
