#ifndef __URDE_CADDITIVEBODYSTATE_HPP__
#define __URDE_CADDITIVEBODYSTATE_HPP__

#include "RetroTypes.hpp"
#include "CharacterCommon.hpp"
#include "CBodyStateCmdMgr.hpp"

namespace urde
{
class CBodyController;
class CStateManager;
class CActor;
class CAdditiveBodyState
{
public:
    virtual ~CAdditiveBodyState() = default;
    virtual bool ApplyHeadTracking() const { return true; }
    virtual bool CanShoot() const { return true; }
    virtual void Start(CBodyController& bc, CStateManager& mgr) = 0;
    virtual pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) = 0;
    virtual void Shutdown(CBodyController& bc) = 0;
};

class CABSAim : public CAdditiveBodyState
{
    bool x4_needsIdle = false;
    s32 x8_anims[4];
    float x18_angles[4];
    float x28_hWeight = 0.f;
    float x2c_hWeightVel = 0.f;
    float x30_vWeight = 0.f;
    float x34_vWeightVel = 0.f;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc);
};

class CABSFlinch : public CAdditiveBodyState
{
    float x4_weight = 1.f;
    u32 x8_anim = 0;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc) {}
};

class CABSIdle : public CAdditiveBodyState
{
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr) {}
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc) {}
};

class CABSReaction : public CAdditiveBodyState
{
    float x4_weight = 1.f;
    s32 x8_anim = -1;
    pas::EReactionType xc_type = pas::EReactionType::Invalid;
    bool x10_active = false;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
    void StopAnimation(CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc) { StopAnimation(bc); }
};

}

#endif // __URDE_CADDITIVEBODYSTATE_HPP__
