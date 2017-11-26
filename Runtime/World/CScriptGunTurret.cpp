#include "CScriptGunTurret.hpp"

namespace urde
{

static const CMaterialList skGunMaterialList = { EMaterialTypes::Solid, EMaterialTypes::Character,
                                                 EMaterialTypes::Orbit, EMaterialTypes::Target };
static const CMaterialList skTurretMaterialList = { EMaterialTypes::Character };

CScriptGunTurret::CScriptGunTurret(TUniqueId uid, std::string_view name, ETurretComponent comp, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabb,
                                   const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                   const CActorParameters& aParms, const CScriptGunTurretData& turretData)
: CPhysicsActor(uid, true, name, info, xf, std::move(mData),
                comp == ETurretComponent::Turret ? skTurretMaterialList : skGunMaterialList,
                aabb, SMoverData(1000.f), aParms, 0.3f, 0.1f)
{

}

}
