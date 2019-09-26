#pragma once

#include <vector>
#include "Runtime/RetroTypes.hpp"

namespace urde {
class CStateManager;

struct CStaticInterferenceSource {
  TUniqueId id;
  float magnitude;
  float timeLeft;
};

class CStaticInterference {
  std::vector<CStaticInterferenceSource> m_sources;

public:
  CStaticInterference(int sourceCount);
  void RemoveSource(TUniqueId id);
  void Update(CStateManager&, float dt);
  float GetTotalInterference() const;
  void AddSource(TUniqueId id, float magnitude, float duration);
};

} // namespace urde
