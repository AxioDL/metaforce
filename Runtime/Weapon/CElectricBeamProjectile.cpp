#include "Runtime/Weapon/CElectricBeamProjectile.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CElectricBeamProjectile::CElectricBeamProjectile(const TToken<CWeaponDescription>& wDesc, EWeaponType wType,
                                                 const SElectricBeamInfo& elec, const zeus::CTransform& xf,
                                                 EMaterialTypes matTypes, const CDamageInfo& dInfo, TUniqueId uid,
                                                 TAreaId areaId, TUniqueId owner, EProjectileAttrib attribs)
: CBeamProjectile(wDesc, "ElectricBeamProjectile"sv, wType, xf, u32(elec.x8_maxLength), elec.xc_radius,
                  elec.x10_travelSpeed, matTypes, dInfo, uid, areaId, owner, attribs, false)
, x468_electric(std::make_unique<CParticleElectric>(elec.x0_electricDescription))
, x46c_genDescription(g_SimplePool->GetObj({SBIG('PART'), elec.x14_particleId}))
, x478_elementGen(std::make_unique<CElementGen>(x46c_genDescription))
, x47c_(elec.x18_)
, x488_(elec.x1c_) {
  x478_elementGen->SetParticleEmission(false);
  x468_electric->SetParticleEmission(false);
}

void CElectricBeamProjectile::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CElectricBeamProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
    CauseDamage(true);
  } else if (msg == EScriptObjectMessage::Deleted) {
    DeleteProjectileLight(mgr);
  }
  CGameProjectile::AcceptScriptMsg(msg, uid, mgr);
}

void CElectricBeamProjectile::PreRender(CStateManager&, const zeus::CFrustum&) {
  if (!GetActive())
    return;

  g_Renderer->AddParticleGen(*x478_elementGen);
  g_Renderer->AddParticleGen(*x468_electric);
}

void CElectricBeamProjectile::UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (x484_ <= 0.f)
    CauseDamage(true);

  if (GetDamageType() == EDamageType::Actor) {
    x484_ = x488_;
    CauseDamage(false);
  }

  x484_ -= dt;
  if (zeus::close_enough(x47c_, 0.f)) {
    x480_ = 1.f;
  } else {
    float fVar1 = x48c_ ? 1.f : -1.f;
    x480_ = std::min(1.f, dt * (fVar1 / x47c_) + x480_);
    if (x480_ < 0.f) {
      ResetBeam(mgr, true);
    }
  }

  CBeamProjectile::UpdateFx(xf, dt, mgr);

  x478_elementGen->SetModulationColor(zeus::CColor::lerp(zeus::skBlack, zeus::skWhite, x480_));
  bool hasDamage = GetDamageType() != EDamageType::None;
  if (hasDamage) {
    x478_elementGen->SetGlobalOrientation(zeus::lookAt(zeus::skZero3f, GetSurfaceNormal(), zeus::skUp));
    x478_elementGen->SetGlobalTranslation(GetCurrentPos() + (0.001f * GetSurfaceNormal()));
  }
  x478_elementGen->SetParticleEmission(hasDamage);
  x478_elementGen->Update(dt);

  x468_electric->SetModulationColor(zeus::CColor::lerp(zeus::skBlack, zeus::skWhite, x480_));
  x468_electric->SetParticleEmission(true);
  zeus::CVector3f dist = GetCurrentPos() - GetBeamTransform().origin;
  if (dist.canBeNormalized()) {
    dist.normalize();
  }
  x468_electric->SetOverrideIPos(GetBeamTransform().origin);
  x468_electric->SetOverrideIVel(dist);
  x468_electric->SetOverrideFPos(GetCurrentPos());
  x468_electric->SetOverrideFVel(-dist);
  x468_electric->Update(dt);
}

void CElectricBeamProjectile::ResetBeam(CStateManager& mgr, bool b) {
  if (b) {
    SetActive(false);
    x478_elementGen->SetParticleEmission(false);
    x468_electric->SetParticleEmission(false);
    CBeamProjectile::ResetBeam(mgr, true);
  } else {
    x48c_ = false;
  }
}

void CElectricBeamProjectile::Fire(const zeus::CTransform&, CStateManager&, bool) {
  x48c_ = true;
  SetActive(true);
  x480_ = 0.f;
}
} // namespace urde
