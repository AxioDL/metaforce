#ifndef CBODYSTATE_HPP
#define CBODYSTATE_HPP
#include "RetroTypes.hpp"
#include "CharacterCommon.hpp"

namespace urde
{
class CBodyController;
class CStateManager;
class CBodyState
{
public:
    virtual bool IsInAir(const CBodyController&) const { return false; }
    virtual bool IsDead() const { return false; }
    virtual bool IsMoving() const { return false; }
    virtual bool ApplyGravity() const { return true; }
    virtual bool ApplyHeadTracking() const { return true; }
    virtual bool ApplyAnimationDeltas() const { return true; }
    virtual bool CanShoot() const { return false; }
    virtual void Start(CBodyController&, CStateManager&) = 0;
    virtual void UpdateBody(float, CBodyController&, CStateManager&) = 0;
    virtual void Shutdown(CBodyController&)=0;
};

class CBSAttack : public CBodyState
{
public:
    virtual bool CanShoot() const { return false; }
    virtual void Start(CBodyController &, CStateManager &) {}
    virtual void UpdateBody(float, CBodyController &, CStateManager&);
    virtual void Shutdown(CBodyController&) {}
};

class CBSDie : public CBodyState
{
    bool x8_isDead = false;
public:
    bool IsDead() const { return x8_isDead; }
    void Start(CBodyController &, CStateManager &) {}
    void UpdateBody(float, CBodyController &, CStateManager&) {}
    void Shutdown(CBodyController &) {}
};

class CBSFall : public CBodyState
{
public:
    void Start(CBodyController &, CStateManager &) {}
    void UpdateBody(float, CBodyController &, CStateManager&) {}
    void Shutdown(CBodyController &) {}
};

class CBSGetup : public CBodyState
{
public:
    void Start(CBodyController &, CStateManager &) {}
    void UpdateBody(float, CBodyController &, CStateManager&) {}
    void Shutdown(CBodyController &) {}
};

class CBSKnockBack : public CBodyState
{
public:
    bool IsMoving() const { return true; }
    void Start(CBodyController &, CStateManager &) {}
    void UpdateBody(float, CBodyController &, CStateManager&) {}
    void Shutdown(CBodyController &) {}
};

class CBSLieOnGround : public CBodyState
{
public:
    void Start(CBodyController &, CStateManager &) {}
    void UpdateBody(float, CBodyController &, CStateManager&) {}
    void Shutdown(CBodyController &) {}
};

class CBSStep : public CBodyState
{
public:
    bool IsMoving() const { return true; }
    bool CanShoot() const { return true; }
    void Start(CBodyController &, CStateManager &) {}
    void UpdateBody(float, CBodyController &, CStateManager&) {}
    void Shutdown(CBodyController &) {}
};

class CBSTurn : public CBodyState
{
public:
    virtual bool CanShoot() const { return true; }
    virtual void Start(CBodyController &, CStateManager &) {}
    virtual void UpdateBody(float, CBodyController &, CStateManager&) {}
    virtual void Shutdown(CBodyController &) {}
    virtual void GetBodyStateTransition(float, CBodyController&);
};

class CBSFlyerTurn : public CBSTurn
{
public:
    virtual void Start(CBodyController &, CStateManager &) {}
    virtual void UpdateBody(float, CBodyController &, CStateManager&) {}
};

class CBSLoopAttack : public CBodyState
{
public:
    virtual bool CanShoot() const { return true; }
    virtual void Start(CBodyController &, CStateManager &) {}
    virtual void UpdateBody(float, CBodyController &, CStateManager&);
    virtual void Shutdown(CBodyController&) {}
};

class CBSLocomotion : public CBodyState
{
public:
    virtual bool IsMoving() const = 0;
    virtual bool CanShoot() const { return true; }
    virtual void Start(CBodyController &, CStateManager &) {}
    virtual void UpdateBody(float, CBodyController &, CStateManager&) {}
    virtual void Shutdown() const {}
    virtual bool IsPitchable() const { return true; }
    virtual float GetLocomotionSpeed(pas::ELocomotionType, pas::ELocomotionAnim) = 0;
    virtual void ApplyLocomotionPysics(float, CBodyController&) {}
    virtual void UpdateLocomotionAnimation(float, const CBodyController& )=0;
    virtual void GetBodyStateTransition(float, CBodyState&) {}
};
}
#endif // CBODYSTATE_HPP
