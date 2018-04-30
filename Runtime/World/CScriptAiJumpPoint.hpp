#ifndef __URDE_CSCRIPTAIJUMPPOINT_HPP__
#define __URDE_CSCRIPTAIJUMPPOINT_HPP__

#include "CActor.hpp"

namespace urde
{
class CScriptAiJumpPoint : public CActor
{
private:
    float xe8_;
    std::experimental::optional<zeus::CAABox> xec_;
    union {
        struct
        {
            bool x108_24 : 1;
        };
        u8 dummy = 0;
    };
    TUniqueId x10a_occupant = kInvalidUniqueId;
    TUniqueId x10c_currentWaypoint = kInvalidUniqueId;
    TUniqueId x10e_nextWaypoint = kInvalidUniqueId;
    float x110_timeRemaining = 0.f;

public:
    CScriptAiJumpPoint(TUniqueId, std::string_view, const CEntityInfo&, zeus::CTransform&, bool, float);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}
    void Render(const CStateManager&) const {}
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    bool GetInUse(TUniqueId uid) const;
};
}
#endif // __URDE_CSCRIPTAIJUMPPOINT_HPP__
