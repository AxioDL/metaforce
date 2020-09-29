#include "Runtime/Input/CRumbleGenerator.hpp"

#include "Runtime/GameGlobalObjects.hpp"

namespace urde {

CRumbleGenerator::CRumbleGenerator() {
  HardStopAll();
}

CRumbleGenerator::~CRumbleGenerator() { HardStopAll(); }

#define PWM_MONITOR 0
#if PWM_MONITOR
static bool b_tp = false;
static std::chrono::steady_clock::time_point s_tp;
#endif

void CRumbleGenerator::Update(float dt) {
#if PWM_MONITOR
  std::chrono::milliseconds::rep ms = 0;
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  if (!b_tp) {
    b_tp = true;
    s_tp = now;
  } else {
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_tp).count();
  }
#endif

  if (!xf0_24_disabled) {
    bool updated = false;
    for (size_t i = 0; i < x0_voices.size(); ++i) {
      const float intensity = x0_voices[i].GetIntensity();
      if (!x0_voices[i].Update(dt) || intensity <= 0.f) {
        xc0_periodTime[i] = 0.f;
        xd0_onTime[i] = 0.f;
        if (xe0_commandArray[i] != EMotorState::Stop) {
#if PWM_MONITOR
          s_tp = now;
          fmt::print(FMT_STRING("{}ms ON\n"), ms);
#endif
          xe0_commandArray[i] = EMotorState::Stop;
          updated = true;
        }
      } else {
        xc0_periodTime[i] += dt;
        if (xc0_periodTime[i] >= 1.f / (30.f * intensity)) {
          xc0_periodTime[i] = 0.f;
          if (xe0_commandArray[i] != EMotorState::Rumble) {
#if PWM_MONITOR
            s_tp = now;
            fmt::print(FMT_STRING("{}ms Off\n"), ms);
#endif
            xe0_commandArray[i] = EMotorState::Rumble;
            updated = true;
          }
        } else {
          xd0_onTime[i] += dt;
          if (xd0_onTime[i] >= (1.f / 30.f)) {
            xd0_onTime[i] = 0.f;
            if (xe0_commandArray[i] != EMotorState::Stop) {
#if PWM_MONITOR
              s_tp = now;
              fmt::print(FMT_STRING("{}ms ON\n"), ms);
#endif
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

void CRumbleGenerator::HardStopAll() {
  static constexpr std::array HardStopCommands{
      EMotorState::StopHard,
      EMotorState::StopHard,
      EMotorState::StopHard,
      EMotorState::StopHard,
  };

  xc0_periodTime.fill(0.0f);
  xd0_onTime.fill(0.0f);
  xe0_commandArray.fill(EMotorState::Stop);
  for (auto& voice : x0_voices) {
    voice.HardReset();
  }

  g_InputGenerator->ControlAllMotors(HardStopCommands);
}

s16 CRumbleGenerator::Rumble(const SAdsrData& adsr, float gain, ERumblePriority prio, EIOPort port) {
  CRumbleVoice& vox = x0_voices[size_t(port)];
  const s16 freeChan = vox.GetFreeChannel();
  if (prio >= vox.GetPriority(freeChan)) {
    xc0_periodTime[size_t(port)] = 0.f;
    return vox.Activate(adsr, freeChan, gain, prio);
  }
  return -1;
}

void CRumbleGenerator::Stop(s16 id, EIOPort port) {
  CRumbleVoice& vox = x0_voices[size_t(port)];
  vox.Deactivate(id, false);
}

} // namespace urde
