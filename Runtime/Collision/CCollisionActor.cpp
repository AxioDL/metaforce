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

void CCollisionActor::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) {}

CHealthInfo* CCollisionActor::HealthInfo() { return &x28c_healthInfo; }

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability() const { return &x294_damageVuln; }

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                    const CDamageInfo&) const
{
    return GetDamageVulnerability();
}

void CCollisionActor::SetDamageVulnerability(const CDamageVulnerability& vuln) { x294_damageVuln = vuln; }
}
