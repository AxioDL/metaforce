#ifndef __URDE_CSCRIPTCAMERAWAYPOINT_HPP__
#define __URDE_CSCRIPTCAMERAWAYPOINT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptCameraWaypoint : public CActor
{
    float xe8_;
    u32 xec_;
public:
    CScriptCameraWaypoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                          const zeus::CTransform& xf, bool active, float, u32);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}
    void Render(const CStateManager&) const {}
};

}

#endif // __URDE_CSCRIPTCAMERAWAYPOINT_HPP__
