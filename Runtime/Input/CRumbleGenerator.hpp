#ifndef CRUMBLEGENERATOR_HPP
#define CRUMBLEGENERATOR_HPP

#include "CRumbleVoice.hpp"
#include "CInputGenerator.hpp"

namespace urde
{
class CRumbleGenerator
{
    CRumbleVoice x0_voices[4];
    float xc0_periodTime[4];
    float xd0_onTime[4];
    EMotorState xe0_commandArray[4];
    bool xf0_24_disabled : 1;
public:
    CRumbleGenerator();
    ~CRumbleGenerator();
    void Update(float dt);
    void HardStopAll();
    s16 Rumble(const SAdsrData& adsr, float, ERumblePriority prio, EIOPort port);
    void Stop(s16 id, EIOPort port);
    bool IsDisabled() const { return xf0_24_disabled; }
    void SetDisabled(bool disabled) { xf0_24_disabled = disabled; }
};
}

#endif // CRUMBLEGENERATOR_HPP
