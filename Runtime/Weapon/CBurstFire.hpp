#pragma once

#include <array>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/rstl.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CStateManager;

struct SBurst {
  s32 x0_randomSelectionWeight;
  std::array<s32, 8> x4_shotAngles;
  float x24_timeToNextShot;
  float x28_timeToNextShotVariance;
};

class CBurstFire {
  s32 x0_burstType = -1;
  s32 x4_angleIdx = -1;
  float x8_timeToNextShot = 0.f;
  s32 xc_firstBurstIdx = 0;
  s32 x10_firstBurstCounter;
  bool x14_24_shouldFire : 1 = false;
  bool x14_25_avoidAccuracy : 1 = false;

  const SBurst* x18_curBursts = nullptr;
  rstl::reserved_vector<const SBurst*, 16> x1c_burstDefs;

public:
  CBurstFire(const SBurst* const* burstDefs, s32 firstBurstCount);

  void SetAvoidAccuracy(bool b) { x14_25_avoidAccuracy = b; }
  void SetBurstType(s32 type) { x0_burstType = type; }
  void SetTimeToNextShot(float t) { x8_timeToNextShot = t; }
  float GetTimeToNextShot() const { return x8_timeToNextShot; }
  s32 GetBurstType() const { return x0_burstType; }
  void Start(CStateManager& mgr);
  void Update(CStateManager& mgr, float dt);
  zeus::CVector3f GetError(float xMag, float zMag) const;
  zeus::CVector3f GetDistanceCompensatedError(float dist, float maxErrDist) const;
  float GetMaxXError() const;
  float GetMaxZError() const;
  void SetFirstBurstIndex(s32 idx) { xc_firstBurstIdx = idx; }
  bool ShouldFire() const { return x14_24_shouldFire; }
  bool IsBurstSet() const { return x18_curBursts != nullptr; }
};
} // namespace urde
