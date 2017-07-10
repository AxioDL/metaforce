#ifndef __URDE_CSCRIPTWAYPOINT_HPP__
#define __URDE_CSCRIPTWAYPOINT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptWaypoint : public CActor
{
    u16 xfa_jumpFlags;
public:
    CScriptWaypoint(TUniqueId, const std::string&, const CEntityInfo&,
                    const zeus::CTransform&, bool, float, float,
                    u32, u32, u32, u32, u32, u32, u32);

    void Accept(IVisitor& visitor);

    const CScriptWaypoint* NextWaypoint(CStateManager&) const { return nullptr; }
};
}

#endif // __URDE_CSCRIPTWAYPOINT_HPP__
