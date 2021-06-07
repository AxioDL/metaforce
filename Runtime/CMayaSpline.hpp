#pragma once

#include <vector>
#include "RetroTypes.hpp"

#include <zeus/CVector2f.hpp>
#include <rstl.hpp>
#include <cmath>
#include <cfloat>

namespace metaforce {
class CMayaSplineKnot {
  float x0_time;
  float x4_amplitude;
  u8 x8_;
  u8 x9_;
  bool xa_24_dirty : 1 = true;
  u8 xb_;
  zeus::CVector2f xc_cachedTangentA;
  zeus::CVector2f x14_cachedTangentB;

public:
  CMayaSplineKnot(CInputStream& in);

  float GetTime() const { return x0_time; }
  float GetAmplitude() const { return x4_amplitude; }
  u8 GetX8() const { return x8_; }
  u8 GetX9() const { return x9_; }
  void GetTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next, zeus::CVector2f& tangentA, zeus::CVector2f& tangentB);
  void CalculateTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next);
};

class CMayaSpline {
  u32 x0_preInfinity;
  u32 x4_postInfinity;
  std::vector<CMayaSplineKnot> x8_knots;
  u32 x18_clampMode;
  float x1c_minAmplitudeTime;
  float x20_maxAmplitudeTime;
  s32 x24_chachedKnotIndex = -1;
  s32 x28_ = -1;
  bool x2c_24_dirty = false;
  float x30_cachedMinTime;
  float x34_cachedHermitCoefs[4];

public:
  CMayaSpline(CInputStream& in, s32 count);
  u32 GetKnotCount() const { return x8_knots.size(); }
  const std::vector<CMayaSplineKnot>& GetKnots() const { return x8_knots; }
  float GetMinTime() const;
  float GetMaxTime() const;
  float GetDuration() const;

  float EvaluateAt(float time);
  float EvaluateAtUnclamped(float time);
  float EvaluateInfinities(float time, bool Pre);
  float EvaluateHermite(float time);
  bool FindKnot(float time, int& knotIndex);
  void FindControlPoints(s32 knotIndex, rstl::reserved_vector<zeus::CVector2f, 4>& controlPoints);
  void CalculateHermiteCoefficients(const rstl::reserved_vector<zeus::CVector2f, 4>& controlPoits, float* coefs);
};
} // namespace metaforce
