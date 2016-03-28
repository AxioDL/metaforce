#include "CGameState.hpp"
#include "IOStreams.hpp"
#include "zeus/Math.hpp"

namespace urde
{

CGameState::CGameState(CBitStreamReader& stream)
: m_stateFlag(stream.readUint32Big()), m_playerState(stream), m_gameTime(stream.readFloatBig())
{}

void CGameState::SetCurrentWorldId(unsigned int id, const std::string& name)
{
}

void CGameState::SetTotalPlayTime(float time)
{
    xa0_playTime = zeus::clamp<double>(0.0, time, 359999.0);
}

}
