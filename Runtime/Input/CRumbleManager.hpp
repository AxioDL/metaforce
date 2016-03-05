#ifndef __PSHAG_CRUMBLEMANAGER_HPP__
#define __PSHAG_CRUMBLEMANAGER_HPP__

#include "CRumbleGenerator.hpp"

namespace urde
{
class CStateManager;
class CRumbleManager
{
    CRumbleGenerator x0_rumbleGenerator;
public:
    CRumbleManager() = default;
    void Update(float);
    void StopRumble(u16) {}
    void Rumble(ERumbleFxId, CStateManager&, ERumblePriority priority);
    void Rumble(ERumbleFxId, float, CStateManager&, ERumblePriority priority);
};
}

#endif // __PSHAG_CRUMBLEMANAGER_HPP__
