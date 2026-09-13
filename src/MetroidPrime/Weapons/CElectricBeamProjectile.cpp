#include "MetroidPrime/Weapons/CElectricBeamProjectile.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"


#include "rstl/math.hpp"

CElectricBeamProjectile::CElectricBeamProjectile(const TToken< CWeaponDescription >& wDesc,
                                                 EWeaponType wType, const SElectricBeamInfo& elec,
                                                 const CTransform4f& xf, EMaterialTypes matTypes,
                                                 const CDamageInfo& dInfo, TUniqueId uid,
                                                 TAreaId areaId, TUniqueId owner,
                                                 EProjectileAttrib attribs)
: CBeamProjectile(wDesc, rstl::string_l("ElectricBeamProjectile"), wType, xf, elec.x8_maxLength,
                  elec.xc_radius, elec.x10_travelSpeed, matTypes, dInfo, uid, areaId, owner,
                  attribs, false)
, x468_electric(rs_new CParticleElectric(elec.x0_electricDescription))
, x46c_genDescription(gpSimplePool->GetObj(SObjectTag('PART', elec.x14_particleId)))
, x478_elementGen(rs_new CElementGen(x46c_genDescription))
, x47c_fadeSpeed(elec.x18_fadeSpeed)
, x484_damageTimer(0.f)
, x488_damageInterval(elec.x1c_damageInterval)
, x48c_(false) {
  x478_elementGen->SetParticleEmission(false);
  x468_electric->SetParticleEmission(false);
}

ENTITY_ACCEPT_IMPL(CElectricBeamProjectile)

void CElectricBeamProjectile::Touch(CActor&, CStateManager&) {}

void CElectricBeamProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (x484_damageTimer <= 0.f)
    CauseDamage(true);

  if (GetDamageType() == kDT_Actor) {
    x484_damageTimer = x488_damageInterval;
    CauseDamage(false);
  }

  x484_damageTimer -= dt;
  if (!close_enough(x47c_fadeSpeed, 0.f)) {
    float fVar1 = x48c_ ? 1.f : -1.f;
    x480_intensity = rstl::min_val(1.f, dt * (fVar1 / x47c_fadeSpeed) + x480_intensity);
    if (x480_intensity < 0.f) {
      ResetBeam(mgr, true);
    }
  } else {
    x480_intensity = 1.f;
  }

  CBeamProjectile::UpdateFx(xf, dt, mgr);

  x478_elementGen->SetModulationColor(
      CColor::Lerp(CColor::Black(), CColor::White(), x480_intensity));
  bool hasDamage = GetDamageType() != kDT_None;
  if (hasDamage) {
    x478_elementGen->SetGlobalOrientation(
        CTransform4f::LookAt(CVector3f::Zero(), GetSurfaceNormal(), CVector3f::Up()));
    x478_elementGen->SetGlobalTranslation(GetCurrentPos() + (0.001f * GetSurfaceNormal()));
  }
  x478_elementGen->SetParticleEmission(hasDamage);
  x478_elementGen->Update(dt);

  x468_electric->SetModulationColor(CColor::Lerp(CColor::Black(), CColor::White(), x480_intensity));
  x468_electric->SetParticleEmission(true);
  CVector3f dist = GetCurrentPos() - GetBeamTransform().GetTranslation();
  if (dist.CanBeNormalized()) {
    dist.Normalize();
  }
  x468_electric->SetOverrideIPos(GetBeamTransform().GetTranslation());
  x468_electric->SetOverrideIVel(dist);
  x468_electric->SetOverrideFPos(GetCurrentPos());
  x468_electric->SetOverrideFVel(-dist);
  x468_electric->Update(dt);
}

void CElectricBeamProjectile::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  if (GetActive()) {
    gpRender->AddParticleGen(*x478_elementGen);
    gpRender->AddParticleGen(*x468_electric);
  }
}

void CElectricBeamProjectile::Fire(const CTransform4f&, CStateManager&, bool) {
  x48c_ = true;
  SetActive(true);
  x480_intensity = 0.f;
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

void CElectricBeamProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                              CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    mgr.AddWeaponId(GetOwnerId(), GetType());
    CauseDamage(true);
    break;
  case kSM_Deleted:
    DeleteProjectileLight(mgr);
    break;
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, uid, mgr);
}
