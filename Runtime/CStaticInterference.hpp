#pragma once

#include <vector>
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CStateManager;

struct CStaticInterferenceSource {
  TUniqueId x0_id;
  float x4_magnitude;
  float x8_timeLeft;
};

class CStaticInterference {
  std::vector<CStaticInterferenceSource> x0_sources;

public:
  explicit CStaticInterference(size_t sourceCount);
  void RemoveSource(TUniqueId id);
  void Update(CStateManager&, float dt);
  float GetTotalInterference() const;
  void AddSource(TUniqueId id, float magnitude, float duration);
};

} // namespace metaforce
