#pragma once

#include "World/CPhysicsActor.hpp"
#include "World/CHealthInfo.hpp"
#include "World/CDamageVulnerability.hpp"
namespace urde
{
class CCollidableSphere;
class CCollidableOBBTreeGroup;
class CCollidableOBBTreeGroupContainer;
class CCollisionActor : public CPhysicsActor
{
    enum class EPrimitiveType
    {
        OBBTreeGroup,
        AABox,
        Sphere
    };

    EPrimitiveType x258_primitiveType;
    TUniqueId x25c_owner;
    zeus::CVector3f x260_boxSize;
    zeus::CVector3f x26c_;
    std::unique_ptr<CCollidableOBBTreeGroupContainer> x278_obbContainer;
    std::unique_ptr<CCollidableOBBTreeGroup> x27c_obbTreeGroupPrimitive;
    std::unique_ptr<CCollidableAABox> x280_aaboxPrimitive;
    std::unique_ptr<CCollidableSphere> x284_spherePrimitive;
    float x288_sphereRadius;
    CHealthInfo x28c_healthInfo = CHealthInfo(0.f);
    CDamageVulnerability x294_damageVuln = CDamageVulnerability::NormalVulnerabilty();
    TUniqueId x2fc_lastTouched = kInvalidUniqueId;
    EWeaponCollisionResponseTypes x300_responseType = EWeaponCollisionResponseTypes::EnemyNormal;
    zeus::CVector3f x304_extendedTouchBounds = zeus::CVector3f::skZero;
public:
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, const zeus::CVector3f&, const zeus::CVector3f&, bool, float);
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, const zeus::CVector3f&, bool, float);
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, bool, float, float);

    void Accept(IVisitor &visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    CHealthInfo* HealthInfo(CStateManager&);
    const CDamageVulnerability* GetDamageVulnerability() const;
    const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                       const CDamageInfo&) const;
    void OnScanStateChanged(EScanState, CStateManager &);

    void Touch(CActor &, CStateManager &);
    zeus::CVector3f GetOrbitPosition(const CStateManager &) const;
    const CCollisionPrimitive* GetCollisionPrimitive() const;
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                           const CWeaponMode&, EProjectileAttrib) const;
    zeus::CTransform GetPrimitiveTransform() const;
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    void SetDamageVulnerability(const CDamageVulnerability& vuln);
    const zeus::CVector3f& GetBoxSize() const { return x260_boxSize; }
    TUniqueId GetOwnerId() const { return x25c_owner; }
    TUniqueId GetLastTouchedObject() const { return x2fc_lastTouched; }
    zeus::CVector3f GetScanObjectIndicatorPosition(const CStateManager &) const;
    void SetExtendedTouchBounds(const zeus::CVector3f& boundExt) { x304_extendedTouchBounds = boundExt; }
    float GetSphereRadius() const { return x288_sphereRadius; }
};
}

