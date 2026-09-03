#pragma once

#include "Runtime/World/CActor.hpp"

namespace metaforce {

class CEffect : public CActor {
public:
  DEFINE_ENTITY
  CEffect(TUniqueId uid, const CEntityInfo& info, bool active, std::string_view name, const zeus::CTransform4f& xf);

  void AddToRenderer(const zeus::CFrustumPlanes&, CStateManager&) override {}
  void Render(CStateManager&) override {}
};

} // namespace metaforce
