#include "CScriptPickup.hpp"

namespace urde
{
CScriptPickup::CScriptPickup(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, CModelData&& mData, const CActorParameters& aParams,
                             const zeus::CAABox& aabb, s32, s32, s32, s32, float, float, float, float, bool active)
    : CPhysicsActor(uid, active, name, info, xf, std::move(mData), CMaterialList(), aabb, SMoverData(1.f), aParams,
                    0.3f, 0.1f)
{
}
}
