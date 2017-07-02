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
    virtual float VTimeScaleIntegral(const float&, const float&) const=0;
    virtual float VFindUpperLimit(const float&, const float&) const=0;
    virtual std::shared_ptr<IVaryingAnimationTimeScale> VClone() const=0;
    virtual std::shared_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float&) const=0;
    std::shared_ptr<IVaryingAnimationTimeScale> Clone() const;
};

class CConstantAnimationTimeScale  : public IVaryingAnimationTimeScale
{
private:
    float x4_;
public:
    CConstantAnimationTimeScale(float f) : x4_(f) {}

    EVaryingAnimationTimeScaleType GetType() const { return EVaryingAnimationTimeScaleType::Constant; }
    float VTimeScaleIntegral(const float &, const float &) const;
    float VFindUpperLimit(const float &, const float &) const;
    std::shared_ptr<IVaryingAnimationTimeScale> VClone() const;
    std::shared_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float &) const;
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
    float VTimeScaleIntegral(const float&, const float&) const;
    float TimeScaleIntegralWithSortedLimits(const CFunctionDescription& desc, const float&, const float&);
    float VFindUpperLimit(const float&, const float&) const;
    float FindUpperLimitFromRoot(const CFunctionDescription& desc, const float&, const float&);
    std::shared_ptr<IVaryingAnimationTimeScale> VClone() const;
    std::shared_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float&) const;
};

}

#endif // __URDE_CTIMESCALEFUNCTIONS_HPP__
