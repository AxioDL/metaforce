#include "Kyoto/Math/CMayaSpline.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/functional.hpp"

#include <float.h>
#include <math.h>

CMayaSplineKnot::CMayaSplineKnot(float time, float amplitude, ETangentType inTangentType,
                               ETangentType outTangentType, const CAbsAngle& inAngle,
                               const CAbsAngle& outAngle)
: mTime(time)
, mAmplitude(amplitude)
, mInTangentType(inTangentType)
, mOutTangentType(outTangentType)
, mDirty(true)
, mCachedTangentA(CVector2f(0.f, 0.f))
, mCachedTangentB(CVector2f(0.f, 0.f)) {
  if (inTangentType == kTT_Fixed) {
    mCachedTangentA = CVector2f(3.f * cosf(inAngle.AsRadians()), 3.f * sinf(inAngle.AsRadians()));
  }
  if (outTangentType == kTT_Fixed) {
    mCachedTangentB = CVector2f(3.f * cosf(outAngle.AsRadians()), 3.f * sinf(outAngle.AsRadians()));
  }
}

void CMayaSplineKnot::GetTangents(const CMayaSplineKnot* previous, const CMayaSplineKnot* next,
                                CVector2f& tangentA, CVector2f& tangentB) const {
  if (mDirty) {
    CalculateTangents(previous, next);
  }
  tangentA = mCachedTangentA;
  tangentB = mCachedTangentB;
}

static void ValidateTangent(CVector2f& tangent) {
  if (tangent[0] < 0.f) {
    tangent[0] = 0.f;
  }
  const float magnitude = tangent.Magnitude();
  if (magnitude != 0.f) {
    tangent /= magnitude;
  }
  if (tangent[0] == 0.f && tangent[1] != 0.f) {
    tangent[0] = 0.0001f;
    tangent[1] = 5729578.f * tangent[0] * (tangent[1] < 0.f ? -1.f : 1.f);
  }
}

