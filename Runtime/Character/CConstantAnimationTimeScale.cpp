#include "CConstantAnimationTimeScale.hpp"

namespace urde
{

float CConstantAnimationTimeScale::VTimeScaleIntegral(const float& a, const float& b) const { return (b - a) * x4_; }
float CConstantAnimationTimeScale::VFindUpperLimit(const float& a, const float& b) const { return (b / x4_) + a; }

std::unique_ptr<IVaryingAnimationTimeScale> CConstantAnimationTimeScale::VClone() const
{
    return std::make_unique<CConstantAnimationTimeScale>(x4_);
}

std::unique_ptr<IVaryingAnimationTimeScale> CConstantAnimationTimeScale::VGetFunctionMirrored(const float&) const
{
    return Clone();
}

}
