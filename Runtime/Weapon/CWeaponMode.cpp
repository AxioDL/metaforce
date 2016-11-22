#include "CWeaponMode.hpp"

namespace urde
{
CWeaponMode::CWeaponMode(EWeaponType type, bool, bool)
    : x0_weaponType(type)
{
}

CWeaponMode CWeaponMode::Invalid()
{
    return CWeaponMode(EWeaponType::None, false, false);
}

CWeaponMode CWeaponMode::Phazon()
{
    return CWeaponMode(EWeaponType::Phazon, false, false);
}

CWeaponMode CWeaponMode::Plasma()
{
    return CWeaponMode(EWeaponType::Plasma, false, false);
}

CWeaponMode CWeaponMode::Wave()
{
    return CWeaponMode(EWeaponType::Wave, false, false);
}

CWeaponMode CWeaponMode::BoostBall()
{
    return CWeaponMode(EWeaponType::BoostBall, false, false);
}

CWeaponMode CWeaponMode::Ice()
{
    return CWeaponMode(EWeaponType::Ice, false, false);
}

CWeaponMode CWeaponMode::Power()
{
    return CWeaponMode(EWeaponType::Power, false, false);
}

CWeaponMode CWeaponMode::Bomb()
{
    return CWeaponMode(EWeaponType::Bomb, false, false);
}

}
