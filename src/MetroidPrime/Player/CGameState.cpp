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

CSystemState::CSystemState() : mNesState(static_cast< uchar >(0))
, x68_(static_cast< uchar >(0))
#if VERSION >= VERSION_GM8P_00
, mLanguage(gpMain->GetLanguage())
#endif
, mAutoMapperKeyState(0)
, mFrozenFpsCount(0)
, mFrozenBallCount(0)
, mPowerBombAmmoCount(0)
, mLogScanPercent(0)
, mFusionLinked(false)
, mNormalModeBeat(false)
, mHardModeBeat(false)
, mFusionBeat(false)
, mFusionSuitActive(false)
, mAllItemsCollected(false) {}

CSystemState::CSystemState(CInputStream& in) : mNesState(static_cast< uchar >(0))
, x68_(static_cast< uchar >(0))
#if VERSION >= VERSION_GM8P_00
, mLanguage(0)
#endif
, mAutoMapperKeyState(0)
, mFrozenFpsCount(0)
, mFrozenBallCount(0)
, mPowerBombAmmoCount(0)
, mLogScanPercent(0)
, mFusionLinked(false)
, mNormalModeBeat(false)
, mHardModeBeat(false)
, mFusionBeat(false)
, mFusionSuitActive(false)
, mAllItemsCollected(false) {
  for (int i = 0; i < kNESStateSize; ++i)
    mNesState[i] = in.ReadBits(8);
  for (int i = 0; i < 64; ++i)
    x68_[i] = in.ReadBits(8);
  mFrozenFpsCount = in.ReadBits(GetBitCount(3));
  mFrozenBallCount = in.ReadBits(GetBitCount(3));
  mPowerBombAmmoCount = in.ReadBits(GetBitCount(1));
  mLogScanPercent = in.ReadBits(GetBitCount(100));
  mFusionLinked = in.ReadBits(1) != 0;
  mNormalModeBeat = in.ReadBits(1) != 0;
  mHardModeBeat = in.ReadBits(1) != 0;
  mFusionBeat = in.ReadBits(1) != 0;
  mAllItemsCollected = in.ReadBits(1) != 0;
  mAutoMapperKeyState = in.ReadBits(2);
#if VERSION >= VERSION_GM8P_00
  mLanguage = in.ReadBits(GetBitCount(7));
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
    out.WriteBits(mNesState[i], 8);
  for (int i = 0; i < 64; ++i)
    out.WriteBits(x68_[i], 8);
  out.WriteBits(mFrozenFpsCount, GetBitCount(3));
  out.WriteBits(mFrozenBallCount, GetBitCount(3));
  out.WriteBits(mPowerBombAmmoCount, GetBitCount(1));
  out.WriteBits(mLogScanPercent, GetBitCount(100));
  out.WriteBits(mFusionLinked ? 1 : 0, 1);
  out.WriteBits(mNormalModeBeat ? 1 : 0, 1);
  out.WriteBits(mHardModeBeat ? 1 : 0, 1);
  out.WriteBits(mFusionBeat ? 1 : 0, 1);
  out.WriteBits(mAllItemsCollected ? 1 : 0, 1);
  out.WriteBits(mAutoMapperKeyState, 2);
#if VERSION >= VERSION_GM8P_00
  out.WriteBits(mLanguage, GetBitCount(7));
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
  for (AUTO(it, mCinematicStates.begin()); it != mCinematicStates.end(); ++it) {
    if (*it == cineId)
      return true;
  }
  return false;
}

void CSystemState::SetCinematicState(rstl::pair< CAssetId, TEditorId > cineId, bool state) {
  for (AUTO(it, mCinematicStates.begin()); it != mCinematicStates.end(); ++it) {
    if (*it == cineId) {
      if (!state)
        mCinematicStates.erase(it);
      return;
    }
  }
  if (state) {
    mCinematicStates.reserve(mCinematicStates.size() + 1);
    mCinematicStates.push_back(cineId);
  }
}

bool CSystemState::GetShowFrozenFpsMessage() const { return mFrozenFpsCount < 3u; }
void CSystemState::IncNumFreezeInstructionsPrintedFirstPerson() {
  mFrozenFpsCount = rstl::min_val(mFrozenFpsCount + 1, 3);
}
bool CSystemState::GetShowFrozenBallMessage() const { return mFrozenBallCount < 3u; }
void CSystemState::IncrementFrozenBallCount() {
  mFrozenBallCount = rstl::min_val(mFrozenBallCount + 1, 3);
}
bool CSystemState::GetShowPowerBombAmmoMessage() const { return mPowerBombAmmoCount < 1u; }
void CSystemState::IncrementPowerBombAmmoCount() {
  mPowerBombAmmoCount = rstl::min_val(mPowerBombAmmoCount + 1, 1);
}
void CSystemState::SetFusionLinked(bool v) { mFusionLinked = v; }
int CSystemState::GetLogScanCount() const { return mLogScanPercent; }
void CSystemState::SetLogScanCount(int count) { mLogScanPercent = count; }
void CSystemState::SetHasHardMode(bool v) { mNormalModeBeat = v; }
void CSystemState::SetHardModeBeat(bool v) { mHardModeBeat = v; }
void CSystemState::SetFusionBeat(bool v) { mFusionBeat = v; }
void CSystemState::SetHasFusion(bool v) { mFusionSuitActive = v; }
void CSystemState::SetAllItemsCollected(bool v) { mAllItemsCollected = v; }
void CSystemState::SetAutoMapperKeyState(int state) { mAutoMapperKeyState = state; }

