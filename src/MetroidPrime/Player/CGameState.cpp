#include "MetroidPrime/Player/CGameState.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CWorldSaveGameInfo.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "dolphin/os.h"

union SGameTime {
  double value;
  u64 bits;
};

uint CSystemState::GetBitCount(uint value) {
  uint count = 0;
  for (; value != 0; value >>= 1)
    ++count;
  return count;
}

CSystemState::CSystemState() : x0_nesState(static_cast< uchar >(0))
, x68_(static_cast< uchar >(0))
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
, x6c_language(gpMain->GetLanguage())
#endif
, xbc_autoMapperKeyState(0)
, xc0_frozenFpsCount(0)
, xc4_frozenBallCount(0)
, xc8_powerBombAmmoCount(0)
, xcc_logScanPercent(0)
, xd0_24_fusionLinked(false)
, xd0_25_normalModeBeat(false)
, xd0_26_hardModeBeat(false)
, xd0_27_fusionBeat(false)
, xd0_28_fusionSuitActive(false)
, xd0_29_allItemsCollected(false) {}

CSystemState::CSystemState(CInputStream& in) : x0_nesState(static_cast< uchar >(0))
, x68_(static_cast< uchar >(0))
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
, x6c_language(0)
#endif
, xbc_autoMapperKeyState(0)
, xc0_frozenFpsCount(0)
, xc4_frozenBallCount(0)
, xc8_powerBombAmmoCount(0)
, xcc_logScanPercent(0)
, xd0_24_fusionLinked(false)
, xd0_25_normalModeBeat(false)
, xd0_26_hardModeBeat(false)
, xd0_27_fusionBeat(false)
, xd0_28_fusionSuitActive(false)
, xd0_29_allItemsCollected(false) {
  for (int i = 0; i < kNESStateSize; ++i)
    x0_nesState[i] = in.ReadBits(8);
  for (int i = 0; i < 64; ++i)
    x68_[i] = in.ReadBits(8);
  xc0_frozenFpsCount = in.ReadBits(GetBitCount(3));
  xc4_frozenBallCount = in.ReadBits(GetBitCount(3));
  xc8_powerBombAmmoCount = in.ReadBits(GetBitCount(1));
  xcc_logScanPercent = in.ReadBits(GetBitCount(100));
  xd0_24_fusionLinked = in.ReadBits(1) != 0;
  xd0_25_normalModeBeat = in.ReadBits(1) != 0;
  xd0_26_hardModeBeat = in.ReadBits(1) != 0;
  xd0_27_fusionBeat = in.ReadBits(1) != 0;
  xd0_29_allItemsCollected = in.ReadBits(1) != 0;
  xbc_autoMapperKeyState = in.ReadBits(2);
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
  x6c_language = in.ReadBits(GetBitCount(7));
#endif

  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  int cinematicCount = 0;
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    TLockedToken< CWorldSaveGameInfo > saveWorld = gpSimplePool->GetObj(
        SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    cinematicCount += saveWorld->GetCinematicCount();
  }
  rstl::vector< bool > cinematicStates(cinematicCount, false);
  for (int i = 0; i < cinematicCount; ++i)
    cinematicStates[i] = in.ReadBits(1) != 0;
  int stateIdx = 0;
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    CAssetId worldId = it->first;
    TLockedToken< CWorldSaveGameInfo > saveWorld = gpSimplePool->GetObj(
        SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    const CWorldSaveGameInfo& world = **saveWorld;
    const rstl::vector< TEditorId >& cinematics = world.GetCinematics();
    for (int i = 0; i < cinematics.size(); ++i) {
      if (cinematicStates[stateIdx])
        SetCinematicState(rstl::pair< CAssetId, TEditorId >(worldId, cinematics[i]), true);
      ++stateIdx;
    }
  }
}

