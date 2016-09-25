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
class CSaveWorldMemory;

/* TODO: Figure out */
class CWorldSomethingState
{
    std::vector<u32> x0_;
    u32 x10_bitCount = 0;
    std::vector<u32> x14_;
public:
    CWorldSomethingState(CBitStreamReader& reader, const CSaveWorld& saveWorld)
    {
        u32 bitCount = reader.ReadEncoded(10);
        u32 wordCount = (bitCount + 31) / 32;
        x14_.resize(wordCount);

        for (u32 i=0 ; i<bitCount ; ++i)
        {
            ++x10_bitCount;
            bool bit = reader.ReadEncoded(1);
            if (bit)
                x14_[i / 32] |= 1 << (i % 32);
            else
                x14_[i / 32] &= ~(1 << (i % 32));
        }
    }
};

class CWorldState
{
    ResId x0_mlvlId;
    TAreaId x4_areaId = kInvalidAreaId;
    std::shared_ptr<CRelayTracker> x8_relayTracker;
    std::shared_ptr<CMapWorldInfo> xc_mapWorldInfo;
    u32 x10_;
    std::shared_ptr<CWorldSomethingState> x14_;
public:
    CWorldState(ResId id) : x0_mlvlId(id) {}
    CWorldState(CBitStreamReader& reader, ResId mlvlId, const CSaveWorld& saveWorld);
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
    double xa0_playTime;
    CPersistentOptions xa8_systemOptions;
    CGameOptions x17c_gameOptions;
    CHintOptions x1f8_hintOptions;

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
    CPersistentOptions& SystemOptions() { return xa8_systemOptions; }
    CGameOptions& GameOptions() { return x17c_gameOptions; }
    CWorldState& StateForWorld(ResId mlvlId);
    CWorldState& CurrentWorldState() { return StateForWorld(x84_mlvlId); }
    ResId CurrentWorldAssetId() const { return x84_mlvlId; }
};

}

#endif // __URDE_CGAMESTATE_HPP__
