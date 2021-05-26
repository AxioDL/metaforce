#pragma once

#include "Runtime/World/CActor.hpp"

namespace metaforce {

class CEffect : public CActor {
public:
  DEFINE_ENTITY
  CEffect(TUniqueId uid, const CEntityInfo& info, bool active, std::string_view name, const zeus::CTransform& xf);

  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override {}
  void Render(CStateManager&) override {}
};

} // namespace metaforce
