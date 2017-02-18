#ifndef CRUMBLEGENERATOR_HPP
#define CRUMBLEGENERATOR_HPP

#include "CRumbleVoice.hpp"
#include "CInputGenerator.hpp"

namespace urde
{
class CRumbleGenerator
{
public:
    CRumbleGenerator();
    void Update(float);
    void HardStopAll();
    void Rumble(const SAdsrData& adsr, float, ERumblePriority prio, EIOPort port);
};
}

#endif // CRUMBLEGENERATOR_HPP
