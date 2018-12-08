#include "CTimeScaleFunctions.hpp"
#include "zeus/Math.hpp"

namespace urde {

std::unique_ptr<IVaryingAnimationTimeScale> IVaryingAnimationTimeScale::Clone() const { return VClone(); }

float CConstantAnimationTimeScale::VTimeScaleIntegral(float lowerLimit, float upperLimit) const {
  return (upperLimit - lowerLimit) * x4_scale;
}

float CConstantAnimationTimeScale::VFindUpperLimit(float lowerLimit, float root) const {
  return (root / x4_scale) + lowerLimit;
}

std::unique_ptr<IVaryingAnimationTimeScale> CConstantAnimationTimeScale::VClone() const {
  return std::make_unique<CConstantAnimationTimeScale>(x4_scale);
}

std::unique_ptr<IVaryingAnimationTimeScale> CConstantAnimationTimeScale::VGetFunctionMirrored(float) const {
  return Clone();
}

CLinearAnimationTimeScale::CLinearAnimationTimeScale(const CCharAnimTime& t1, float y1, const CCharAnimTime& t2,
                                                     float y2) {
  float y2my1 = y2 - y1;
  float t2mt1 = (t2 - t1).GetSeconds();
  x4_desc.x4_slope = y2my1 / t2mt1;
  x4_desc.x8_yIntercept = y1 - y2my1 / t2mt1 * t1.GetSeconds();
  x4_desc.xc_t1 = t1.GetSeconds();
  x4_desc.x10_t2 = t2.GetSeconds();
}

std::unique_ptr<IVaryingAnimationTimeScale>
CLinearAnimationTimeScale::CFunctionDescription::FunctionMirroredAround(float value) const {
  float slope = -x4_slope;
  float t1 = 2.f * value - x10_t2;
  float t2 = 2.f * value - xc_t1;
  float newYInt = x8_yIntercept - x4_slope * 2.f * value;
  float y1 = slope * t1 + newYInt;
  float y2 = slope * t2 + newYInt;
  return std::make_unique<CLinearAnimationTimeScale>(t1, y1, t2, y2);
}

float CLinearAnimationTimeScale::VTimeScaleIntegral(float lowerLimit, float upperLimit) const {
  if (lowerLimit <= upperLimit)
    return TimeScaleIntegralWithSortedLimits(x4_desc, lowerLimit, upperLimit);
  else
    return -TimeScaleIntegralWithSortedLimits(x4_desc, upperLimit, lowerLimit);
}

float CLinearAnimationTimeScale::TimeScaleIntegralWithSortedLimits(const CFunctionDescription& desc, float lowerLimit,
                                                                   float upperLimit) {
  float lowerEval = desc.x4_slope * lowerLimit + desc.x8_yIntercept;
  float upperEval = desc.x4_slope * upperLimit + desc.x8_yIntercept;
  return (upperLimit - lowerLimit) * 0.5f * (lowerEval + upperEval);
}

float CLinearAnimationTimeScale::VFindUpperLimit(float lowerLimit, float root) const {
  return FindUpperLimitFromRoot(x4_desc, lowerLimit, root);
}

float CLinearAnimationTimeScale::FindUpperLimitFromRoot(const CFunctionDescription& desc, float lowerLimit,
                                                        float root) {
  float M = 0.5f * desc.x4_slope;
  float upperLimit = lowerLimit;
  float m = 2.f * M;
  float lowerIntegration = M * lowerLimit * lowerLimit + desc.x8_yIntercept * lowerLimit;
  for (int i = 0; i < 16; ++i) {
    float factor = (M * upperLimit * upperLimit + desc.x8_yIntercept * upperLimit - lowerIntegration - root) /
                   (m * upperLimit + desc.x8_yIntercept);
    upperLimit -= factor;
    if (zeus::close_enough(factor, 0.f))
      return upperLimit;
  }
  return -1.f;
}

std::unique_ptr<IVaryingAnimationTimeScale> CLinearAnimationTimeScale::VClone() const {
  float y1 = x4_desc.x4_slope * x4_desc.xc_t1 + x4_desc.x8_yIntercept;
  float y2 = x4_desc.x4_slope * x4_desc.x10_t2 + x4_desc.x8_yIntercept;
  return std::make_unique<CLinearAnimationTimeScale>(x4_desc.xc_t1, y1, x4_desc.x10_t2, y2);
}

std::unique_ptr<IVaryingAnimationTimeScale> CLinearAnimationTimeScale::VGetFunctionMirrored(float value) const {
  return x4_desc.FunctionMirroredAround(value);
}

} // namespace urde
