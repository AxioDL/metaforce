#ifndef _CMAIN
#define _CMAIN

#include "types.h"

#include "rstl/reserved_vector.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CDvdRequestManager.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TReservedAverage.hpp"
#include "MetroidPrime/CGameArchitectureSupport.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweaks.hpp"

class CMain;

class CSaveRegion {
public:
  CSaveRegion(CMain& main);

  static void* GetSaveBuffer() { return mSaveBuffer; }
  static void* GetNonVolatileSettingsBuffer() { return mNonVolatileSettingsBuf; }

private:
  static void* mSaveBuffer;
  static void* mNonVolatileSettingsBuf;
};

class CMain {
public:
  enum ERestartMode {
    kRM_None,
    kRM_WinBad,
    kRM_WinGood,
    kRM_WinBest,
    kRM_LoseGame,
    kRM_Default,
    kRM_StateSetter,
    kRM_PreFrontEnd,
    kRM_FrontEnd,
    kRM_Game,
    kRM_GameExit,
  };

  CMain();
  ~CMain();

  bool LoadAudio();
  void UpdateStreamedAudio();
  void RegisterResourceTweaks();
  void ResetGameState();
  static void ReloadStringTables();
  void StreamNewGameState(CInputStream& in, int saveIdx);
  void RefreshGameState();
  void AddWorldPaks();
  void AsyncIdle(uint time);
  int RsMain(int argc, const char* const* argv);
  void InitializeSubsystems();
  void FillInAssetIDs();
  void ShutdownSubsystems();
  void MemoryCardInitializePump();
  void DoPredrawMetrics();
  void DrawDebugMetrics(double dt, CStopwatch& stopWatch);
  bool CheckTerminate();
  bool CheckReset();
  void CheckTweakManagerDebugOptions();
  COsContext& OpenWindow();
  int GetLanguage();
  static void SetTiming();
  void SetRestartMode(const ERestartMode s) { mRestartMode = s; }
  ERestartMode GetRestartMode() const { return mRestartMode; }
  void SetCardBusy(bool v) { mCardBusy = v; }
  void SetManageCard(bool v) { mManageCard = v; }
  bool GetCardBusy() const { return mCardBusy; }

  void SetMaxSpeed(bool v) {
    // ?
    mScreenFading = v;
  }

  void SetGameFlowBuilt(const bool built) { mMfGameBuilt = built; }
  float GetAverageTickTime() const { return mAverageTickTime; }
  float GetAverageDrawTime() const { return mAverageDrawTime; }
  bool GetScreenFading() const { return mScreenFading; }
  void SetScreenFading(const bool fading) { mScreenFading = fading; }
  void SetGameFrameDrawn(const bool drawn) { mGameFrameDrawn = drawn; }

  void SetX30(bool v) { mGameExitReset = v; }

  static void EnsureWorldPaksReady();
  static void EnsureWorldPakReady(CAssetId id);

  COsContext& OsContext() { return mOsContext; }
  const COsContext& GetOsContext() const { return mOsContext; }

  CGameArchitectureSupport* GetArchitectureSupport() const { return mArchSupport; }

private:
  COsContext mOsContext;
  CSaveRegion mSaveRegion;
#if !defined(TARGET_PC)
  CMemorySys mMemorySys;
#endif
  CDvdRequestSys mDvdRequestSys;
  CTweaks mTweaks;
  double mUnknown;
  TReservedAverage< float, 4 > mTickTimes;
  TReservedAverage< float, 4 > mDrawTimes;
  float mAverageTickTime;
  float mAverageDrawTime;
  float mSoftResetHoldTime;
  float mResetInputDelay;
  CGameGlobalObjects* mGameGlobalObjects;
  ERestartMode mRestartMode;
  rstl::reserved_vector< uint, 10 > mFrameTimes;
  int mFrameTimeIdx;
  bool mFinished : 1;
  bool mMfGameBuilt : 1;
  bool mScreenFading : 1;
  bool mResetButtonHeld : 1;
  bool mManageCard : 1;
  bool mResetRequested : 1;
  bool mGameExitReset : 1;
  bool mCardBusy : 1;
  bool mGameFrameDrawn : 1;
  CGameArchitectureSupport* mArchSupport;
};
CHECK_SIZEOF(CMain, 0x168)

extern CMain* gpMain;

#endif // _CMAIN
