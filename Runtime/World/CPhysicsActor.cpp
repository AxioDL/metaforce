#include "CPhysicsActor.hpp"

namespace urde
{

CPhysicsActor::CPhysicsActor(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, CModelData&& mData, const CMaterialList& matList,
                             const zeus::CAABox& box, const SMoverData& moverData, const CActorParameters& actorParms,
                             float, float)
    : CActor(uid, active, name, info, xf, std::move(mData), matList, actorParms, kInvalidUniqueId)
{
}

zeus::CAABox CPhysicsActor::GetBoundingBox()
{
    zeus::CAABox ret;
}

}
