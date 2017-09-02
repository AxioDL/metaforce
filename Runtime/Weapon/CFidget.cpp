#include "CFidget.hpp"

namespace urde
{

CFidget::EState CFidget::Update(int fire, bool bobbing, bool inStrikeCooldown, float dt, CStateManager& mgr)
{
    return EState::Zero;
}

void CFidget::ResetMinor()
{

}

void CFidget::ResetAll()
{
    x0_state = EState::Zero;
    x4_type = SamusGun::EFidgetType::Invalid;
    x18_ = 0.f;
    x1c_ = 0.f;
    x14_ = 0.f;
    x24_ = 0.f;
    x28_ = 0.f;
    x2c_ = 0.f;
    x8_ = 0;
    xc_parm2 = -1;
    x10_ = 3;
    x34_24_loading = false;
}

}
