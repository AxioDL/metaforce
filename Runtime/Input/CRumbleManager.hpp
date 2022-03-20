#pragma once

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Input/CRumbleGenerator.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CStateManager;
class CRumbleManager {
  CRumbleGenerator x0_rumbleGenerator;

public:
  bool IsDisabled() const { return x0_rumbleGenerator.IsDisabled(); }
  void SetDisabled(bool disabled) { x0_rumbleGenerator.SetDisabled(disabled); }
  void Update(float dt) { x0_rumbleGenerator.Update(dt); }
  void StopRumble(s16 id) {
    if (id == -1)
      return;
    x0_rumbleGenerator.Stop(id, EIOPort::Player1);
  }
  void HardStopAll() { x0_rumbleGenerator.HardStopAll(); }
  s16 Rumble(CStateManager& mgr, const zeus::CVector3f& pos, ERumbleFxId fx, float dist, ERumblePriority priority);
  s16 Rumble(CStateManager& mgr, ERumbleFxId fx, float gain, ERumblePriority priority);
};
} // namespace metaforce
