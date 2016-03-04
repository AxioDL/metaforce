#include "CRandom16.hpp"

namespace urde
{

static CRandom16 DefaultRandom(0);
static CGlobalRandom DefaultGlobalRandom(DefaultRandom);

CRandom16* CRandom16::g_randomNumber = &DefaultRandom;
CGlobalRandom* CGlobalRandom::g_currentGlobalRandom = &DefaultGlobalRandom;

}
