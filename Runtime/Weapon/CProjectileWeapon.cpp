#include "CProjectileWeapon.hpp"
#include "Particle/CWeaponDescription.hpp"

namespace urde
{

CRandom16 CProjectileWeapon::g_GlobalSeed = 99;

CProjectileWeapon::CProjectileWeapon(const TToken<CWeaponDescription>& wDesc, const zeus::CVector3f&, const zeus::CTransform&,
                                     const zeus::CVector3f&, s32)
: x4_weaponDesc(wDesc)
{
}

}
