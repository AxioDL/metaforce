#pragma once

#include <array>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Input/CInputGenerator.hpp"
#include "Runtime/Input/CRumbleVoice.hpp"

namespace urde {
class CRumbleGenerator {
  std::array<CRumbleVoice, 4> x0_voices;
  std::array<float, 4> xc0_periodTime;
  std::array<float, 4> xd0_onTime;
  std::array<EMotorState, 4> xe0_commandArray;
  bool xf0_24_disabled : 1 = false;

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
} // namespace urde
