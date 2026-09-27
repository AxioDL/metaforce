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

  CSystemState& SystemState() { return mSystemState; }
  CGameOptions& GameOptions() { return mGameOptions; }
  CHintOptions& HintOptions() { return mHintOptions; }
  uint& SaveIdx() { return mSaveIdx; }
  u64& CardSerial() { return mCardSerial; }
  rstl::vector< uchar >& BackupBuf() { return mBackupBuf; }
  u32 GetFileIdx() const { return mSaveIdx; }
  void SetFileIdx(u32 idx) { mSaveIdx = idx; }
  void SetCardSerial(u64 serial) { mCardSerial = serial; }
  u64 GetCardSerial() const { return mCardSerial; }
  bool GetHardMode() const { return mHardMode; }
  void SetHardMode(bool v);
  bool GetInitPowerupsAtFirstSpawn() const { return mInitPowerupsAtFirstSpawn; }
  double GetTotalPlayTime() const { return mPlayTime; }
  float GetHardModeDamageMultiplier() const;
  float GetHardModeWeaponMultiplier() const;
  rstl::ncrc_ptr< CWorldTransManager >& WorldTransitionManager();

  struct GameFileStateInfo {
    double mPlayTime;
    CAssetId mMlvlId;
    float mHealth;
    uint mEnergyTanks;
    uint mTimestamp;
    uint mItemPercent;
    float mScanPercent;
    bool mHardMode;
  };
  static GameFileStateInfo LoadGameFileState(const void* data);

private:
  void InitializeMemoryWorlds();

  rstl::reserved_vector< uchar, 128 > x0_;
  CAssetId mMlvlId;
  rstl::vector< CWorldState > mWorldStates;
  rstl::ncrc_ptr< CPlayerState > mPlayerState;
  rstl::ncrc_ptr< CWorldTransManager > mTransManager;
  double mPlayTime;
  CSystemState mSystemState;
  CGameOptions mGameOptions;
  CHintOptions mHintOptions;
#if VERSION == VERSION_GM8J_00
  rstl::vector< rstl::pair< CAssetId, TEditorId > > mCinematicStates;
#endif
  uint mSaveIdx;
  u64 mCardSerial;
  rstl::vector< uchar > mBackupBuf;
  bool mHardMode : 1;
  bool mInitPowerupsAtFirstSpawn : 1;
};
#if VERSION == VERSION_GM8J_00
CHECK_SIZEOF(CGameState, 0x7e0)
#elif VERSION >= VERSION_GM8P_00
CHECK_SIZEOF(CGameState, 0x1e8)
#else
CHECK_SIZEOF(CGameState, 0x230)
#endif

extern CGameState* gpGameState;

#endif // _CGAMESTATE
