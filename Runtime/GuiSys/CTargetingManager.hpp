#pragma once

#include "Runtime/GuiSys/CCompoundTargetReticle.hpp"
#include "Runtime/GuiSys/COrbitPointMarker.hpp"

namespace metaforce {
class CStateManager;
class CTargetingManager {
  CCompoundTargetReticle x0_targetReticule;
  COrbitPointMarker x21c_orbitPointMarker;

public:
  explicit CTargetingManager(const CStateManager& stateMgr);
  bool CheckLoadComplete() const;
  void Update(float, const CStateManager& stateMgr);
  void Draw(const CStateManager& stateMgr, bool hideLockon);
  void Touch();
  CCompoundTargetReticle& CompoundTargetReticle() { return x0_targetReticule; }
};
} // namespace metaforce
