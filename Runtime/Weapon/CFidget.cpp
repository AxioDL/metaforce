#include "CFidget.hpp"

namespace urde
{

void CFidget::ResetAll()
{
    x0_state = EState::Zero;
    x4_ = -1;
    x18_ = 0.f;
    x1c_ = 0.f;
    x14_ = 0.f;
    x24_ = 0.f;
    x28_ = 0.f;
    x2c_ = 0.f;
    x8_ = 0;
    xc_ = -1;
    x10_ = 3;
    x34_24_ = false;
}

}