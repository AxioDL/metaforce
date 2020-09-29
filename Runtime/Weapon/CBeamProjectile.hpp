#pragma once

#include "Runtime/rstl.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CBeamProjectile : public CGameProjectile {
public:
  enum class EDamageType {
    None,
    Actor,
    World
  };

private:
  s32 x2e8_intMaxLength;
  float x2ec_maxLength;
  float x2f0_invMaxLength;
  float x2f4_beamRadius;
  EDamageType x2f8_damageType = EDamageType::None;
  TUniqueId x2fc_ = kInvalidUniqueId;
  TUniqueId x2fe_collisionActorId = kInvalidUniqueId;
  float x300_intBeamLength;
  float x304_beamLength;
  float x308_travelSpeed;
  zeus::CVector3f x30c_collisionNormal = zeus::skUp;
  zeus::CVector3f x318_collisionPoint = zeus::skZero3f;
  zeus::CTransform x324_xf;
  zeus::CAABox x354_ = zeus::skNullBox;
  zeus::CAABox x36c_ = zeus::skNullBox;
  rstl::reserved_vector<zeus::CVector3f, 10> x384_;
  rstl::reserved_vector<zeus::CVector3f, 8> x400_pointCache;
  bool x464_24_growingBeam : 1;
  bool x464_25_enableTouchDamage : 1 = false;

  void SetCollisionResultData(EDamageType dType, CRayCastResult& res, TUniqueId id);

public:
  CBeamProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                  const zeus::CTransform& xf, s32 maxLength, float beamRadius, float travelSpeed,
                  EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                  EProjectileAttrib attribs, bool growingBeam);

  void Accept(IVisitor& visitor) override;
  float GetMaxRadius() const { return x2f4_beamRadius; }
  const zeus::CVector3f& GetSurfaceNormal() const { return x30c_collisionNormal; }
  EDamageType GetDamageType() const { return x2f8_damageType; }
  const zeus::CVector3f& GetCurrentPos() const { return x318_collisionPoint; }
  rstl::reserved_vector<zeus::CVector3f, 8>& PointCache() { return x400_pointCache; }
  const rstl::reserved_vector<zeus::CVector3f, 8>& GetPointCache() const { return x400_pointCache; }
  void CauseDamage(bool b) { x464_25_enableTouchDamage = b; }
  const zeus::CTransform& GetBeamTransform() const { return x324_xf; }
  float GetInvMaxLength() const { return x2f0_invMaxLength; }
  float GetCurrentLength() const { return x304_beamLength; }
  float GetMaxLength() const { return x2ec_maxLength; }
  s32 GetIntMaxLength() const { return x2e8_intMaxLength; }
  TUniqueId GetCollisionActorId() const { return x2fe_collisionActorId; }

  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void CalculateRenderBounds() override;
  virtual void ResetBeam(CStateManager&, bool);
  virtual void UpdateFx(const zeus::CTransform&, float, CStateManager&);
  virtual void Fire(const zeus::CTransform&, CStateManager&, bool) = 0;
};
} // namespace urde
