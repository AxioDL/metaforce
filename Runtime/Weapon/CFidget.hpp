#ifndef __URDE_CFIDGET_HPP__
#define __URDE_CFIDGET_HPP__

#include "RetroTypes.hpp"
#include "CGunMotion.hpp"

namespace urde
{
class CStateManager;

class CFidget
{
public:
    enum class EState
    {
        NoFidget,
        MinorFidget,
        MajorFidget,
        HolsterBeam,
        StillMinorFidget,
        StillMajorFidget,
        StillHolsterBeam,
        Loading
    };
private:
    EState x0_state = EState::NoFidget;
    SamusGun::EFidgetType x4_type = SamusGun::EFidgetType::Invalid;
    u32 x8_delayTriggerBits = 0;
    // 0: panel, 1: panel reset, 2: adjust nozzle, 3: panel buttons
    s32 xc_animSet = -1;
    u32 x10_delayTimerEnableBits = 3;
    float x14_timeSinceFire = 0.f;
    float x18_timeSinceStrikeCooldown = 0.f;
    float x1c_timeSinceUnmorph = 0.f;
    float x20_timeSinceBobbing = 0.f;
    float x24_minorDelayTimer = 0.f;
    float x28_majorDelayTimer = 0.f;
    float x2c_holsterTimeSinceFire = 0.f;
    float x30_timeUntilHolster = 105.f;
    bool x34_24_loading = false;
public:
    EState GetState() const { return x0_state; }
    SamusGun::EFidgetType GetType() const { return x4_type; }
    s32 GetAnimSet() const { return xc_animSet; }
    EState Update(int fireButtonStates, bool bobbing, bool inStrikeCooldown, float dt, CStateManager& mgr);
    void ResetMinor();
    void ResetAll();
    void DoneLoading() { x34_24_loading = false; }
};

}

#endif // __URDE_CFIDGET_HPP__
