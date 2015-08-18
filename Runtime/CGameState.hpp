#ifndef __RETRO_CGAMESTATE_HPP__
#define __RETRO_CGAMESTATE_HPP__

#include <memory>
#include "CPlayerState.hpp"
#include "CGameOptions.hpp"

namespace Retro
{

class CGameState
{
    TOneStatic<CPlayerState> m_playerState;
    CGameOptions m_gameOpts;
public:
    CGameState()
    {
    }
};

}

#endif // __RETRO_CGAMESTATE_HPP__
