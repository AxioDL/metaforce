#ifndef __PSHAG_CGAMESTATE_HPP__
#define __PSHAG_CGAMESTATE_HPP__

#include <memory>
#include "CBasics.hpp"
#include "CPlayerState.hpp"
#include "CGameOptions.hpp"
#include "CWorldTransManager.hpp"

namespace urde
{

class CGameState
{
    int m_stateFlag = -1;
    TOneStatic<CPlayerState> m_playerState;
    CWorldTransManager m_transManager;
    float m_gameTime = 0.0;
    CGameOptions m_gameOpts;
public:
    CGameState() {}
    CGameState(CInputStream& stream);
    void SetCurrentWorldId(unsigned int id, const std::string& name);
    CWorldTransManager& WorldTransitionManager() {return m_transManager;}
};

}

#endif // __PSHAG_CGAMESTATE_HPP__
