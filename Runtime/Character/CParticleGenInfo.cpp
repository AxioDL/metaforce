#include "CParticleGenInfo.hpp"
#include "Particle/CParticleGen.hpp"

namespace urde
{

CParticleGenInfo::CParticleGenInfo(const SObjectTag& part, int frameCount, const std::string& boneName,
                                   const zeus::CVector3f& scale, CParticleData::EParentedMode parentMode,
                                   int a)
: x4_part(part), xc_seconds(frameCount / 60.f), x10_boneName(boneName), x28_parentMode(parentMode),
  x2c_a(a), x30_particleScale(scale)
{}

static TUniqueId _initializeLight(const std::weak_ptr<CParticleGen>& system,
                                  CStateManager& stateMgr, int lightId)
{
    std::shared_ptr<CParticleGen> systemRef = system.lock();
    if (systemRef->SystemHasLight())
    {

    }
    return kInvalidUniqueId;
}

CParticleGenInfoGeneric::CParticleGenInfoGeneric(const SObjectTag& part,
                                                 const std::weak_ptr<CParticleGen>& system,
                                                 int frameCount, const std::string& boneName,
                                                 const zeus::CVector3f& scale,
                                                 CParticleData::EParentedMode parentMode,
                                                 int a, CStateManager& stateMgr, int lightId)
: CParticleGenInfo(part, frameCount, boneName, scale, parentMode, a), x80_system(system)
{
    if (lightId == -1)
        x84_lightId = kInvalidUniqueId;
    else
        x84_lightId = _initializeLight(system, stateMgr, lightId);
}

void CParticleGenInfoGeneric::AddToRenderer()
{
}

void CParticleGenInfoGeneric::Render()
{
}

void CParticleGenInfoGeneric::Update(float dt, CStateManager& stateMgr)
{
}

void CParticleGenInfoGeneric::SetOrientation(const zeus::CTransform& xf, CStateManager& stateMgr)
{
}

void CParticleGenInfoGeneric::SetTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr)
{
}

void CParticleGenInfoGeneric::SetGlobalOrientation(const zeus::CTransform& xf, CStateManager& stateMgr)
{
}

void CParticleGenInfoGeneric::SetGlobalTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr)
{
}

void CParticleGenInfoGeneric::SetGlobalScale(const zeus::CVector3f& scale)
{
}

void CParticleGenInfoGeneric::SetParticleEmission(bool, CStateManager& stateMgr)
{
}

bool CParticleGenInfoGeneric::IsSystemDeletable() const
{
    return false;
}

zeus::CAABox CParticleGenInfoGeneric::GetBounds() const
{
    return {};
}

bool CParticleGenInfoGeneric::HasActiveParticles() const
{
    return false;
}

void CParticleGenInfoGeneric::DestroyParticles()
{
}

bool CParticleGenInfoGeneric::HasLight() const
{
    return false;
}

TUniqueId CParticleGenInfoGeneric::GetLightId() const
{
    return 0;
}

void CParticleGenInfoGeneric::SetModulationColor(const zeus::CColor& color)
{
}

}