void CSystemState::PutTo(COutputStream& out) {
  for (int i = 0; i < kNESStateSize; ++i)
    out.WriteBits(x0_nesState[i], 8);
  for (int i = 0; i < 64; ++i)
    out.WriteBits(x68_[i], 8);
  out.WriteBits(xc0_frozenFpsCount, GetBitCount(3));
  out.WriteBits(xc4_frozenBallCount, GetBitCount(3));
  out.WriteBits(xc8_powerBombAmmoCount, GetBitCount(1));
  out.WriteBits(xcc_logScanPercent, GetBitCount(100));
  out.WriteBits(xd0_24_fusionLinked ? 1 : 0, 1);
  out.WriteBits(xd0_25_normalModeBeat ? 1 : 0, 1);
  out.WriteBits(xd0_26_hardModeBeat ? 1 : 0, 1);
  out.WriteBits(xd0_27_fusionBeat ? 1 : 0, 1);
  out.WriteBits(xd0_29_allItemsCollected ? 1 : 0, 1);
  out.WriteBits(xbc_autoMapperKeyState, 2);
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
  out.WriteBits(x6c_language, GetBitCount(7));
#endif
  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  int cinematicCount = 0;
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    TLockedToken< CWorldSaveGameInfo > saveWorld = gpSimplePool->GetObj(
        SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    cinematicCount += saveWorld->GetCinematicCount();
  }
  rstl::vector< bool > cinematicStates;
  cinematicStates.reserve(cinematicCount);
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    CAssetId worldId = it->first;
    TLockedToken< CWorldSaveGameInfo > saveWorld = gpSimplePool->GetObj(
        SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    const CWorldSaveGameInfo& world = **saveWorld;
    for (int i = 0; i < world.GetCinematicCount(); ++i)
      cinematicStates.push_back(GetCinematicState(
          rstl::pair< CAssetId, TEditorId >(worldId, world.GetCinematics()[i])));
  }
  for (int i = 0; i < cinematicCount; ++i)
    out.WriteBits(cinematicStates[i] ? 1 : 0, 1);
}

bool CSystemState::GetCinematicState(rstl::pair< CAssetId, TEditorId > cineId) const {
  for (AUTO(it, xac_cinematicStates.begin()); it != xac_cinematicStates.end(); ++it) {
    if (*it == cineId)
      return true;
  }
  return false;
}

void CSystemState::SetCinematicState(rstl::pair< CAssetId, TEditorId > cineId, bool state) {
  for (AUTO(it, xac_cinematicStates.begin()); it != xac_cinematicStates.end(); ++it) {
    if (*it == cineId) {
      if (!state)
        xac_cinematicStates.erase(it);
      return;
    }
  }
  if (state) {
    xac_cinematicStates.reserve(xac_cinematicStates.size() + 1);
    xac_cinematicStates.push_back(cineId);
  }
}

bool CSystemState::GetShowFrozenFpsMessage() const { return xc0_frozenFpsCount < 3u; }
void CSystemState::IncNumFreezeInstructionsPrintedFirstPerson() {
  xc0_frozenFpsCount = rstl::min_val(xc0_frozenFpsCount + 1, 3);
}
bool CSystemState::GetShowFrozenBallMessage() const { return xc4_frozenBallCount < 3u; }
void CSystemState::IncrementFrozenBallCount() {
  xc4_frozenBallCount = rstl::min_val(xc4_frozenBallCount + 1, 3);
}
bool CSystemState::GetShowPowerBombAmmoMessage() const { return xc8_powerBombAmmoCount < 1u; }
void CSystemState::IncrementPowerBombAmmoCount() {
  xc8_powerBombAmmoCount = rstl::min_val(xc8_powerBombAmmoCount + 1, 1);
}
void CSystemState::SetFusionLinked(bool v) { xd0_24_fusionLinked = v; }
int CSystemState::GetLogScanCount() const { return xcc_logScanPercent; }
void CSystemState::SetLogScanCount(int count) { xcc_logScanPercent = count; }
void CSystemState::SetHasHardMode(bool v) { xd0_25_normalModeBeat = v; }
void CSystemState::SetHardModeBeat(bool v) { xd0_26_hardModeBeat = v; }
void CSystemState::SetFusionBeat(bool v) { xd0_27_fusionBeat = v; }
void CSystemState::SetHasFusion(bool v) { xd0_28_fusionSuitActive = v; }
void CSystemState::SetAllItemsCollected(bool v) { xd0_29_allItemsCollected = v; }
void CSystemState::SetAutoMapperKeyState(int state) { xbc_autoMapperKeyState = state; }

