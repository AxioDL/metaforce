#include "CWeapon.hpp"
#include "World/CActorParameters.hpp"
#include "World/CScriptWater.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde {

CWeapon::CWeapon(TUniqueId uid, TAreaId aid, bool active, TUniqueId owner, EWeaponType type, std::string_view name,
                 const zeus::CTransform& xf, const CMaterialFilter& filter, const CMaterialList& mList,
                 const CDamageInfo& dInfo, EProjectileAttrib attribs, CModelData&& mData)
: CActor(uid, active, name, CEntityInfo(aid, CEntity::NullConnectionList), xf, std::move(mData), mList,
         CActorParameters::None().HotInThermal(true), kInvalidUniqueId)
, xe8_projectileAttribs(attribs)
, xec_ownerId(owner)
, xf0_weaponType(type)
, xf8_filter(filter)
, x110_origDamageInfo(dInfo)
, x12c_curDamageInfo(dInfo) {}

void CWeapon::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CWeapon::Think(float dt, CStateManager& mgr) {
  x148_curTime += dt;
  if ((xe8_projectileAttribs & EProjectileAttrib::DamageFalloff) == EProjectileAttrib::DamageFalloff) {
    float damMul = std::max(0.f, 1.f - x148_curTime * x14c_damageFalloffSpeed);
    x12c_curDamageInfo.SetDamage(x110_origDamageInfo.GetDamage() * damMul);
    x12c_curDamageInfo.SetRadius(x110_origDamageInfo.GetRadius() * damMul);
    x12c_curDamageInfo.SetKnockBackPower(x110_origDamageInfo.GetKnockBackPower() * damMul);
    x12c_curDamageInfo.SetWeaponMode(x110_origDamageInfo.GetWeaponMode());
    x12c_curDamageInfo.SetNoImmunity(false);
  } else {
    x12c_curDamageInfo = x110_origDamageInfo;
  }
  CEntity::Think(dt, mgr);
}

void CWeapon::Render(const CStateManager&) const {
  // Empty
}

EWeaponCollisionResponseTypes CWeapon::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                const CWeaponMode&, EProjectileAttrib) const {
  return EWeaponCollisionResponseTypes::Projectile;
}

void CWeapon::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  bool doRipple = true;
  float mag = 0.f;
  switch (xf0_weaponType) {
  case EWeaponType::Power:
    mag = 0.1f;
    break;
  case EWeaponType::Ice:
    mag = 0.3f;
    break;
  case EWeaponType::Wave:
    mag = 0.1f;
    break;
  case EWeaponType::Plasma:
    break;
  case EWeaponType::Missile:
    mag = 0.5f;
    break;
  case EWeaponType::Phazon:
    mag = 0.1f;
    break;
  default:
    doRipple = false;
    break;
  }

  if (True(xe8_projectileAttribs & EProjectileAttrib::ComboShot) &&
      state != EFluidState::InFluid)
    mag += 0.5f;
  if (True(xe8_projectileAttribs & EProjectileAttrib::Charged))
    mag += 0.25f;
  if (mag > 1.f)
    mag = 1.f;

  if (doRipple) {
    zeus::CVector3f pos = GetTranslation();
    pos.z() = float(water.GetTriggerBoundsWR().max.z());
    if (True(xe8_projectileAttribs & EProjectileAttrib::ComboShot)) {
      if (!water.CanRippleAtPoint(pos))
        doRipple = false;
    } else if (state == EFluidState::InFluid) {
      doRipple = false;
    }

    if (doRipple) {
      water.GetFluidPlane().AddRipple(mag, x8_uid, pos, water, mgr);
      mgr.GetFluidPlaneManager()->CreateSplash(x8_uid, mgr, water, pos, mag,
                                               state == EFluidState::EnteredFluid || state == EFluidState::LeftFluid);
    }
  }
}

} // namespace urde
