#pragma once

#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CEnergyDrainSource.hpp"

namespace urde {
class CStateManager;
class CPlayerEnergyDrain {
  std::vector<CEnergyDrainSource> x0_sources;
  float x10_energyDrainTime;

public:
  CPlayerEnergyDrain(u32);
  const std::vector<CEnergyDrainSource>& GetEnergyDrainSources() const { return x0_sources; }
  void AddEnergyDrainSource(TUniqueId, float);
  void RemoveEnergyDrainSource(TUniqueId id);
  float GetEnergyDrainIntensity() const;
  float GetEnergyDrainTime() const { return x10_energyDrainTime; }
  void ProcessEnergyDrain(const CStateManager&, float);
};
} // namespace urde
