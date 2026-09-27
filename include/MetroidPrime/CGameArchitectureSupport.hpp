#ifndef _CGAMEARCHITECTURESUPPORT
#define _CGAMEARCHITECTURESUPPORT

#include "types.h"

#include "GuiSys/CGuiSys.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/TOneStatic.hpp"

#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CIOWinManager.hpp"
#include "MetroidPrime/CInputGenerator.hpp"

#include "rstl/vector.hpp"

class CToken;

class CGameArchitectureSupport : public TOneStatic< CGameArchitectureSupport > {
public:
  enum EAudioLoadStatus { kALS_Loading, kALS_Loaded, kALS_Uninitialized };

  CGameArchitectureSupport(COsContext&);
  ~CGameArchitectureSupport();

  void PreloadAudio();
  bool UpdateTicks();
  void Update();
  bool LoadAudio();
  void UnloadAudio();

  inline CStopwatch& GetStopwatch1() { return mTickStopwatch; }
  inline CStopwatch& GetStopwatch2() { return mFrameStopwatch; }
  inline CIOWinManager& GetIOWinManager() { return mIoWinMgr; }
  inline int& GetFramesDrawn() { return mGameFrameCount; }
  bool IsInfiniteLoopAlarmSet() const { return mInfiniteLoopAlarmSet; }
  OSAlarm& GetInfiniteLoopAlarm() { return mInfiniteLoopAlarm; }
  void SetInfiniteLoopAlarmSet(bool set) { mInfiniteLoopAlarmSet = set; }

private:
  CAudioSys mAudioSys;
  CArchitectureQueue mArchQueue;
  CStopwatch mTickStopwatch;
  CStopwatch mFrameStopwatch;
  CInputGenerator mInputGenerator;
  CGuiSys mGuiSys;
  CIOWinManager mIoWinMgr;
  int mGameFrameCount;
  float mTickRemainder;
  float mPreviousTickRemainder2;
  float mPreviousTickRemainder;
  EAudioLoadStatus mAudioLoadStatus;
  rstl::vector< CToken > mPendingAudioGroups;
  OSAlarm mInfiniteLoopAlarm;
  bool mInfiniteLoopAlarmSet;
};
CHECK_SIZEOF(CGameArchitectureSupport, 0xd0)

#endif // _CGAMEARCHITECTURESUPPORT
