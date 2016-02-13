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
}

const Zeus::CVector3f& CParticleSwoosh::GetTranslation() const
{
}

const Zeus::CTransform& CParticleSwoosh::GetGlobalOrientation() const
{
}

const Zeus::CVector3f& CParticleSwoosh::GetGlobalTranslation() const
{
}

const Zeus::CVector3f& CParticleSwoosh::GetGlobalScale() const
{
}

const Zeus::CColor& CParticleSwoosh::GetModulationColor() const
{
}

bool CParticleSwoosh::IsSystemDeletable() const
{
}

std::pair<Zeus::CAABox, bool> CParticleSwoosh::GetBounds() const
{
}

u32 CParticleSwoosh::GetParticleCount() const
{
}

bool CParticleSwoosh::SystemHasLight() const
{
}

CLight CParticleSwoosh::GetLight() const
{
}

bool CParticleSwoosh::GetParticleEmission() const
{
}

void CParticleSwoosh::DestroyParticles()
{
}

}
