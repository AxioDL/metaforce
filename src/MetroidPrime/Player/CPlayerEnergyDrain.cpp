#include "MetroidPrime/Player/CPlayerEnergyDrain.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "rstl/algorithm.hpp"

CPlayerEnergyDrain::CPlayerEnergyDrain(uint numSources) : mEnergyDrainTime(0.f) {
  mSources.reserve(numSources);
}

bool CPlayerEnergyDrain::AddEnergyDrainSource(TUniqueId id, float intensity) {
  const CEnergyDrainSource source(id, intensity);
  AUTO(it, rstl::binary_find(mSources.begin(), mSources.end(), source));
  if (it != mSources.end()) {
    it->SetEnergyDrainIntensity(intensity);
    return true;
  }
  if (mSources.size() < mSources.capacity()) {
    AUTO(insertAt, rstl::lower_bound(mSources.begin(), mSources.end(), source));
    mSources.insert(insertAt, source);
    return true;
  }
  return false;
}

void CPlayerEnergyDrain::RemoveEnergyDrainSource(TUniqueId id) {
  const CEnergyDrainSource source(id, 0.f);
  AUTO(it, rstl::binary_find(mSources.begin(), mSources.end(), source));
  if (it != mSources.end()) {
    mSources.erase(it);
  }
}

float CPlayerEnergyDrain::GetEnergyDrainIntensity() const {
  float intensity = 0.f;
  for (AUTO(it, mSources.begin()); it != mSources.end(); ++it) {
    intensity += it->GetEnergyDrainIntensity();
  }
  return intensity;
}

void CPlayerEnergyDrain::ProcessEnergyDrain(const CStateManager& mgr, float dt) {
  for (AUTO(it, mSources.begin()); it != mSources.end(); ++it) {
    if (!mgr.GetObjectById(it->GetEnergyDrainSourceId())) {
      RemoveEnergyDrainSource(it->GetEnergyDrainSourceId());
    }
  }
  mEnergyDrainTime = mSources.size() > 0 ? mEnergyDrainTime + dt : 0.f;
}
