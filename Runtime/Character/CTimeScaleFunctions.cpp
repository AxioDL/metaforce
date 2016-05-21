#include "CTimeScaleFunctions.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{

std::shared_ptr<IVaryingAnimationTimeScale> IVaryingAnimationTimeScale::Clone() const
{
    return VClone();
}

void CConstantAnimationTimeScale::VTimeScaleIntegral(const float&, const float&) const
{
}

void CConstantAnimationTimeScale::VFindUpperLimit(const float&, const float&) const
{
}

std::shared_ptr<IVaryingAnimationTimeScale> CConstantAnimationTimeScale::VClone() const
{
    return {};
}

std::shared_ptr<IVaryingAnimationTimeScale>
CConstantAnimationTimeScale::VGetFunctionMirrored(const float&) const
{
    return {};
}

void CLinearAnimationTimeScale::VTimeScaleIntegral(const float&, const float&) const
{
}

void CLinearAnimationTimeScale::TimeScaleIntegralWithSortedLimits(const CFunctionDescription& desc,
                                                                  const float&, const float&)
{
}

void CLinearAnimationTimeScale::VFindUpperLimit(const float&, const float&) const
{
}

void CLinearAnimationTimeScale::FindUpperLimitFromRoot(const CFunctionDescription& desc,
                                                       const float&, const float&)
{
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
