#pragma once

#include "CActor.hpp"

namespace urde
{

class CScriptWaypoint : public CActor
{
    float xe8_speed;
    u32 xec_animation;
    float xf0_pause;
    u8 xf4_patternTranslate;
    u8 xf5_patternOrient;
    u8 xf6_patternFit;
    u8 xf7_behaviour;
    u8 xf8_behaviourOrient;
    u16 xfa_behaviourModifiers;
public:
    CScriptWaypoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                    const zeus::CTransform& xf, bool active, float speed, float pause,
                    u32 patternTranslate, u32 patternOrient, u32 patternFit, u32 behaviour,
                    u32 behaviourOrient, u32 behaviourModifiers, u32 animation);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    TUniqueId FollowWaypoint(CStateManager& mgr) const;
    TUniqueId NextWaypoint(CStateManager& mgr) const;
    float GetSpeed() const { return xe8_speed; }
    u32 GetAnimation() const { return xec_animation; }
    float GetPause() const { return xf0_pause; }
    u8 GetPatternTranslate() const { return xf4_patternTranslate; }
    u8 GetPatternOrient() const { return xf5_patternOrient; }
    u8 GetPatternFit() const { return xf6_patternFit; }
    u8 GetBehaviour() const { return xf7_behaviour; }
    u8 GetBehaviourOrient() const { return xf8_behaviourOrient; }
    u16 GetBehaviourModifiers() const { return xfa_behaviourModifiers; }
};
}

