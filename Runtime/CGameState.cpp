#include "Runtime/CGameState.hpp"

#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/CWorldSaveGameInfo.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/MP1/MP1.hpp"

#include <zeus/Math.hpp>

namespace metaforce {
union BitsToDouble {
  struct {
#if BYTE_ORDER == __LITTLE_ENDIAN
    u32 high;
    u32 low;
#else
    u32 low;
    u32 high;
#endif
  };
  double doub;
};

/** Word Bitmap reader/writer */
void WordBitmap::read(athena::io::IStreamReader& reader, size_t bitCount) {
  m_bitCount = bitCount;
  size_t wordCount = (bitCount + 31) / 32;
  m_words.clear();
  m_words.reserve(wordCount);
  for (size_t w = 0; w < wordCount; ++w)
    m_words.push_back(reader.readUint32Big());
}
void WordBitmap::write(athena::io::IStreamWriter& writer) const {
  for (atUint32 word : m_words)
    writer.writeUint32Big(word);
}
void WordBitmap::binarySize(size_t& __isz) const { __isz += m_words.size() * 4; }

CScriptLayerManager::CScriptLayerManager(CInputStream& reader, const CWorldSaveGameInfo& saveWorld) {
  const u32 bitCount = reader.ReadBits(10);
  x10_saveLayers.reserve(bitCount);

  for (u32 i = 0; i < bitCount; ++i) {
    const bool bit = reader.ReadBits(1) != 0;
    if (bit) {
      x10_saveLayers.setBit(i);
    } else {
      x10_saveLayers.unsetBit(i);
    }
  }
}

void CScriptLayerManager::PutTo(COutputStream& writer) const {
  u32 totalLayerCount = 0;
  for (size_t i = 0; i < x0_areaLayers.size(); ++i) {
    totalLayerCount += GetAreaLayerCount(s32(i)) - 1;
  }

  writer.WriteBits(totalLayerCount, 10);

  for (size_t i = 0; i < x0_areaLayers.size(); ++i) {
    const u32 count = GetAreaLayerCount(s32(i));
    for (u32 l = 1; l < count; ++l) {
      writer.WriteBits(IsLayerActive(s32(i), s32(l)), 1);
    }
  }
}

void CScriptLayerManager::InitializeWorldLayers(const std::vector<CWorldLayers::Area>& layers) {
  if (!x0_areaLayers.empty()) {
    return;
  }

  x0_areaLayers = layers;
  if (x10_saveLayers.getBitCount() == 0) {
    return;
  }

  u32 a = 0;
  u32 b = 0;
  for (const CWorldLayers::Area& area : x0_areaLayers) {
    for (u32 l = 1; l < area.m_layerCount; ++l)
      SetLayerActive(a, l, x10_saveLayers.getBit(b++));
    ++a;
  }

  x10_saveLayers.clear();
}

CWorldState::CWorldState(CAssetId id) : x0_mlvlId(id), x4_areaId(0) {
  x8_mailbox = std::make_shared<CScriptMailbox>();
  xc_mapWorldInfo = std::make_shared<CMapWorldInfo>();
  x10_desiredAreaAssetId = {};
  x14_layerState = std::make_shared<CScriptLayerManager>();
}

CWorldState::CWorldState(CInputStream& reader, CAssetId mlvlId, const CWorldSaveGameInfo& saveWorld)
: x0_mlvlId(mlvlId) {
  x4_areaId = TAreaId(reader.ReadBits(32));
  x10_desiredAreaAssetId = u32(reader.ReadBits(32));
  x8_mailbox = std::make_shared<CScriptMailbox>(reader, saveWorld);
  xc_mapWorldInfo = std::make_shared<CMapWorldInfo>(reader, saveWorld, mlvlId);
  x14_layerState = std::make_shared<CScriptLayerManager>(reader, saveWorld);
}

void CWorldState::PutTo(COutputStream& writer, const CWorldSaveGameInfo& savw) const {
  writer.WriteBits(x4_areaId, 32);
  writer.WriteBits(u32(x10_desiredAreaAssetId.Value()), 32);
  x8_mailbox->PutTo(writer, savw);
  xc_mapWorldInfo->PutTo(writer, savw, x0_mlvlId);
  x14_layerState->PutTo(writer);
}

CGameState::GameFileStateInfo CGameState::LoadGameFileState(const u8* data) {
  CMemoryInStream stream(data, 4096, CMemoryInStream::EOwnerShip::NotOwned);
  GameFileStateInfo ret;

  for (u32 i = 0; i < 128; i++) {
    stream.ReadBits(8);
  }
  ret.x14_timestamp = stream.ReadBits(32);

  ret.x20_hardMode = stream.ReadBits(1) != 0;
  stream.ReadBits(1);
  const CAssetId origMLVL = u32(stream.ReadBits(32));
  ret.x8_mlvlId = origMLVL;

  BitsToDouble conv;
  conv.low = stream.ReadBits(32);
  conv.high = stream.ReadBits(32);
  ret.x0_playTime = conv.doub;

  CPlayerState playerState(stream);
  ret.x10_energyTanks = playerState.GetItemCapacity(CPlayerState::EItemType::EnergyTanks);
  ret.xc_health = playerState.GetHealthInfo().GetHP();

  u32 itemPercent;
  if (origMLVL == 0x158EFE17u)
    itemPercent = 0;
  else
    itemPercent = playerState.CalculateItemCollectionRate() * 100 / playerState.GetPickupTotal();

  ret.x18_itemPercent = itemPercent;

  float scanPercent;
  if (playerState.GetTotalLogScans() == 0)
    scanPercent = 0.f;
  else
    scanPercent = 100.f * playerState.GetLogScans() / float(playerState.GetTotalLogScans());
  ret.x1c_scanPercent = scanPercent;

  return ret;
}

CGameState::CGameState() {
  x98_playerState = std::make_shared<CPlayerState>();
  x9c_transManager = std::make_shared<CWorldTransManager>();

  if (g_MemoryCardSys != nullptr) {
    InitializeMemoryStates();
  }
}

CGameState::CGameState(CInputStream& stream, u32 saveIdx) : x20c_saveFileIdx(saveIdx) {
  x9c_transManager = std::make_shared<CWorldTransManager>();
  x228_24_hardMode = false;
  x228_25_initPowerupsAtFirstSpawn = true;

  for (bool& value : x0_) {
    value = stream.ReadBits(8) != 0;
  }
  stream.ReadBits(32);

  x228_24_hardMode = stream.ReadBits(1) != 0;
  x228_25_initPowerupsAtFirstSpawn = stream.ReadBits(1) != 0;
  x84_mlvlId = u32(stream.ReadBits(32));
  MP1::CMain::EnsureWorldPakReady(x84_mlvlId);

  BitsToDouble conv;
  conv.low = stream.ReadBits(32);
  conv.high = stream.ReadBits(32);
  xa0_playTime = conv.doub;

  x98_playerState = std::make_shared<CPlayerState>(stream);

  x17c_gameOptions = CGameOptions(stream);
  x1f8_hintOptions = CHintOptions(stream);

  const auto& memWorlds = g_MemoryCardSys->GetMemoryWorlds();
  x88_worldStates.reserve(memWorlds.size());
  for (const auto& memWorld : memWorlds) {
    TLockedToken<CWorldSaveGameInfo> saveWorld =
        g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), memWorld.second.GetSaveWorldAssetId()});
    x88_worldStates.emplace_back(stream, memWorld.first, *saveWorld);
  }

  InitializeMemoryWorlds();
  WriteBackupBuf();
}

