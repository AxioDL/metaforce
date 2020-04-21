#pragma once

#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"

namespace urde {
class CScriptTargetingPoint : public CActor {
private:
  bool xe8_e4_ : 1 = false;
  TUniqueId xea_;
  float xec_time = 0.f;

public:
  CScriptTargetingPoint(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Render(CStateManager&) override {}

  bool GetLocked() const;
};
} // namespace urde
