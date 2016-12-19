#ifndef __URDE_CSCRIPTCAMERAHINT_HPP__
#define __URDE_CSCRIPTCAMERAHINT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptCameraHint : public CActor
{
public:
    CScriptCameraHint(TUniqueId, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf, bool,
                      u32, u32, u32, float, float, float, const zeus::CVector3f&, const zeus::CVector3f&,
                      const zeus::CVector3f&, float, float, float, float, float, float, float, float, float, float);
};
}

#endif // __URDE_CSCRIPTCAMERAHINT_HPP__
