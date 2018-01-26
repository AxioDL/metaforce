#include "CRumbleGenerator.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

CRumbleGenerator::CRumbleGenerator()
{
    xf0_24_disabled = false;
    HardStopAll();
}

CRumbleGenerator::~CRumbleGenerator()
{
    HardStopAll();
}

void CRumbleGenerator::Update(float dt)
{
    if (!xf0_24_disabled)
    {
        bool updated = false;
        for (int i=0 ; i<4 ; ++i)
        {
            float intensity = x0_voices[i].GetIntensity();
            if (!x0_voices[i].Update(dt) || intensity <= 0.f)
            {
                xc0_periodTime[i] = 0.f;
                xd0_onTime[i] = 0.f;
                if (xe0_commandArray[i] != EMotorState::Stop)
                {
                    xe0_commandArray[i] = EMotorState::Stop;
                    updated = true;
                }
            }
            else
            {
                xc0_periodTime[i] += dt;
                if (xc0_periodTime[i] >= 1.f / (30.f * intensity))
                {
                    xc0_periodTime[i] = 0.f;
                    if (xe0_commandArray[i] != EMotorState::Rumble)
                    {
                        xe0_commandArray[i] = EMotorState::Rumble;
                        updated = true;
                    }
                }
                else
                {
                    xd0_onTime[i] += dt;
                    if (xd0_onTime[i] >= (1.f / 30.f))
                    {
                        xd0_onTime[i] = 0.f;
                        if (xe0_commandArray[i] != EMotorState::Stop)
                        {
                            xe0_commandArray[i] = EMotorState::Stop;
                            updated = true;
                        }
                    }
                }
            }
        }
        if (updated)
            g_InputGenerator->ControlAllMotors(xe0_commandArray);
    }
}

static const EMotorState HardStopCommands[] = { EMotorState::StopHard,
                                                EMotorState::StopHard,
                                                EMotorState::StopHard,
                                                EMotorState::StopHard };

void CRumbleGenerator::HardStopAll()
{
    for (int i=0 ; i<4 ; ++i)
    {
        xc0_periodTime[i] = 0.f;
        xd0_onTime[i] = 0.f;
        xe0_commandArray[i] = EMotorState::Stop;
        x0_voices[i].HardReset();
    }
    g_InputGenerator->ControlAllMotors(HardStopCommands);
}

s16 CRumbleGenerator::Rumble(const SAdsrData& adsr, float gain, ERumblePriority prio, EIOPort port)
{
    CRumbleVoice& vox = x0_voices[int(port)];
    s16 freeChan = vox.GetFreeChannel();
    if (prio >= vox.GetPriority(freeChan))
    {
        xc0_periodTime[int(port)] = 0.f;
        return vox.Activate(adsr, freeChan, gain, prio);
    }
    return -1;
}

void CRumbleGenerator::Stop(s16 id, EIOPort port)
{
    CRumbleVoice& vox = x0_voices[int(port)];
    vox.Deactivate(id, false);
}

}
