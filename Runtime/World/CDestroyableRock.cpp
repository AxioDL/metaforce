#include "Runtime/World/CDestroyableRock.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path
namespace urde {

CDestroyableRock::CDestroyableRock(TUniqueId id, bool active, std::string_view name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& modelData, float mass,
                                   const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                                   const CMaterialList& matList, CAssetId fsm, const CActorParameters& actParams,
                                   const CModelData& modelData2, s32)
: CAi(id, active, name, info, xf, std::move(modelData), modelData.GetBounds(), mass, health, vulnerability, matList,
      fsm, actParams, 0.3f, 0.8f) {}

void CDestroyableRock::Accept(urde::IVisitor& visitor) {
  visitor.Visit(this);
}
void CDestroyableRock::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {}
void CDestroyableRock::KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type,
                                 bool inDeferred, float magnitude) {}
} // namespace urde
