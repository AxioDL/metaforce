#include "Kyoto/Input/CRumbleVoice.hpp"
#include "rstl/math.hpp"

CRumbleVoice::CRumbleVoice()
: x0_datas(4, SAdsrData())
, x10_deltas(4, SAdsrDelta::Stopped())
, x20_handleIds(0)
, x2c_usedChannels(0)
, x2e_lastId(0) {}

short CRumbleVoice::Activate(const SAdsrData& data, ushort idx, float gain, ERumblePriority prio) {
  if (gain > 0.f) {
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

void CRumbleVoice::Deactivate(short id, bool b1) {
  if (id == -1 || !OwnsSustained(id)) {
    return;
  }

  if (x2c_usedChannels & (1 << GetChannelId(id))) {
    x10_deltas[GetChannelId(id)].x20_phase = SAdsrDelta::kP_Release;
  }
}

void CRumbleVoice::HardReset() {
  x2c_usedChannels = 0;
  for (ushort i = 0; i < 4; ++i) {
    x10_deltas[i] = SAdsrDelta::Stopped();
    x20_handleIds[i] = 0;
  }
}

inline float CRumbleVoice::EnvelopeLerp(float t, float start, float end) {
  return start * (1.f - t) + end * t;
}

inline void CRumbleVoice::UpdateStage(SAdsrDelta::EPhase& phase, float& intensity, float& time,
                                     float start, float end, float duration,
                                     SAdsrDelta::EPhase nextPhase, float dt) {
  if (time < duration) {
    const float t = time / duration;
    intensity = EnvelopeLerp(t, start, end);
    time += dt;
  } else {
    intensity = end;
    phase = nextPhase;
  }
}

bool CRumbleVoice::UpdateChannel(SAdsrDelta& delta, const SAdsrData& data, float dt) {
  switch (delta.x20_phase) {
  case SAdsrDelta::kP_PrePulse:
    if (delta.x4_attackTime < (1.f / 30.f)) {
      delta.x4_attackTime += dt;
    } else {
      delta.x20_phase = SAdsrDelta::kP_Attack;
      delta.x0_curIntensity = 0.f;
      delta.x4_attackTime = 0.f;
    }
    break;
  case SAdsrDelta::kP_Attack:
    UpdateStage(delta.x20_phase, delta.x0_curIntensity, delta.x4_attackTime, 0.f,
                delta.x14_attackIntensity, data.x8_attackDur, SAdsrDelta::kP_Decay, dt);
    break;
  case SAdsrDelta::kP_Decay:
    if (data.x18_24_hasSustain) {
      UpdateStage(delta.x20_phase, delta.x0_curIntensity, delta.x8_decayTime,
                  delta.x14_attackIntensity, delta.x18_sustainIntensity, data.xc_decayDur,
                  SAdsrDelta::kP_Sustain, dt);
    } else {
      UpdateStage(delta.x20_phase, delta.x0_curIntensity, delta.x8_decayTime,
                  delta.x14_attackIntensity, 0.f, data.xc_decayDur, SAdsrDelta::kP_Stop, dt);
      if (delta.x20_phase != SAdsrDelta::kP_Decay) {
        delta.x20_phase = SAdsrDelta::kP_Stop;
        return true;
      }
    }
    break;
  case SAdsrDelta::kP_Release: {
    float a = data.x18_24_hasSustain ? delta.x18_sustainIntensity : 0.f;
    UpdateStage(delta.x20_phase, delta.x0_curIntensity, delta.xc_releaseTime, a, 0.f,
                data.x14_releaseDur, SAdsrDelta::kP_Stop, dt);
    if (delta.x20_phase != SAdsrDelta::kP_Release) {
      delta.x20_phase = SAdsrDelta::kP_Stop;
      return true;
    }
  } break;
  default:
    break;
  }

  if (data.x18_25_autoRelease) {
    if (delta.x10_autoReleaseTime < data.x4_autoReleaseDur)
      delta.x10_autoReleaseTime += dt;
    else if (delta.x20_phase == SAdsrDelta::kP_Sustain)
      delta.x20_phase = SAdsrDelta::kP_Release;
  }

  return false;
}

bool CRumbleVoice::Update(float dt) {
  if (x2c_usedChannels != 0) {
    for (ushort i = 0; i < 4; ++i) {
      if (x2c_usedChannels & (1 << i)) {
        if (UpdateChannel(x10_deltas[i], x0_datas[i], dt)) {
          x2c_usedChannels &= ~(1 << i);
          x10_deltas[i] = SAdsrDelta::Stopped();
        }
      }
    }
    return true;
  }
  return false;
}

ushort CRumbleVoice::GetFreeChannel() const {
  for (ushort i = 0; i < 4; ++i) {
    if ((x2c_usedChannels & (1 << i)) == 0) {
      return (ushort)i;
    }
  }
  return 0;
}

float CRumbleVoice::GetIntensity() const {
  float ret = x10_deltas[0].x0_curIntensity;
  for (int i = 1; i < 4; ++i) {
    if (ret < x10_deltas[i].x0_curIntensity) {
      ret = x10_deltas[i].x0_curIntensity;
    }
  }

  if (ret > 2.f) {
    return 2.f;
  }

  return ret;
}

bool CRumbleVoice::OwnsSustained(short handle) const {
  const ushort i = GetChannelId(handle);
  const uint owner = GetOwnerId(handle);
  return i < 4 ? x20_handleIds[i] == owner : false;
}

/* TODO: Fake matched, find real solution */
short CRumbleVoice::CreateRumbleHandle(const ushort idx) {
  ++x2e_lastId;
  if (x2e_lastId == 0)
    x2e_lastId = 1;
  ushort* h = &x20_handleIds[idx];
  *h = x2e_lastId;
  return ((x2e_lastId << 8) | idx) & 0xFFFF;
}
