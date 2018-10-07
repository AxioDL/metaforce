#pragma once

#include "RetroTypes.hpp"

namespace urde
{

class CAnimData;
class CStateManager;
class CGSComboFire
{
    float x0_delay = 0.f;
    s32 x4_loopState = -1; // In, loop, out
    s32 x8_cueAnimId = -1;
    s32 xc_gunId = -1;

    union
    {
        struct
        {
            bool x10_24_over : 1;
            bool x10_25_idle : 1;
        };
        u8 _dummy = 0;
    };
public:
    bool IsComboOver() const { return x10_24_over; }
    s32 GetLoopState() const { return x4_loopState; }
    void SetLoopState(s32 l) { x4_loopState = l; }
    void SetIdle(bool i) { x10_25_idle = i; }
    s32 GetGunId() const { return xc_gunId; }
    bool Update(CAnimData& data, float dt, CStateManager& mgr);
    s32 SetAnim(CAnimData& data, s32 gunId, s32 loopState, CStateManager& mgr, float delay);
};

}

