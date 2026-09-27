#include "Kyoto/Animation/CTimeScaleFunctions.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

#include <math.h>

float CConstantAnimationTimeScale::VTimeScaleIntegral(const float& lowerLimit,
                                                      const float& upperLimit) const {
  return mScale * (upperLimit - lowerLimit);
}

float CConstantAnimationTimeScale::VFindUpperLimit(const float& lowerLimit,
                                                   const float& root) const {
  return lowerLimit + root / mScale;
}

rstl::ownership_transfer< IVaryingAnimationTimeScale > CConstantAnimationTimeScale::VClone() const {
  return rs_new CConstantAnimationTimeScale(mScale);
}

rstl::ownership_transfer< IVaryingAnimationTimeScale >
CConstantAnimationTimeScale::VGetFunctionMirrored(const float& value) const {
  return Clone();
}

float CLinearAnimationTimeScale::TimeScaleIntegralWithSortedLimits(const CFunctionDescription& desc,
                                                                   const float& lowerLimit,
                                                                   const float& upperLimit) {
  return (upperLimit - lowerLimit) *
         (0.5f * (GetScale(desc, lowerLimit) + GetScale(desc, upperLimit)));
}

float CLinearAnimationTimeScale::VTimeScaleIntegral(const float& lowerLimit,
                                                    const float& upperLimit) const {
  if (lowerLimit <= upperLimit) {
    return TimeScaleIntegralWithSortedLimits(mDesc, lowerLimit, upperLimit);
  } else {
    return -TimeScaleIntegralWithSortedLimits(mDesc, upperLimit, lowerLimit);
  }
}

float CLinearAnimationTimeScale::FindUpperLimitFromRoot(const CFunctionDescription& desc,
                                                        const float& lowerLimit,
                                                        const float& root) {
  float halfSlope = 0.5f * desc.mSlope;
  float yIntercept = desc.mYIntercept;
  float upperLimit = lowerLimit;
  float lowerIntegral = halfSlope * lowerLimit * lowerLimit + yIntercept * lowerLimit;
  for (int i = 0; i < 20; ++i) {
    float step =
        (halfSlope * upperLimit * upperLimit + yIntercept * upperLimit - lowerIntegral - root) /
        (2.f * halfSlope * upperLimit + yIntercept);
    upperLimit -= step;
    if (fabs(step) < 0.00001f) {
      return upperLimit;
    }
  }
  return -1.f;
}

float CLinearAnimationTimeScale::VFindUpperLimit(const float& lowerLimit, const float& root) const {
  return FindUpperLimitFromRoot(mDesc, lowerLimit, root);
}

rstl::ownership_transfer< IVaryingAnimationTimeScale > CLinearAnimationTimeScale::VClone() const {
  return rs_new CLinearAnimationTimeScale(
      CCharAnimTime(mDesc.mT1), GetScale(mDesc, mDesc.mT1), CCharAnimTime(mDesc.mT2),
      GetScale(mDesc, mDesc.mT2));
}

rstl::ownership_transfer< IVaryingAnimationTimeScale >
CLinearAnimationTimeScale::VGetFunctionMirrored(const float& value) const {
  const CFunctionDescription mirrored = mDesc.FunctionMirroredAround(value);
  return rs_new CLinearAnimationTimeScale(
      CCharAnimTime(mirrored.mT1), GetScale(mirrored, mirrored.mT1),
      CCharAnimTime(mirrored.mT2), GetScale(mirrored, mirrored.mT2));
}
