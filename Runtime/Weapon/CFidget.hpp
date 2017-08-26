#ifndef __URDE_CFIDGET_HPP__
#define __URDE_CFIDGET_HPP__

#include "RetroTypes.hpp"

namespace urde
{

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
    s32 x4_ = -1;
    u32 x8_ = 0;
    s32 xc_ = -1;
    u32 x10_ = 3;
    float x14_ = 0.f;
    float x18_ = 0.f;
    float x1c_ = 0.f;
    float x20_ = 0.f;
    float x24_ = 0.f;
    float x28_ = 0.f;
    float x2c_ = 0.f;
    float x30_ = 105.f;
    bool x34_24_ = false;
public:
    EState GetState() const { return x0_state; }
    void ResetAll();
};

}

#endif // __URDE_CFIDGET_HPP__
