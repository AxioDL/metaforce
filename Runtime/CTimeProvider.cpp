#include "Runtime/CTimeProvider.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

namespace metaforce {
static CTimeProvider* s_currentTimeProvider = nullptr;

CTimeProvider::CTimeProvider(const float& time) : x0_currentTime(time), x8_lastProvider(s_currentTimeProvider) {
  if (x8_lastProvider != nullptr) {
    x8_lastProvider->x4_first = false;
  }

  s_currentTimeProvider = this;

  CGraphics::SetExternalTimeProvider(this);
}

CTimeProvider::~CTimeProvider() {
  s_currentTimeProvider = x8_lastProvider;
  if (s_currentTimeProvider != nullptr) {
    s_currentTimeProvider->x4_first = true;
  }
  CGraphics::SetExternalTimeProvider(s_currentTimeProvider);
}

} // namespace metaforce
