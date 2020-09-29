#include "Runtime/Weapon/CProjectileWeapon.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"

namespace urde {

u16 CProjectileWeapon::g_GlobalSeed = 99;

CProjectileWeapon::CProjectileWeapon(const TToken<CWeaponDescription>& wDesc, const zeus::CVector3f& worldOffset,
                                     const zeus::CTransform& localToWorld, const zeus::CVector3f& scale, s32 flags)
: x4_weaponDesc(wDesc)
, x10_random(g_GlobalSeed)
, x14_localToWorldXf(localToWorld)
, x74_worldOffset(worldOffset)
, xe4_flags(flags) {
  CGlobalRandom gr(x10_random);
  x124_31_VMD2 = x4_weaponDesc->x10_VMD2;
  x124_25_APSO = x4_weaponDesc->x28_APSO;
  if (x4_weaponDesc->x34_APSM) {
    xfc_APSMGen = std::make_unique<CElementGen>(
        x4_weaponDesc->x34_APSM.m_token, CElementGen::EModelOrientationType::Normal,
        (xe4_flags & 0x1) == 0x1 ? CElementGen::EOptionalSystemFlags::Two : CElementGen::EOptionalSystemFlags::One);
    xfc_APSMGen->SetGlobalScale(scale);
  }
  if (x4_weaponDesc->x44_APS2) {
    x100_APS2Gen = std::make_unique<CElementGen>(
        x4_weaponDesc->x44_APS2.m_token, CElementGen::EModelOrientationType::Normal,
        (xe4_flags & 0x1) == 0x1 ? CElementGen::EOptionalSystemFlags::Two : CElementGen::EOptionalSystemFlags::One);
    x100_APS2Gen->SetGlobalScale(scale);
  }
  if (x4_weaponDesc->x54_ASW1) {
    x118_swoosh1 = std::make_unique<CParticleSwoosh>(x4_weaponDesc->x54_ASW1.m_token, 0);
    x118_swoosh1->SetGlobalScale(scale);
  }
  if (x4_weaponDesc->x64_ASW2) {
    x11c_swoosh2 = std::make_unique<CParticleSwoosh>(x4_weaponDesc->x64_ASW2.m_token, 0);
    x11c_swoosh2->SetGlobalScale(scale);
  }
  if (x4_weaponDesc->x74_ASW3) {
    x120_swoosh3 = std::make_unique<CParticleSwoosh>(x4_weaponDesc->x74_ASW3.m_token, 0);
    x120_swoosh3->SetGlobalScale(scale);
  }
  if (CIntElement* pslt = x4_weaponDesc->x14_PSLT.get())
    pslt->GetValue(0, xe8_lifetime);
  else
    xe8_lifetime = 0x7FFFFF;
  if (CVectorElement* ivec = x4_weaponDesc->x4_IVEC.get())
    ivec->GetValue(0, xb0_velocity);
  if (CVectorElement* iorn = x4_weaponDesc->x0_IORN.get()) {
    zeus::CTransform xf;
    zeus::CVector3f orn;
    iorn->GetValue(0, orn);
    xf.rotateLocalX(zeus::degToRad(orn.x()));
    xf.rotateLocalY(zeus::degToRad(orn.y()));
    xf.rotateLocalZ(zeus::degToRad(orn.z()));
    SetRelativeOrientation(xf);
  } else {
    SetRelativeOrientation(zeus::CTransform());
  }
  if (x4_weaponDesc->x84_OHEF)
    x108_model.emplace(x4_weaponDesc->x84_OHEF.m_token);
  x124_26_AP11 = x4_weaponDesc->x2a_AP11;
  x124_27_AP21 = x4_weaponDesc->x2b_AP21;
  x124_28_AS11 = x4_weaponDesc->x2c_AS11;
  x124_29_AS12 = x4_weaponDesc->x2d_AS12;
  x124_30_AS13 = x4_weaponDesc->x2e_AS13;
  UpdateChildParticleSystems(1.f / 60.f);
}

zeus::CTransform CProjectileWeapon::GetTransform() const { return x14_localToWorldXf * x44_localXf; }

zeus::CVector3f CProjectileWeapon::GetTranslation() const {
  return x14_localToWorldXf * (x44_localXf * x8c_projOffset + x80_localOffset) + x74_worldOffset;
}

std::optional<zeus::CAABox> CProjectileWeapon::GetBounds() const {
  zeus::CAABox aabb;
  bool ret = false;

  if (xfc_APSMGen) {
    if (auto b = xfc_APSMGen->GetBounds()) {
      aabb.accumulateBounds(*b);
      ret = true;
    }
  }

  if (x100_APS2Gen) {
    if (auto b = x100_APS2Gen->GetBounds()) {
      aabb.accumulateBounds(*b);
      ret = true;
    }
  }

  if (x118_swoosh1) {
    if (auto b = x118_swoosh1->GetBounds()) {
      aabb.accumulateBounds(*b);
      ret = true;
    }
  }

  if (x11c_swoosh2) {
    if (auto b = x11c_swoosh2->GetBounds()) {
      aabb.accumulateBounds(*b);
      ret = true;
    }
  }

  if (x120_swoosh3) {
    if (auto b = x120_swoosh3->GetBounds()) {
      aabb.accumulateBounds(*b);
      ret = true;
    }
  }

  if (ret) {
    return {aabb};
  }
  return std::nullopt;
}

float CProjectileWeapon::GetAudibleFallOff() const {
  if (!x4_weaponDesc->x94_COLR)
    return 0.f;
  return x4_weaponDesc->x94_COLR.m_res->GetAudibleFallOff();
}

float CProjectileWeapon::GetAudibleRange() const {
  if (!x4_weaponDesc->x94_COLR)
    return 0.f;
  return x4_weaponDesc->x94_COLR.m_res->GetAudibleRange();
}

std::optional<TLockedToken<CDecalDescription>>
CProjectileWeapon::GetDecalForCollision(EWeaponCollisionResponseTypes type) const {
  if (!x4_weaponDesc->x94_COLR) {
    return std::nullopt;
  }
  return x4_weaponDesc->x94_COLR.m_res->GetDecalDescription(type);
}

s32 CProjectileWeapon::GetSoundIdForCollision(EWeaponCollisionResponseTypes type) const {
  if (!x4_weaponDesc->x94_COLR)
    return -1;
  return x4_weaponDesc->x94_COLR.m_res->GetSoundEffectId(type);
}

std::optional<TLockedToken<CGenDescription>> CProjectileWeapon::CollisionOccured(
    EWeaponCollisionResponseTypes type, bool deflected, bool useTarget, const zeus::CVector3f& pos,
    const zeus::CVector3f& normal, const zeus::CVector3f& target) {
  x80_localOffset = x14_localToWorldXf.transposeRotate(pos - x74_worldOffset) - x8c_projOffset;
  zeus::CVector3f posToTarget = target - GetTranslation();
  if (deflected) {
    if (useTarget && posToTarget.canBeNormalized()) {
      SetWorldSpaceOrientation(zeus::lookAt(zeus::skZero3f, posToTarget.normalized()));
    } else {
      zeus::CTransform xf = GetTransform();
      SetWorldSpaceOrientation(
          zeus::lookAt(zeus::skZero3f, xf.basis[1] - normal * (normal.dot(xf.basis[1]) * 2.f), normal));
    }
    return std::nullopt;
  } else {
    x124_24_active = false;
    if (xfc_APSMGen) {
      xfc_APSMGen->SetParticleEmission(false);
    }
    if (x100_APS2Gen) {
      x100_APS2Gen->SetParticleEmission(false);
    }
    if (x118_swoosh1) {
      x118_swoosh1->SetParticleEmission(false);
    }
    if (x11c_swoosh2) {
      x11c_swoosh2->SetParticleEmission(false);
    }
    if (x120_swoosh3) {
      x120_swoosh3->SetParticleEmission(false);
    }
    if (!x4_weaponDesc->x94_COLR) {
      return std::nullopt;
    }
    return x4_weaponDesc->x94_COLR.m_res->GetParticleDescription(type);
  }
}

void CProjectileWeapon::RenderParticles() const {
  if (xfc_APSMGen)
    xfc_APSMGen->Render();
  if (x100_APS2Gen)
    x100_APS2Gen->Render();
  if (x118_swoosh1)
    x118_swoosh1->Render();
  if (x11c_swoosh2)
    x11c_swoosh2->Render();
  if (x120_swoosh3)
    x120_swoosh3->Render();
  if (x104_)
    x104_->Render();
}

void CProjectileWeapon::AddToRenderer() {
  if (xfc_APSMGen)
    g_Renderer->AddParticleGen(*xfc_APSMGen);
  if (x100_APS2Gen)
    g_Renderer->AddParticleGen(*x100_APS2Gen);
  if (x118_swoosh1)
    g_Renderer->AddParticleGen(*x118_swoosh1);
  if (x11c_swoosh2)
    g_Renderer->AddParticleGen(*x11c_swoosh2);
  if (x120_swoosh3)
    g_Renderer->AddParticleGen(*x120_swoosh3);
  if (x104_)
    g_Renderer->AddParticleGen(*x104_);
}

void CProjectileWeapon::Render() {
  if (xf4_curFrame > xe8_lifetime || !x124_24_active || !x108_model)
    return;

  CGraphics::SetModelMatrix(
      zeus::CTransform::Translate(x74_worldOffset) * x14_localToWorldXf *
      zeus::CTransform::Translate(x44_localXf * x8c_projOffset + x80_localOffset + xa4_localOffset2) *
      zeus::CTransform::Scale(x98_scale) * x44_localXf);

  std::vector<CLight> useLights;
  useLights.push_back(CLight::BuildLocalAmbient({}, xc8_ambientLightColor));
  (**x108_model).GetInstance().ActivateLights(useLights);
  constexpr CModelFlags flags(0, 0, 3, zeus::skWhite);
  (*x108_model)->Draw(flags);
}

bool CProjectileWeapon::IsSystemDeletable() const {
  if (xfc_APSMGen && !xfc_APSMGen->IsSystemDeletable())
    return false;
  if (x100_APS2Gen && !x100_APS2Gen->IsSystemDeletable())
    return false;
  if (x118_swoosh1 && !x118_swoosh1->IsSystemDeletable())
    return false;
  if (x11c_swoosh2 && !x11c_swoosh2->IsSystemDeletable())
    return false;
  if (x120_swoosh3 && !x120_swoosh3->IsSystemDeletable())
    return false;
  if (x104_ && !x104_->IsSystemDeletable())
    return false;
  if (x124_24_active)
    return xf4_curFrame >= xe8_lifetime;
  return true;
}

void CProjectileWeapon::UpdateChildParticleSystems(float dt) {
  double useDt;
  if (zeus::close_enough(dt, 1.f / 60.f))
    useDt = 1.0 / 60.0;
  else
    useDt = dt;

  if (xfc_APSMGen) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        xfc_APSMGen->SetParticleEmission(false);
        xfc_APSMGen->EndLifetime();
      } else {
        if (x124_26_AP11)
          xfc_APSMGen->SetGlobalTranslation(GetTranslation());
        else
          xfc_APSMGen->SetTranslation(GetTranslation());
        if (x124_25_APSO)
          xfc_APSMGen->SetOrientation(GetTransform());
      }
    }
    xfc_APSMGen->Update(useDt);
    if (xfc_APSMGen->IsSystemDeletable())
      xfc_APSMGen.reset();
  }

  if (x100_APS2Gen) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        x100_APS2Gen->SetParticleEmission(false);
        x100_APS2Gen->EndLifetime();
      } else {
        if (x124_27_AP21)
          x100_APS2Gen->SetGlobalTranslation(GetTranslation());
        else
          x100_APS2Gen->SetTranslation(GetTranslation());
        if (x124_25_APSO)
          x100_APS2Gen->SetOrientation(GetTransform());
      }
    }
    x100_APS2Gen->Update(useDt);
    if (x100_APS2Gen->IsSystemDeletable())
      x100_APS2Gen.reset();
  }

  if (x118_swoosh1) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        x118_swoosh1->SetParticleEmission(false);
      } else {
        if (x124_28_AS11)
          x118_swoosh1->SetGlobalTranslation(GetTranslation());
        else
          x118_swoosh1->SetTranslation(GetTranslation());
        x118_swoosh1->SetOrientation(GetTransform());
      }
    }
    x118_swoosh1->DoWarmupUpdate();
    if (x118_swoosh1->IsSystemDeletable())
      x118_swoosh1.reset();
  }

  if (x11c_swoosh2) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        x11c_swoosh2->SetParticleEmission(false);
      } else {
        if (x124_29_AS12)
          x11c_swoosh2->SetGlobalTranslation(GetTranslation());
        else
          x11c_swoosh2->SetTranslation(GetTranslation());
        x11c_swoosh2->SetOrientation(GetTransform());
      }
    }
    x11c_swoosh2->DoWarmupUpdate();
    if (x11c_swoosh2->IsSystemDeletable())
      x11c_swoosh2.reset();
  }

  if (x120_swoosh3) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        x120_swoosh3->SetParticleEmission(false);
      } else {
        if (x124_30_AS13)
          x120_swoosh3->SetGlobalTranslation(GetTranslation());
        else
          x120_swoosh3->SetTranslation(GetTranslation());
        x120_swoosh3->SetOrientation(GetTransform());
      }
    }
    x120_swoosh3->DoWarmupUpdate();
    if (x120_swoosh3->IsSystemDeletable())
      x120_swoosh3.reset();
  }

  if (x104_) {
    x104_->Update(useDt);
    if (x104_->IsSystemDeletable())
      x104_.reset();
  }

  xf8_lastParticleFrame = xf4_curFrame;
}

