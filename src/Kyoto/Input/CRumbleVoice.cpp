#include "Kyoto/Input/CRumbleVoice.hpp"
#include "rstl/math.hpp"

CRumbleVoice::CRumbleVoice()
: mDatas(4, SAdsrData())
, mDeltas(4, SAdsrDelta::Stopped())
, mHandleIds(0)
, mUsedChannels(0)
, mLastId(0) {}

short CRumbleVoice::Activate(const SAdsrData& data, ushort idx, float gain, ERumblePriority prio) {
  if (gain > 0.f) {
    mDatas[idx] = data;
    mDeltas[idx] = SAdsrDelta::Start(prio, mUsedChannels == 0);
    mDeltas[idx].mAttackIntensity = gain * mDatas[idx].mAttackGain;
    mDeltas[idx].mSustainIntensity = gain * mDatas[idx].mSustainGain;
    mUsedChannels |= 1 << idx;
    if (data.mHasSustain)
      return CreateRumbleHandle(idx);
  }
  return -1;
}

void CRumbleVoice::Deactivate(short id, bool b1) {
  if (id == -1 || !OwnsSustained(id)) {
    return;
  }

  if (mUsedChannels & (1 << GetChannelId(id))) {
    mDeltas[GetChannelId(id)].mPhase = SAdsrDelta::kP_Release;
  }
}

void CRumbleVoice::HardReset() {
  mUsedChannels = 0;
  for (ushort i = 0; i < 4; ++i) {
    mDeltas[i] = SAdsrDelta::Stopped();
    mHandleIds[i] = 0;
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
  switch (delta.mPhase) {
  case SAdsrDelta::kP_PrePulse:
    if (delta.mAttackTime < (1.f / 30.f)) {
      delta.mAttackTime += dt;
    } else {
      delta.mPhase = SAdsrDelta::kP_Attack;
      delta.mCurIntensity = 0.f;
      delta.mAttackTime = 0.f;
    }
    break;
  case SAdsrDelta::kP_Attack:
    UpdateStage(delta.mPhase, delta.mCurIntensity, delta.mAttackTime, 0.f,
                delta.mAttackIntensity, data.mAttackDur, SAdsrDelta::kP_Decay, dt);
    break;
  case SAdsrDelta::kP_Decay:
    if (data.mHasSustain) {
      UpdateStage(delta.mPhase, delta.mCurIntensity, delta.mDecayTime,
                  delta.mAttackIntensity, delta.mSustainIntensity, data.mDecayDur,
                  SAdsrDelta::kP_Sustain, dt);
    } else {
      UpdateStage(delta.mPhase, delta.mCurIntensity, delta.mDecayTime,
                  delta.mAttackIntensity, 0.f, data.mDecayDur, SAdsrDelta::kP_Stop, dt);
      if (delta.mPhase != SAdsrDelta::kP_Decay) {
        delta.mPhase = SAdsrDelta::kP_Stop;
        return true;
      }
    }
    break;
  case SAdsrDelta::kP_Release: {
    float a = data.mHasSustain ? delta.mSustainIntensity : 0.f;
    UpdateStage(delta.mPhase, delta.mCurIntensity, delta.mReleaseTime, a, 0.f,
                data.mReleaseDur, SAdsrDelta::kP_Stop, dt);
    if (delta.mPhase != SAdsrDelta::kP_Release) {
      delta.mPhase = SAdsrDelta::kP_Stop;
      return true;
    }
  } break;
  default:
    break;
  }

  if (data.mAutoRelease) {
    if (delta.mAutoReleaseTime < data.mAutoReleaseDur)
      delta.mAutoReleaseTime += dt;
    else if (delta.mPhase == SAdsrDelta::kP_Sustain)
      delta.mPhase = SAdsrDelta::kP_Release;
  }

  return false;
}

bool CRumbleVoice::Update(float dt) {
  if (mUsedChannels != 0) {
    for (ushort i = 0; i < 4; ++i) {
      if (mUsedChannels & (1 << i)) {
        if (UpdateChannel(mDeltas[i], mDatas[i], dt)) {
          mUsedChannels &= ~(1 << i);
          mDeltas[i] = SAdsrDelta::Stopped();
        }
      }
    }
    return true;
  }
  return false;
}

ushort CRumbleVoice::GetFreeChannel() const {
  for (ushort i = 0; i < 4; ++i) {
    if ((mUsedChannels & (1 << i)) == 0) {
      return (ushort)i;
    }
  }
  return 0;
}

float CRumbleVoice::GetIntensity() const {
  float ret = mDeltas[0].mCurIntensity;
  for (int i = 1; i < 4; ++i) {
    if (ret < mDeltas[i].mCurIntensity) {
      ret = mDeltas[i].mCurIntensity;
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
  return i < 4 ? mHandleIds[i] == owner : false;
}

/* TODO: Fake matched, find real solution */
short CRumbleVoice::CreateRumbleHandle(const ushort idx) {
  ++mLastId;
  if (mLastId == 0)
    mLastId = 1;
  ushort* h = &mHandleIds[idx];
  *h = mLastId;
  return ((mLastId << 8) | idx) & 0xFFFF;
}
