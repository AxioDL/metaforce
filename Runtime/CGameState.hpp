#pragma once

#include <array>
#include <memory>
#include <vector>

#include "DataSpec/DNACommon/DNACommon.hpp"

#include "Runtime/CBasics.hpp"
#include "Runtime/CGameOptions.hpp"
#include "Runtime/CPlayerState.hpp"
#include "Runtime/CRelayTracker.hpp"
#include "Runtime/AutoMapper/CMapWorldInfo.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/CWorldTransManager.hpp"

namespace urde {
class CSaveWorldMemory;

class CWorldLayerState {
  friend class CSaveWorldIntermediate;
  std::vector<CWorldLayers::Area> x0_areaLayers;
  DataSpec::WordBitmap x10_saveLayers;

public:
  CWorldLayerState() = default;
  CWorldLayerState(CBitStreamReader& reader, const CSaveWorld& saveWorld);

  bool IsLayerActive(int areaIdx, int layerIdx) const { return ((x0_areaLayers[areaIdx].m_layerBits >> layerIdx) & 1); }

  void SetLayerActive(int areaIdx, int layerIdx, bool active) {
    if (active)
      x0_areaLayers[areaIdx].m_layerBits |= uint64_t(1) << layerIdx;
    else
      x0_areaLayers[areaIdx].m_layerBits &= ~(uint64_t(1) << layerIdx);
  }

  void InitializeWorldLayers(const std::vector<CWorldLayers::Area>& layers);

  u32 GetAreaLayerCount(int areaIdx) const { return x0_areaLayers[areaIdx].m_layerCount; }
  u32 GetAreaCount() const { return x0_areaLayers.size(); }

  void PutTo(CBitStreamWriter& writer) const;
};

class CWorldState {
  CAssetId x0_mlvlId;
  TAreaId x4_areaId = kInvalidAreaId;
  std::shared_ptr<CRelayTracker> x8_relayTracker;
  std::shared_ptr<CMapWorldInfo> xc_mapWorldInfo;
  CAssetId x10_desiredAreaAssetId;
  std::shared_ptr<CWorldLayerState> x14_layerState;

public:
  explicit CWorldState(CAssetId id);
  CWorldState(CBitStreamReader& reader, CAssetId mlvlId, const CSaveWorld& saveWorld);
  CAssetId GetWorldAssetId() const { return x0_mlvlId; }
  void SetAreaId(TAreaId aid) { x4_areaId = aid; }
  TAreaId GetCurrentAreaId() const { return x4_areaId; }
  CAssetId GetDesiredAreaAssetId() const { return x10_desiredAreaAssetId; }
  void SetDesiredAreaAssetId(CAssetId id) { x10_desiredAreaAssetId = id; }
  const std::shared_ptr<CRelayTracker>& RelayTracker() const { return x8_relayTracker; }
  const std::shared_ptr<CMapWorldInfo>& MapWorldInfo() const { return xc_mapWorldInfo; }
  const std::shared_ptr<CWorldLayerState>& GetLayerState() const { return x14_layerState; }
  void PutTo(CBitStreamWriter& writer, const CSaveWorld& savw) const;
};

class CGameState {
  friend class CStateManager;

  std::array<bool, 128> x0_{};
  u32 x80_ = 0;
  CAssetId x84_mlvlId;
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
  bool x228_24_hardMode : 1 = false;
  bool x228_25_initPowerupsAtFirstSpawn : 1 = true;

public:
  CGameState();
  CGameState(CBitStreamReader& stream, u32 saveIdx);
  void SetCurrentWorldId(CAssetId id);
  std::shared_ptr<CPlayerState> GetPlayerState() const { return x98_playerState; }
  std::shared_ptr<CWorldTransManager> GetWorldTransitionManager() const { return x9c_transManager; }
  void SetTotalPlayTime(double time);
  double GetTotalPlayTime() const { return xa0_playTime; }
  CPersistentOptions& SystemOptions() { return xa8_systemOptions; }
  CGameOptions& GameOptions() { return x17c_gameOptions; }
  CHintOptions& HintOptions() { return x1f8_hintOptions; }
  CWorldState& StateForWorld(CAssetId mlvlId);
  CWorldState& CurrentWorldState() { return StateForWorld(x84_mlvlId); }
  CAssetId CurrentWorldAssetId() const { return x84_mlvlId; }
  void SetHardMode(bool v) { x228_24_hardMode = v; }
  bool GetHardMode() const { return x228_24_hardMode; }
  void ReadPersistentOptions(CBitStreamReader& r);
  void SetPersistentOptions(const CPersistentOptions& opts) { xa8_systemOptions = opts; }
  void ImportPersistentOptions(const CPersistentOptions& opts);
  void ExportPersistentOptions(CPersistentOptions& opts) const;
  void SetGameOptions(const CGameOptions& opts) { x17c_gameOptions = opts; }
  void WriteBackupBuf();
  std::vector<u8>& BackupBuf() { return x218_backupBuf; }
  u32 GetFileIdx() const { return x20c_saveFileIdx; }
  void SetFileIdx(u32 idx) { x20c_saveFileIdx = idx; }
  void SetCardSerial(u64 serial) { x210_cardSerial = serial; }
  u64 GetCardSerial() const { return x210_cardSerial; }
  void PutTo(CBitStreamWriter& writer);
  float GetHardModeDamageMultiplier() const;
  float GetHardModeWeaponMultiplier() const;
  void InitializeMemoryWorlds();
  void InitializeMemoryStates();

  struct GameFileStateInfo {
    double x0_playTime;
    CAssetId x8_mlvlId;
    float xc_health;
    u32 x10_energyTanks;
    u32 x14_timestamp;
    u32 x18_itemPercent;
    float x1c_scanPercent;
    bool x20_hardMode;
  };
  static GameFileStateInfo LoadGameFileState(const u8* data);
};
} // namespace urde
