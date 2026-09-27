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
: CBeamProjectile(wDesc, rstl::string_l("ElectricBeamProjectile"), wType, xf, elec.mMaxLength,
                  elec.mRadius, elec.mTravelSpeed, matTypes, dInfo, uid, areaId, owner,
                  attribs, false)
, mElectric(rs_new CParticleElectric(elec.mElectricDescription))
, mGenDescription(gpSimplePool->GetObj(SObjectTag('PART', elec.mParticleId)))
, mElementGen(rs_new CElementGen(mGenDescription))
, mFadeSpeed(elec.mFadeSpeed)
, mDamageTimer(0.f)
, mDamageInterval(elec.mDamageInterval)
, x48c_(false) {
  mElementGen->SetParticleEmission(false);
  mElectric->SetParticleEmission(false);
}

ENTITY_ACCEPT_IMPL(CElectricBeamProjectile)

void CElectricBeamProjectile::Touch(CActor&, CStateManager&) {}

void CElectricBeamProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (mDamageTimer <= 0.f)
    CauseDamage(true);

  if (GetDamageType() == kDT_Actor) {
    mDamageTimer = mDamageInterval;
    CauseDamage(false);
  }

  mDamageTimer -= dt;
  if (!close_enough(mFadeSpeed, 0.f)) {
    float fVar1 = x48c_ ? 1.f : -1.f;
    mIntensity = rstl::min_val(1.f, dt * (fVar1 / mFadeSpeed) + mIntensity);
    if (mIntensity < 0.f) {
      ResetBeam(mgr, true);
    }
  } else {
    mIntensity = 1.f;
  }

  CBeamProjectile::UpdateFx(xf, dt, mgr);

  mElementGen->SetModulationColor(
      CColor::Lerp(CColor::Black(), CColor::White(), mIntensity));
  bool hasDamage = GetDamageType() != kDT_None;
  if (hasDamage) {
    mElementGen->SetGlobalOrientation(
        CTransform4f::LookAt(CVector3f::Zero(), GetSurfaceNormal(), CVector3f::Up()));
    mElementGen->SetGlobalTranslation(GetCurrentPos() + (0.001f * GetSurfaceNormal()));
  }
  mElementGen->SetParticleEmission(hasDamage);
  mElementGen->Update(dt);

  mElectric->SetModulationColor(CColor::Lerp(CColor::Black(), CColor::White(), mIntensity));
  mElectric->SetParticleEmission(true);
  CVector3f dist = GetCurrentPos() - GetBeamTransform().GetTranslation();
  if (dist.CanBeNormalized()) {
    dist.Normalize();
  }
  mElectric->SetOverrideIPos(GetBeamTransform().GetTranslation());
  mElectric->SetOverrideIVel(dist);
  mElectric->SetOverrideFPos(GetCurrentPos());
  mElectric->SetOverrideFVel(-dist);
  mElectric->Update(dt);
}

void CElectricBeamProjectile::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  if (GetActive()) {
    gpRender->AddParticleGen(*mElementGen);
    gpRender->AddParticleGen(*mElectric);
  }
}

void CElectricBeamProjectile::Fire(const CTransform4f&, CStateManager&, bool) {
  x48c_ = true;
  SetActive(true);
  mIntensity = 0.f;
}

void CElectricBeamProjectile::ResetBeam(CStateManager& mgr, bool b) {
  if (b) {
    SetActive(false);
    mElementGen->SetParticleEmission(false);
    mElectric->SetParticleEmission(false);
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
