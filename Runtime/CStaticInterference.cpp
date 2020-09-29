#include "Runtime/CStaticInterference.hpp"

#include <zeus/Math.hpp>

namespace urde {

CStaticInterference::CStaticInterference(size_t sourceCount) { x0_sources.reserve(sourceCount); }

void CStaticInterference::RemoveSource(TUniqueId id) {
  const auto iter =
      std::find_if(x0_sources.cbegin(), x0_sources.cend(), [id](const auto& src) { return src.x0_id == id; });

  if (iter == x0_sources.cend()) {
    return;
  }

  x0_sources.erase(iter);
}

void CStaticInterference::Update(CStateManager&, float dt) {
  std::vector<CStaticInterferenceSource> newSources;
  newSources.reserve(x0_sources.size());
  for (CStaticInterferenceSource& src : x0_sources) {
    if (src.x8_timeLeft >= 0.f) {
      src.x8_timeLeft -= dt;
      newSources.push_back(src);
    }
  }
  x0_sources = std::move(newSources);
}

float CStaticInterference::GetTotalInterference() const {
  float validAccum = 0.f;
  float invalidAccum = 0.f;
  for (const CStaticInterferenceSource& src : x0_sources) {
    if (src.x0_id == kInvalidUniqueId)
      invalidAccum += src.x4_magnitude;
    else
      validAccum += src.x4_magnitude;
  }
  if (validAccum > 0.80000001f)
    validAccum = 0.80000001f;
  validAccum += invalidAccum;
  if (validAccum > 1.f)
    return 1.f;
  return validAccum;
}

void CStaticInterference::AddSource(TUniqueId id, float magnitude, float duration) {
  magnitude = zeus::clamp(0.f, magnitude, 1.f);
  const auto search = std::find_if(x0_sources.begin(), x0_sources.end(),
                                   [id](const CStaticInterferenceSource& source) { return source.x0_id == id; });
  if (search != x0_sources.cend()) {
    search->x4_magnitude = magnitude;
    search->x8_timeLeft = duration;
    return;
  }

  if (x0_sources.size() < x0_sources.capacity()) {
    x0_sources.push_back({id, magnitude, duration});
  }
}

} // namespace urde
