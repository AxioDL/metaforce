#ifndef _CRUMBLEGENERATOR
#define _CRUMBLEGENERATOR

#include "types.h"

#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Input/InputTypes.hpp"

class CRumbleGenerator {
private:
  static const EMotorState kStopAll[4];
  CRumbleVoice mVoices[4];
  float mPeriodTime[4];
  float mOnTime[4];
  EMotorState mCommandArray[4];
  bool mDisabled : 1;

public:
  CRumbleGenerator();
  ~CRumbleGenerator();

  short Rumble(const SAdsrData& adsr, float, ERumblePriority prio, EIOPort port);
  void Stop(short id, EIOPort port) {
    if (id == -1)
      return;
    CRumbleVoice& vox = mVoices[size_t(port)];
    vox.Deactivate(id, false);
  }
  void Deactivate(short, bool);
  void Update(float);
  void HardStopAll();
  bool GetDisabled() const { return mDisabled; }
  void SetDisabled(const bool disabled);
};

#endif // _CRUMBLEGENERATOR