void CMayaSplineKnot::CalculateTangents(const CMayaSplineKnot* previous,
                                      const CMayaSplineKnot* next) const {
  mDirty = false;
  bool calculateSmooth = false;
  if (mInTangentType == kTT_Clamped && previous != nullptr) {
    const float previousDifference = fabsf(previous->GetAmplitude() - GetAmplitude());
    const float nextDifference = next != nullptr
                                     ? fabsf(next->GetAmplitude() - GetAmplitude())
                                     : previousDifference;
    if (nextDifference <= 0.05f || previousDifference <= 0.05f) {
      mInTangentType = kTT_Flat;
    }
  }

  switch (mInTangentType) {
  case kTT_Linear:
    if (previous == nullptr) {
      mCachedTangentA = CVector2f(1.f, 0.f);
    } else {
      mCachedTangentA = CVector2f(GetTime() - previous->GetTime(), GetAmplitude() - previous->GetAmplitude());
    }
    break;
  case kTT_Flat: {
    const float difference = previous != nullptr ? GetTime() - previous->GetTime()
                            : next != nullptr ? next->GetTime() - GetTime()
                                              : 0.f;
    mCachedTangentA = CVector2f(difference, 0.f);
    break;
  }
  case kTT_Step:
    mCachedTangentA = CVector2f(1.f, 0.f);
    break;
  case kTT_Clamped:
    mInTangentType = kTT_Smooth;
  case kTT_Smooth:
    calculateSmooth = true;
    break;
  }

  if (mOutTangentType == kTT_Clamped && next != nullptr) {
    const float nextDifference = fabsf(next->GetAmplitude() - GetAmplitude());
    const float previousDifference = previous != nullptr
                                     ? fabsf(previous->GetAmplitude() - GetAmplitude())
                                     : nextDifference;
    if (nextDifference <= 0.05f || previousDifference <= 0.05f) {
      mOutTangentType = kTT_Flat;
    }
  }

  switch (mOutTangentType) {
  case kTT_Linear:
    if (next == nullptr) {
      mCachedTangentB = CVector2f(1.f, 0.f);
    } else {
      mCachedTangentB = CVector2f(next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude());
    }
    break;
  case kTT_Flat: {
    const float difference = next != nullptr ? next->GetTime() - GetTime()
                            : previous != nullptr ? GetTime() - previous->GetTime()
                                              : 0.f;
    mCachedTangentB = CVector2f(difference, 0.f);
    break;
  }
  case kTT_Step:
    mCachedTangentB = CVector2f(1.f, 0.f);
    break;
  case kTT_Clamped:
    mOutTangentType = kTT_Smooth;
  case kTT_Smooth:
    calculateSmooth = true;
    break;
  }

  if (calculateSmooth) {
    CVector2f tangentA(0.f, 0.f);
    CVector2f tangentB(0.f, 0.f);
    if (previous == nullptr && next != nullptr) {
      tangentA = tangentB = CVector2f(next->GetTime() - GetTime(),
                                     next->GetAmplitude() - GetAmplitude());
    } else if (previous != nullptr && next == nullptr) {
      tangentA = tangentB = CVector2f(GetTime() - previous->GetTime(),
                                     GetAmplitude() - previous->GetAmplitude());
    } else if (previous != nullptr && next != nullptr) {
      const float timeDifference = next->GetTime() - previous->GetTime();
      const float amplitudeDifference = next->GetAmplitude() - previous->GetAmplitude();
      float slope;
      if (timeDifference < 0.0001f) {
        slope = amplitudeDifference > 0.f ? 5729578.f : -5729578.f;
      } else {
        slope = amplitudeDifference / timeDifference;
      }
      float nextTime = next->GetTime() - GetTime();
      float previousTime = GetTime() - previous->GetTime();
      float nextAmplitude;
      float previousAmplitude;
      if (nextTime < 0.0001f) {
        nextAmplitude = slope;
        nextTime = 0.f;
      } else {
        nextAmplitude = nextTime * slope;
      }
      if (previousTime < 0.0001f) {
        previousAmplitude = slope;
        previousTime = 0.f;
      } else {
        previousAmplitude = previousTime * slope;
      }
      tangentB = CVector2f(previousTime, previousAmplitude);
      tangentA = CVector2f(nextTime, nextAmplitude);
    } else {
      tangentA = CVector2f(1.f, 0.f);
      tangentB = CVector2f(1.f, 0.f);
    }
    if (mInTangentType == kTT_Smooth) {
      mCachedTangentA = tangentA;
    }
    if (mOutTangentType == kTT_Smooth) {
      mCachedTangentB = tangentB;
    }
  }
  ValidateTangent(mCachedTangentA);
  ValidateTangent(mCachedTangentB);
}

CMayaSpline::CMayaSpline(const CMayaSpline& other)
: mPreInfinity(other.mPreInfinity)
, mPostInfinity(other.mPostInfinity)
, mKnots(other.mKnots)
, mClampMode(other.mClampMode)
, mMinAmplitude(other.mMinAmplitude)
, mMaxAmplitude(other.mMaxAmplitude)
, mCache(other.mCache) {}

void CMayaSpline::operator=(const CMayaSpline& other) {
#if NONMATCHING
  if (this == &other) {
    return;
  }
#endif
  this->~CMayaSpline();
  new (this) CMayaSpline(other);
}

CMayaSpline::CMayaSpline(const rstl::vector< CMayaSplineKnot >& knots, float minAmplitude,
                         float maxAmplitude, EClampMode clampMode, EInfinityType preInfinity,
                         EInfinityType postInfinity)
: mPreInfinity(preInfinity)
, mPostInfinity(postInfinity)
, mKnots(knots)
, mClampMode(clampMode)
, mMinAmplitude(minAmplitude)
, mMaxAmplitude(maxAmplitude) {
  rstl::sort(mKnots.begin(), mKnots.end(), rstl::less< CMayaSplineKnot >());
}

CMayaSpline::CMayaSpline()
: mPreInfinity(kIT_Constant)
, mPostInfinity(kIT_Constant)
, mClampMode(kCM_None)
, mMinAmplitude(-FLT_MAX)
, mMaxAmplitude(FLT_MAX) {}

