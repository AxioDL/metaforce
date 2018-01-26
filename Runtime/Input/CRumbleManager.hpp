#ifndef __URDE_CRUMBLEMANAGER_HPP__
#define __URDE_CRUMBLEMANAGER_HPP__

#include "CRumbleGenerator.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CStateManager;
class CRumbleManager
{
    CRumbleGenerator x0_rumbleGenerator;
public:
    bool IsDisabled() const { return x0_rumbleGenerator.IsDisabled(); }
    void SetDisabled(bool disabled) { x0_rumbleGenerator.SetDisabled(disabled); }
    void Update(float dt) { x0_rumbleGenerator.Update(dt); }
    void StopRumble(s16 id)
    {
        if (id == -1)
            return;
        x0_rumbleGenerator.Stop(id, EIOPort::Zero);
    }
    s16 Rumble(CStateManager& mgr, const zeus::CVector3f& pos, ERumbleFxId fx, float dist, ERumblePriority priority);
    s16 Rumble(CStateManager& mgr, ERumbleFxId fx, float gain, ERumblePriority priority);
};
}

#endif // __URDE_CRUMBLEMANAGER_HPP__
