#include "CDestroyableRock.hpp"

namespace urde
{

CDestroyableRock::CDestroyableRock(TUniqueId id, bool active, const std::string& name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& modelData, float mass, const CHealthInfo& health,
                                   const CDamageVulnerability& vulnerability, const CMaterialList& matList, ResId fsm,
                                   const CActorParameters& actParams, const CModelData& modelData2)
: CAi(id, active, name, info, xf, std::move(modelData), modelData.GetBounds(),
      mass, health, vulnerability, matList, fsm, actParams, 0.3f, 0.8f)
{

}

}
