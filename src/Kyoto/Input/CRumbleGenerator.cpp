#include "Kyoto/Input/CRumbleGenerator.hpp"

const EMotorState CRumbleGenerator::kStopAll[4] = {
    kMS_StopHard,
    kMS_StopHard,
    kMS_StopHard,
    kMS_StopHard,
};

CRumbleGenerator::CRumbleGenerator() : mDisabled(false) { HardStopAll(); }

CRumbleGenerator::~CRumbleGenerator() { HardStopAll(); }

short CRumbleGenerator::Rumble(const SAdsrData& adsr, float gain, ERumblePriority prio,
                               EIOPort port) {
  ushort freeChan = mVoices[port].GetFreeChannel();
  if (prio >= mVoices[port].GetPriority(freeChan)) {
    mPeriodTime[port] = 0.f;
    mOnTime[port] = 0.f;
    return mVoices[port].Activate(adsr, freeChan, gain, prio);
  }
  return -1;
}

void CRumbleGenerator::Update(float dt) {
  if (!mDisabled) {
    bool updated = false;
    for (int i = 0; i < 4; ++i) {
      const float intensity = mVoices[i].GetIntensity();
      if (!mVoices[i].Update(dt) || intensity <= 0.f) {
        mPeriodTime[i] = 0.f;
        mOnTime[i] = 0.f;
        if (mCommandArray[i] != kMS_Stop) {
          mCommandArray[i] = kMS_Stop;
          updated = true;
        }
      } else {
        mPeriodTime[i] += dt;
        if (mPeriodTime[i] >= 1.f / (30.f * intensity)) {
          mPeriodTime[i] = 0.f;
          if (mCommandArray[i] != kMS_Rumble) {
            mCommandArray[i] = kMS_Rumble;
            updated = true;
          }
        } else {
          mOnTime[i] += dt;
          if (mOnTime[i] >= (1.f / 30.f)) {
            mOnTime[i] = 0.f;
            if (mCommandArray[i] != kMS_Stop) {
              mCommandArray[i] = kMS_Stop;
              updated = true;
            }
          }
        }
      }
    }
    if (updated) {
      PADControlAllMotors(reinterpret_cast< const u32* >(mCommandArray));
    }
  }
}

void CRumbleGenerator::HardStopAll() {

  for (int i = 0; i < 4; ++i) {
    mPeriodTime[i] = 0.f;
    mOnTime[i] = 0.f;
    mCommandArray[i] = kMS_Stop;
    mVoices[i].HardReset();
  }

  PADControlAllMotors(reinterpret_cast< const u32* >(kStopAll));
}

void CRumbleGenerator::SetDisabled(const bool disabled) {
  if (disabled) {
    HardStopAll();
  }

  mDisabled = disabled;
}