CWorldState::CWorldState(CAssetId worldId) : x0_worldId(worldId)
, x4_areaId(0)
, x8_mailbox(rs_new CScriptMailbox)
, xc_mapWorldInfo(rs_new CMapWorldInfo)
, x10_desiredAreaAssetId(kInvalidAssetId)
, x14_layerState(rs_new CScriptLayerManager) {}

CWorldState::CWorldState(CInputStream& in, CAssetId worldId,
                         const CWorldSaveGameInfo& saveWorld)
: x0_worldId(worldId)
, x4_areaId(-1)
, x8_mailbox(nullptr)
, xc_mapWorldInfo(nullptr)
, x10_desiredAreaAssetId(kInvalidAssetId)
, x14_layerState(nullptr) {
  x4_areaId = TAreaId(in.ReadBits(32));
  x10_desiredAreaAssetId = in.ReadBits(32);
  x8_mailbox = rs_new CScriptMailbox(in, saveWorld);
  xc_mapWorldInfo = rs_new CMapWorldInfo(in, saveWorld, x0_worldId);
  x14_layerState = rs_new CScriptLayerManager(in, saveWorld);
}

void CWorldState::PutTo(COutputStream& out, CWorldSaveGameInfo& saveWorld) {
  out.WriteBits(x4_areaId.Value(), 32);
  out.WriteBits(x10_desiredAreaAssetId, 32);
  x8_mailbox->PutTo(out, saveWorld);
  xc_mapWorldInfo->PutTo(out, saveWorld, x0_worldId);
  x14_layerState->PutTo(out, saveWorld);
}

CAssetId CWorldState::GetWorldAssetId() const { return x0_worldId; }
rstl::ncrc_ptr< CScriptMailbox >& CWorldState::Mailbox() { return x8_mailbox; }
rstl::ncrc_ptr< CMapWorldInfo >& CWorldState::MapWorldInfo() { return xc_mapWorldInfo; }
rstl::rc_ptr< CMapWorldInfo > CWorldState::GetMapWorldInfo() const { return xc_mapWorldInfo; }
TAreaId CWorldState::GetCurrentArea() const { return x4_areaId; }
void CWorldState::SetAreaId(TAreaId id) { x4_areaId = id; }
CAssetId CWorldState::GetDesiredAreaAssetId() const { return x10_desiredAreaAssetId; }
void CWorldState::SetDesiredAreaAssetId(CAssetId id) { x10_desiredAreaAssetId = id; }
rstl::ncrc_ptr< CScriptLayerManager >& CWorldState::GetLayerState() { return x14_layerState; }

CGameState::CGameState() : x0_(static_cast< uchar >(0))
, x84_mlvlId(kInvalidAssetId)
, x98_playerState(rs_new CPlayerState)
, x9c_transManager(rs_new CWorldTransManager)
, xa0_playTime(0.0)
, x20c_saveIdx(0)
, x210_cardSerial(0)
, x228_24_hardMode(false)
, x228_25_initPowerupsAtFirstSpawn(true) {
  if (gpMemoryCard != nullptr)
    InitializeMemoryStates();
}

