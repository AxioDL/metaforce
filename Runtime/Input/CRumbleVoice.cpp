#include "CRumbleVoice.hpp"

namespace urde
{

CRumbleVoice::CRumbleVoice()
{
    x0_datas.resize(4);
    x10_deltas.resize(4, SAdsrDelta::Stopped());
    x20_handleIds.resize(4);
}

s16 CRumbleVoice::CreateRumbleHandle(s16 idx)
{
    ++x2e_lastId;
    if (x2e_lastId == 0)
        x2e_lastId = 1;
    x20_handleIds[idx] = x2e_lastId;
    return (x2e_lastId << 8) | idx;
}

bool CRumbleVoice::OwnsSustained(s16 handle) const
{
    int idx = handle & 0xf;
    if (idx < 4)
        return x20_handleIds[idx] == ((handle >> 8) & 0xff);
    return false;
}

s16 CRumbleVoice::GetFreeChannel() const
{
    for (s16 i=0 ; i<4 ; ++i)
        if (!((1 << i) & x2c_usedChannels))
            return i;
    return false;
}

float CRumbleVoice::GetIntensity() const
{
    return std::min(2.f,
                    std::max(x10_deltas[0].x0_curIntensity,
                    std::max(x10_deltas[1].x0_curIntensity,
                    std::max(x10_deltas[2].x0_curIntensity,
                             x10_deltas[3].x0_curIntensity))));
}

bool CRumbleVoice::UpdateChannel(SAdsrDelta& delta, const SAdsrData& data, float dt)
{
    switch (delta.x20_phase)
    {
    case SAdsrDelta::EPhase::PrePulse:
        if (delta.x4_attackTime < (1.f/30.f))
        {
            delta.x4_attackTime += dt;
        }
        else
        {
            delta.x20_phase = SAdsrDelta::EPhase::Attack;
            delta.x0_curIntensity = 0.f;
            delta.x4_attackTime = 0.f;
        }
        break;
    case SAdsrDelta::EPhase::Attack:
        if (delta.x4_attackTime < data.x8_attackDur)
        {
            float t = delta.x4_attackTime / data.x8_attackDur;
            delta.x0_curIntensity = t * delta.x14_attackIntensity;
            delta.x4_attackTime += dt;
        }
        else
        {
            delta.x0_curIntensity = delta.x14_attackIntensity;
            delta.x20_phase = SAdsrDelta::EPhase::Decay;
        }
        break;
    case SAdsrDelta::EPhase::Decay:
        if (data.x18_24_hasSustain)
        {
            if (delta.x8_decayTime < data.xc_decayDur)
            {
                float t = delta.x8_decayTime / data.xc_decayDur;
                delta.x0_curIntensity = (1.f - t) * delta.x14_attackIntensity + t * delta.x18_sustainIntensity;
                delta.x8_decayTime += dt;
            }
            else
            {
                delta.x0_curIntensity = delta.x18_sustainIntensity;
                delta.x20_phase = SAdsrDelta::EPhase::Sustain;
            }
        }
        else
        {
            if (delta.x8_decayTime < data.xc_decayDur)
            {
                float t = delta.x8_decayTime / data.xc_decayDur;
                delta.x0_curIntensity = (1.f - t) * delta.x14_attackIntensity;
                delta.x8_decayTime += dt;
            }
            else
            {
                delta.x0_curIntensity = 0.f;
                delta.x20_phase = SAdsrDelta::EPhase::Stop;
            }
            if (delta.x20_phase != SAdsrDelta::EPhase::Decay)
            {
                delta.x20_phase = SAdsrDelta::EPhase::Stop;
                return true;
            }
        }
        break;
    case SAdsrDelta::EPhase::Release:
    {
        float a = data.x18_24_hasSustain ? delta.x18_sustainIntensity : 0.f;
        if (delta.xc_releaseTime < data.x14_releaseDur)
        {
            float t = delta.xc_releaseTime / data.x14_releaseDur;
            delta.x0_curIntensity = (1.f - t) * a;
            delta.xc_releaseTime += dt;
        }
        else
        {
            delta.x0_curIntensity = 0.f;
            delta.x20_phase = SAdsrDelta::EPhase::Stop;
        }
        if (delta.x20_phase != SAdsrDelta::EPhase::Release)
        {
            delta.x20_phase = SAdsrDelta::EPhase::Stop;
            return true;
        }
    }
    break;
    default:
        break;
    }

    if (data.x18_25_autoRelease)
    {
        if (delta.x10_autoReleaseTime < data.x4_autoReleaseDur)
            delta.x10_autoReleaseTime += dt;
        else if (delta.x20_phase == SAdsrDelta::EPhase::Sustain)
            delta.x20_phase = SAdsrDelta::EPhase::Release;
    }

    return false;
}

bool CRumbleVoice::Update(float dt)
{
    if (x2c_usedChannels != 0)
    {
        for (s16 i=0 ; i<4 ; ++i)
        {
            if ((1 << i) & x2c_usedChannels)
            {
                if (UpdateChannel(x10_deltas[i], x0_datas[i], dt))
                {
                    x2c_usedChannels &= ~(1 << i);
                    x10_deltas[i] = SAdsrDelta::Stopped();
                }
            }
        }
        return true;
    }
    return false;
}

void CRumbleVoice::HardReset()
{
    x2c_usedChannels = 0;
    for (s16 i=0 ; i<4 ; ++i)
    {
        x10_deltas[i] = SAdsrDelta::Stopped();
        x20_handleIds[i] = 0;
    }
}

s16 CRumbleVoice::Activate(const SAdsrData& data, s16 idx, float gain, ERumblePriority prio)
{
    if (gain > 0.f)
    {
        x0_datas[idx] = data;
        x10_deltas[idx] = SAdsrDelta::Start(prio, x2c_usedChannels == 0);
        x10_deltas[idx].x14_attackIntensity = gain * x0_datas[idx].x0_attackGain;
        x10_deltas[idx].x18_sustainIntensity = gain * x0_datas[idx].x10_sustainGain;
        x2c_usedChannels |= 1 << idx;
        if (data.x18_24_hasSustain)
            return CreateRumbleHandle(idx);
    }
    return -1;
}

void CRumbleVoice::Deactivate(s16 id, bool b1)
{
    if (id == -1)
        return;
    if (OwnsSustained(id))
    {
        int handle = (id & 0xf);
        if (x2c_usedChannels & (1 << handle))
            x10_deltas[handle].x20_phase = SAdsrDelta::EPhase::Release;
    }
}

}
