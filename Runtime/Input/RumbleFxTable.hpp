#pragma once

#include <array>
#include "Runtime/Input/CRumbleVoice.hpp"

namespace urde {

using RumbleFXTable = std::array<SAdsrData, 24>;

extern const RumbleFXTable RumbleFxTable;

}
