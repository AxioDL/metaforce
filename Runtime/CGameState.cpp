#include "CGameState.hpp"
#include "IOStreams.hpp"

namespace Retro
{

CGameState::CGameState(CInputStream& stream)
: m_stateFlag(stream.readUint32Big()), m_playerState(stream), m_gameTime(stream.readFloatBig())
{}

void CGameState::SetCurrentWorldId(unsigned int id, const std::string& name)
{
}

}
