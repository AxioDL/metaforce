#pragma once

#include "Runtime/World/CActor.hpp"

namespace urde {

class CEffect : public CActor {
public:
  CEffect(TUniqueId uid, const CEntityInfo& info, bool active, std::string_view name, const zeus::CTransform& xf);

  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override {}
  void Render(const CStateManager&) const override {}
};

} // namespace urde
