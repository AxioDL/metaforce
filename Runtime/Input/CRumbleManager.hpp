#ifndef __URDE_CRUMBLEMANAGER_HPP__
#define __URDE_CRUMBLEMANAGER_HPP__

#include "CRumbleGenerator.hpp"

namespace urde
{
class CStateManager;
class CRumbleManager
{
    CRumbleGenerator x0_rumbleGenerator;
    bool xf0_24_disabled : 1;
public:
    CRumbleManager() { xf0_24_disabled = false; }
    bool IsDisabled() const { return xf0_24_disabled; }
    void SetDisabled(bool disabled);
    void Update(float);
    void StopRumble(s16) {}
    void Rumble(CStateManager&, ERumbleFxId, ERumblePriority priority) {}
    void Rumble(CStateManager&, ERumbleFxId, float, ERumblePriority priority) {}
};
}

#endif // __URDE_CRUMBLEMANAGER_HPP__