float CMayaSpline::EvaluateHermite(float time) const {
  const float t = time - mCache.mMinTime;
  return mCache.mHermiteCoefficients[3] +
         t * (mCache.mHermiteCoefficients[2] +
              t * (mCache.mHermiteCoefficients[1] + t * mCache.mHermiteCoefficients[0]));
}

float CMayaSpline::EvaluateInfinities(float time, bool preInfinity) const {
  if (mKnots.empty()) {
    return 0.f;
  }
  const int last = mKnots.size() - 1;
  const float startTime = mKnots[0].GetTime();
  const float endTime = mKnots[last].GetTime();
  float duration = endTime - startTime;
  if (CMath::IsEpsilon(duration, 0.f, 1.e-5f)) {
    return mKnots[0].GetAmplitude();
  }

  double cycles;
  float fraction;
  if (time > endTime) {
    fraction = fabsf(static_cast< float >(modf((time - endTime) / duration, &cycles)));
  } else {
    fraction = fabsf(static_cast< float >(modf((time - startTime) / duration, &cycles)));
  }
  duration *= fraction;
  cycles = 1.f + fabsf(static_cast< float >(cycles));

  if (preInfinity) {
    if (mPreInfinity == kIT_Oscillate) {
      fraction = fmod(cycles, 2.0);
      if (!CMath::IsEpsilon(fraction, 0.f, 1.e-5f)) {
        duration = startTime + duration;
      } else {
        duration = endTime - duration;
      }
    } else if (mPreInfinity == kIT_Cycle || mPreInfinity == kIT_CycleRelative) {
      duration = endTime - duration;
    } else if (mPreInfinity == kIT_Linear) {
      time = startTime - time;
      CVector2f tangentA(0.f, 0.f);
      CVector2f tangentB(0.f, 0.f);
      mKnots[0].GetTangents(nullptr, &mKnots[1], tangentA, tangentB);
      float amplitude = mKnots[0].GetAmplitude();
      if (!CMath::IsEpsilon(tangentA.GetX(), 0.f, 1.e-5f)) {
        amplitude -= time * tangentA.GetY() / tangentA.GetX();
      }
      return amplitude;
    }
  } else {
    if (mPostInfinity == kIT_Oscillate) {
      fraction = fmod(cycles, 2.0);
      if (!CMath::IsEpsilon(fraction, 0.f, 1.e-5f)) {
        duration = endTime - duration;
      } else {
        duration = startTime + duration;
      }
    } else if (mPostInfinity == kIT_Cycle || mPostInfinity == kIT_CycleRelative) {
      duration = startTime + duration;
    } else if (mPostInfinity == kIT_Linear) {
      time = time - endTime;
      CVector2f tangentA(0.f, 0.f);
      CVector2f tangentB(0.f, 0.f);
      mKnots[last].GetTangents(last > 0 ? &mKnots[last - 1] : nullptr, nullptr,
                                tangentA, tangentB);
      float amplitude = mKnots[last].GetAmplitude();
      if (!CMath::IsEpsilon(tangentB.GetX(), 0.f, 1.e-5f)) {
        amplitude += time * tangentB.GetY() / tangentB.GetX();
      }
      return amplitude;
    }
  }

  float amplitude = EvaluateAt(duration);
  if (preInfinity && mPreInfinity == kIT_CycleRelative) {
    const float delta = mKnots[last].GetAmplitude() - mKnots[0].GetAmplitude();
    amplitude -= static_cast< float >(cycles) * delta;
  } else if (!preInfinity && mPostInfinity == kIT_CycleRelative) {
    const float delta = mKnots[last].GetAmplitude() - mKnots[0].GetAmplitude();
    amplitude += static_cast< float >(cycles) * delta;
  }
  return amplitude;
}

bool CMayaSpline::FindKnot(float time, int& knotIndex) const {
  knotIndex = 0;
  if (!mKnots.empty()) {
    int low = 0;
    int high = mKnots.size() - 1;
    do {
      const int middle = (low + high) >> 1;
      const float knotTime = mKnots[middle].GetTime();
      if (time < knotTime) {
        high = middle - 1;
      } else if (time > knotTime) {
        low = middle + 1;
      } else {
        knotIndex = middle;
        return true;
      }
    } while (low <= high);
    knotIndex = low;
  }
  return false;
}

