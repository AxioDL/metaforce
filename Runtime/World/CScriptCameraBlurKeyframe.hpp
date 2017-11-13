#ifndef __URDE_CSCRIPTCAMERABLURKEYFRAME__
#define __URDE_CSCRIPTCAMERABLURKEYFRAME__

#include "CEntity.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{
class CScriptCameraBlurKeyframe : public CEntity
{
    EBlurType x34_type;
    float x38_amount;
    u32 x3c_;
    float x40_timeIn;
    float x44_timeOut;
public:
    CScriptCameraBlurKeyframe(TUniqueId uid, std::string_view name,
                              const CEntityInfo& info, EBlurType type,
                              float amount, u32 unk, float timeIn,
                              float timeOut, bool active);

    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    void Accept(IVisitor& visitor);
};
}

#endif // __URDE_CSCRIPTCAMERABLURKEYFRAME__
