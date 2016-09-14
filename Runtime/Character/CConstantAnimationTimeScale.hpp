#ifndef __URDE_CONSTANTANIMATIONTIMESCALE_HPP__
#define __URDE_CONSTANTANIMATIONTIMESCALE_HPP__

#include "IVaryingAnimationTimeScale.hpp"

namespace urde
{
class CConstantAnimationTimeScale  : public IVaryingAnimationTimeScale
{
private:
    float x4_;
public:
    CConstantAnimationTimeScale(float f) : x4_(f) {}

    u32 GetType() const { return 0; }
    float VTimeScaleIntegral(const float &, const float &) const;
    float VFindUpperLimit(const float &, const float &) const;
    std::unique_ptr<IVaryingAnimationTimeScale> VClone() const;
    std::unique_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float &) const;
};
}
#endif // __URDE_CONSTANTANIMATIONTIMESCALE_HPP__
