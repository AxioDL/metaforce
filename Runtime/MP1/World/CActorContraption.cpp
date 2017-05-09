#include "MP1/World/CActorContraption.hpp"
#include "Weapon/CFlameThrower.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

MP1::CActorContraption::CActorContraption(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                          const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabox,
                                          const CMaterialList& matList, float f1, float f2, const CHealthInfo& hInfo,
                                          const CDamageVulnerability& dVuln, const CActorParameters& aParams,
                                          ResId part, const CDamageInfo& dInfo, bool active)
: CScriptActor(uid, name, info, xf, std::move(mData), aabox, f1, f2, matList, hInfo, dVuln, aParams, false, active, 0,
               1.f, false, false, false, false)
, x300_flameThrowerGen(g_SimplePool->GetObj("FlameThrower"))
, x308_partId(part)
, x30c_dInfo(dInfo)
{
}

void MP1::CActorContraption::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void MP1::CActorContraption::Think(float dt, CStateManager& mgr)
{
    CScriptActor::Think(dt, mgr);

    for (const std::pair<TUniqueId, std::string>& uid : x2ec_children)
    {
        CFlameThrower* act = static_cast<CFlameThrower*>(mgr.ObjectById(uid.first));

        if (act && act->GetActive())
        {
            act->SetTransform(act->GetScaledLocatorTransform(uid.second));
        }
    }
}
}
