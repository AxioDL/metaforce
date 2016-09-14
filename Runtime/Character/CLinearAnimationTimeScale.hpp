#ifndef __URDE_CLINEARANIMATIONTIMESCALE_HPP__
#define __URDE_CLINEARANIMATIONTIMESCALE_HPP__

#include "IVaryingAnimationTimeScale.hpp"

namespace urde
{
class CLinearAnimationTimeScale : public IVaryingAnimationTimeScale
{
public:
    class CFunctionDescription
    {
    public:
        CFunctionDescription(const float&, const float&, const float&);
        CFunctionDescription FunctionMirroredAround(const float&) const;
    };

private:
public:
    CLinearAnimationTimeScale(const CCharAnimTime&, float, const CCharAnimTime&, float);
    u32 GetType() const { return 1; }
    float VTimeScaleIntegral(const float &, const float &) const { return 0.f; }
    float VFindUpperLimit(const float &, const float &) const { return 0.f; }
    std::unique_ptr<IVaryingAnimationTimeScale> VClone() { return {}; }
    std::unique_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float &) const { return {}; }
    float TimeScaleIntegralWithSortedLimits(const CFunctionDescription&, const float&, const float&);
    float GetScale(const CFunctionDescription&, const float&);
};
}

#endif // __URDE_CLINEARANIMATIONTIMESCALE_HPP__
