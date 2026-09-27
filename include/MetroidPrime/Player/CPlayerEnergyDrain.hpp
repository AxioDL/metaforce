#ifndef _CPLAYERENERGYDRAIN
#define _CPLAYERENERGYDRAIN

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/vector.hpp"

class CStateManager;

class CEnergyDrainSource {
public:
  CEnergyDrainSource(TUniqueId src, float intensity) : mSource(src), mIntensity(intensity) {}
  bool operator<(const CEnergyDrainSource& other) const { return mSource < other.mSource; }
  TUniqueId GetEnergyDrainSourceId() const { return mSource; }
  void SetEnergyDrainIntensity(float in) { mIntensity = in; }
  float GetEnergyDrainIntensity() const { return mIntensity; }

private:
  TUniqueId mSource;
  float mIntensity;
};

CHECK_SIZEOF(CEnergyDrainSource, 0x8)

class CPlayerEnergyDrain {
public:
  CPlayerEnergyDrain(uint numSources);

  bool AddEnergyDrainSource(TUniqueId id, float intensity);
  void RemoveEnergyDrainSource(TUniqueId id);
  float GetEnergyDrainIntensity() const;
  const rstl::vector< CEnergyDrainSource >& GetEnergyDrainSources() const { return mSources; }
  float GetEnergyDrainTime() const { return mEnergyDrainTime; }
  void ProcessEnergyDrain(const CStateManager& mgr, float dt);

private:
  rstl::vector< CEnergyDrainSource > mSources;
  float mEnergyDrainTime;
};
CHECK_SIZEOF(CPlayerEnergyDrain, 0x14)

#endif // _CPLAYERENERGYDRAIN
