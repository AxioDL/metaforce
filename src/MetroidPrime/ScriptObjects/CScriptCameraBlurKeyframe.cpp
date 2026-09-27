#include "MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.hpp"

#include "MetroidPrime/CStateManager.hpp"

CScriptCameraBlurKeyframe::CScriptCameraBlurKeyframe(TUniqueId uid, const rstl::string& name,
                                                     const CEntityInfo& info,
                                                     const CCameraBlurPass::EBlurType type,
                                                     const float amount, const u32 unk,
                                                     const float timeIn, const float timeOut,
                                                     const bool active)
: CEntity(uid, info, active, name)
, mType(type)
, mAmount(amount)
, x3c_(unk)
, mTimeIn(timeIn)
, mTimeOut(timeOut) {}

CScriptCameraBlurKeyframe::~CScriptCameraBlurKeyframe() {}

void CScriptCameraBlurKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                                CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  switch (msg) {
  case kSM_Increment:
    if (GetActive()) {
      stateMgr.CameraBlurPass(CStateManager::kCFS_Three)
          .SetBlur(mType, mAmount, mTimeIn, false);
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      stateMgr.CameraBlurPass(CStateManager::kCFS_Three).DisableBlur(mTimeOut);
    }
    break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptCameraBlurKeyframe)
