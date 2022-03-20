#pragma once
#include "Runtime/Input/PAD.hpp"


namespace metaforce {
enum class EIOPort {
  Player1,
  Player2,
  Player3,
  Player4,
};

enum class EMotorState {
  Stop = 0,
  Rumble = 1,
  StopHard = 2,
};
} // namespace metaforce