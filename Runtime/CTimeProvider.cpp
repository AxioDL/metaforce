#include "CTimeProvider.hpp"
#if 0
#include "CGraphics.hpp"
#endif

namespace pshag
{
CTimeProvider* CTimeProvider::g_currentTimeProvider = nullptr;
CTimeProvider::CTimeProvider(const float& time)
    : x0_currentTime(time), x8_lastProvider(g_currentTimeProvider)
{
    if (x8_lastProvider != nullptr)
        x8_lastProvider->x4_first = false;

    g_currentTimeProvider = this;

#if 0
    CGraphics::SetExternalTimeProvider(this);
#endif
}

CTimeProvider::~CTimeProvider()
{
    g_currentTimeProvider = x8_lastProvider;
    if (g_currentTimeProvider)
        g_currentTimeProvider->x4_first = true;
#if 0
    CGraphics::SetExternalTimeProvider(g_TimeProvider);
#endif
}

}
