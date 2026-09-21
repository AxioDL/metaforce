#ifndef _CMAIN
#define _CMAIN

#include "types.h"

#include "rstl/reserved_vector.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
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

// TODO move to new header
class CDvdRequestSys {
public:
  CDvdRequestSys() {
    if (mManagerInstalled != true) {
      mManagerInstalled = true;
    }
  }
  ~CDvdRequestSys() {
    if (mManagerInstalled == true) {
      mManagerInstalled = false;
    }
  }

private:
  static bool mManagerInstalled;
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
  void ReloadStringTables();
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
  void SetRestartMode(const ERestartMode s) { x12c_restartMode = s; }
  ERestartMode GetRestartMode() const { return x12c_restartMode; }
  void SetCardBusy(bool v) { x160_31_cardBusy = v; }
  void SetManageCard(bool v) { x160_28_manageCard = v; }
  bool GetCardBusy() const { return x160_31_cardBusy; }

  void SetMaxSpeed(bool v) {
    // ?
    x160_26_screenFading = v;
  }

  void SetGameFlowBuilt(const bool built) { x160_25_mfGameBuilt = built; }
  float GetAverageTickTime() const { return x118_averageTickTime; }
  float GetAverageDrawTime() const { return x11c_averageDrawTime; }
  bool GetScreenFading() const { return x160_26_screenFading; }
  void SetScreenFading(const bool fading) { x160_26_screenFading = fading; }
  void SetGameFrameDrawn(const bool drawn) { x161_24_gameFrameDrawn = drawn; }

  void SetX30(bool v) { x160_30_gameExitReset = v; }

  static void EnsureWorldPaksReady();
  static void EnsureWorldPakReady(CAssetId id);

  COsContext& OsContext() { return x0_osContext; }
  const COsContext& GetOsContext() const { return x0_osContext; }

private:
  COsContext x0_osContext;
  CSaveRegion x6c_saveRegion;
#if !defined(TARGET_PC)
  CMemorySys x6d_memorySys;
#endif
  CDvdRequestSys x6e_dvdRequestSys;
  CTweaks x70_tweaks;
  double xe8_unknown;
  TReservedAverage< float, 4 > xf0_tickTimes;
  TReservedAverage< float, 4 > x104_drawTimes;
  float x118_averageTickTime;
  float x11c_averageDrawTime;
  float x120_softResetHoldTime;
  float x124_resetInputDelay;
  CGameGlobalObjects* x128_gameGlobalObjects;
  ERestartMode x12c_restartMode;
  rstl::reserved_vector< uint, 10 > x130_frameTimes;
  int x15c_frameTimeIdx;
  bool x160_24_finished : 1;
  bool x160_25_mfGameBuilt : 1;
  bool x160_26_screenFading : 1;
  bool x160_27_resetButtonHeld : 1;
  bool x160_28_manageCard : 1;
  bool x160_29_resetRequested : 1;
  bool x160_30_gameExitReset : 1;
  bool x160_31_cardBusy : 1;
  bool x161_24_gameFrameDrawn : 1;
  CGameArchitectureSupport* x164_archSupport;
};
CHECK_SIZEOF(CMain, 0x168)

extern CMain* gpMain;

#endif // _CMAIN