void CProjectileWeapon::UpdatePSTranslationAndOrientation() {
  if (xe8_lifetime < xf4_curFrame || !x124_24_active)
    return;

  if (CModVectorElement* psvm = x4_weaponDesc->xc_PSVM.get())
    psvm->GetValue(xf4_curFrame, xb0_velocity, x80_localOffset);

  if (x124_31_VMD2)
    x80_localOffset += x44_localXf * xb0_velocity;
  else
    x80_localOffset += xb0_velocity;

  xb0_velocity += xbc_gravity / 60.f;

  if (CVectorElement* psov = x4_weaponDesc->x8_PSOV.get()) {
    zeus::CVector3f orient;
    psov->GetValue(xf4_curFrame, orient);

    zeus::CTransform xf = x44_localXf;
    xf.rotateLocalX(zeus::degToRad(orient.x()));
    xf.rotateLocalY(zeus::degToRad(orient.y()));
    xf.rotateLocalZ(zeus::degToRad(orient.z()));
    SetRelativeOrientation(xf);
  }

  if (CVectorElement* pscl = x4_weaponDesc->x18_PSCL.get())
    pscl->GetValue(xf4_curFrame, x98_scale);

  if (CColorElement* pcol = x4_weaponDesc->x1c_PCOL.get())
    pcol->GetValue(xf4_curFrame, xc8_ambientLightColor);

  if (CVectorElement* pofs = x4_weaponDesc->x20_POFS.get())
    pofs->GetValue(xf4_curFrame, xa4_localOffset2);

  if (CVectorElement* ofst = x4_weaponDesc->x24_OFST.get())
    ofst->GetValue(xf4_curFrame, x8c_projOffset);
}

