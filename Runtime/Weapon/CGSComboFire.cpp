#include "CGSComboFire.hpp"
#include "Character/CAnimData.hpp"

namespace urde
{

bool CGSComboFire::Update(CAnimData& aData, float dt, CStateManager& mgr)
{
    if (x8_ == -1)
    {

    }
    else
    {
        x0_timeRemaining -= dt;
        if (x0_timeRemaining > 0.f)
            return false;

        //aData.x220_25_loop = (x4_ - 1) != 0;
        //aData.x220_24_animating = true;
    }

    return false;
}

void CGSComboFire::SetAnim(CAnimData&, s32, s32, CStateManager&, float)
{

}

}
