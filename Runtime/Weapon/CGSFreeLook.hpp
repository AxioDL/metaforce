#pragma once

#include "RetroTypes.hpp"

namespace urde
{

class CAnimData;
class CStateManager;
class CGSFreeLook
{
    float x0_delay = 0.f;
    s32 x4_cueAnimId = -1;
    s32 x8_loopState = -1; // In, loop, out
    s32 xc_gunId = 0;
    s32 x10_setId = -1;
    bool x14_idle = false;
public:
    s32 GetSetId() const { return x10_setId; }
    void SetLoopState(s32 l) { x8_loopState = l; }
    s32 GetLoopState() const { return x8_loopState; }
    void SetIdle(bool l) { x14_idle = l; }
    s32 GetGunId() const { return xc_gunId; }
    bool Update(CAnimData& data, float dt, CStateManager& mgr);
    s32 SetAnim(CAnimData& data, s32 gunId, s32 setId, s32 loopState, CStateManager& mgr, float delay);

};

}

