#include "CNESEmulator.hpp"
#include "CGameState.hpp"
#include <string.h>

namespace urde
{
namespace MP1
{

void CNESEmulator::ProcessUserInput(const CFinalInput& input, int)
{

}

void CNESEmulator::Update()
{

}

void CNESEmulator::Draw(const zeus::CColor& mulColor, bool filtering)
{

}

void CNESEmulator::LoadState(const u8* state)
{
    memmove(x39_loadState, state, 18);
    x38_stateLoaded = true;
}

}
}
