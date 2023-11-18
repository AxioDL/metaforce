#pragma once

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CModel.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CStateManager;
class COrbitPointMarker {
  float x0_zOffset;
  bool x4_camRelZPos = true;
  float x8_lagAzimuth = 0.f;
  float xc_azimuth = 0.f;
  zeus::CVector3f x10_lagTargetPos;
  bool x1c_lastFreeOrbit = false;
  float x20_interpTimer = 0.f;
  float x24_curTime = 0.f;
  TLockedToken<CModel> x28_orbitPointModel;
  void ResetInterpolationTimer(float time) { x20_interpTimer = time; }

public:
  COrbitPointMarker();
  bool CheckLoadComplete() const;
  void Update(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr);
};
} // namespace metaforce
