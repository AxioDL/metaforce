#ifndef __RETRO_CGAMESTATE_HPP__
#define __RETRO_CGAMESTATE_HPP__

#include <memory>
#include "CPlayerState.hpp"
#include "CGameOptions.hpp"

class CGameState
{
    std::unique_ptr<CPlayerState> m_playerState;
    CGameOptions m_gameOpts;
public:
    CGameState()
    {
        m_playerState.reset(new CPlayerState);
    }
};

#endif // __RETRO_CGAMESTATE_HPP__