CWorldState::CWorldState(CAssetId worldId) : mWorldId(worldId)
, mAreaId(0)
, mMailbox(rs_new CScriptMailbox)
, mMapWorldInfo(rs_new CMapWorldInfo)
, mDesiredAreaAssetId(kInvalidAssetId)
, mLayerState(rs_new CScriptLayerManager) {}

CWorldState::CWorldState(CInputStream& in, CAssetId worldId,
                         const CWorldSaveGameInfo& saveWorld)
: mWorldId(worldId)
, mAreaId(-1)
, mMailbox(nullptr)
, mMapWorldInfo(nullptr)
, mDesiredAreaAssetId(kInvalidAssetId)
, mLayerState(nullptr) {
  mAreaId = TAreaId(in.ReadBits(32));
  mDesiredAreaAssetId = in.ReadBits(32);
  mMailbox = rs_new CScriptMailbox(in, saveWorld);
  mMapWorldInfo = rs_new CMapWorldInfo(in, saveWorld, mWorldId);
  mLayerState = rs_new CScriptLayerManager(in, saveWorld);
}

void CWorldState::PutTo(COutputStream& out, CWorldSaveGameInfo& saveWorld) {
  out.WriteBits(mAreaId.Value(), 32);
  out.WriteBits(mDesiredAreaAssetId, 32);
  mMailbox->PutTo(out, saveWorld);
  mMapWorldInfo->PutTo(out, saveWorld, mWorldId);
  mLayerState->PutTo(out, saveWorld);
}

CAssetId CWorldState::GetWorldAssetId() const { return mWorldId; }
rstl::ncrc_ptr< CScriptMailbox >& CWorldState::Mailbox() { return mMailbox; }
rstl::ncrc_ptr< CMapWorldInfo >& CWorldState::MapWorldInfo() { return mMapWorldInfo; }
rstl::rc_ptr< CMapWorldInfo > CWorldState::GetMapWorldInfo() const { return mMapWorldInfo; }
TAreaId CWorldState::GetCurrentArea() const { return mAreaId; }
void CWorldState::SetAreaId(TAreaId id) { mAreaId = id; }
CAssetId CWorldState::GetDesiredAreaAssetId() const { return mDesiredAreaAssetId; }
void CWorldState::SetDesiredAreaAssetId(CAssetId id) { mDesiredAreaAssetId = id; }
rstl::ncrc_ptr< CScriptLayerManager >& CWorldState::GetLayerState() { return mLayerState; }

CGameState::CGameState() : x0_(static_cast< uchar >(0))
, mMlvlId(kInvalidAssetId)
, mPlayerState(rs_new CPlayerState)
, mTransManager(rs_new CWorldTransManager)
, mPlayTime(0.0)
, mSaveIdx(0)
, mCardSerial(0)
, mHardMode(false)
, mInitPowerupsAtFirstSpawn(true) {
  if (gpMemoryCard != nullptr)
    InitializeMemoryStates();
}

CGameState::CGameState(CInputStream& in, int saveIdx) : x0_(static_cast< uchar >(0))
, mMlvlId(kInvalidAssetId)
, mPlayerState(nullptr)
, mTransManager(rs_new CWorldTransManager)
, mPlayTime(0.0)
, mSaveIdx(saveIdx)
, mCardSerial(0)
, mHardMode(false)
, mInitPowerupsAtFirstSpawn(false) {
  for (int i = 0; i < x0_.size(); ++i)
    x0_[i] = in.ReadBits(8);
  in.ReadBits(32);
  mHardMode = in.ReadBits(1) != 0;
  mInitPowerupsAtFirstSpawn = in.ReadBits(1) != 0;
  mMlvlId = in.ReadBits(32);
  CMain::EnsureWorldPakReady(mMlvlId);
  uint high = in.ReadBits(32);
  uint low = in.ReadBits(32);
  SGameTime time;
  time.bits = high;
  time.bits <<= 32;
  time.bits |= low;
  mPlayTime = time.value;
  mPlayerState = rs_new CPlayerState(in);
  mGameOptions = CGameOptions(in);
  mHintOptions = CHintOptions(in);

  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  mWorldStates.reserve(worlds.size());
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    const CAssetId worldId = it->first;
    TLockedToken< CWorldSaveGameInfo > saveWorld = gpSimplePool->GetObj(
        SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    mWorldStates.push_back(CWorldState(in, worldId, **saveWorld));
  }
  InitializeMemoryWorlds();
  WriteBackupBuf();
}

