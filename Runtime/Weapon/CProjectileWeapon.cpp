#include "CProjectileWeapon.hpp"

namespace urde
{

u16 CProjectileWeapon::g_GlobalSeed = 99;

CProjectileWeapon::CProjectileWeapon(const TToken<CWeaponDescription>& wDesc, const zeus::CVector3f&, const zeus::CTransform&,
                                     const zeus::CVector3f&, s32)
: x4_weaponDesc(wDesc)
{
}

zeus::CTransform CProjectileWeapon::GetTransform() const
{
    return x14_localToWorldXf * x44_localXf;
}

zeus::CVector3f CProjectileWeapon::GetTranslation() const
{
    return x14_localToWorldXf * (x44_localXf * x8c_ + x80_) + x74_;
}

void CProjectileWeapon::RenderParticles() const
{

}

void CProjectileWeapon::Update(float dt)
{

}

void CProjectileWeapon::UpdateParticleFx()
{

}

void CProjectileWeapon::UpdateChildParticleSystems(float dt)
{

}

void CProjectileWeapon::SetWorldSpaceOrientation(const zeus::CTransform& xf)
{
    x44_localXf = x14_localToWorldXf.inverse() * xf;
}

}