void CGameState::ReadPersistentOptions(CInputStream& r) { xa8_systemOptions = r.Get<CPersistentOptions>(); }

void CGameState::ImportPersistentOptions(const CPersistentOptions& opts) {
  if (opts.xd0_24_fusionLinked)
    xa8_systemOptions.xd0_24_fusionLinked = true;
  if (opts.xd0_27_fusionBeat)
    xa8_systemOptions.xd0_27_fusionBeat = true;
  if (&opts != &xa8_systemOptions)
    xa8_systemOptions.x0_nesState = opts.x0_nesState;
  xa8_systemOptions.SetLogScanPercent(opts.GetLogScanPercent());
  xa8_systemOptions.SetAllItemsCollected(opts.GetAllItemsCollected());
  xa8_systemOptions.SetPlayerBeatNormalMode(opts.GetPlayerBeatNormalMode());
  xa8_systemOptions.SetPlayerBeatHardMode(opts.GetPlayerBeatHardMode());
}

void CGameState::ExportPersistentOptions(CPersistentOptions& opts) const {
  if (xa8_systemOptions.xd0_24_fusionLinked)
    opts.xd0_24_fusionLinked = true;
  if (xa8_systemOptions.xd0_27_fusionBeat)
    opts.xd0_27_fusionBeat = true;
  if (&opts != &xa8_systemOptions)
    opts.x0_nesState = xa8_systemOptions.x0_nesState;
  opts.SetPlayerFusionSuitActive(xa8_systemOptions.GetPlayerFusionSuitActive());
}

