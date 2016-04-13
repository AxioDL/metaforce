#ifndef __URDE_CTIMESCALEFUNCTIONS_HPP__
#define __URDE_CTIMESCALEFUNCTIONS_HPP__

#include "RetroTypes.hpp"

namespace urde
{

enum class EVaryingAnimationTimeScaleType
{
    Constant,
    Linear
};

class IVaryingAnimationTimeScale
{
public:
    virtual ~IVaryingAnimationTimeScale() = default;
    virtual EVaryingAnimationTimeScaleType GetType() const=0;
    virtual void VTimeScaleIntegral(const float&, const float&) const=0;
    virtual void VFindUpperLimit(const float&, const float&) const=0;
    virtual std::shared_ptr<IVaryingAnimationTimeScale> VClone() const=0;
    virtual std::shared_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float&) const=0;
    std::shared_ptr<IVaryingAnimationTimeScale> Clone() const;
};

class CConstantAnimationTimeScale : public IVaryingAnimationTimeScale
{
public:
    EVaryingAnimationTimeScaleType GetType() const {return EVaryingAnimationTimeScaleType::Constant;}
    void VTimeScaleIntegral(const float&, const float&) const;
    void VFindUpperLimit(const float&, const float&) const;
    std::shared_ptr<IVaryingAnimationTimeScale> VClone() const;
    std::shared_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float&) const;
};

class CLinearAnimationTimeScale : public IVaryingAnimationTimeScale
{
    float x4_;
    float x8_;
    float xc_;
    float x10_;
public:
    struct CFunctionDescription
    {
    };

    EVaryingAnimationTimeScaleType GetType() const {return EVaryingAnimationTimeScaleType::Linear;}
    void VTimeScaleIntegral(const float&, const float&) const;
    void TimeScaleIntegralWithSortedLimits(const CFunctionDescription& desc, const float&, const float&);
    void VFindUpperLimit(const float&, const float&) const;
    void FindUpperLimitFromRoot(const CFunctionDescription& desc, const float&, const float&);
    std::shared_ptr<IVaryingAnimationTimeScale> VClone() const;
    std::shared_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float&) const;
};

}

#endif // __URDE_CTIMESCALEFUNCTIONS_HPP__
