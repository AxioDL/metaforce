#include "CCollisionActorManager.hpp"
#include "TCastTo.hpp"
#include "World/CActor.hpp"
#include "CStateManager.hpp"
#include "CMaterialList.hpp"

namespace urde
{

CCollisionActorManager::CCollisionActorManager(CStateManager&, TUniqueId, TAreaId,
                                               const std::vector<CJointCollisionDescription>&, bool)
{
}

void CCollisionActorManager::SetActive(CStateManager&, bool)
{

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

}