void CGameState::InitializeMemoryStates() {
  mPlayerState->InitializeScanTimes();
  mHintOptions.InitializeMemoryState();
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
  result.mHardMode = in.ReadBits(1) != 0;
  in.ReadBits(1);
  result.mMlvlId = in.ReadBits(32);
  uint high = in.ReadBits(32);
  uint low = in.ReadBits(32);
  SGameTime time;
  time.bits = high;
  time.bits <<= 32;
  time.bits |= low;
  result.mPlayTime = time.value;
  CPlayerState player(in);
  result.mHealth = player.GetHealthInfo().GetHP();
  result.mEnergyTanks = player.GetItemCapacity(CPlayerState::kIT_EnergyTanks);
  result.mTimestamp = timestamp;
  uint itemPercent;
  if (result.mMlvlId == 0x158efe17)
    itemPercent = 0;
  else
    itemPercent = player.CalculateItemCollectionRate() * 100 / player.GetTotalPickupCount();
  result.mItemPercent = itemPercent;
  float scanPercent;
  if (player.GetTotalLogScans() == 0)
    scanPercent = 0.f;
  else
    scanPercent = 100.f * (float(player.GetLogScans()) / float(player.GetTotalLogScans()));
  result.mScanPercent = scanPercent;
  return result;
}

void CGameState::PutTo(COutputStream& out) {
  for (int i = 0; i < x0_.size(); ++i)
    out.WriteBits(x0_[i], 8);
  out.WriteBits(OSTicksToSeconds(OSGetTime()), 32);
  out.WriteBits(mHardMode ? 1 : 0, 1);
  out.WriteBits(mInitPowerupsAtFirstSpawn ? 1 : 0, 1);
  out.WriteBits(mMlvlId, 32);
  u64 time = *reinterpret_cast< const u64* >(&mPlayTime);
  out.WriteBits(time >> 32, 32);
  out.WriteBits(time & 0xffffffff, 32);
  mPlayerState->PutTo(out);
  mGameOptions.PutTo(out);
  mHintOptions.PutTo(out);
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

void CGameState::ReadSystemOptions(CInputStream& in) { mSystemState = CSystemState(in); }
void CGameState::WriteSystemOptions(COutputStream& out) { mSystemState.PutTo(out); }

void CGameState::ImportPersistentOptions(const CSystemState& options) {
  if (options.GetFusionLinked())
    SystemState().SetFusionLinked(true);
  if (options.GetFusionBeat())
    SystemState().SetFusionBeat(true);
  SystemState().mNesState = options.mNesState;
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
  options.mNesState = SystemState().mNesState;
  options.SetHasFusion(SystemState().GetHasFusion());
  options.mNesState = SystemState().mNesState;
}

void CGameState::WriteBackupBuf() {
  mBackupBuf.assign(940);
  CMemoryStreamOut out(mBackupBuf.data(), 940);
  PutTo(out);
}

CWorldState& CGameState::StateForWorld(CAssetId worldId) {
  AUTO(it, mWorldStates.begin());
  for (; it != mWorldStates.end(); ++it) {
    if (worldId == it->GetWorldAssetId())
      break;
  }
  if (it != mWorldStates.end())
    return *it;
  mWorldStates.reserve(mWorldStates.size() + 1);
  mWorldStates.push_back(CWorldState(worldId));
  return mWorldStates.back();
}

CAssetId CGameState::CurrentWorldAssetId() const { return mMlvlId; }
CWorldState& CGameState::CurrentWorldState() { return StateForWorld(mMlvlId); }
const CWorldState& CGameState::GetCurrentWorldState() const {
  return const_cast< CGameState* >(this)->StateForWorld(mMlvlId);
}

void CGameState::SetCurrentWorldId(CAssetId worldId) {
  StateForWorld(worldId);
  mMlvlId = worldId;
  CMain::EnsureWorldPakReady(worldId);
}

rstl::ncrc_ptr< CPlayerState >& CGameState::PlayerState() { return mPlayerState; }
rstl::rc_ptr< CPlayerState > CGameState::GetPlayerState() const { return mPlayerState; }
rstl::ncrc_ptr< CWorldTransManager >& CGameState::WorldTransitionManager() {
  return mTransManager;
}

void CGameState::SetTotalPlayTime(double time) {
  static const double kMaxPlayTime = 359999.0;
  mPlayTime = CMath::Clamp(0.0, time, kMaxPlayTime);
}

void CGameState::SetHardMode(bool hardMode) { mHardMode = hardMode; }
float CGameState::GetHardModeDamageMultiplier() const {
  return gpTweakGame->GetHardModeDamageMultiplier();
}
float CGameState::GetHardModeWeaponMultiplier() const {
  return gpTweakGame->GetHardModeWeaponMultiplier();
}
void CGameState::SetDeferPowerupInit(bool defer) { mInitPowerupsAtFirstSpawn = defer; }
