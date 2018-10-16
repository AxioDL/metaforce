#pragma once

#include "CScriptTrigger.hpp"

namespace urde
{
class CScriptBallTrigger : public CScriptTrigger
{
    float x150_force;
    float x154_minAngle;
    float x158_maxDistance;
    zeus::CVector3f x15c_forceAngle = zeus::CVector3f::skZero;
    bool x168_24_canApplyForce : 1;
    bool x168_25_stopPlayer : 1;
public:
    CScriptBallTrigger(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CVector3f&,
                       const zeus::CVector3f&, bool, float, float, float, const zeus::CVector3f&, bool);

    void Accept(IVisitor&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void Think(float, CStateManager& mgr);
    void InhabitantAdded(CActor&, CStateManager&);
    void InhabitantExited(CActor&, CStateManager&);
};
}
