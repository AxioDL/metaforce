#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CAABox.hpp>

namespace urde {
class CScriptAiJumpPoint : public CActor {
private:
  float xe8_apex;
  zeus::CAABox xec_touchBounds;
  bool x108_24 : 1 = false;
  TUniqueId x10a_occupant = kInvalidUniqueId;
  TUniqueId x10c_currentWaypoint = kInvalidUniqueId;
  TUniqueId x10e_nextWaypoint = kInvalidUniqueId;
  float x110_timeRemaining = 0.f;

public:
  CScriptAiJumpPoint(TUniqueId, std::string_view, const CEntityInfo&, zeus::CTransform&, bool, float);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override {}
  void Render(CStateManager&) override {}
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  bool GetInUse(TUniqueId uid) const;
  TUniqueId GetJumpPoint() const { return x10c_currentWaypoint; }
  TUniqueId GetJumpTarget() const { return x10e_nextWaypoint; }
  float GetJumpApex() const { return xe8_apex; }
};
} // namespace urde
