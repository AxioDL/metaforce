#pragma once

#include <array>
#include "Runtime/Input/CRumbleVoice.hpp"

namespace metaforce {

using RumbleFXTable = std::array<SAdsrData, 24>;

extern const RumbleFXTable RumbleFxTable;

}
