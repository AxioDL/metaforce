#pragma once

#include <memory>
#include <string_view>

#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CHealthInfo.hpp"
#include "Runtime/World/CPhysicsActor.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCollidableSphere;
class CCollidableOBBTreeGroup;
class CCollidableOBBTreeGroupContainer;

class CCollisionActor : public CPhysicsActor {
  enum class EPrimitiveType { OBBTreeGroup, AABox, Sphere };

  EPrimitiveType x258_primitiveType;
  TUniqueId x25c_owner;
  zeus::CVector3f x260_boxSize;
  zeus::CVector3f x26c_center;
  std::unique_ptr<CCollidableOBBTreeGroupContainer> x278_obbContainer;
  std::unique_ptr<CCollidableOBBTreeGroup> x27c_obbTreeGroupPrimitive;
  std::unique_ptr<CCollidableAABox> x280_aaboxPrimitive;
  std::unique_ptr<CCollidableSphere> x284_spherePrimitive;
  float x288_sphereRadius;
  CHealthInfo x28c_healthInfo = CHealthInfo(0.f);
  CDamageVulnerability x294_damageVuln = CDamageVulnerability::NormalVulnerabilty();
  TUniqueId x2fc_lastTouched = kInvalidUniqueId;
  EWeaponCollisionResponseTypes x300_responseType = EWeaponCollisionResponseTypes::EnemyNormal;
  zeus::CVector3f x304_extendedTouchBounds = zeus::skZero3f;

public:
  DEFINE_ENTITY
  CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner, const zeus::CVector3f& extent,
                  const zeus::CVector3f& center, bool active, float mass, std::string_view name);
  CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner, const zeus::CVector3f& boxSize, bool active,
                  float mass, std::string_view name);
  CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner, bool active, float radius, float mass,
                  std::string_view name);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void DebugDraw();
  CHealthInfo* HealthInfo(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& vec1, const zeus::CVector3f& vec2,
                                                     const CDamageInfo& dInfo) const override;
  void OnScanStateChanged(EScanState state, CStateManager& mgr) override;

  void Touch(CActor& actor, CStateManager& mgr) override;
  zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& vec1, const zeus::CVector3f& vec2,
                                                         const CWeaponMode& mode,
                                                         EProjectileAttrib attribute) const override;
  void SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes type) { x300_responseType = type; }
  zeus::CTransform GetPrimitiveTransform() const override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void SetDamageVulnerability(const CDamageVulnerability& vuln);
  const zeus::CVector3f& GetBoxSize() const { return x260_boxSize; }
  TUniqueId GetOwnerId() const { return x25c_owner; }
  TUniqueId GetLastTouchedObject() const { return x2fc_lastTouched; }
  zeus::CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;
  void SetExtendedTouchBounds(const zeus::CVector3f& boundExt) { x304_extendedTouchBounds = boundExt; }
  void SetSphereRadius(float radius);
  float GetSphereRadius() const { return x288_sphereRadius; }
};
} // namespace metaforce
