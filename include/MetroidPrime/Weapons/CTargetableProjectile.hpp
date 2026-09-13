#ifndef _CTARGETABLEPROJECTILE
#define _CTARGETABLEPROJECTILE

#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

class CTargetableProjectile : public CEnergyProjectile {
public:
  CTargetableProjectile(
      const TToken< CWeaponDescription >& desc, const EWeaponType type, const CTransform4f& xf,
      const EMaterialTypes materials, const CDamageInfo& damage, const CDamageInfo& damage2,
      const TUniqueId uid, const TAreaId aid, const TUniqueId owner,
      const TToken< CWeaponDescription >& weapDesc, const TUniqueId homingTarget,
      const EProjectileAttrib attribs,
      const rstl::optional_object< TLockedToken< CGenDescription > >& visorParticle,
      const ushort visorSfx, const bool sendCollideMsg);

  // CEntity
  ~CTargetableProjectile() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;

  // CACtor
  CVector3f GetAimPosition(const CStateManager&, float) const override;

  // CGameProjectile
  void ResolveCollisionWithActor(const CRayCastResult& res, CActor& act,
                                 CStateManager& mgr) override;

  // CEnergyProjectile
  const bool Explode(const CVector3f& pos, const CVector3f& normal,
               const EWeaponCollisionResponseTypes type, CStateManager& mgr,
               const CDamageVulnerability& dVuln, const TUniqueId hitActor) override;

private:
  TToken< CWeaponDescription > x3d8_weaponDesc;
  CDamageInfo x3e0_damage;
};

#endif // _CTARGETABLEPROJECTILE
