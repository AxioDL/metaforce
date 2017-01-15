#ifndef __URDE_CSCRIPTCAMERAWAYPOINT_HPP__
#define __URDE_CSCRIPTCAMERAWAYPOINT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptCameraWaypoint : public CActor
{
public:
    CScriptCameraWaypoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                          const zeus::CTransform& xf, bool active, float, u32);

    void Accept(IVisitor& visitor);
};

}

#endif // __URDE_CSCRIPTCAMERAWAYPOINT_HPP__
