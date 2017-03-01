#ifndef __URDE_CCOLLISIONACTOR_HPP__
#define __URDE_CCOLLISIONACTOR_HPP__

#include "World/CPhysicsActor.hpp"
#include "World/CHealthInfo.hpp"
#include "World/CDamageVulnerability.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"
#include "Collision/CCollidableSphere.hpp"

namespace urde
{
class CCollisionActor : public CPhysicsActor
{
    u32 x258_;
    TUniqueId x25c_;
    zeus::CVector3f x260_boxSize;
    std::unique_ptr<CCollidableOBBTreeGroup> x278_obbTG1;
    std::unique_ptr<CCollidableOBBTreeGroup> x27c_obbTG2;
    std::unique_ptr<CCollidableSphere> x284_;
    float x288_sphereRadius;
    CHealthInfo x28c_healthInfo = CHealthInfo(0.f);
    CDamageVulnerability x294_damageVuln = CDamageVulnerability::NormalVulnerabilty();
    TUniqueId x2fc_lastTouched;
    EWeaponCollisionResponseTypes x300_responseType;
public:
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, const zeus::CVector3f&, const zeus::CVector3f&, bool, float);
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, const zeus::CVector3f&, bool, float);
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, bool, float, float);

    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    CHealthInfo* HealthInfo();
    const CDamageVulnerability* GetDamageVulnerability() const;
    const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                       const CDamageInfo&) const;
    void SetDamageVulnerability(const CDamageVulnerability& vuln);
};
}

#endif // __URDE_CCOLLISIONACTOR_HPP__
