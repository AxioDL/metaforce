#ifndef _CGAMESTATE
#define _CGAMESTATE

#include "types.h"

#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CHintOptions.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CSystemState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CGameState {
public:
  CGameState();
  CGameState(CInputStream& in, int saveIdx);

  void ReadSystemOptions(CInputStream& in);
  void PutTo(COutputStream& out);
  void WriteSystemOptions(COutputStream& out);

  void SetCurrentWorldId(CAssetId);
  void InitializeMemoryStates();
  void SetDeferPowerupInit(bool);
  void SetTotalPlayTime(double);

  rstl::ncrc_ptr< CPlayerState >& PlayerState();
  rstl::rc_ptr< CPlayerState > GetPlayerState() const;
  CAssetId CurrentWorldAssetId() const;
  void WriteBackupBuf();

  CWorldState& StateForWorld(CAssetId mlvlId);
  CWorldState& CurrentWorldState();
  const CWorldState& GetCurrentWorldState() const;

  void ImportPersistentOptions(const CSystemState&);
  void ExportPersistentOptions(CSystemState&);

  CSystemState& SystemState() { return xa8_systemState; }
  CGameOptions& GameOptions() { return x17c_gameOptions; }
  CHintOptions& HintOptions() { return x1f8_hintOptions; }
  uint& SaveIdx() { return x20c_saveIdx; }
  u64& CardSerial() { return x210_cardSerial; }
  rstl::vector< uchar >& BackupBuf() { return x218_backupBuf; }
  u32 GetFileIdx() const { return x20c_saveIdx; }
  void SetFileIdx(u32 idx) { x20c_saveIdx = idx; }
  void SetCardSerial(u64 serial) { x210_cardSerial = serial; }
  u64 GetCardSerial() const { return x210_cardSerial; }
  bool GetHardMode() const { return x228_24_hardMode; }
  void SetHardMode(bool v);
  bool GetInitPowerupsAtFirstSpawn() const { return x228_25_initPowerupsAtFirstSpawn; }
  double GetTotalPlayTime() const { return xa0_playTime; }
  float GetHardModeDamageMultiplier() const;
  float GetHardModeWeaponMultiplier() const;
  rstl::ncrc_ptr< CWorldTransManager >& WorldTransitionManager();

  struct GameFileStateInfo {
    double x0_playTime;
    CAssetId x8_mlvlId;
    float xc_health;
    uint x10_energyTanks;
    uint x14_timestamp;
    uint x18_itemPercent;
    float x1c_scanPercent;
    bool x20_hardMode;
  };
  static GameFileStateInfo LoadGameFileState(const void* data);

private:
  void InitializeMemoryWorlds();

  rstl::reserved_vector< uchar, 128 > x0_;
  CAssetId x84_mlvlId;
  rstl::vector< CWorldState > x88_worldStates;
  rstl::ncrc_ptr< CPlayerState > x98_playerState;
  rstl::ncrc_ptr< CWorldTransManager > x9c_transManager;
  double xa0_playTime;
  CSystemState xa8_systemState;
  CGameOptions x17c_gameOptions;
  CHintOptions x1f8_hintOptions;
  uint x20c_saveIdx;
  u64 x210_cardSerial;
  rstl::vector< uchar > x218_backupBuf;
  bool x228_24_hardMode : 1;
  bool x228_25_initPowerupsAtFirstSpawn : 1;
};
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
CHECK_SIZEOF(CGameState, 0x1e8)
#else
CHECK_SIZEOF(CGameState, 0x230)
#endif

extern CGameState* gpGameState;

#endif // _CGAMESTATE
