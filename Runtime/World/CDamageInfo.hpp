#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Weapon/CWeaponMgr.hpp"
#include "Runtime/Weapon/CWeaponMode.hpp"

namespace DataSpec {
struct SShotParam;
}

namespace metaforce {
class CDamageVulnerability;
class CDamageInfo {
  CWeaponMode x0_weaponMode;
  float x8_damage = 0.f;
  float xc_radiusDamage = 0.f;
  float x10_radius = 0.f;
  float x14_knockback = 0.f;
  bool x18_24_noImmunity : 1 = false;

public:
  constexpr CDamageInfo() = default;
  explicit CDamageInfo(CInputStream& in) {
    in.ReadLong();
    x0_weaponMode = CWeaponMode(EWeaponType(in.ReadLong()));
    x8_damage = in.ReadFloat();
    xc_radiusDamage = x8_damage;
    x10_radius = in.ReadFloat();
    x14_knockback = in.ReadFloat();
  }
  constexpr CDamageInfo(const CWeaponMode& mode, float damage, float radius, float knockback)
  : x0_weaponMode(mode), x8_damage(damage), xc_radiusDamage(damage), x10_radius(radius), x14_knockback(knockback) {}

  constexpr CDamageInfo(const CDamageInfo&) = default;
  constexpr CDamageInfo& operator=(const CDamageInfo&) = default;

  constexpr CDamageInfo(CDamageInfo&&) = default;
  constexpr CDamageInfo& operator=(CDamageInfo&&) = default;

  CDamageInfo(const CDamageInfo&, float);
  explicit CDamageInfo(const DataSpec::SShotParam& other);
  CDamageInfo& operator=(const DataSpec::SShotParam& other);

  const CWeaponMode& GetWeaponMode() const { return x0_weaponMode; }
  void SetWeaponMode(const CWeaponMode& mode) { x0_weaponMode = mode; }
  float GetRadius() const { return x10_radius; }
  void SetRadius(float r) { x10_radius = r; }
  float GetKnockBackPower() const { return x14_knockback; }
  void SetKnockBackPower(float k) { x14_knockback = k; }
  float GetDamage() const { return x8_damage; }
  void SetDamage(float d) { x8_damage = d; }
  float GetDamage(const CDamageVulnerability& dVuln) const;
  float GetRadiusDamage() const { return xc_radiusDamage; }
  void SetRadiusDamage(float r) { xc_radiusDamage = r; }
  float GetRadiusDamage(const CDamageVulnerability& dVuln) const;
  bool NoImmunity() const { return x18_24_noImmunity; }
  void SetNoImmunity(bool b) { x18_24_noImmunity = b; }
  void MultiplyDamage(float m) {
    x8_damage *= m;
    xc_radiusDamage *= m;
    x14_knockback *= m;
  }
  void MultiplyDamageAndRadius(float m) {
    x8_damage *= m;
    xc_radiusDamage *= m;
    x10_radius *= m;
    x14_knockback *= m;
  }
};
} // namespace metaforce
