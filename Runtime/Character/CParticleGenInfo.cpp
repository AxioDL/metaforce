#include "Runtime/Character/CParticleGenInfo.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/IRenderer.hpp"
#include "Runtime/Particle/CParticleGen.hpp"
#include "Runtime/World/CGameLight.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CParticleGenInfo::CParticleGenInfo(const SObjectTag& part, int frameCount, std::string_view boneName,
                                   const zeus::CVector3f& scale, CParticleData::EParentedMode parentMode, int flags,
                                   EParticleGenType type)
: x4_part(part)
, xc_seconds(frameCount / 60.f)
, x10_boneName(boneName)
, x28_parentMode(parentMode)
, x2c_flags(flags)
, x30_particleScale(scale)
, x80_type(type) {}

static TUniqueId _initializeLight(const std::weak_ptr<CParticleGen>& system, CStateManager& stateMgr, TAreaId areaId,
                                  int lightId) {
  TUniqueId ret = kInvalidUniqueId;
  std::shared_ptr<CParticleGen> systemRef = system.lock();
  if (systemRef->SystemHasLight()) {
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
: CParticleGenInfo(part, frameCount, boneName, scale, parentMode, flags, state), x84_system(system) {
  if (lightId == -1)
    x88_lightId = kInvalidUniqueId;
  else
    x88_lightId = _initializeLight(system, stateMgr, areaId, lightId);
}

void CParticleGenInfoGeneric::AddToRenderer() { g_Renderer->AddParticleGen(*x84_system); }

void CParticleGenInfoGeneric::Render() { x84_system->Render(); }

void CParticleGenInfoGeneric::Update(float dt, CStateManager& stateMgr) {
  x84_system->Update(dt);

  if (x88_lightId == kInvalidUniqueId) {
    return;
  }

  if (const TCastToPtr<CGameLight> gl = stateMgr.ObjectById(x88_lightId)) {
    gl->SetLight(x84_system->GetLight());
  }
}

void CParticleGenInfoGeneric::SetOrientation(const zeus::CTransform& xf, CStateManager& stateMgr) {
  x84_system->SetOrientation(xf);

  if (x88_lightId == kInvalidUniqueId) {
    return;
  }

  if (const TCastToPtr<CGameLight> gl = stateMgr.ObjectById(x88_lightId)) {
    gl->SetRotation(zeus::CQuaternion(xf.buildMatrix3f()));
  }
}

void CParticleGenInfoGeneric::SetTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr) {
  x84_system->SetTranslation(trans);

  if (x88_lightId == kInvalidUniqueId) {
    return;
  }

  if (const TCastToPtr<CGameLight> gl = stateMgr.ObjectById(x88_lightId)) {
    gl->SetTranslation(trans);
  }
}

void CParticleGenInfoGeneric::SetGlobalOrientation(const zeus::CTransform& xf, CStateManager& stateMgr) {
  x84_system->SetGlobalOrientation(xf);

  if (x88_lightId == kInvalidUniqueId) {
    return;
  }

  if (const TCastToPtr<CGameLight> gl = stateMgr.ObjectById(x88_lightId)) {
    gl->SetRotation(zeus::CQuaternion(xf.buildMatrix3f()));
  }
}

void CParticleGenInfoGeneric::SetGlobalTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr) {
  x84_system->SetGlobalTranslation(trans);

  if (x88_lightId == kInvalidUniqueId) {
    return;
  }

  if (const TCastToPtr<CGameLight> gl = stateMgr.ObjectById(x88_lightId)) {
    gl->SetTranslation(trans);
  }
}

void CParticleGenInfoGeneric::SetGlobalScale(const zeus::CVector3f& scale) { x84_system->SetGlobalScale(scale); }

void CParticleGenInfoGeneric::SetParticleEmission(bool isActive, CStateManager& stateMgr) {
  x84_system->SetParticleEmission(isActive);

  if (x88_lightId == kInvalidUniqueId) {
    return;
  }

  if (const TCastToPtr<CGameLight> gl = stateMgr.ObjectById(x88_lightId)) {
    gl->SetActive(isActive);
  }
}

bool CParticleGenInfoGeneric::IsSystemDeletable() const { return x84_system->IsSystemDeletable(); }

std::optional<zeus::CAABox> CParticleGenInfoGeneric::GetBounds() const { return x84_system->GetBounds(); }

bool CParticleGenInfoGeneric::HasActiveParticles() const { return x84_system->GetParticleCount() != 0; }

void CParticleGenInfoGeneric::DestroyParticles() { x84_system->DestroyParticles(); }

bool CParticleGenInfoGeneric::HasLight() const { return x84_system->SystemHasLight(); }

TUniqueId CParticleGenInfoGeneric::GetLightId() const { return x88_lightId; }

void CParticleGenInfoGeneric::DeleteLight(CStateManager& mgr) {
  if (x88_lightId != kInvalidUniqueId) {
    mgr.FreeScriptObject(x88_lightId);
    x88_lightId = kInvalidUniqueId;
  }
}

void CParticleGenInfoGeneric::SetModulationColor(const zeus::CColor& color) { x84_system->SetModulationColor(color); }
} // namespace metaforce
