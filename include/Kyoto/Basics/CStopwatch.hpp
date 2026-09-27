#ifndef _CSTOPWATCH
#define _CSTOPWATCH

#include "types.h"

#include "dolphin/os.h"

class CStopwatch {
public:
  class CSWData {
  public:
    CSWData() : mTimerFreq(0), mTimerFreqO1M(0), mTimerPeriod(0.f) {}

    bool Initialize();
    void Wait(float) const;

    s64 GetTimerFreq() const { return mTimerFreq; }
    s64 GetTimerFreqO1M() const { return mTimerFreqO1M; }
    float GetTimerPeriod() const { return mTimerPeriod; }
    s64 GetCPUCycles() const { return OSGetTime(); }

  private:
    s64 mTimerFreq;
    s64 mTimerFreqO1M;
    float mTimerPeriod;
  };

  CStopwatch() : mStartTime(mData.GetCPUCycles()) {}
  static bool InitGlobalTimer();
  static CStopwatch& GetGlobalTimerObj();
  inline void Reset() {
    if (mData.GetTimerFreq() == 0) {
      mData.Initialize();
    }
    mStartTime = mData.GetCPUCycles();
  }
  inline float GetElapsedTime() const {
    return (mData.GetCPUCycles() - mStartTime) * mData.GetTimerPeriod();
  }
  inline s64 GetElapsedMicros() const {
    return (mData.GetCPUCycles() - mStartTime) / mData.GetTimerFreqO1M();
  }

  s64 GetCurrMicros() const { return mData.GetCPUCycles() / mData.GetTimerFreqO1M(); }

  static void Wait(float);
  
  static float GetGlobalTime() { return mGlobalTimer.GetElapsedTime(); }

  static s64 GetGlobalMicros() {
    return mGlobalTimer.GetElapsedMicros();
  }

private:
  static CSWData mData;
  static CStopwatch mGlobalTimer;

  s64 mStartTime;
};

#endif // _CSTOPWATCH
