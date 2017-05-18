#include "MP1/World/CActorContraption.hpp"
#include "Weapon/CFlameThrower.hpp"
#include "Character/CInt32POINode.hpp"
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

    for (const std::pair<TUniqueId, std::string>& uid : x2e4_children)
    {
        CFlameThrower* act = static_cast<CFlameThrower*>(mgr.ObjectById(uid.first));

        if (act && act->GetActive())
            act->SetTransform(x34_transform * act->GetScaledLocatorTransform(uid.second));
    }
}

void MP1::CActorContraption::DoUserAnimEvent(CStateManager& mgr, CInt32POINode& node, EUserEventType evType)
{
    if (evType == EUserEventType::DamageOff)
    {
        for (const std::pair<TUniqueId, std::string>& uid : x2e4_children)
        {
            CFlameThrower* act = static_cast<CFlameThrower*>(mgr.ObjectById(uid.first));
            if (act && act->GetX400_25())
                act->Reset(mgr, false);
        }
    }
    else if (evType == EUserEventType::DamageOn)
    {
        CFlameThrower* fl = CreateFlameThrower(node.GetLocatorName(), mgr);
        if (fl && fl->GetX400_25())
            fl->Fire(GetTransform(), mgr, false);
    }
    else
        CActor::DoUserAnimEvent(mgr, node, evType);
}

CFlameThrower* MP1::CActorContraption::CreateFlameThrower(const std::string&, CStateManager&)
{
    return nullptr;
}
}
