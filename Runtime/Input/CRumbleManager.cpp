#include "CRumbleManager.hpp"

namespace urde
{

void CRumbleManager::SetDisabled(bool disabled)
{
    if (disabled)
        x0_rumbleGenerator.HardStopAll();
    xf0_24_disabled = disabled;
}

void CRumbleManager::Update(float dt) { x0_rumbleGenerator.Update(dt); }

}
