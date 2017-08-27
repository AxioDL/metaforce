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
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven
    };
private:
    EState x0_state = EState::Zero;
    SamusGun::EFidgetType x4_type = SamusGun::EFidgetType::Invalid;
    u32 x8_ = 0;
    s32 xc_parm2 = -1;
    u32 x10_ = 3;
    float x14_ = 0.f;
    float x18_ = 0.f;
    float x1c_ = 0.f;
    float x20_ = 0.f;
    float x24_ = 0.f;
    float x28_ = 0.f;
    float x2c_ = 0.f;
    float x30_ = 105.f;
    bool x34_24_loading = false;
public:
    EState GetState() const { return x0_state; }
    SamusGun::EFidgetType GetType() const { return x4_type; }
    s32 GetParm2() const { return xc_parm2; }
    EState Update(int fire, bool bobbing, bool b1, float dt, CStateManager& mgr);
    void ResetMinor();
    void ResetAll();
    void DoneLoading() { x34_24_loading = false; }
};

}

#endif // __URDE_CFIDGET_HPP__
