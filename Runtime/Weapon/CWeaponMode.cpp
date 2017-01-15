#include "CWeaponMode.hpp"

namespace urde
{
CWeaponMode::CWeaponMode(EWeaponType type, bool b1, bool b2, bool b3)
    : x0_weaponType(type)
    , x4_24_(b1)
    , x4_25_(b2)
    , x4_26_instantKill(b3)
{
}

EWeaponType CWeaponMode::GetType() const
{
    return x0_weaponType;
}

CWeaponMode CWeaponMode::Invalid()
{
    return CWeaponMode(EWeaponType::None);
}

CWeaponMode CWeaponMode::Phazon()
{
    return CWeaponMode(EWeaponType::Phazon);
}

CWeaponMode CWeaponMode::Plasma()
{
    return CWeaponMode(EWeaponType::Plasma);
}

CWeaponMode CWeaponMode::Wave()
{
    return CWeaponMode(EWeaponType::Wave);
}

CWeaponMode CWeaponMode::BoostBall()
{
    return CWeaponMode(EWeaponType::BoostBall);
}

CWeaponMode CWeaponMode::Ice()
{
    return CWeaponMode(EWeaponType::Ice);
}

CWeaponMode CWeaponMode::Power()
{
    return CWeaponMode(EWeaponType::Power);
}

CWeaponMode CWeaponMode::Bomb()
{
    return CWeaponMode(EWeaponType::Bomb);
}

}
