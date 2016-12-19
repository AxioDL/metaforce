#include "CScriptCameraBlurKeyframe.hpp"

namespace urde
{
CScriptCameraBlurKeyframe::CScriptCameraBlurKeyframe(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                                     u32, float, u32, float, float, bool active)
: CEntity(uid, info, active, name)
{
}
}
