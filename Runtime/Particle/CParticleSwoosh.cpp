#include "CParticleSwoosh.hpp"

namespace pshag
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

void CParticleSwoosh::SetOrientation(const Zeus::CTransform&)
{
}

void CParticleSwoosh::SetTranslation(const Zeus::CVector3f&)
{
}

void CParticleSwoosh::SetGlobalOrientation(const Zeus::CTransform&)
{
}

void CParticleSwoosh::SetGlobalTranslation(const Zeus::CVector3f&)
{
}

void CParticleSwoosh::SetGlobalScale(const Zeus::CVector3f&)
{
}

void CParticleSwoosh::SetLocalScale(const Zeus::CVector3f&)
{
}

void CParticleSwoosh::SetParticleEmission(bool)
{
}

void CParticleSwoosh::SetModulationColor(const Zeus::CColor&)
{
}

const Zeus::CTransform& CParticleSwoosh::GetOrientation() const
{
    static Zeus::CTransform dummy;
    return dummy;
}

const Zeus::CVector3f& CParticleSwoosh::GetTranslation() const
{
    static Zeus::CVector3f dummy;
    return dummy;
}

const Zeus::CTransform& CParticleSwoosh::GetGlobalOrientation() const
{
    static Zeus::CTransform dummy;
    return dummy;
}

const Zeus::CVector3f& CParticleSwoosh::GetGlobalTranslation() const
{
    static Zeus::CVector3f dummy;
    return dummy;
}

const Zeus::CVector3f& CParticleSwoosh::GetGlobalScale() const
{
    static Zeus::CVector3f dummy;
    return dummy;
}

const Zeus::CColor& CParticleSwoosh::GetModulationColor() const
{
    static Zeus::CColor dummy;
    return dummy;
}

bool CParticleSwoosh::IsSystemDeletable() const
{
    return false;
}

std::pair<Zeus::CAABox, bool> CParticleSwoosh::GetBounds() const
{
    return std::make_pair(Zeus::CAABox(), false);
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
    return CLight();
}

bool CParticleSwoosh::GetParticleEmission() const
{
    return false;
}

void CParticleSwoosh::DestroyParticles()
{
}

}
