#ifndef __URDE_CSCRIPTWAYPOINT_HPP__
#define __URDE_CSCRIPTWAYPOINT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptWaypoint : public CActor
{
public:
    CScriptWaypoint(TUniqueId, const std::string&, const CEntityInfo&,
                    const zeus::CTransform&, bool, float, float,
                    u32, u32, u32, u32, u32, u32, u32);
};

}

#endif // __URDE_CSCRIPTWAYPOINT_HPP__
