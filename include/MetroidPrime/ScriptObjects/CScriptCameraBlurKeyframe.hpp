#ifndef _CSCRIPTCAMERABLURKEYFRAME
#define _CSCRIPTCAMERABLURKEYFRAME

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"

class CScriptCameraBlurKeyframe : public CEntity {
  CCameraBlurPass::EBlurType x34_type;
  float x38_amount;
  u32 x3c_;
  float x40_timeIn;
  float x44_timeOut;

public:
  CScriptCameraBlurKeyframe(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                            CCameraBlurPass::EBlurType type, float amount, u32 unk, float timeIn,
                            float timeOut, bool active);
  ~CScriptCameraBlurKeyframe();

  DECLARE_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};

#endif // _CSCRIPTCAMERABLURKEYFRAME
