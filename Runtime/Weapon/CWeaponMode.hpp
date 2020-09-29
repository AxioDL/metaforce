#pragma once

#include "Runtime/Weapon/WeaponCommon.hpp"

namespace urde {
class CWeaponMode {
  EWeaponType x0_weaponType = EWeaponType::None;
  bool x4_24_charged : 1 = false;
  bool x4_25_comboed : 1 = false;
  bool x4_26_instantKill : 1 = false;

public:
  constexpr CWeaponMode() = default;
  constexpr explicit CWeaponMode(EWeaponType type, bool charged = false, bool comboed = false, bool instaKill = false)
  : x0_weaponType(type), x4_24_charged(charged), x4_25_comboed(comboed), x4_26_instantKill(instaKill) {}
  constexpr EWeaponType GetType() const { return x0_weaponType; }

  constexpr bool IsCharged() const { return x4_24_charged; }
  constexpr bool IsComboed() const { return x4_25_comboed; }
  constexpr bool IsInstantKill() const { return x4_26_instantKill; }

  static constexpr CWeaponMode Invalid() { return CWeaponMode(EWeaponType::None); }
  static constexpr CWeaponMode Phazon() { return CWeaponMode(EWeaponType::Phazon); }
  static constexpr CWeaponMode Plasma() { return CWeaponMode(EWeaponType::Plasma); }
  static constexpr CWeaponMode Wave() { return CWeaponMode(EWeaponType::Wave); }
  static constexpr CWeaponMode BoostBall() { return CWeaponMode(EWeaponType::BoostBall); }
  static constexpr CWeaponMode Ice() { return CWeaponMode(EWeaponType::Ice); }
  static constexpr CWeaponMode Power() { return CWeaponMode(EWeaponType::Power); }
  static constexpr CWeaponMode Bomb() { return CWeaponMode(EWeaponType::Bomb); }
  static constexpr CWeaponMode PowerBomb() { return CWeaponMode(EWeaponType::PowerBomb); }
};
} // namespace urde
