#pragma once

#include "World/CPatterned.hpp"
#include "Collision/CCollisionActorManager.hpp"

namespace urde::MP1
{
class CSpankWeed : public CPatterned
{
    float x568_maxDetectionRange;
    float x56c_detectionHeightRange;
    float x570_maxHearingRange;
    float x574_maxSightRange;
    float x578_hideTime;
    bool x57c_canKnockBack = false;
    /* float x580_ = 0.f; unused */
    zeus::CVector3f x584_retreatOrigin;
    TUniqueId x590_ = kInvalidUniqueId;
    std::unique_ptr<CCollisionActorManager> x594_collisionMgr;
    bool x598_isHiding = true;
    zeus::CVector3f x59c_lockonOffset;
    zeus::CVector3f x5a8_lockonTarget;
    s32 x5b4_ = -1;
    s32 x5b8_ = -1;
    s32 x5bc_ = -1;

    float GetPlayerDistance(CStateManager&) const;
public:
    DEFINE_PATTERNED(SpankWeed)

    CSpankWeed(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
               const CActorParameters&, const CPatternedInfo&, float, float, float, float);

    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void Think(float, CStateManager&);
    zeus::CVector3f GetOrbitPosition(const CStateManager&) const;
    zeus::CVector3f GetAimPosition(const CStateManager&, float) const;
    bool AnimOver(CStateManager&, float) { return x5bc_ == 3; }
    void Flinch(CStateManager&, EStateMsg, float);
    bool Delay(CStateManager&, float);
    bool InRange(CStateManager&, float);
    bool HearPlayer(CStateManager&, float);
    bool InDetectionRange(CStateManager&, float);
    void Attack(CStateManager&, EStateMsg, float);
    void TargetPatrol(CStateManager&, EStateMsg, float);
    void Lurk(CStateManager&, EStateMsg, float);
    void FadeOut(CStateManager&, EStateMsg, float);
    void FadeIn(CStateManager&, EStateMsg, float);
    void Patrol(CStateManager&, EStateMsg, float);
    void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info,
                            EKnockBackType type, bool inDeferred, float magnitude);
};
}