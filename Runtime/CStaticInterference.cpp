#include "Runtime/CStaticInterference.hpp"

#include <zeus/Math.hpp>

namespace urde {

CStaticInterference::CStaticInterference(int sourceCount) { m_sources.reserve(sourceCount); }

void CStaticInterference::RemoveSource(TUniqueId id) {
  auto iter = std::find_if(m_sources.begin(), m_sources.end(),
                           [id](const CStaticInterferenceSource& src) -> bool { return src.id == id; });
  if (iter != m_sources.end())
    m_sources.erase(iter);
}

void CStaticInterference::Update(CStateManager&, float dt) {
  std::vector<CStaticInterferenceSource> newSources;
  newSources.reserve(m_sources.size());
  for (CStaticInterferenceSource& src : m_sources) {
    if (src.timeLeft >= 0.f) {
      src.timeLeft -= dt;
      newSources.push_back(src);
    }
  }
  m_sources = std::move(newSources);
}

float CStaticInterference::GetTotalInterference() const {
  float validAccum = 0.f;
  float invalidAccum = 0.f;
  for (const CStaticInterferenceSource& src : m_sources) {
    if (src.id == kInvalidUniqueId)
      invalidAccum += src.magnitude;
    else
      validAccum += src.magnitude;
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
  auto search = std::find_if(m_sources.begin(), m_sources.end(),
                             [id](CStaticInterferenceSource& source) { return source.id == id; });
  if (search != m_sources.end()) {
    search->magnitude = magnitude;
    search->timeLeft = duration;
    return;
  }
  if (m_sources.size() < m_sources.capacity())
    m_sources.push_back({id, magnitude, duration});
}

} // namespace urde
