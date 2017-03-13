#include "CCollisionActorManager.hpp"

namespace urde
{

CCollisionActorManager::CCollisionActorManager(CStateManager&, TUniqueId, TAreaId,
                                               const std::vector<CJointCollisionDescription>&, bool)
{
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