void CGameState::WriteBackupBuf() {
  x218_backupBuf.resize(940);
  CMemoryStreamOut w(x218_backupBuf.data(), 940);
  PutTo(w);
}

void CGameState::PutTo(COutputStream& writer) {
  for (const bool value : x0_) {
    writer.WriteBits(u32(value), 8);
  }

  writer.WriteBits(CBasics::GetTime() / CBasics::TICKS_PER_SECOND, 32);
  writer.WriteBits(x228_24_hardMode, 1);
  writer.WriteBits(x228_25_initPowerupsAtFirstSpawn, 1);
  writer.WriteBits(u32(x84_mlvlId.Value()), 32);

  BitsToDouble conv;
  conv.doub = xa0_playTime;
  writer.WriteBits(conv.low, 32);
  writer.WriteBits(conv.high, 32);

  x98_playerState->PutTo(writer);
  x17c_gameOptions.PutTo(writer);
  x1f8_hintOptions.PutTo(writer);

  const auto& memWorlds = g_MemoryCardSys->GetMemoryWorlds();
  for (const auto& memWorld : memWorlds) {
    TLockedToken<CWorldSaveGameInfo> saveWorld =
        g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), memWorld.second.GetSaveWorldAssetId()});
    const CWorldState& wld = StateForWorld(memWorld.first);
    wld.PutTo(writer, *saveWorld);
  }
}

void CGameState::SetCurrentWorldId(CAssetId id) {
  StateForWorld(id);
  x84_mlvlId = id;
  MP1::CMain::EnsureWorldPakReady(x84_mlvlId);
}

void CGameState::SetTotalPlayTime(double time) { xa0_playTime = zeus::clamp(0.0, time, 359999.0); }

CWorldState& CGameState::StateForWorld(CAssetId mlvlId) {
  auto it = x88_worldStates.begin();
  for (; it != x88_worldStates.end(); ++it) {
    if (it->GetWorldAssetId() == mlvlId)
      break;
  }

  if (it == x88_worldStates.end()) {
    x88_worldStates.emplace_back(mlvlId);
    return x88_worldStates.back();
  }
  return *it;
}

float CGameState::GetHardModeDamageMultiplier() const { return g_tweakGame->GetHardModeDamageMultiplier(); }

float CGameState::GetHardModeWeaponMultiplier() const { return g_tweakGame->GetHardModeWeaponMultiplier(); }

void CGameState::InitializeMemoryWorlds() {
  const auto& memoryWorlds = g_MemoryCardSys->GetMemoryWorlds();
  for (const auto& wld : memoryWorlds) {
    const auto& layerState = StateForWorld(wld.first).GetLayerState();
    layerState->InitializeWorldLayers(wld.second.GetDefaultLayerStates());
  }
}

void CGameState::InitializeMemoryStates() {
  x98_playerState->InitializeScanTimes();
  x1f8_hintOptions.InitializeMemoryState();
  InitializeMemoryWorlds();
  WriteBackupBuf();
}

} // namespace metaforce
