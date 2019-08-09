#pragma once

#include "CActor.hpp"

namespace urde {

class CScriptCameraWaypoint : public CActor {
  float xe8_hfov;
  u32 xec_;

public:
  CScriptCameraWaypoint(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                        bool active, float hfov, u32);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override {}
  void Render(const CStateManager&) const override {}
  TUniqueId GetRandomNextWaypointId(CStateManager& mgr) const;
  float GetHFov() const { return xe8_hfov; }
};

} // namespace urde
