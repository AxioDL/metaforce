#pragma once
#include <dolphin/pad.h>

namespace metaforce {
enum class EIOPort {
  Player1 = PAD_CHAN0,
  Player2 = PAD_CHAN1,
  Player3 = PAD_CHAN2,
  Player4 = PAD_CHAN3,
};

enum class EMotorState {
  Stop = PAD_MOTOR_STOP,
  Rumble = PAD_MOTOR_RUMBLE,
  StopHard = PAD_MOTOR_STOP_HARD,
};
} // namespace metaforce
