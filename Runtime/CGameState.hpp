#ifndef __URDE_CGAMESTATE_HPP__
#define __URDE_CGAMESTATE_HPP__

#include <memory>
#include "CBasics.hpp"
#include "CPlayerState.hpp"
#include "CGameOptions.hpp"
#include "World/CWorldTransManager.hpp"

namespace urde
{

class CWorldState
{
    ResId x0_mlvlId;
    TAreaId x4_areaId;
public:
    CWorldState(ResId id) : x0_mlvlId(id) {}
    ResId GetWorldAssetId() const {return x0_mlvlId;}
    void SetAreaId(TAreaId aid) { x4_areaId = aid; }
};

class CGameState
{
    friend class CStateManager;

    int m_stateFlag = -1;
    ResId x84_mlvlId = -1;
    std::vector<CWorldState> x88_worldStates;
    CPlayerState x98_playerState;
    CWorldTransManager x9c_transManager;
    float m_gameTime = 0.0;
    CGameOptions m_gameOpts;
    double xa0_playTime;

    union
    {
        struct
        {
            bool x228_24_;
            bool x228_25_deferPowerupInit;
        };
        u8 _dummy = 0;
    };
public:
    CGameState();
    CGameState(CBitStreamReader& stream);
    void SetCurrentWorldId(unsigned int id, const std::string& name);
    CWorldTransManager& WorldTransitionManager() {return x9c_transManager;}
    void SetTotalPlayTime(float time);
    CWorldState& StateForWorld(ResId mlvlId);
    CWorldState& CurrentWorldState() { return StateForWorld(x84_mlvlId); }
};

}

#endif // __URDE_CGAMESTATE_HPP__