float CMayaSpline::EvaluateAt(float time) const {
  const float amplitude = EvaluateAtUnclamped(time);
  switch (mClampMode) {
  case kCM_Clamp:
    return CMath::FastMin(CMath::FastMax(mMinAmplitude, amplitude), mMaxAmplitude);
  case kCM_Wrap: {
    const float range = mMaxAmplitude - mMinAmplitude;
    if (range > 0.f) {
      if (amplitude > FLT_EPSILON + mMaxAmplitude) {
        return amplitude - range * (static_cast< int >((amplitude - mMaxAmplitude) / range) + 1);
      }
      if (amplitude < mMinAmplitude - FLT_EPSILON) {
        return amplitude + range *
                               (abs(static_cast< int >((amplitude - mMinAmplitude) / range)) + 1);
      }
      return amplitude;
    }
    return mMinAmplitude;
  }
  case kCM_None:
    return amplitude;
  default:
    return 0.f;
  }
}

float CMayaSpline::EvaluateAtUnclamped(float time) const {
  if (GetKnots().empty()) {
    return 0.f;
  }
  const int last = GetKnots().size() - 1;
  if (time < GetKnots()[0].GetTime()) {
    if (mPreInfinity == kIT_Constant) {
      return GetKnots()[0].GetAmplitude();
    }
    return EvaluateInfinities(time, true);
  }
  if (time > GetKnots()[last].GetTime()) {
    if (mPostInfinity == kIT_Constant) {
      return GetKnots()[last].GetAmplitude();
    }
    return EvaluateInfinities(time, false);
  }

  int knotIndex = -1;
  bool found = false;
  const int& cached = mCache.mKnotIndex;
  if (cached != -1) {
#if NONMATCHING
    // Use the cached knot to enable the forward shortcut.
    if (cached < last && time > GetKnots()[cached].GetTime()) {
#else
    // The original checks the final knot, making this shortcut unreachable for finite times.
    if (cached < last && time > GetKnots()[last].GetTime()) {
#endif
      const int next = cached + 1;
      if (time == GetKnots()[next].GetTime()) {
#if NONMATCHING
        mCache.mKnotIndex = next;
        return GetKnots()[next].GetAmplitude();
#else
        mCache.mKnotIndex = last;
        return GetKnots()[last].GetAmplitude();
#endif
      }
      if (time < GetKnots()[next].GetTime()) {
        knotIndex = next;
        found = true;
      }
    } else if (cached > 0 && time < GetKnots()[mCache.mKnotIndex].GetTime()) {
      const int previous = cached - 1;
      if (time > GetKnots()[previous].GetTime()) {
        knotIndex = cached;
        found = true;
      }
      if (time == GetKnots()[previous].GetTime()) {
        mCache.mKnotIndex = previous;
        return GetKnots()[mCache.mKnotIndex].GetAmplitude();
      }
    }
  }
  if (!found && FindKnot(time, knotIndex)) {
#if NONMATCHING
    // An exact knot hit has the same amplitude regardless of the preceding segment's mode.
    mCache.mKnotIndex = knotIndex;
    return GetKnots()[knotIndex].GetAmplitude();
#else
    if (knotIndex == 0) {
      mCache.mKnotIndex = knotIndex;
      return GetKnots()[knotIndex].GetAmplitude();
    }
    if (knotIndex == GetKnots().size()) {
      mCache.mKnotIndex = 0;
      return GetKnots()[last].GetAmplitude();
    }
#endif
  }

  const int segment = knotIndex - 1;
  if (mCache.mSegmentIndex != segment) {
    mCache.mKnotIndex = segment;
    mCache.mSegmentIndex = segment;
    if (GetKnots()[segment].GetOutTangentType() == CMayaSplineKnot::kTT_Step) {
      mCache.mStep = true;
    } else {
      mCache.mStep = false;
      rstl::reserved_vector< CVector2f, 4 > points;
      FindControlPoints(segment, points);
      CalculateHermiteCoefficients(points, mCache.mHermiteCoefficients);
      mCache.mMinTime = points[0].GetX();
    }
  }
  if (mCache.mStep) {
#if NONMATCHING
    // Exact knot hits can move the lookup index without changing the cached segment.
    return GetKnots()[mCache.mSegmentIndex].GetAmplitude();
#else
    return GetKnots()[mCache.mKnotIndex].GetAmplitude();
#endif
  }
  return EvaluateHermite(time);
}

CMayaSpline CMayaSpline::BuildLinearSpline(float timeA, float amplitudeA, float timeB,
                                          float amplitudeB) {
  rstl::vector< CMayaSplineKnot > knots;
  knots.reserve(2);
  knots.push_back_unsafe(CMayaSplineKnot(timeA, amplitudeA, CMayaSplineKnot::kTT_Linear,
                                 CMayaSplineKnot::kTT_Linear));
  knots.push_back_unsafe(CMayaSplineKnot(timeB, amplitudeB, CMayaSplineKnot::kTT_Linear,
                                 CMayaSplineKnot::kTT_Linear));
  return CMayaSpline(knots, amplitudeA, amplitudeB, kCM_None, kIT_Constant, kIT_Constant);
}

CMayaSpline CMayaSpline::BuildSpline(const CMayaSplineKnot* knots, uint count, EClampMode clampMode,
                                    EInfinityType preInfinity, EInfinityType postInfinity,
                                    float minAmplitude, float maxAmplitude) {
  rstl::vector< CMayaSplineKnot > knotVector(count);
  for (uint i = 0; i < count; ++i) {
    knotVector.push_back_unsafe(knots[i]);
  }
  return CMayaSpline(knotVector, minAmplitude, maxAmplitude, clampMode, preInfinity, postInfinity);
}

void CMayaSpline::FindControlPoints(int knotIndex,
                                    rstl::reserved_vector< CVector2f, 4 >& points) const {
  const CMayaSplineKnot* knot = &mKnots[knotIndex];
  points.push_back(CVector2f(knot->GetTime(), knot->GetAmplitude()));
  CVector2f tangentA(0.f, 0.f);
  CVector2f tangentB(0.f, 0.f);
  knot->GetTangents(knotIndex - 1 >= 0 ? &mKnots[knotIndex - 1] : nullptr,
                    knotIndex + 1 < mKnots.size() ? &mKnots[knotIndex + 1] : nullptr,
                    tangentA, tangentB);
  points.push_back(points[0] + tangentB * (1.f / 3.f));

  ++knotIndex;
  knot = &mKnots[knotIndex];
  CVector2f tangentC(0.f, 0.f);
  CVector2f tangentD(0.f, 0.f);
  knot->GetTangents(knotIndex - 1 >= 0 ? &mKnots[knotIndex - 1] : nullptr,
                     knotIndex + 1 < mKnots.size() ? &mKnots[knotIndex + 1] : nullptr,
                     tangentC, tangentD);
  const CVector2f end(knot->GetTime(), knot->GetAmplitude());
  points.push_back(end - tangentC * (1.f / 3.f));
  points.push_back(end);
}

void CMayaSpline::CalculateHermiteCoefficients(
    const rstl::reserved_vector< CVector2f, 4 >& points, float* coefficients) const {
  const CVector2f span = points[3] - points[0];
  float slopeA = 5729578.f;
  const CVector2f tangentA = points[1] - points[0];
  if (tangentA.GetX() != 0.f) {
    slopeA = tangentA.GetY() / tangentA.GetX();
  }
  float slopeB = 5729578.f;
  const CVector2f tangentB = points[3] - points[2];
  if (tangentB.GetX() != 0.f) {
    slopeB = tangentB.GetY() / tangentB.GetX();
  }
  const float& dy = span[1];
  const float dx = span.GetX();
  const float invSquare = 1.f / (dx * dx);
  const float scaledA = slopeA * dx;
  const float scaledB = slopeB * dx;
  coefficients[0] = invSquare * (scaledA + scaledB - dy - dy) / dx;
  coefficients[1] = invSquare * (dy + (dy + dy) - scaledA - scaledA - scaledB);
  coefficients[2] = slopeA;
  coefficients[3] = points[0].GetY();
}
