#include "CScriptCameraFilterKeyframe.hpp"

namespace urde
{
CScriptCameraFilterKeyframe::CScriptCameraFilterKeyframe(TUniqueId uid, const std::string& name,
                                                         const CEntityInfo& info, u32, u32, u32, u32,
                                                         const zeus::CColor&, float, float, u32, bool active)
: CEntity(uid, info, active, name)
{
}
}
