#include "CParticleGenInfo.hpp"
#include "Graphics/IRenderer.hpp"
#include "Particle/CParticleGen.hpp"
#include "GameGlobalObjects.hpp"
#include "World/CGameLight.hpp"
#include "CStateManager.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "TCastTo.hpp"

namespace urde
{

CParticleGenInfo::CParticleGenInfo(const SObjectTag& part, int frameCount, std::string_view boneName,
                                   const zeus::CVector3f& scale, CParticleData::EParentedMode parentMode,
                                   int flags, EParticleGenType type)
: x4_part(part)
, xc_seconds(frameCount / 60.f)
, x10_boneName(boneName)
, x28_parentMode(parentMode)
, x2c_flags(flags)
, x30_particleScale(scale)
, x80_type(type)
{
}

static TUniqueId _initializeLight(const std::weak_ptr<CParticleGen>& system, CStateManager& stateMgr, TAreaId areaId,
                                  int lightId)
{
    TUniqueId ret = kInvalidUniqueId;
    std::shared_ptr<CParticleGen> systemRef = system.lock();
    if (systemRef->SystemHasLight())
    {
        ret = stateMgr.AllocateUniqueId();
        stateMgr.AddObject(
            new CGameLight(ret, areaId, false, "ParticleLight",
                           zeus::CTransform(systemRef->GetOrientation().buildMatrix3f(), systemRef->GetTranslation()),
                           kInvalidUniqueId, systemRef->GetLight(), u32(lightId), 0, 0.f));
    }
    return ret;
}

CParticleGenInfoGeneric::CParticleGenInfoGeneric(const SObjectTag& part, const std::weak_ptr<CParticleGen>& system,
                                                 int frameCount, std::string_view boneName,
                                                 const zeus::CVector3f& scale, CParticleData::EParentedMode parentMode,
                                                 int flags, CStateManager& stateMgr, TAreaId areaId, int lightId,
                                                 EParticleGenType state)
: CParticleGenInfo(part, frameCount, boneName, scale, parentMode, flags, state), x84_system(system)
{
    if (lightId == -1)
        x88_lightId = kInvalidUniqueId;
    else
        x88_lightId = _initializeLight(system, stateMgr, areaId, lightId);
}

void CParticleGenInfoGeneric::AddToRenderer() { g_Renderer->AddParticleGen(*x84_system.get()); }

void CParticleGenInfoGeneric::Render() { x84_system->Render(); }

void CParticleGenInfoGeneric::Update(float dt, CStateManager& stateMgr)
{
    x84_system->Update(dt);

    if (x88_lightId != kInvalidUniqueId)
    {
        TCastToPtr<CGameLight> gl(stateMgr.ObjectById(x88_lightId));
        if (gl)
            gl->SetLight(x84_system->GetLight());
    }
}

void CParticleGenInfoGeneric::SetOrientation(const zeus::CTransform& xf, CStateManager& stateMgr)
{
    x84_system->SetOrientation(xf);

    if (x88_lightId != kInvalidUniqueId)
    {
        TCastToPtr<CGameLight> gl(stateMgr.ObjectById(x88_lightId));
        if (gl)
            gl->SetRotation(zeus::CQuaternion(xf.buildMatrix3f()));
    }
}

void CParticleGenInfoGeneric::SetTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr)
{
    x84_system->SetTranslation(trans);

    if (x88_lightId != kInvalidUniqueId)
    {
        TCastToPtr<CGameLight> gl(stateMgr.ObjectById(x88_lightId));
        if (gl)
            gl->SetTranslation(trans);
    }
}

void CParticleGenInfoGeneric::SetGlobalOrientation(const zeus::CTransform& xf, CStateManager& stateMgr)
{
    x84_system->SetGlobalOrientation(xf);

    if (x88_lightId != kInvalidUniqueId)
    {
        TCastToPtr<CGameLight> gl(stateMgr.ObjectById(x88_lightId));
        if (gl)
            gl->SetRotation(zeus::CQuaternion(xf.buildMatrix3f()));
    }
}

void CParticleGenInfoGeneric::SetGlobalTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr)
{
    x84_system->SetGlobalTranslation(trans);

    if (x88_lightId != kInvalidUniqueId)
    {
        TCastToPtr<CGameLight> gl(stateMgr.ObjectById(x88_lightId));
        if (gl)
            gl->SetTranslation(trans);
    }
}

void CParticleGenInfoGeneric::SetGlobalScale(const zeus::CVector3f& scale) { x84_system->SetGlobalScale(scale); }

void CParticleGenInfoGeneric::SetParticleEmission(bool emission, CStateManager& stateMgr)
{
    x84_system->SetParticleEmission(emission);

    TCastToPtr<CGameLight> gl(stateMgr.ObjectById(x88_lightId));

    if (gl)
        gl->SetActive(emission);
}

bool CParticleGenInfoGeneric::IsSystemDeletable() const { return x84_system->IsSystemDeletable(); }

std::experimental::optional<zeus::CAABox> CParticleGenInfoGeneric::GetBounds() const { return x84_system->GetBounds(); }

bool CParticleGenInfoGeneric::HasActiveParticles() const { return x84_system->GetParticleCount() != 0; }

void CParticleGenInfoGeneric::DestroyParticles() { x84_system->DestroyParticles(); }

bool CParticleGenInfoGeneric::HasLight() const { return x84_system->SystemHasLight(); }

TUniqueId CParticleGenInfoGeneric::GetLightId() const { return x88_lightId; }

void CParticleGenInfoGeneric::DeleteLight(CStateManager& mgr)
{
    if (x88_lightId != kInvalidUniqueId)
    {
        mgr.FreeScriptObject(x88_lightId);
        x88_lightId = kInvalidUniqueId;
    }
}

void CParticleGenInfoGeneric::SetModulationColor(const zeus::CColor& color) { x84_system->SetModulationColor(color); }
}
