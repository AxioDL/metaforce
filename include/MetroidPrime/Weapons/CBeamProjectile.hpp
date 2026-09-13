#ifndef _CBEAMPROJECTILE
#define _CBEAMPROJECTILE

#include "types.h"

#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "rstl/reserved_vector.hpp"

class CBeamProjectile : public CGameProjectile {
public:
  enum EDamageType {
    kDT_None,
    kDT_Actor,
    kDT_World,
  };

  CBeamProjectile(const TToken<CWeaponDescription>& wDesc, const rstl::string& name, EWeaponType wType,
                  const CTransform4f& xf, int maxLength, float beamRadius, float travelSpeed,
                  EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                  EProjectileAttrib attribs, bool growingBeam);

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  ~CBeamProjectile() override;

  // CActor
  rstl::optional_object<CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  void CalculateRenderBounds() override;

  // CBeamProjectile
  virtual void UpdateFx(const CTransform4f&, float, CStateManager&);
  virtual void ResetBeam(CStateManager&, bool);
  virtual void Fire(const CTransform4f&, CStateManager&, bool) = 0;
  
  void CauseDamage(const bool b) { x464_25_enableTouchDamage = b; }
  EDamageType GetDamageType() const { return x2f8_damageType; }
  const CVector3f& GetCurrentPos() const { return x318_collisionPoint; }
  const CVector3f& GetSurfaceNormal() const { return x30c_collisionNormal; }
  const CTransform4f& GetBeamTransform() const { return x324_xf; }
  float GetCurrentLength() const { return x304_beamLength; }
  float GetMaxLength() const { return x2ec_maxLength; }
  float GetInvMaxLength() const { return x2f0_invMaxLength; }
  float GetMaxRadius() const { return x2f4_beamRadius; }
  TUniqueId GetCollisionActorId() const { return x2fe_collisionActorId; }
  const rstl::reserved_vector< CVector3f, 8 >& GetPointCache() const { return x400_pointCache; }
  rstl::reserved_vector< CVector3f, 8 >& PointCache() { return x400_pointCache; }

private:
  int x2e8_intMaxLength;
  float x2ec_maxLength;
  float x2f0_invMaxLength;
  float x2f4_beamRadius;
  EDamageType x2f8_damageType;
  TUniqueId x2fc_;
  TUniqueId x2fe_collisionActorId;
  float x300_intBeamLength;
  float x304_beamLength;
  float x308_travelSpeed;
  CVector3f x30c_collisionNormal;
  CVector3f x318_collisionPoint;
  CTransform4f x324_xf;
  CAABox x354_;
  CAABox x36c_;
  rstl::reserved_vector< CVector3f, 10 > x384_;
  rstl::reserved_vector< CVector3f, 8 > x400_pointCache;
  bool x464_24_growingBeam : 1;
  bool x464_25_enableTouchDamage : 1;

  void SetCollisionResultData(EDamageType dType, CRayCastResult& res, TUniqueId id);
};
CHECK_SIZEOF(CBeamProjectile, 0x468)

#endif // _CBEAMPROJECTILE
