#include "Runtime/Weapon/CBurstFire.hpp"

#include <algorithm>
#include <cmath>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CStateManager.hpp"

#include <zeus/Math.hpp>

namespace urde {
CBurstFire::CBurstFire(const SBurst* const* burstDefs, s32 firstBurstCount) : x10_firstBurstCounter(firstBurstCount) {
  while (*burstDefs) {
    x1c_burstDefs.push_back(*burstDefs);
    ++burstDefs;
  }
}

void CBurstFire::Update(CStateManager& mgr, float dt) {
  x14_24_shouldFire = false;
  if (!x18_curBursts) {
    return;
  }

  x8_timeToNextShot -= dt;
  if (x8_timeToNextShot >= 0.f) {
    return;
  }

  x4_angleIdx += 1;
  if (x18_curBursts->x4_shotAngles[x4_angleIdx] > 0) {
    x14_24_shouldFire = true;
    x8_timeToNextShot = x18_curBursts->x24_timeToNextShot;
    x8_timeToNextShot += (mgr.GetActiveRandom()->Float() - 0.5f) * x18_curBursts->x28_timeToNextShotVariance;
  } else {
    x18_curBursts = nullptr;
  }
}

zeus::CVector3f CBurstFire::GetDistanceCompensatedError(float dist, float maxErrDist) const {
  float xErr = GetMaxXError();
  float zErr = GetMaxZError();
  xErr = std::min(xErr, dist / maxErrDist * xErr);
  zErr = std::min(zErr, dist / maxErrDist * zErr);
  return GetError(xErr, zErr);
}

void CBurstFire::Start(CStateManager& mgr) {
  s32 burstIdx = -1;
  const SBurst* bursts = x1c_burstDefs[x0_burstType];
  if (x10_firstBurstCounter-- > 0) {
    burstIdx = xc_firstBurstIdx < 0 ? 0 : xc_firstBurstIdx;
  } else {
    int random = mgr.GetActiveRandom()->Range(0, 100);
    int advanceAccum = 0;
    do {
      burstIdx += 1;
      s32 advanceWeight = bursts[burstIdx].x0_randomSelectionWeight;
      if (advanceWeight == 0) {
        advanceAccum = 100;
        burstIdx -= 1;
      }
      advanceAccum += advanceWeight;
    } while (random > advanceAccum);
  }
  x18_curBursts = &bursts[burstIdx];
  x4_angleIdx = -1;
  x8_timeToNextShot = 0.f;
  x14_24_shouldFire = false;
}

zeus::CVector3f CBurstFire::GetError(float xMag, float zMag) const {
  if (!x14_24_shouldFire || !x18_curBursts) {
    return {};
  }

  s32 r0 = x18_curBursts->x4_shotAngles[x4_angleIdx];
  if (x14_25_avoidAccuracy && (r0 == 4 || r0 == 12)) {
    r0 =
        x4_angleIdx > 0 ? x18_curBursts->x4_shotAngles[x4_angleIdx - 1] : x18_curBursts->x4_shotAngles[x4_angleIdx + 1];
  }

  if (r0 <= 0) {
    return {};
  }

  const float angle = r0 * zeus::degToRad(-22.5f);
  zeus::CVector3f ret;
  ret.x() = std::cos(angle) * xMag;
  ret.z() = std::sin(angle) * zMag;

  return ret;
}

float CBurstFire::GetMaxXError() const { return g_tweakPlayer->GetPlayerXYHalfExtent() * 3.625f + 0.2f; }

float CBurstFire::GetMaxZError() const { return g_tweakPlayer->GetEyeOffset(); }

} // namespace urde
