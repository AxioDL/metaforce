#include "Kyoto/Basics/CStopwatch.hpp"

#include "dolphin/os.h"

void CStopwatch::CSWData::Wait(float v) const {
  OSTime duration = OSSecondsToTicks(v);
  OSTime end = OSGetTime() + duration;
  volatile OSTime current;
  volatile int diff;
  do {
    current = OSGetTime();
    diff = current - end;
  } while (diff < 0);
}

bool CStopwatch::CSWData::Initialize() {
  mTimerFreq = OS_TIMER_CLOCK;
  mTimerFreqO1M = mTimerFreq / 1000000ll;
  mTimerPeriod = 1.f / static_cast< float >(mTimerFreq);
  return true;
}
