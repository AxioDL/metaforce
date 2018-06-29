#ifndef __URDE_CSCRIPTWAYPOINT_HPP__
#define __URDE_CSCRIPTWAYPOINT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptWaypoint : public CActor
{
    float xe8_speed;
    u32 xec_;
    float xf0_;
    bool xf4_;
    bool xf5_;
    bool xf6_;
    bool xf7_;
    bool xf8_;
    u16 xfa_jumpFlags;
public:
    CScriptWaypoint(TUniqueId, std::string_view, const CEntityInfo&,
                    const zeus::CTransform&, bool, float, float,
                    u32, u32, u32, u32, u32, u32, u32);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    TUniqueId FollowWaypoint(CStateManager& mgr) const;
    TUniqueId NextWaypoint(CStateManager& mgr) const;
    float GetSpeed() const { return xe8_speed; }
};
}

#endif // __URDE_CSCRIPTWAYPOINT_HPP__
