#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Runtime/AutoMapper/CMapWorldInfo.hpp"
#include "Runtime/CBasics.hpp"
#include "Runtime/CGameOptions.hpp"
#include "Runtime/CPlayerState.hpp"
#include "Runtime/CScriptMailbox.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/CWorldTransManager.hpp"

namespace metaforce {
class CSaveWorldMemory;

class WordBitmap {
  std::vector<u32> x0_words;
  size_t x10_bitCount = 0;

public:
  void Reserve(size_t bitCount) { x0_words.reserve((bitCount + 31) / 32); }
  [[nodiscard]] size_t GetBitCount() const { return x10_bitCount; }
  [[nodiscard]] bool GetBit(size_t idx) const {
    size_t wordIdx = idx / 32;
    if (wordIdx >= x0_words.size()) {
      return false;
    }
    size_t wordCur = idx % 32;
    return ((x0_words[wordIdx] >> wordCur) & 0x1) != 0u;
  }
  void SetBit(size_t idx) {
    size_t wordIdx = idx / 32;
    while (wordIdx >= x0_words.size()) {
      x0_words.push_back(0);
    }
    size_t wordCur = idx % 32;
    x0_words[wordIdx] |= (1 << wordCur);
    x10_bitCount = std::max(x10_bitCount, idx + 1);
  }
  void UnsetBit(size_t idx) {
    size_t wordIdx = idx / 32;
    while (wordIdx >= x0_words.size()) {
      x0_words.push_back(0);
    }
    size_t wordCur = idx % 32;
    x0_words[wordIdx] &= ~(1 << wordCur);
    x10_bitCount = std::max(x10_bitCount, idx + 1);
  }
  void Clear() {
    x0_words.clear();
    x10_bitCount = 0;
  }

  class Iterator {
    friend class WordBitmap;
    const WordBitmap& m_bmp;
    size_t m_idx = 0;
    Iterator(const WordBitmap& bmp, size_t idx) : m_bmp(bmp), m_idx(idx) {}

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = bool;
    using difference_type = std::ptrdiff_t;
    using pointer = bool*;
    using reference = bool&;

    Iterator& operator++() {
      ++m_idx;
      return *this;
    }
    bool operator*() const { return m_bmp.GetBit(m_idx); }
    bool operator!=(const Iterator& other) const { return m_idx != other.m_idx; }
  };
  [[nodiscard]] Iterator begin() const { return Iterator(*this, 0); }
  [[nodiscard]] Iterator end() const { return Iterator(*this, x10_bitCount); }
};

class CScriptLayerManager {
  friend class CSaveWorldIntermediate;
  std::vector<CWorldLayers::Area> x0_areaLayers;
  WordBitmap x10_saveLayers;

public:
  CScriptLayerManager() = default;
  CScriptLayerManager(CInputStream& reader, const CWorldSaveGameInfo& saveWorld);

  [[nodiscard]] bool IsLayerActive(int areaIdx, int layerIdx) const {
    return ((x0_areaLayers[areaIdx].m_layerBits >> layerIdx) & 1) != 0u;
  }

  void SetLayerActive(int areaIdx, int layerIdx, bool active) {
    if (active) {
      x0_areaLayers[areaIdx].m_layerBits |= uint64_t(1) << layerIdx;
    } else {
      x0_areaLayers[areaIdx].m_layerBits &= ~(uint64_t(1) << layerIdx);
    }
  }

  void InitializeWorldLayers(const std::vector<CWorldLayers::Area>& layers);

  [[nodiscard]] u32 GetAreaLayerCount(int areaIdx) const { return x0_areaLayers[areaIdx].m_layerCount; }
  [[nodiscard]] u32 GetAreaCount() const { return x0_areaLayers.size(); }

  void PutTo(COutputStream& writer) const;
};

class CWorldState {
  CAssetId x0_mlvlId;
  TAreaId x4_areaId = kInvalidAreaId;
  std::shared_ptr<CScriptMailbox> x8_mailbox;
  std::shared_ptr<CMapWorldInfo> xc_mapWorldInfo;
  CAssetId x10_desiredAreaAssetId;
  std::shared_ptr<CScriptLayerManager> x14_layerState;

public:
  explicit CWorldState(CAssetId id);
  CWorldState(CInputStream& reader, CAssetId mlvlId, const CWorldSaveGameInfo& saveWorld);
  CAssetId GetWorldAssetId() const { return x0_mlvlId; }
  void SetAreaId(TAreaId aid) { x4_areaId = aid; }
  TAreaId GetCurrentAreaId() const { return x4_areaId; }
  CAssetId GetDesiredAreaAssetId() const { return x10_desiredAreaAssetId; }
  void SetDesiredAreaAssetId(CAssetId id) { x10_desiredAreaAssetId = id; }
  const std::shared_ptr<CScriptMailbox>& Mailbox() const { return x8_mailbox; }
  const std::shared_ptr<CMapWorldInfo>& MapWorldInfo() const { return xc_mapWorldInfo; }
  const std::shared_ptr<CScriptLayerManager>& GetLayerState() const { return x14_layerState; }
  void PutTo(COutputStream& writer, const CWorldSaveGameInfo& savw) const;
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
  CGameState(CInputStream& stream, u32 saveIdx);
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
  void ReadPersistentOptions(CInputStream& r);
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
  void PutTo(COutputStream& writer);
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
} // namespace metaforce
