#pragma once

#include <memory>

#include "Runtime/Character/CCharAnimTime.hpp"

namespace urde {
class IVaryingAnimationTimeScale {
public:
  virtual u32 GetType() const = 0;
  virtual float VTimeScaleIntegral(const float&, const float&) const = 0;
  virtual float VFindUpperLimit(const float&, const float&) const = 0;
  virtual std::unique_ptr<IVaryingAnimationTimeScale> VClone() const = 0;
  virtual std::unique_ptr<IVaryingAnimationTimeScale> VGetFunctionMirrored(const float&) const = 0;
  CCharAnimTime FindUpperLimit(const CCharAnimTime& a, const CCharAnimTime& b) const { return VFindUpperLimit(a, b); }

  CCharAnimTime TimeScaleIntegral(const CCharAnimTime& a, const CCharAnimTime& b) const {
    return VTimeScaleIntegral(a, b);
  }

  std::unique_ptr<IVaryingAnimationTimeScale> Clone() const { return VClone(); }
};
} // namespace urde
