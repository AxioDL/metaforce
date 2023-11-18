#include "Runtime/Weapon/CTargetableProjectile.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CTargetableProjectile::CTargetableProjectile(
    const TToken<CWeaponDescription>& desc, EWeaponType type, const zeus::CTransform& xf, EMaterialTypes materials,
    const CDamageInfo& damage, const CDamageInfo& damage2, TUniqueId uid, TAreaId aid, TUniqueId owner,
    const TLockedToken<CWeaponDescription>& weapDesc, TUniqueId homingTarget, EProjectileAttrib attribs,
    const std::optional<TLockedToken<CGenDescription>>& visorParticle, u16 visorSfx, bool sendCollideMsg)
: CEnergyProjectile(true, desc, type, xf, materials, damage, uid, aid, owner, homingTarget,
                    attribs | EProjectileAttrib::BigProjectile | EProjectileAttrib::PartialCharge |
                        EProjectileAttrib::PlasmaProjectile,
                    false, zeus::skOne3f, visorParticle, visorSfx, sendCollideMsg)
, x3d8_weaponDesc(weapDesc)
, x3e0_damage(damage2) {
  x68_material.Add(EMaterialTypes::Target);
  x68_material.Add(EMaterialTypes::Orbit);
}

void CTargetableProjectile::Accept(IVisitor& visitor) { visitor.Visit(this); }

zeus::CVector3f CTargetableProjectile::GetAimPosition(const CStateManager& mgr, float dt) const {
  static constexpr float tickRecip = 1.f / CProjectileWeapon::GetTickPeriod();
  return (dt * dt * 0.5f * tickRecip * x170_projectile.GetGravity()) +
         (dt * tickRecip * x170_projectile.GetVelocity() + GetTranslation());
}

bool CTargetableProjectile::Explode(const zeus::CVector3f& pos, const zeus::CVector3f& normal,
                                    EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                    const CDamageVulnerability& dVuln, TUniqueId hitActor) {
  bool ret = CEnergyProjectile::Explode(pos, normal, type, mgr, dVuln, hitActor);

  if (x2e4_24_active || x2c4_hitProjectileOwner == kInvalidUniqueId ||
      x2c4_hitProjectileOwner != mgr.GetPlayer().GetUniqueId()) {
    return ret;
  }

  if (TCastToConstPtr<CActor> act = mgr.GetObjectById(xec_ownerId)) {
    TUniqueId uid = mgr.AllocateUniqueId();
    zeus::CTransform xf = zeus::lookAt(x170_projectile.GetTranslation(), act->GetAimPosition(mgr, 0.f), zeus::skUp);
    auto* projectile = new CEnergyProjectile(true, x3d8_weaponDesc, xf0_weaponType, xf, EMaterialTypes::Player,
                                             x3e0_damage, uid, GetAreaIdAlways(), x2c4_hitProjectileOwner, xec_ownerId,
                                             EProjectileAttrib::None, false, zeus::skOne3f, {}, 0xFFFF, false);
    mgr.AddObject(projectile);
    projectile->AddMaterial(EMaterialTypes::Orbit);
    mgr.GetPlayer().ResetAimTargetPrediction(uid);
    mgr.GetPlayer().SetOrbitTargetId(uid, mgr);
    x2c4_hitProjectileOwner = kInvalidUniqueId;
  }

  return ret;
}

void CTargetableProjectile::ResolveCollisionWithActor(const CRayCastResult& res, CActor& act, CStateManager& mgr) {
  zeus::CTransform  xf = zeus::lookAt(GetTranslation(), GetAimPosition(mgr, 0.1f));
  xf.origin = GetTranslation();
  SetTransform(xf);
  CEnergyProjectile::ResolveCollisionWithActor(res, act, mgr);
}

} // namespace metaforce
