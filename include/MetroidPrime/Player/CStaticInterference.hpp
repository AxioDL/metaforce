#ifndef _CSTATICINTERFERENCE
#define _CSTATICINTERFERENCE

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/vector.hpp"

class CStaticInterferenceSource {
public:
  CStaticInterferenceSource(TUniqueId id, float magnitude, float timeLeft)
  : mId(id), mMagnitude(magnitude), mTimeLeft(timeLeft) {}

  const TUniqueId GetSourceId() const { return mId; }
  const float GetIntensity() const { return mMagnitude; }
  void SetIntensity(const float v) { mMagnitude = v; }
  const float GetTime() const { return mTimeLeft; }
  void SetTime(const float v) { mTimeLeft = v; }

private:
  TUniqueId mId;
  float mMagnitude;
  float mTimeLeft;
};

class CStateManager;

class CStaticInterference {
public:
  CStaticInterference(int sourceCount);

  void AddSource(TUniqueId id, float magnitude, float duration);
  void RemoveSource(TUniqueId id);
  void Update(CStateManager&, float dt);
  float GetTotalInterference() const;

private:
  rstl::vector< CStaticInterferenceSource > sources;
};

#endif // _CSTATICINTERFERENCE
