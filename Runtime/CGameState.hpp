#ifndef __URDE_CGAMESTATE_HPP__
#define __URDE_CGAMESTATE_HPP__

#include <memory>
#include "CBasics.hpp"
#include "CPlayerState.hpp"
#include "CGameOptions.hpp"
#include "CRelayTracker.hpp"
#include "World/CWorldTransManager.hpp"
#include "AutoMapper/CMapWorldInfo.hpp"
namespace urde
{

class CWorldState
{
    ResId x0_mlvlId;
    TAreaId x4_areaId;
    std::shared_ptr<CRelayTracker> x8_relayTracker;
    std::shared_ptr<CMapWorldInfo> xc_mapWorldInfo;
    /* std::shared_ptr<> x14_ */
public:
    CWorldState(ResId id) : x0_mlvlId(id) {}
    ResId GetWorldAssetId() const {return x0_mlvlId;}
    void SetAreaId(TAreaId aid) { x4_areaId = aid; }
    const TAreaId& GetCurrentAreaId() const { return x4_areaId; }
    std::shared_ptr<CRelayTracker> RelayTracker() { return x8_relayTracker; }
    std::shared_ptr<CMapWorldInfo> MapWorldInfo() { return xc_mapWorldInfo; }
};

class CGameState
{
    friend class CStateManager;

    bool x0_[128] = {};
    ResId x84_mlvlId = -1;
    std::vector<CWorldState> x88_worldStates;
    std::shared_ptr<CPlayerState> x98_playerState;
    std::shared_ptr<CWorldTransManager> x9c_transManager;
    CGameOptions m_gameOpts;
    double xa0_playTime;
    u32 xa4_;

    /* x17c_ */
    /* x1f8_ */

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
    void SetCurrentWorldId(unsigned int id);
    std::shared_ptr<CPlayerState> GetPlayerState() {return x98_playerState;}
    std::shared_ptr<CWorldTransManager> GetWorldTransitionManager() {return x9c_transManager;}
    void SetTotalPlayTime(float time);
    CWorldState& StateForWorld(ResId mlvlId);
    CWorldState& CurrentWorldState() { return StateForWorld(x84_mlvlId); }
    ResId CurrentWorldAssetId() const { return x84_mlvlId; }
};

}

#endif // __URDE_CGAMESTATE_HPP__
