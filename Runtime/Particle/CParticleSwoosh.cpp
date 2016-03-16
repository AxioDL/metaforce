#include "CParticleSwoosh.hpp"

namespace urde
{

CParticleSwoosh::CParticleSwoosh(const TToken<CSwooshDescription>& desc, int)
{
}

void CParticleSwoosh::Update(double)
{
}

void CParticleSwoosh::Render()
{
}

void CParticleSwoosh::SetOrientation(const zeus::CTransform&)
{
}

void CParticleSwoosh::SetTranslation(const zeus::CVector3f&)
{
}

void CParticleSwoosh::SetGlobalOrientation(const zeus::CTransform&)
{
}

void CParticleSwoosh::SetGlobalTranslation(const zeus::CVector3f&)
{
}

void CParticleSwoosh::SetGlobalScale(const zeus::CVector3f&)
{
}

void CParticleSwoosh::SetLocalScale(const zeus::CVector3f&)
{
}

void CParticleSwoosh::SetParticleEmission(bool)
{
}

void CParticleSwoosh::SetModulationColor(const zeus::CColor&)
{
}

const zeus::CTransform& CParticleSwoosh::GetOrientation() const
{
    static zeus::CTransform dummy;
    return dummy;
}

const zeus::CVector3f& CParticleSwoosh::GetTranslation() const
{
    static zeus::CVector3f dummy;
    return dummy;
}

const zeus::CTransform& CParticleSwoosh::GetGlobalOrientation() const
{
    static zeus::CTransform dummy;
    return dummy;
}

const zeus::CVector3f& CParticleSwoosh::GetGlobalTranslation() const
{
    static zeus::CVector3f dummy;
    return dummy;
}

const zeus::CVector3f& CParticleSwoosh::GetGlobalScale() const
{
    static zeus::CVector3f dummy;
    return dummy;
}

const zeus::CColor& CParticleSwoosh::GetModulationColor() const
{
    static zeus::CColor dummy;
    return dummy;
}

bool CParticleSwoosh::IsSystemDeletable() const
{
    return false;
}

std::pair<zeus::CAABox, bool> CParticleSwoosh::GetBounds() const
{
    return std::make_pair(zeus::CAABox(), false);
}

u32 CParticleSwoosh::GetParticleCount() const
{
    return 0;
}

bool CParticleSwoosh::SystemHasLight() const
{
    return false;
}

CLight CParticleSwoosh::GetLight() const
{
    return CLight(zeus::CVector3f::skZero, zeus::CVector3f::skZero,
                  zeus::CColor::skBlack, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f);
}

bool CParticleSwoosh::GetParticleEmission() const
{
    return false;
}

void CParticleSwoosh::DestroyParticles()
{
}

}
