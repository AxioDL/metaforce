#include "CCollisionActor.hpp"
#include "World/CActorParameters.hpp"

namespace urde
{
static const CMaterialList gkDefaultCollisionActorMaterials =
    CMaterialList(EMaterialTypes::Solid, EMaterialTypes::CollisionActor, EMaterialTypes::ScanPassthrough,
                  EMaterialTypes::CameraPassthrough);

CCollisionActor::CCollisionActor(TUniqueId uid1, TAreaId aId, TUniqueId uid2, const zeus::CVector3f& vec1,
                                 const zeus::CVector3f& vec2, bool active, float mass)
: CPhysicsActor(uid1, active, "CollisionActor", CEntityInfo(aId, CEntity::NullConnectionList),
                zeus::CTransform::Identity(), CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                zeus::CAABox::skNullBox, SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
{
}

CCollisionActor::CCollisionActor(TUniqueId uid1, TAreaId aId, TUniqueId uid2, const zeus::CVector3f& vec, bool active,
                                 float mass)
: CPhysicsActor(uid1, active, "CollisionActor", CEntityInfo(aId, CEntity::NullConnectionList),
                zeus::CTransform::Identity(), CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                zeus::CAABox::skNullBox, SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
{
}

CCollisionActor::CCollisionActor(TUniqueId uid1, TAreaId aId, TUniqueId uid2, bool active, float, float mass)
: CPhysicsActor(uid1, active, "CollisionActor", CEntityInfo(aId, CEntity::NullConnectionList),
                zeus::CTransform::Identity(), CModelData::CModelDataNull(), gkDefaultCollisionActorMaterials,
                zeus::CAABox::skNullBox, SMoverData(mass), CActorParameters::None(), 0.3f, 0.1f)
{
}
}
