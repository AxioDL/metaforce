#include "MP1/World/CActorContraption.hpp"
#include "Weapon/CFlameThrower.hpp"
#include "Weapon/CFlameInfo.hpp"
#include "Character/CInt32POINode.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

MP1::CActorContraption::CActorContraption(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                          const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabox,
                                          const CMaterialList& matList, float mass, float zMomentum, const CHealthInfo& hInfo,
                                          const CDamageVulnerability& dVuln, const CActorParameters& aParams,
                                          CAssetId part, const CDamageInfo& dInfo, bool active)
: CScriptActor(uid, name, info, xf, std::move(mData), aabox, mass, zMomentum, matList, hInfo, dVuln, aParams, false, active, 0,
               1.f, false, false, false, false)
, x300_flameThrowerGen(g_SimplePool->GetObj("FlameThrower"))
, x308_flameFxId(part)
, x30c_dInfo(dInfo)
{
}

void MP1::CActorContraption::Accept(IVisitor& visitor) { visitor.Visit(this); }

void MP1::CActorContraption::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    bool curActive = GetActive();
    if (msg == EScriptObjectMessage::Registered)
        AddMaterial(EMaterialTypes::ScanPassthrough, mgr);
    else if (msg == EScriptObjectMessage::SetToZero)
        ResetFlameThrowers(mgr);

    CScriptActor::AcceptScriptMsg(msg, uid, mgr);
    if (curActive == GetActive() || !GetActive())
        return;

    ResetFlameThrowers(mgr);
}

void MP1::CActorContraption::Think(float dt, CStateManager& mgr)
{
    CScriptActor::Think(dt, mgr);

    for (const std::pair<TUniqueId, std::string>& uid : x2e8_children)
    {
        CFlameThrower* act = static_cast<CFlameThrower*>(mgr.ObjectById(uid.first));

        if (act && act->GetActive())
            act->SetTransform(x34_transform * act->GetScaledLocatorTransform(uid.second));
    }
}

void MP1::CActorContraption::ResetFlameThrowers(CStateManager& mgr)
{
    for (const std::pair<TUniqueId, std::string>& uid : x2e8_children)
    {
        CFlameThrower* act = static_cast<CFlameThrower*>(mgr.ObjectById(uid.first));
        if (act && !act->GetX400_25())
            act->Reset(mgr, false);
    }
}

void MP1::CActorContraption::DoUserAnimEvent(CStateManager& mgr, CInt32POINode& node, EUserEventType evType, float dt)
{
    if (evType == EUserEventType::DamageOff)
    {
        ResetFlameThrowers(mgr);
    }
    else if (evType == EUserEventType::DamageOn)
    {
        CFlameThrower* fl = CreateFlameThrower(node.GetLocatorName(), mgr);
        if (fl && fl->GetX400_25())
            fl->Fire(GetTransform(), mgr, false);
    }
    else
        CActor::DoUserAnimEvent(mgr, node, evType, dt);
}

CFlameThrower* MP1::CActorContraption::CreateFlameThrower(const std::string& name, CStateManager& mgr)
{
    const auto& it = std::find_if(x2e8_children.begin(), x2e8_children.end(),
                           [&name](const std::pair<TUniqueId, std::string>& p) { return p.second == name; });

    if (it == x2e8_children.end())
    {
        TUniqueId id = mgr.AllocateUniqueId();
        CFlameInfo flameInfo(6, 6, x308_flameFxId, 20, 0.5f, 1.f, 1.f);
        CFlameThrower* ret = new CFlameThrower(x300_flameThrowerGen, name, EWeaponType::Missile, flameInfo,
                                               zeus::CTransform::Identity(), EMaterialTypes::CollisionActor, x30c_dInfo,
                                               id, GetAreaId(), GetUniqueId(), 0, -1, -1, -1);

        x2e8_children.emplace_back(id, name);

        mgr.AddObject(ret);
        return ret;
    }
    return static_cast<CFlameThrower*>(mgr.ObjectById(it->first));
}
}
