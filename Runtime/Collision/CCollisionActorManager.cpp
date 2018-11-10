#include "CCollisionActorManager.hpp"
#include "TCastTo.hpp"
#include "World/CActor.hpp"
#include "CStateManager.hpp"
#include "CMaterialList.hpp"

namespace urde
{

CCollisionActorManager::CCollisionActorManager(CStateManager& mgr, TUniqueId owner, TAreaId area,
                                               const std::vector<CJointCollisionDescription>& descs, bool b1)
    : x0_jointDescriptions(descs)
    , x10_ownerId(owner)
    , x12_(b1)
{
}

void CCollisionActorManager::Destroy(CStateManager& mgr) const
{
    for (const CJointCollisionDescription& desc : x0_jointDescriptions)
        mgr.FreeScriptObject(desc.GetCollisionActorId());

    const_cast<CCollisionActorManager&>(*this).x13_ = true;
}

void CCollisionActorManager::SetActive(CStateManager& mgr, bool active)
{
    for (const CJointCollisionDescription& jDesc : x0_jointDescriptions)
    {
        TCastToPtr<CActor> act(mgr.ObjectById(jDesc.GetCollisionActorId()));

        if (act)
        {
            bool curActive = act->GetActive();
            if (curActive != active)
                act->SetActive(active);

            if (!curActive)
                Update(0.f, mgr, EUpdateOptions::One);
        }
    }
}

void CCollisionActorManager::AddMaterial(CStateManager& mgr, const CMaterialList& list)
{
    for (const CJointCollisionDescription& jDesc : x0_jointDescriptions)
    {
        TCastToPtr<CActor> act(mgr.ObjectById(jDesc.GetCollisionActorId()));

        if (act)
            act->AddMaterial(list);
    }
}

u32 CCollisionActorManager::GetNumCollisionActors() const
{
    return x0_jointDescriptions.size();
}

CJointCollisionDescription CCollisionActorManager::GetCollisionDescFromIndex(u32 idx) const
{
    return x0_jointDescriptions[idx];
}

void CCollisionActorManager::Update(float, CStateManager&, CCollisionActorManager::EUpdateOptions) const
{
}

}
