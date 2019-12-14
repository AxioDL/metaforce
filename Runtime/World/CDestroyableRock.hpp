#pragma once

#include <string_view>

#include "Runtime/World/CAi.hpp"

namespace urde {

class CDestroyableRock : public CAi {

  float x32c_;
public:
  CDestroyableRock(TUniqueId id, bool active, std::string_view name, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& modelData, float mass, const CHealthInfo& health,
                   const CDamageVulnerability& vulnerability, const CMaterialList& matList, CAssetId fsm,
                   const CActorParameters& actParams, const CModelData& modelData2, s32);

  void Accept(IVisitor& visitor) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type, bool inDeferred,
                 float magnitude) override;

  void Set_x32c(float val) { x32c_ = val; }
};

} // namespace urde
