#include "CWeaponMode.hpp"
#include "DataSpec/DNACommon/Tweaks/ITweakPlayerGun.hpp"

namespace urde
{
CWeaponMode::CWeaponMode(EWeaponType type, bool charged, bool comboed, bool instaKill)
: x0_weaponType(type), x4_24_charged(charged), x4_25_comboed(comboed), x4_26_instantKill(instaKill)
{
}

CWeaponMode CWeaponMode::Invalid() { return CWeaponMode(EWeaponType::None); }

CWeaponMode CWeaponMode::Phazon() { return CWeaponMode(EWeaponType::Phazon); }

CWeaponMode CWeaponMode::Plasma() { return CWeaponMode(EWeaponType::Plasma); }

CWeaponMode CWeaponMode::Wave() { return CWeaponMode(EWeaponType::Wave); }

CWeaponMode CWeaponMode::BoostBall() { return CWeaponMode(EWeaponType::BoostBall); }

CWeaponMode CWeaponMode::Ice() { return CWeaponMode(EWeaponType::Ice); }

CWeaponMode CWeaponMode::Power() { return CWeaponMode(EWeaponType::Power); }

CWeaponMode CWeaponMode::Bomb() { return CWeaponMode(EWeaponType::Bomb); }

CWeaponMode CWeaponMode::PowerBomb() { return CWeaponMode(EWeaponType::PowerBomb); }
}
