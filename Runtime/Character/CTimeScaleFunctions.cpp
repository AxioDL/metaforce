#include "CTimeScaleFunctions.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{

std::shared_ptr<IVaryingAnimationTimeScale> IVaryingAnimationTimeScale::Clone() const
{
    return VClone();
}

float CConstantAnimationTimeScale::VTimeScaleIntegral(const float& a, const float& b) const { return (b - a) * x4_; }
float CConstantAnimationTimeScale::VFindUpperLimit(const float& a, const float& b) const { return (b / x4_) + a; }

std::shared_ptr<IVaryingAnimationTimeScale> CConstantAnimationTimeScale::VClone() const
{
    CConstantAnimationTimeScale* ret = new CConstantAnimationTimeScale(x4_);
    return std::shared_ptr<IVaryingAnimationTimeScale>(ret);
}

std::shared_ptr<IVaryingAnimationTimeScale> CConstantAnimationTimeScale::VGetFunctionMirrored(const float&) const
{
    return Clone();
}
float CLinearAnimationTimeScale::VTimeScaleIntegral(const float&, const float&) const
{
    return 0.f;
}

float CLinearAnimationTimeScale::TimeScaleIntegralWithSortedLimits(const CFunctionDescription& desc,
                                                                  const float&, const float&)
{
    return 0.f;
}

float CLinearAnimationTimeScale::VFindUpperLimit(const float&, const float&) const
{
    return 0.f;
}

float CLinearAnimationTimeScale::FindUpperLimitFromRoot(const CFunctionDescription& desc,
                                                       const float&, const float&)
{
    return 0.f;
}

std::shared_ptr<IVaryingAnimationTimeScale> CLinearAnimationTimeScale::VClone() const
{
    CCharAnimTime timeA(x10_);
    CCharAnimTime timeB(xc_);

    CLinearAnimationTimeScale* ret = new CLinearAnimationTimeScale();
    float f30 = x4_ * xc_ + x8_;
    ret->x4_ = (x4_ * x10_ + x8_ - f30) / timeB;
    ret->x8_ = -((x4_ * x10_ + x8_ - f30) / (timeA - timeB) * timeB - f30);
    ret->xc_ = timeB;
    ret->x10_ = timeA;

    return std::shared_ptr<IVaryingAnimationTimeScale>(ret);
}

std::shared_ptr<IVaryingAnimationTimeScale>
CLinearAnimationTimeScale::VGetFunctionMirrored(const float& parm) const
{
    float f27 = -(x4_ * parm * 2.f - x8_);
    float f31 = -x4_ * parm * 2.f - x10_ + f27;
    CCharAnimTime timeA(2.f * parm - xc_);
    CCharAnimTime timeB(2.f * parm - x10_);

    CLinearAnimationTimeScale* ret = new CLinearAnimationTimeScale();
    ret->x4_ = (-x4_ * 2.f * parm - xc_ + f27 - f31) / (timeA - timeB);
    ret->x8_ = -(((-x4_ * 2.f * parm - xc_ + f27 - f31) / (timeA - timeB)) * timeB - f31);
    ret->xc_ = timeB;
    ret->x10_ = timeA;

    return std::shared_ptr<IVaryingAnimationTimeScale>(ret);
}

}
