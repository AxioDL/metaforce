#include "CPhysicsActor.hpp"

namespace urde
{

CPhysicsActor::CPhysicsActor(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, const CModelData& mData, const CMaterialList& matList,
                             const zeus::CAABox& box, const SMoverData& moverData, const CActorParameters& actorParms,
                             float, float)
: CActor(uid, active, name, info, xf, mData, matList, actorParms, kInvalidUniqueId)
{
}

}
