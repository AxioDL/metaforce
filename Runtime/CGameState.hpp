#ifndef __URDE_CGAMESTATE_HPP__
#define __URDE_CGAMESTATE_HPP__

#include <memory>
#include "CBasics.hpp"
#include "CPlayerState.hpp"
#include "CGameOptions.hpp"
#include "CRelayTracker.hpp"
#include "World/CWorldTransManager.hpp"
#include "AutoMapper/CMapWorldInfo.hpp"
#include "World/CWorld.hpp"
#include "DataSpec/DNACommon/DNACommon.hpp"

namespace urde
{
class CSaveWorldMemory;

class CWorldLayerState
{
    friend class CSaveWorldIntermediate;
    std::vector<CWorldLayers::Area> x0_areaLayers;
    DataSpec::WordBitmap x10_saveLayers;

public:
    CWorldLayerState() = default;
    CWorldLayerState(CBitStreamReader& reader, const CSaveWorld& saveWorld);

    bool IsLayerActive(int areaIdx, int layerIdx) const { return (x0_areaLayers[areaIdx].m_layerBits >> layerIdx) & 1; }

    void SetLayerActive(int areaIdx, int layerIdx, bool active)
    {
        if (active)
            x0_areaLayers[areaIdx].m_layerBits |= uint64_t(1) << layerIdx;
        else
            x0_areaLayers[areaIdx].m_layerBits &= ~(uint64_t(1) << layerIdx);
    }

    void InitializeWorldLayers(const std::vector<CWorldLayers::Area>& layers);

    u32 GetAreaLayerCount(int areaIdx) const { return x0_areaLayers[areaIdx].m_layerCount; }

    void PutTo(CBitStreamWriter& writer) const;
};

class CWorldState
{
    ResId x0_mlvlId;
    TAreaId x4_areaId = kInvalidAreaId;
    std::shared_ptr<CRelayTracker> x8_relayTracker;
    std::shared_ptr<CMapWorldInfo> xc_mapWorldInfo;
    ResId x10_desiredAreaAssetId;
    std::shared_ptr<CWorldLayerState> x14_layerState;

public:
    CWorldState(ResId id);
    CWorldState(CBitStreamReader& reader, ResId mlvlId, const CSaveWorld& saveWorld);
    ResId GetWorldAssetId() const { return x0_mlvlId; }
    void SetAreaId(TAreaId aid) { x4_areaId = aid; }
    TAreaId GetCurrentAreaId() const { return x4_areaId; }
    ResId GetDesiredAreaAssetId() const { return x10_desiredAreaAssetId; }
    const std::shared_ptr<CRelayTracker>& RelayTracker() const { return x8_relayTracker; }
    const std::shared_ptr<CMapWorldInfo>& MapWorldInfo() const { return xc_mapWorldInfo; }
    const std::shared_ptr<CWorldLayerState>& GetLayerState() const { return x14_layerState; }
    void PutTo(CBitStreamWriter& writer, const CSaveWorld& savw) const;
};

class CGameState
{
    friend class CStateManager;

    bool x0_[128] = {};
    u32 x80_;
    ResId x84_mlvlId = -1;
    std::vector<CWorldState> x88_worldStates;
    std::shared_ptr<CPlayerState> x98_playerState;
    std::shared_ptr<CWorldTransManager> x9c_transManager;
    double xa0_playTime = 0.0;
    CPersistentOptions xa8_systemOptions;
    CGameOptions x17c_gameOptions;
    CHintOptions x1f8_hintOptions;
    u32 x20c_saveFileIdx = 0;
    u64 x210_cardSerial = 0;
    std::vector<u8> x218_backupBuf;

    union
    {
        struct
        {
            bool x228_24_hardMode : 1;
            bool x228_25_deferPowerupInit : 1;
        };
        u8 _dummy = 0;
    };

    static void EnsureWorldPakReady(ResId mlvl);

public:
    CGameState();
    CGameState(CBitStreamReader& stream, u32 saveIdx);
    void SetCurrentWorldId(ResId id);
    std::shared_ptr<CPlayerState> GetPlayerState() { return x98_playerState; }
    std::shared_ptr<CWorldTransManager> GetWorldTransitionManager() { return x9c_transManager; }
    void SetTotalPlayTime(float time);
    CPersistentOptions& SystemOptions() { return xa8_systemOptions; }
    CGameOptions& GameOptions() { return x17c_gameOptions; }
    CHintOptions& HintOptions() { return x1f8_hintOptions; }
    CWorldState& StateForWorld(ResId mlvlId);
    CWorldState& CurrentWorldState() { return StateForWorld(x84_mlvlId); }
    ResId CurrentWorldAssetId() const { return x84_mlvlId; }
    void SetHardMode(bool v) { x228_24_hardMode = v; }
    void ReadPersistentOptions(CBitStreamReader& r);
    void SetPersistentOptions(const CPersistentOptions& opts) { xa8_systemOptions = opts; }
    void ImportPersistentOptions(const CPersistentOptions& opts);
    void ExportPersistentOptions(CPersistentOptions& opts) const;
    void SetGameOptions(const CGameOptions& opts) { x17c_gameOptions = opts; }
    void WriteBackupBuf();
    u32 GetFileIdx() const { return x20c_saveFileIdx; }
    void SetFileIdx(u32 idx) { x20c_saveFileIdx = idx; }
    void SetCardSerial(u64 serial) { x210_cardSerial = serial; }
    u64 GetCardSerial() const { return x210_cardSerial; }
    void PutTo(CBitStreamWriter& writer) const;
    float GetHardModeDamageMultiplier() const;
    float GetHardModeWeaponMultiplier() const;
    void InitializeMemoryWorlds();
    void InitializeMemoryStates();

    struct GameFileStateInfo
    {
        double x0_playTime;
        u32 x8_mlvlId;
        float xc_health;
        u32 x10_energyTanks;
        u32 x14_timestamp;
        u32 x18_itemPercent;
        float x1c_scanPercent;
        bool x20_hardMode;
    };
    static GameFileStateInfo LoadGameFileState(const u8* data);
};
}

#endif // __URDE_CGAMESTATE_HPP__
