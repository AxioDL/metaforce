#ifndef _CTIMESCALEFUNCTIONS
#define _CTIMESCALEFUNCTIONS

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "rstl/ownership_transfer.hpp"

enum EVaryingAnimationTimeScaleType { kVATST_Constant, kVATST_Linear };

class IVaryingAnimationTimeScale {
public:
  virtual EVaryingAnimationTimeScaleType GetType() const = 0;
  virtual float VTimeScaleIntegral(const float& lowerLimit, const float& upperLimit) const = 0;
  virtual float VFindUpperLimit(const float& lowerLimit, const float& root) const = 0;
  virtual rstl::ownership_transfer< IVaryingAnimationTimeScale > VClone() const = 0;
  virtual rstl::ownership_transfer< IVaryingAnimationTimeScale >
  VGetFunctionMirrored(const float& value) const = 0;

  rstl::ownership_transfer< IVaryingAnimationTimeScale > Clone() const { return VClone(); }
  CCharAnimTime TimeScaleIntegral(const CCharAnimTime& lower, const CCharAnimTime& upper) const {
    return CCharAnimTime(VTimeScaleIntegral(lower.GetSeconds(), upper.GetSeconds()));
  }
  CCharAnimTime FindUpperLimit(const CCharAnimTime& lower, const CCharAnimTime& root) const {
    return CCharAnimTime(VFindUpperLimit(lower.GetSeconds(), root.GetSeconds()));
  }
};
CHECK_SIZEOF(IVaryingAnimationTimeScale, 0x4)

class CConstantAnimationTimeScale : public IVaryingAnimationTimeScale {
public:
  EVaryingAnimationTimeScaleType GetType() const override { return kVATST_Constant; }
  float VTimeScaleIntegral(const float& lowerLimit, const float& upperLimit) const override;
  float VFindUpperLimit(const float& lowerLimit, const float& root) const override;
  rstl::ownership_transfer< IVaryingAnimationTimeScale > VClone() const override;
  rstl::ownership_transfer< IVaryingAnimationTimeScale >
  VGetFunctionMirrored(const float& value) const override;

  explicit CConstantAnimationTimeScale(float scale) : x4_scale(scale) {}

private:
  float x4_scale;
};
CHECK_SIZEOF(CConstantAnimationTimeScale, 0x8)

class CLinearAnimationTimeScale : public IVaryingAnimationTimeScale {
public:
  EVaryingAnimationTimeScaleType GetType() const override { return kVATST_Linear; }
  float VTimeScaleIntegral(const float& lowerLimit, const float& upperLimit) const override;
  float VFindUpperLimit(const float& lowerLimit, const float& root) const override;
  rstl::ownership_transfer< IVaryingAnimationTimeScale > VClone() const override;
  rstl::ownership_transfer< IVaryingAnimationTimeScale >
  VGetFunctionMirrored(const float& value) const override;

  CLinearAnimationTimeScale(const CCharAnimTime& t1, float y1, const CCharAnimTime& t2, float y2)
  : x4_desc((y2 - y1) / (t2 - t1).GetSeconds(),
            y1 - (y2 - y1) / (t2 - t1).GetSeconds() * t1.GetSeconds(), t1.GetSeconds(),
            t2.GetSeconds()) {}

private:
  class CFunctionDescription {
  public:
    CFunctionDescription(float slope, const float& yIntercept, const float& t1, const float& t2)
    : x0_slope(slope), x4_yIntercept(yIntercept), x8_t1(t1), xc_t2(t2) {}

    CFunctionDescription FunctionMirroredAround(const float& value) const {
#if VERSION >= VERSION_GM8P_00
      float twiceValue = 2.f * value;
      return CFunctionDescription(-x0_slope, x4_yIntercept - x0_slope * twiceValue,
                                  twiceValue - xc_t2, twiceValue - x8_t1);
#else
      CFunctionDescription result(*this);
      float twiceValue = 2.f * value;
      result.x0_slope = -x0_slope;
      result.x4_yIntercept = x4_yIntercept - x0_slope * twiceValue;
      result.x8_t1 = twiceValue - xc_t2;
      result.xc_t2 = twiceValue - x8_t1;
      return result;
#endif
    }

    float x0_slope;
    float x4_yIntercept;
    float x8_t1;
    float xc_t2;
  };

  static float FindUpperLimitFromRoot(const CFunctionDescription& desc, const float& lowerLimit,
                                      const float& root);
  static float TimeScaleIntegralWithSortedLimits(const CFunctionDescription& desc,
                                                 const float& lowerLimit, const float& upperLimit);
  static float GetScale(const CFunctionDescription& desc, const float& time) {
    return desc.x0_slope * time + desc.x4_yIntercept;
  }
  CFunctionDescription x4_desc;
};
CHECK_SIZEOF(CLinearAnimationTimeScale, 0x14)

#endif // _CTIMESCALEFUNCTIONS
