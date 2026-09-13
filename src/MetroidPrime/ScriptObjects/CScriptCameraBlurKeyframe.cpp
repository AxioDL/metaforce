#include "MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.hpp"

#include "MetroidPrime/CStateManager.hpp"

CScriptCameraBlurKeyframe::CScriptCameraBlurKeyframe(TUniqueId uid, const rstl::string& name,
                                                     const CEntityInfo& info,
                                                     const CCameraBlurPass::EBlurType type,
                                                     const float amount, const u32 unk,
                                                     const float timeIn, const float timeOut,
                                                     const bool active)
: CEntity(uid, info, active, name)
, x34_type(type)
, x38_amount(amount)
, x3c_(unk)
, x40_timeIn(timeIn)
, x44_timeOut(timeOut) {}

CScriptCameraBlurKeyframe::~CScriptCameraBlurKeyframe() {}

void CScriptCameraBlurKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                                CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  switch (msg) {
  case kSM_Increment:
    if (GetActive()) {
      stateMgr.CameraBlurPass(CStateManager::kCFS_Three)
          .SetBlur(x34_type, x38_amount, x40_timeIn, false);
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      stateMgr.CameraBlurPass(CStateManager::kCFS_Three).DisableBlur(x44_timeOut);
    }
    break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptCameraBlurKeyframe)
