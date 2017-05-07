#include "MP1/World/CActorContraption.hpp"
#include "TCastTo.hpp"

namespace urde
{

MP1::CActorContraption::CActorContraption(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                          const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabox,
                                          const CMaterialList& matList, float f1, float f2, const CHealthInfo& hInfo,
                                          const CDamageVulnerability& dVuln, const CActorParameters& aParams,
                                          ResId part, const CDamageInfo& dInfo, bool active)
: CScriptActor(uid, name, info, xf, std::move(mData), aabox, f1, f2, matList, hInfo, dVuln, aParams, false, active, 0,
               0.f, false, false, false, false)
{
}

void MP1::CActorContraption::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}
}
