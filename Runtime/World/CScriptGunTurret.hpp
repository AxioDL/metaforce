#ifndef __URDE_CSCRIPTGUNTURRET_HPP__
#define __URDE_CSCRIPTGUNTURRET_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{

class CScriptGunTurretData
{

};

class CScriptGunTurret : public CPhysicsActor
{
public:
    enum class ETurretComponent
    {
        Turret,
        Gun
    };
private:
public:
    CScriptGunTurret(TUniqueId uid, std::string_view name, ETurretComponent comp, const CEntityInfo& info,
                     const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabb,
                     const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                     const CActorParameters& aParms, const CScriptGunTurretData& turretData);
};

}

#endif // __URDE_CSCRIPTGUNTURRET_HPP__
