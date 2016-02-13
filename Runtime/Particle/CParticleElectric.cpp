#include "CParticleElectric.hpp"

namespace pshag
{

CParticleElectric::CParticleElectric(const TToken<CElectricDescription>& desc)
{
}

void CParticleElectric::Update(double)
{
}

void CParticleElectric::Render()
{
}

void CParticleElectric::SetOrientation(const Zeus::CTransform&)
{
}

void CParticleElectric::SetTranslation(const Zeus::CVector3f&)
{
}

void CParticleElectric::SetGlobalOrientation(const Zeus::CTransform&)
{
}

void CParticleElectric::SetGlobalTranslation(const Zeus::CVector3f&)
{
}

void CParticleElectric::SetGlobalScale(const Zeus::CVector3f&)
{
}

void CParticleElectric::SetLocalScale(const Zeus::CVector3f&)
{
}

void CParticleElectric::SetParticleEmission(bool)
{
}

void CParticleElectric::SetModulationColor(const Zeus::CColor&)
{
}

const Zeus::CTransform& CParticleElectric::GetOrientation() const
{
}

const Zeus::CVector3f& CParticleElectric::GetTranslation() const
{
}

const Zeus::CTransform& CParticleElectric::GetGlobalOrientation() const
{
}

const Zeus::CVector3f& CParticleElectric::GetGlobalTranslation() const
{
}

const Zeus::CVector3f& CParticleElectric::GetGlobalScale() const
{
}

const Zeus::CColor& CParticleElectric::GetModulationColor() const
{
}

bool CParticleElectric::IsSystemDeletable() const
{
}

std::pair<Zeus::CAABox, bool> CParticleElectric::GetBounds() const
{
}

u32 CParticleElectric::GetParticleCount() const
{
}

bool CParticleElectric::SystemHasLight() const
{
}

CLight CParticleElectric::GetLight() const
{
}

bool CParticleElectric::GetParticleEmission() const
{
}

void CParticleElectric::DestroyParticles()
{
}

}