CGameState::CGameState(CInputStream& in, int saveIdx) : x0_(static_cast< uchar >(0))
, x84_mlvlId(kInvalidAssetId)
, x98_playerState(nullptr)
, x9c_transManager(rs_new CWorldTransManager)
, xa0_playTime(0.0)
, x20c_saveIdx(saveIdx)
, x210_cardSerial(0)
, x228_24_hardMode(false)
, x228_25_initPowerupsAtFirstSpawn(false) {
  for (int i = 0; i < x0_.size(); ++i)
    x0_[i] = in.ReadBits(8);
  in.ReadBits(32);
  x228_24_hardMode = in.ReadBits(1) != 0;
  x228_25_initPowerupsAtFirstSpawn = in.ReadBits(1) != 0;
  x84_mlvlId = in.ReadBits(32);
  CMain::EnsureWorldPakReady(x84_mlvlId);
  uint high = in.ReadBits(32);
  uint low = in.ReadBits(32);
  SGameTime time;
  time.bits = high;
  time.bits <<= 32;
  time.bits |= low;
  xa0_playTime = time.value;
  x98_playerState = rs_new CPlayerState(in);
  x17c_gameOptions = CGameOptions(in);
  x1f8_hintOptions = CHintOptions(in);

  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  x88_worldStates.reserve(worlds.size());
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    const CAssetId worldId = it->first;
    TLockedToken< CWorldSaveGameInfo > saveWorld = gpSimplePool->GetObj(
        SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    x88_worldStates.push_back(CWorldState(in, worldId, **saveWorld));
  }
  InitializeMemoryWorlds();
  WriteBackupBuf();
}

void CGameState::InitializeMemoryStates() {
  x98_playerState->InitializeScanTimes();
  x1f8_hintOptions.InitializeMemoryState();
  InitializeMemoryWorlds();
  WriteBackupBuf();
}

void CGameState::InitializeMemoryWorlds() {
  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    rstl::rc_ptr< CScriptLayerManager > layers = StateForWorld(it->first).GetLayerState();
    rstl::rc_ptr< rstl::vector< rstl::string > > names;
    rstl::rc_ptr< rstl::vector< int > > offsets;
    layers->InitializeWorldLayers(it->second.GetDefaultLayerStates(), names, offsets);
  }
}

CGameState::GameFileStateInfo CGameState::LoadGameFileState(const void* data) {
  GameFileStateInfo result;
  CMemoryInStream in(data, 4096);
  for (int i = 0; i < 128; ++i)
    in.ReadBits(8);
  uint timestamp = in.ReadBits(32);
  result.x20_hardMode = in.ReadBits(1) != 0;
  in.ReadBits(1);
  result.x8_mlvlId = in.ReadBits(32);
  uint high = in.ReadBits(32);
  uint low = in.ReadBits(32);
  SGameTime time;
  time.bits = high;
  time.bits <<= 32;
  time.bits |= low;
  result.x0_playTime = time.value;
  CPlayerState player(in);
  result.xc_health = player.GetHealthInfo().GetHP();
  result.x10_energyTanks = player.GetItemCapacity(CPlayerState::kIT_EnergyTanks);
  result.x14_timestamp = timestamp;
  uint itemPercent;
  if (result.x8_mlvlId == 0x158efe17)
    itemPercent = 0;
  else
    itemPercent = player.CalculateItemCollectionRate() * 100 / player.GetTotalPickupCount();
  result.x18_itemPercent = itemPercent;
  float scanPercent;
  if (player.GetTotalLogScans() == 0)
    scanPercent = 0.f;
  else
    scanPercent = 100.f * (float(player.GetLogScans()) / float(player.GetTotalLogScans()));
  result.x1c_scanPercent = scanPercent;
  return result;
}

void CGameState::PutTo(COutputStream& out) {
  for (int i = 0; i < x0_.size(); ++i)
    out.WriteBits(x0_[i], 8);
  out.WriteBits(OSTicksToSeconds(OSGetTime()), 32);
  out.WriteBits(x228_24_hardMode ? 1 : 0, 1);
  out.WriteBits(x228_25_initPowerupsAtFirstSpawn ? 1 : 0, 1);
  out.WriteBits(x84_mlvlId, 32);
  u64 time = *reinterpret_cast< const u64* >(&xa0_playTime);
  out.WriteBits(time >> 32, 32);
  out.WriteBits(time & 0xffffffff, 32);
  x98_playerState->PutTo(out);
  x17c_gameOptions.PutTo(out);
  x1f8_hintOptions.PutTo(out);
  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    const CAssetId worldId = it->first;
    TLockedToken< CWorldSaveGameInfo > saveWorld = gpSimplePool->GetObj(
        SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    CWorldSaveGameInfo& world = **saveWorld;
    CWorldState& state = StateForWorld(worldId);
    state.PutTo(out, world);
  }
}

