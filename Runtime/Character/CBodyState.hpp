#ifndef CBODYSTATE_HPP
#define CBODYSTATE_HPP

#include "RetroTypes.hpp"
#include "CharacterCommon.hpp"
#include "CBodyStateCmdMgr.hpp"

namespace urde
{
class CBodyController;
class CStateManager;
class CActor;
class CBodyState
{
public:
    virtual bool IsInAir(const CBodyController&) const { return false; }
    virtual bool IsDead() const { return false; }
    virtual bool IsDying() const { return false; }
    virtual bool IsMoving() const { return false; }
    virtual bool ApplyGravity() const { return true; }
    virtual bool ApplyHeadTracking() const { return true; }
    virtual bool ApplyAnimationDeltas() const { return true; }
    virtual bool CanShoot() const { return false; }
    virtual void Start(CBodyController&, CStateManager&) = 0;
    virtual pas::EAnimationState UpdateBody(float, CBodyController&, CStateManager&) = 0;
    virtual void Shutdown(CBodyController&)=0;
};

class CBSAttack : public CBodyState
{
    pas::EAnimationState x4_nextState = pas::EAnimationState::Invalid;
    CBCSlideCmd x8_slide;
    zeus::CVector3f x20_targetPos;
    float x2c_alignTargetPosStartTime = -1.f;
    float x30_alignTargetPosTime = -1.f;
    float x34_curTime = 0.f;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
    void UpdatePhysicsActor(CBodyController& bc, float dt);
public:
    bool CanShoot() const { return false; }
    void Start(CBodyController& bc, CStateManager & mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSProjectileAttack : public CBodyState
{
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    bool CanShoot() const { return true; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSDie : public CBodyState
{
    float x4_remTime = 0.f;
    bool x8_isDead = false;
public:
    bool IsDead() const { return x8_isDead; }
    bool IsDying() const { return true; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSFall : public CBodyState
{
    float x4_rotateSpeed = 0.f;
    float x8_remTime = 0.f;
    pas::EFallState xc_fallState = pas::EFallState::Invalid;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc);
};

class CBSGetup : public CBodyState
{
    pas::EFallState x4_fallState = pas::EFallState::Invalid;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc);
};

class CBSKnockBack : public CBodyState
{
    float x4_curTime = 0.f;
    float x8_rotateSpeed = 0.f;
    float xc_remTime = 0.f;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    bool IsMoving() const { return true; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSLieOnGround : public CBodyState
{
    bool x4_24_hasGroundHit : 1;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    CBSLieOnGround(CActor& actor);
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc);
};

class CBSStep : public CBodyState
{
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    bool IsMoving() const { return true; }
    bool CanShoot() const { return true; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSTurn : public CBodyState
{
protected:
    float x4_rotateSpeed = 0.f;
    zeus::CVector2f x8_dest;
    pas::ETurnDirection x10_turnDir = pas::ETurnDirection::Invalid;
    bool FacingDest(CBodyController& bc) const;
public:
    bool CanShoot() const { return true; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
    virtual pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};

class CBSFlyerTurn : public CBSTurn
{
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
};

class CBSLoopAttack : public CBodyState
{
    pas::ELoopState x4_state = pas::ELoopState::Invalid;
    pas::ELoopAttackType x8_loopAttackType = pas::ELoopAttackType::Invalid;
    bool xc_24_waitForAnimOver : 1;
    bool xc_25_advance : 1;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    CBSLoopAttack() { xc_24_waitForAnimOver = false; xc_25_advance = false; }
    bool CanShoot() const { return true; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSLoopReaction : public CBodyState
{
    pas::ELoopState x4_state = pas::ELoopState::Invalid;
    pas::EReactionType x8_reactionType = pas::EReactionType::Invalid;
    bool xc_24_loopHit : 1;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
    bool PlayExitAnimation(CBodyController& bc, CStateManager& mgr) const;
public:
    CBSLoopReaction() { xc_24_loopHit = false; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSGroundHit : public CBodyState
{
    float x4_rotateSpeed = 0.f;
    float x8_remTime = 0.f;
    pas::EFallState xc_fallState = pas::EFallState::Invalid;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc);
};

class CBSGenerate : public CBodyState
{
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSJump : public CBodyState
{
    pas::EJumpState x4_state = pas::EJumpState::Invalid;
    pas::EJumpType x8_jumpType;
    zeus::CVector3f xc_waypoint1;
    zeus::CVector3f x18_velocity;
    zeus::CVector3f x24_waypoint2;
    union
    {
        struct
        {
            bool x30_24_bodyForceSet : 1;
            bool x30_25_wallJump : 1;
            bool x30_26_wallBounceRight : 1;
            bool x30_27_wallBounceComplete : 1;
            bool x30_28_startInJumpLoop : 1;
        };
        u32 _dummy = 0;
    };
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
    bool CheckForWallJump(CBodyController& bc, CStateManager& mgr);
    void CheckForLand(CBodyController& bc, CStateManager& mgr);
    void PlayJumpLoop(CStateManager& mgr, CBodyController& bc);
public:
    bool IsMoving() const { return true; }
    bool ApplyHeadTracking() const { return false; }
    bool CanShoot() const;
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    bool ApplyAnimationDeltas() const;
    bool IsInAir(const CBodyController& bc) const;
    void Shutdown(CBodyController&) {}
};

class CBSHurled : public CBodyState
{
    pas::EHurledState x4_state = pas::EHurledState::Invalid;
    float x8_knockAngle = 0.f;
    int xc_animSeries = -1;
    float x10_rotateSpeed = 0.f;
    float x14_remTime = 0.f;
    float x18_curTime = 0.f;
    mutable zeus::CVector3f x1c_lastTranslation;
    mutable float x28_landedDur = 0.f;
    bool x2c_24_needsRecover : 1;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
    void Recover(CStateManager& mgr, CBodyController& bc, pas::EHurledState state);
    void PlayStrikeWallAnimation(CBodyController& bc, CStateManager& mgr);
    void PlayLandAnimation(CBodyController& bc, CStateManager& mgr);
    bool ShouldStartStrikeWall(CBodyController& bc) const;
    bool ShouldStartLand(float dt, CBodyController& bc) const;
public:
    CBSHurled() { x2c_24_needsRecover = false; }
    bool IsMoving() const { return true; }
    bool IsInAir(const CBodyController&) const { return true; }
    bool ApplyHeadTracking() const { return false; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSSlide : public CBodyState
{
    float x4_rotateSpeed = 0.f;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    bool ApplyHeadTracking() const { return false; }
    bool IsMoving() const { return true; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSTaunt : public CBodyState
{
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSScripted : public CBodyState
{
    union
    {
        struct
        {
            bool x4_24_loopAnim : 1;
            bool x4_25_timedLoop : 1;
        };
        u32 _dummy = 0;
    };
    float x8_remTime = 0.f;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    bool ApplyHeadTracking() const { return false; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSCover : public CBodyState
{
    pas::ECoverState x4_state = pas::ECoverState::Invalid;
    pas::ECoverDirection x8_coverDirection = pas::ECoverDirection::Invalid;
    bool xc_needsExit = false;
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
public:
    bool ApplyHeadTracking() const { return false; }
    bool IsMoving() const { return true; }
    bool CanShoot() const { return x4_state == pas::ECoverState::Lean; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSWallHang : public CBodyState
{
    pas::EWallHangState x4_state = pas::EWallHangState::Invalid;
    TUniqueId x8_wpId = kInvalidUniqueId;
    zeus::CVector3f xc_launchVel;
    union
    {
        struct
        {
            bool x18_24_launched : 1;
            bool x18_25_needsExit : 1;
        };
        u32 _dummy = 0;
    };
    pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
    void FixInPlace(CBodyController& bc);
    bool CheckForLand(CBodyController& bc, CStateManager& mgr);
    bool CheckForWall(CBodyController& bc, CStateManager& mgr);
    void SetLaunchVelocity(CBodyController& bc);
public:
    bool IsMoving() const { return true; }
    bool CanShoot() const { return x4_state == pas::EWallHangState::WallHang; }
    bool IsInAir(const CBodyController& bc) const;
    bool ApplyGravity() const;
    bool ApplyHeadTracking() const;
    bool ApplyAnimationDeltas() const;
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController&) {}
};

class CBSLocomotion : public CBodyState
{
protected:
    pas::ELocomotionType x4_locomotionType = pas::ELocomotionType::Invalid;
    float GetStartVelocityMagnitude(CBodyController& bc);
    void ReStartBodyState(CBodyController& bc, bool);
    float ComputeWeightPercentage(const std::pair<s32, float>& a,
                                  const std::pair<s32, float>& b, float f) const;
public:
    bool IsMoving() const = 0;
    bool CanShoot() const { return true; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    void Shutdown(CBodyController& bc);
    virtual bool IsPitchable() const { return false; }
    virtual float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const = 0;
    virtual float ApplyLocomotionPhysics(float dt, CBodyController& bc);
    virtual float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) = 0;
    virtual pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};

class CBSBiPedLocomotion : public CBSLocomotion
{
protected:
    rstl::reserved_vector<rstl::reserved_vector<std::pair<s32, float>, 8>, 14> x8_anims;
    pas::ELocomotionAnim x3c4_anim = pas::ELocomotionAnim::Invalid;
    float x3c8_primeTime;
    float UpdateRun(float vel, CBodyController& bc, pas::ELocomotionAnim anim);
    float UpdateWalk(float vel, CBodyController& bc, pas::ELocomotionAnim anim);
    float UpdateStrafe(float vel, CBodyController& bc, pas::ELocomotionAnim anim);
    const std::pair<s32, float>&
    GetLocoAnimation(pas::ELocomotionType type, pas::ELocomotionAnim anim) const
    { return x8_anims[int(type)][int(anim)]; }
public:
    CBSBiPedLocomotion(CActor& actor);
    bool IsMoving() const { return x3c4_anim != pas::ELocomotionAnim::Idle; }
    void Start(CBodyController& bc, CStateManager& mgr);
    pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr);
    float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const;
    float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init);
    virtual bool IsStrafing(const CBodyController& bc) const;
};

class CBSFlyerLocomotion : public CBSBiPedLocomotion
{
    bool x3cc_pitchable;
public:
    CBSFlyerLocomotion(CActor& actor, bool pitchable);
    bool IsPitchable() const { return x3cc_pitchable; }
    float ApplyLocomotionPhysics(float dt, CBodyController& bc);
    virtual bool IsBackPedal(CBodyController& bc) const { return false; }
};

class CBSWallWalkerLocomotion : public CBSBiPedLocomotion
{
public:
    CBSWallWalkerLocomotion(CActor& actor);
    float ApplyLocomotionPhysics(float dt, CBodyController& bc);
};

class CBSNewFlyerLocomotion : public CBSBiPedLocomotion
{
public:
    CBSNewFlyerLocomotion(CActor& actor);
    float ApplyLocomotionPhysics(float dt, CBodyController& bc);
    float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init);
};

class CBSRestrictedLocomotion : public CBSLocomotion
{
    rstl::reserved_vector<s32, 14> x8_anims;
    pas::ELocomotionAnim x44_anim = pas::ELocomotionAnim::Invalid;
public:
    CBSRestrictedLocomotion(CActor& actor);
    bool IsMoving() const { return false; }
    float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const { return 0.f; }
    float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init);
};

class CBSRestrictedFlyerLocomotion : public CBSRestrictedLocomotion
{
public:
    CBSRestrictedFlyerLocomotion(CActor& actor);
    float ApplyLocomotionPhysics(float dt, CBodyController& bc);
};
}
#endif // CBODYSTATE_HPP
