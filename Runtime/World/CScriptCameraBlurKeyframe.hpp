#ifndef __URDE_CSCRIPTCAMERABLURKEYFRAME__
#define __URDE_CSCRIPTCAMERABLURKEYFRAME__

#include "CEntity.hpp"

namespace urde
{
class CScriptCameraBlurKeyframe : public CEntity
{
public:
    CScriptCameraBlurKeyframe(TUniqueId, const std::string&, const CEntityInfo&, u32, float, u32, float, float, bool);
};
}

#endif // __URDE_CSCRIPTCAMERABLURKEYFRAME__
