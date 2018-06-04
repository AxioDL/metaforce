#ifndef __URDE_CAMBIENTAI_HPP__
#define __URDE_CAMBIENTAI_HPP__
#include "CPhysicsActor.hpp"
#include "CDamageVulnerability.hpp"

namespace urde
{
class CAmbientAI : public CPhysicsActor
{
    enum class EAnimationState
    {
        Ready,
        Alert,
        Impact
    };

    CHealthInfo x258_initialHealthInfo;
    CHealthInfo x260_healthInfo;
    CDamageVulnerability x268_dVuln;
    EAnimationState x2d0_animState= EAnimationState::Ready;
    float x2d4_alertRange;
    float x2d8_impactRange;
    s32 x2dc_defaultAnim;
    s32 x2e0_alertAnim;
    s32 x2e4_impactAnim;

    union
    {
        struct { bool x2e8_24_dead : 1; bool x2e8_25_animating : 1; };
        u32 _dummy = 0;
    };

public:
    CAmbientAI(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&,
               CModelData&&, const zeus::CAABox&, const CMaterialList&, float, const CHealthInfo&, const CDamageVulnerability&,
               const CActorParameters&, float, float, s32, s32, bool);

    void Accept(IVisitor&);
    void Think(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    CHealthInfo* HealthInfo(CStateManager&) { return &x260_healthInfo; }
    const CDamageVulnerability* GetDamageVulnerability() const { return &x268_dVuln; }
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager&) {}
    void RandomizePlaybackRate(CStateManager&);
};

}
#endif // __URDE_CAMBIENTAI_HPP