void CGameState::ReadSystemOptions(CInputStream& in) { xa8_systemState = CSystemState(in); }
void CGameState::WriteSystemOptions(COutputStream& out) { xa8_systemState.PutTo(out); }

void CGameState::ImportPersistentOptions(const CSystemState& options) {
  if (options.GetFusionLinked())
    SystemState().SetFusionLinked(true);
  if (options.GetFusionBeat())
    SystemState().SetFusionBeat(true);
  SystemState().x0_nesState = options.x0_nesState;
  SystemState().SetLogScanCount(options.GetLogScanCount());
  SystemState().SetAllItemsCollected(options.GetAllItemsCollected());
  SystemState().SetHasHardMode(options.GetNormalModeBeat());
  SystemState().SetHardModeBeat(options.GetHardModeBeat());
}

void CGameState::ExportPersistentOptions(CSystemState& options) {
  if (SystemState().GetFusionLinked())
    options.SetFusionLinked(true);
  if (SystemState().GetFusionBeat())
    options.SetFusionBeat(true);
  options.x0_nesState = SystemState().x0_nesState;
  options.SetHasFusion(SystemState().GetHasFusion());
  options.x0_nesState = SystemState().x0_nesState;
}

void CGameState::WriteBackupBuf() {
  x218_backupBuf.assign(940);
  CMemoryStreamOut out(x218_backupBuf.data(), 940);
  PutTo(out);
}

CWorldState& CGameState::StateForWorld(CAssetId worldId) {
  AUTO(it, x88_worldStates.begin());
  for (; it != x88_worldStates.end(); ++it) {
    if (worldId == it->GetWorldAssetId())
      break;
  }
  if (it != x88_worldStates.end())
    return *it;
  x88_worldStates.reserve(x88_worldStates.size() + 1);
  x88_worldStates.push_back(CWorldState(worldId));
  return x88_worldStates.back();
}

CAssetId CGameState::CurrentWorldAssetId() const { return x84_mlvlId; }
CWorldState& CGameState::CurrentWorldState() { return StateForWorld(x84_mlvlId); }
const CWorldState& CGameState::GetCurrentWorldState() const {
  return const_cast< CGameState* >(this)->StateForWorld(x84_mlvlId);
}

void CGameState::SetCurrentWorldId(CAssetId worldId) {
  StateForWorld(worldId);
  x84_mlvlId = worldId;
  CMain::EnsureWorldPakReady(worldId);
}

rstl::ncrc_ptr< CPlayerState >& CGameState::PlayerState() { return x98_playerState; }
rstl::rc_ptr< CPlayerState > CGameState::GetPlayerState() const { return x98_playerState; }
rstl::ncrc_ptr< CWorldTransManager >& CGameState::WorldTransitionManager() {
  return x9c_transManager;
}

void CGameState::SetTotalPlayTime(double time) {
  static const double kMaxPlayTime = 359999.0;
  xa0_playTime = CMath::Clamp(0.0, time, kMaxPlayTime);
}

void CGameState::SetHardMode(bool hardMode) { x228_24_hardMode = hardMode; }
float CGameState::GetHardModeDamageMultiplier() const {
  return gpTweakGame->GetHardModeDamageMultiplier();
}
float CGameState::GetHardModeWeaponMultiplier() const {
  return gpTweakGame->GetHardModeWeaponMultiplier();
}
void CGameState::SetDeferPowerupInit(bool defer) { x228_25_initPowerupsAtFirstSpawn = defer; }
