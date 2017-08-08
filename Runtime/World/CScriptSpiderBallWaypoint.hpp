#ifndef __URDE_CSCRIPTSPIDERBALLWAYPOINT_HPP__
#define __URDE_CSCRIPTSPIDERBALLWAYPOINT_HPP__

#include "CActor.hpp"

namespace urde
{
class CScriptSpiderBallWaypoint : public CActor
{
    enum class ECheckActiveWaypoint
    {
        No,
        Yes
    };
    u32 xe8_;
    std::vector<TUniqueId> xec_waypoints;
    std::experimental::optional<zeus::CAABox> xfc_aabox;
public:
    CScriptSpiderBallWaypoint(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, bool, u32);
    void Accept(IVisitor&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void Render(const CStateManager& mgr) const { CActor::Render(mgr); }
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) {}
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const { return xfc_aabox; }
    void AccumulateBounds(const zeus::CVector3f&);
    void BuildWaypointListAndBounds(CStateManager&);
    void AddPreviousWaypoint(TUniqueId);
    TUniqueId NextWaypoint(const CStateManager&, ECheckActiveWaypoint);
};
}

#endif // __URDE_CSCRIPTSPIDERBALLWAYPOINT_HPP__