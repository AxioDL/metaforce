#include "Runtime/World/CPlayerEnergyDrain.hpp"

#include <numeric>

#include "Runtime/CStateManager.hpp"

namespace metaforce {

CPlayerEnergyDrain::CPlayerEnergyDrain(u32 numSources) { x0_sources.reserve(numSources); }

void CPlayerEnergyDrain::AddEnergyDrainSource(TUniqueId id, float intensity) { x0_sources.emplace_back(id, intensity); }

void CPlayerEnergyDrain::RemoveEnergyDrainSource(TUniqueId id) {
  auto it = rstl::binary_find(x0_sources.begin(), x0_sources.end(), id,
                              [](const CEnergyDrainSource& item) { return item.GetEnergyDrainSourceId(); });
  if (it != x0_sources.end())
    x0_sources.erase(it);
}

float CPlayerEnergyDrain::GetEnergyDrainIntensity() const {
  return std::accumulate(x0_sources.cbegin(), x0_sources.cend(), 0.0f,
                         [](float value, const auto& src) { return value + src.GetEnergyDrainIntensity(); });
}

void CPlayerEnergyDrain::ProcessEnergyDrain(const CStateManager& mgr, float dt) {
  auto it = x0_sources.begin();

  for (; it != x0_sources.end(); ++it) {
    if (mgr.GetObjectById((*it).GetEnergyDrainSourceId()) == nullptr)
      RemoveEnergyDrainSource((*it).GetEnergyDrainSourceId());
  }

  if (x0_sources.empty())
    x10_energyDrainTime = 0.f;
  else
    x10_energyDrainTime += dt;
}
} // namespace metaforce