void CProjectileWeapon::SetWorldSpaceOrientation(const zeus::CTransform& xf) {
  x44_localXf = x14_localToWorldXf.inverse() * xf;
}

void CProjectileWeapon::UpdateParticleFX() {
  for (int i = 0; i < xec_childSystemUpdateRate; ++i)
    UpdateChildParticleSystems(1.f / 60.f);
}

void CProjectileWeapon::Update(float dt) {
  CGlobalRandom gr(x10_random);
  xec_childSystemUpdateRate = 0;
  double useDt;
  if (zeus::close_enough(dt, 1.f / 60.f))
    useDt = 1.0 / 60.0;
  else
    useDt = dt;
  useDt = std::max(0.0, useDt);
  xd0_curTime += useDt;

  double actualTime = xf4_curFrame * (1.0 / 60.0);

  while (actualTime < xd0_curTime && !zeus::close_enough(actualTime, xd0_curTime)) {
    if (xf4_curFrame < xe8_lifetime) {
      CParticleGlobals::instance()->SetEmitterTime(xf4_curFrame);
      CParticleGlobals::instance()->SetParticleLifetime(xe8_lifetime);
      CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(xf4_curFrame);
      UpdatePSTranslationAndOrientation();
    }
    actualTime += (1.0 / 60.0);
    ++xf4_curFrame;
    ++xec_childSystemUpdateRate;
  }

  if (zeus::close_enough(actualTime, xd0_curTime))
    xd0_curTime = actualTime;

  xd8_remainderTime = (actualTime - xd0_curTime) * 60.0;

  if (xf4_curFrame < xe8_lifetime) {
    xe0_maxTurnRate = 0.f;
    if (CRealElement* trat = x4_weaponDesc->x30_TRAT.get())
      trat->GetValue(0, xe0_maxTurnRate);
  }
}

} // namespace urde
