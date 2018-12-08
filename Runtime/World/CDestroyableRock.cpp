#include "CDestroyableRock.hpp"

namespace urde {

CDestroyableRock::CDestroyableRock(TUniqueId id, bool active, std::string_view name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& modelData, float mass,
                                   const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                                   const CMaterialList& matList, CAssetId fsm, const CActorParameters& actParams,
                                   const CModelData& modelData2)
: CAi(id, active, name, info, xf, std::move(modelData), modelData.GetBounds(), mass, health, vulnerability, matList,
      fsm, actParams, 0.3f, 0.8f) {}

} // namespace urde
