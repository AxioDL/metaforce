#include "CTimeProvider.hpp"
#if 0
#include "CGraphics.hpp"
#endif

namespace Retro
{
CTimeProvider* g_TimeProvider = nullptr;
CTimeProvider::CTimeProvider(const float& time)
    : x0_currentTime(time)
{
    x8_lastProvider = g_TimeProvider;

    if (x8_lastProvider != nullptr)
        x8_lastProvider->x4_first = false;

#if 0
    CGraphics::SetExternalTimeProvider(this);
#endif
}

CTimeProvider::~CTimeProvider()
{
    g_TimeProvider = x8_lastProvider;
    if (g_TimeProvider)
        g_TimeProvider->x4_first = true;
#if 0
    CGraphics::SetExternalTimeProvider(g_TimeProvider);
#endif
}

}
