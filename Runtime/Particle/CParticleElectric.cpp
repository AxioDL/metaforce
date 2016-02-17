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
    static Zeus::CTransform dummy;
    return dummy;
}

const Zeus::CVector3f& CParticleElectric::GetTranslation() const
{
    static Zeus::CVector3f dummy;
    return dummy;
}

const Zeus::CTransform& CParticleElectric::GetGlobalOrientation() const
{
    static Zeus::CTransform dummy;
    return dummy;
}

const Zeus::CVector3f& CParticleElectric::GetGlobalTranslation() const
{
    static Zeus::CVector3f dummy;
    return dummy;
}

const Zeus::CVector3f& CParticleElectric::GetGlobalScale() const
{
    static Zeus::CVector3f dummy;
    return dummy;
}

const Zeus::CColor& CParticleElectric::GetModulationColor() const
{
    static Zeus::CColor dummy;
    return dummy;
}

bool CParticleElectric::IsSystemDeletable() const
{
    return false;
}

std::pair<Zeus::CAABox, bool> CParticleElectric::GetBounds() const
{
    return std::make_pair(Zeus::CAABox(), false);
}

u32 CParticleElectric::GetParticleCount() const
{
    return 0;
}

bool CParticleElectric::SystemHasLight() const
{
    return false;
}

CLight CParticleElectric::GetLight() const
{
    return CLight();
}

bool CParticleElectric::GetParticleEmission() const
{
    return false;
}

void CParticleElectric::DestroyParticles()
{
}

}
