#ifndef __RETRO__CTIMEPROVIDER_HPP__
#define __RETRO__CTIMEPROVIDER_HPP__
namespace Retro
{
class CTimeProvider;
extern CTimeProvider* g_TimeProvider;
class CTimeProvider
{
public:
    const float&         x0_currentTime; // in seconds
    bool                 x4_first = true;
    CTimeProvider* x8_lastProvider = nullptr;

    CTimeProvider(const float& time);
    ~CTimeProvider();
};
}

#endif
