#include "CElectricBeamProjectile.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "Particle/CElectricDescription.hpp"
#include "Particle/CParticleElectric.hpp"
#include "Particle/CElementGen.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde {
CElectricBeamProjectile::CElectricBeamProjectile(const TToken<CWeaponDescription>& wDesc, EWeaponType wType,
                                                 const SElectricBeamInfo& elec, const zeus::CTransform& xf,
                                                 EMaterialTypes matTypes, const CDamageInfo& dInfo, TUniqueId uid,
                                                 TAreaId areaId, TUniqueId owner, EProjectileAttrib attribs)
: CBeamProjectile(wDesc, "ElectricBeamProjectile"sv, wType, xf, u32(elec.x8_maxLength), elec.xc_radius,
                  elec.x10_travelSpeed, matTypes, dInfo, uid, areaId, owner, attribs, false)
, x468_electric(new CParticleElectric(elec.x0_electricDescription))
, x46c_genDescription(g_SimplePool->GetObj({SBIG('PART'), elec.x14_particleId}))
, x478_elementGen(new CElementGen(x46c_genDescription))
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

  g_Renderer->AddParticleGen(*x478_elementGen.get());
  g_Renderer->AddParticleGen(*x468_electric.get());
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
  float f2 = zeus::close_enough(x47c_, 0.f) && x48c_ != 0 ? 1.f : -1.f;
  /* TODO: Finish */
}

void CElectricBeamProjectile::ResetBeam(CStateManager& mgr, bool b)
{
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