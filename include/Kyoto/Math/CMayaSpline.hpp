#ifndef _CMAYASPLINE
#define _CMAYASPLINE

#include "types.h"

#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CMayaSplineKnot {
public:
  // Enum spellings inferred from the tangent calculations.
  enum ETangentType {
    kTT_Linear,
    kTT_Flat,
    kTT_Smooth,
    kTT_Step,
    kTT_Clamped,
    kTT_Fixed
  };

  CMayaSplineKnot(float time, float amplitude, ETangentType inTangentType,
                  ETangentType outTangentType, const CAbsAngle& inAngle = CAbsAngle::FromRadians(0.f),
                  const CAbsAngle& outAngle = CAbsAngle::FromRadians(0.f));

  float GetTime() const { return mTime; }
  float GetAmplitude() const { return mAmplitude; }
  ETangentType GetInTangentType() const { return static_cast< ETangentType >(mInTangentType); }
  ETangentType GetOutTangentType() const { return static_cast< ETangentType >(mOutTangentType); }
  void GetTangents(const CMayaSplineKnot* previous, const CMayaSplineKnot* next,
                   CVector2f& tangentA, CVector2f& tangentB) const;
  bool operator<(const CMayaSplineKnot& other) const { return GetTime() < other.GetTime(); }

private:
  void CalculateTangents(const CMayaSplineKnot* previous, const CMayaSplineKnot* next) const;

  float mTime;
  float mAmplitude;
  mutable uint mInTangentType : 8;
  mutable uint mOutTangentType : 8;
  mutable uint mDirty : 1;
  mutable CVector2f mCachedTangentA;
  mutable CVector2f mCachedTangentB;
};
CHECK_SIZEOF(CMayaSplineKnot, 0x1c)

namespace rstl {
template <>
struct is_trivially_destructible< CMayaSplineKnot > {
  enum { value = true };
};

template <>
inline void construct< CMayaSplineKnot >(void* dest, const CMayaSplineKnot& src) {
  *static_cast< CMayaSplineKnot* >(dest) = src;
}
} // namespace rstl

class CMayaSpline {
public:
  // Enum spellings inferred; values follow the original Maya infinity modes.
  enum EInfinityType { kIT_Constant, kIT_Linear, kIT_Cycle, kIT_CycleRelative, kIT_Oscillate };
  enum EClampMode { kCM_None, kCM_Clamp, kCM_Wrap };

  CMayaSpline();
  CMayaSpline(const CMayaSpline& other);
  CMayaSpline(const rstl::vector< CMayaSplineKnot >& knots, float minAmplitude,
              float maxAmplitude, EClampMode clampMode, EInfinityType preInfinity,
              EInfinityType postInfinity);
  void operator=(const CMayaSpline& other);

  float EvaluateAt(float time) const;
  uint GetKnotCount() const { return mKnots.size(); }
  const rstl::vector< CMayaSplineKnot >& GetKnots() const { return mKnots; }
  float GetMinTime() const { return mKnots.front().GetTime(); }
  float GetMaxTime() const { return mKnots.back().GetTime(); }
  float GetDuration() const { return GetMaxTime() - GetMinTime(); }

  static CMayaSpline BuildLinearSpline(float timeA, float amplitudeA, float timeB,
                                       float amplitudeB);
  // Factory spelling inferred from its callers in CTweakPlayerControl.
  static CMayaSpline BuildSpline(const CMayaSplineKnot* knots, uint count, EClampMode clampMode,
                                 EInfinityType preInfinity, EInfinityType postInfinity,
                                 float minAmplitude, float maxAmplitude);

private:
  float EvaluateHermite(float time) const;
  float EvaluateInfinities(float time, bool preInfinity) const;
  bool FindKnot(float time, int& knotIndex) const;
  float EvaluateAtUnclamped(float time) const;
  void FindControlPoints(int knotIndex, rstl::reserved_vector< CVector2f, 4 >& points) const;
  void CalculateHermiteCoefficients(const rstl::reserved_vector< CVector2f, 4 >& points,
                                     float* coefficients) const;

  EInfinityType mPreInfinity;
  EInfinityType mPostInfinity;
  rstl::vector< CMayaSplineKnot > mKnots;
  EClampMode mClampMode;
  float mMinAmplitude;
  float mMaxAmplitude;
  struct SCache {
    SCache()
    : mKnotIndex(-1), mSegmentIndex(-1), mStep(false), mMinTime(0.f) {}

    int mKnotIndex;
    int mSegmentIndex;
    bool mStep : 1;
    float mMinTime;
    float mHermiteCoefficients[4];
  };
  mutable SCache mCache;
};
CHECK_SIZEOF(CMayaSpline, 0x40)

#endif // _CMAYASPLINE
