#ifndef __URDE_CGAMESTATE_HPP__
#define __URDE_CGAMESTATE_HPP__

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
    double xa0_playTime;
public:
    CGameState() {}
    CGameState(CBitStreamReader& stream);
    void SetCurrentWorldId(unsigned int id, const std::string& name);
    CWorldTransManager& WorldTransitionManager() {return m_transManager;}
    void SetTotalPlayTime(float time);
};

}

#endif // __URDE_CGAMESTATE_HPP__
