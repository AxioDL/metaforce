#pragma once

#include "World/CActor.hpp"

namespace urde {
class CScriptTargetingPoint : public CActor {
private:
  union {
    struct {
      bool xe8_e4_ : 1;
    };
    u8 xe8_dummy = 0;
  };
  TUniqueId xea_;
  float xec_time = 0.f;

public:
  CScriptTargetingPoint(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Render(const CStateManager&) const override {}

  bool GetLocked() const;
};
} // namespace urde
