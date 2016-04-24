#ifndef __URDE_CCAMERAFILTER_HPP__
#define __URDE_CCAMERAFILTER_HPP__

#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"

namespace urde
{

class CCameraFilterPass
{
public:
    enum class EFilterType
    {
        Zero,
        One
    };
    enum class EFilterShape
    {
        Zero,
        One
    };
private:
public:
    void SetFilter(EFilterType type, EFilterShape shape, float, const zeus::CColor& color, u32) {}
    void DisableFilter(float) {}
};

class CCameraBlurPass
{
};

}

#endif // __URDE_CCAMERAFILTER_HPP__
