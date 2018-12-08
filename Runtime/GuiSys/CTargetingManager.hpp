#pragma once

#include "CCompoundTargetReticle.hpp"
#include "COrbitPointMarker.hpp"

namespace urde {
class CStateManager;
class CTargetingManager {
  CCompoundTargetReticle x0_targetReticule;
  COrbitPointMarker x21c_orbitPointMarker;

public:
  CTargetingManager(const CStateManager& stateMgr);
  bool CheckLoadComplete();
  void Update(float, const CStateManager& stateMgr);
  void Draw(const CStateManager& stateMgr, bool hideLockon) const;
  void Touch();
  CCompoundTargetReticle& CompoundTargetReticle() { return x0_targetReticule; }
};
} // namespace urde
