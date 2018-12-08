#include "CTimeProvider.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde {
CTimeProvider* CTimeProvider::g_currentTimeProvider = nullptr;
CTimeProvider::CTimeProvider(const float& time) : x0_currentTime(time), x8_lastProvider(g_currentTimeProvider) {
  if (x8_lastProvider != nullptr)
    x8_lastProvider->x4_first = false;

  g_currentTimeProvider = this;

  CGraphics::SetExternalTimeProvider(this);
}

CTimeProvider::~CTimeProvider() {
  g_currentTimeProvider = x8_lastProvider;
  if (g_currentTimeProvider)
    g_currentTimeProvider->x4_first = true;
  CGraphics::SetExternalTimeProvider(g_currentTimeProvider);
}

} // namespace urde
