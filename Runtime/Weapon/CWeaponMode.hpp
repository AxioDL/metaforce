#pragma once

#include "WeaponCommon.hpp"

namespace urde {
class CWeaponMode {
  EWeaponType x0_weaponType = EWeaponType::None;
  bool x4_24_charged : 1;
  bool x4_25_comboed : 1;
  bool x4_26_instantKill : 1;

public:
  CWeaponMode() {
    x4_24_charged = false;
    x4_25_comboed = false;
    x4_26_instantKill = false;
  }
  CWeaponMode(EWeaponType type, bool charged = false, bool comboed = false, bool instaKill = false)
  : x0_weaponType(type), x4_24_charged(charged), x4_25_comboed(comboed), x4_26_instantKill(instaKill) {}
  EWeaponType GetType() const { return x0_weaponType; }

  bool IsCharged() const { return x4_24_charged; }
  bool IsComboed() const { return x4_25_comboed; }
  bool IsInstantKill() const { return x4_26_instantKill; }

  static CWeaponMode Invalid() { return CWeaponMode(EWeaponType::None); }
  static CWeaponMode Phazon() { return CWeaponMode(EWeaponType::Phazon); }
  static CWeaponMode Plasma() { return CWeaponMode(EWeaponType::Plasma); }
  static CWeaponMode Wave() { return CWeaponMode(EWeaponType::Wave); }
  static CWeaponMode BoostBall() { return CWeaponMode(EWeaponType::BoostBall); }
  static CWeaponMode Ice() { return CWeaponMode(EWeaponType::Ice); }
  static CWeaponMode Power() { return CWeaponMode(EWeaponType::Power); }
  static CWeaponMode Bomb() { return CWeaponMode(EWeaponType::Bomb); }
  static CWeaponMode PowerBomb() { return CWeaponMode(EWeaponType::PowerBomb); }
};
} // namespace urde
