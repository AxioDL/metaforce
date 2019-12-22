#include "Runtime/CRandom16.hpp"

namespace urde {

CRandom16* CRandom16::g_randomNumber = nullptr;                // &DefaultRandom;
CGlobalRandom* CGlobalRandom::g_currentGlobalRandom = nullptr; //&DefaultGlobalRandom;

} // namespace urde
