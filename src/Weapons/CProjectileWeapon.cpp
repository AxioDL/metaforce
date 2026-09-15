#include "Weapons/CProjectileWeapon.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Weapons/IWeaponRenderer.hpp"
#include "rstl/optional_object.hpp"

#include <Kyoto/Alloc/CMemory.hpp>
#include <Kyoto/CRandom16.hpp>
#include <Kyoto/Graphics/CGraphics.hpp>
#include <Kyoto/Graphics/CModel.hpp>
#include <Kyoto/Graphics/CModelFlags.hpp>
#include <Kyoto/Math/CRelAngle.hpp>
#include <Kyoto/Math/CTransform4f.hpp>
#include <Kyoto/Math/CloseEnough.hpp>
#include <Kyoto/Particles/CElementGen.hpp>
#include <Kyoto/Particles/CParticleGlobals.hpp>
#include <Kyoto/Particles/CParticleSwoosh.hpp>
#include <Kyoto/TToken.hpp>
#include <MetaRender/CCubeRenderer.hpp>
#include <Weapons/CCollisionResponseData.hpp>

uint CProjectileWeapon::skGlobalSeed = 99;

float CProjectileWeapon::GetTickPeriod() { return 1 / 60.f; }

CProjectileWeapon::CProjectileWeapon(const TToken< CWeaponDescription >& description,
                                     const CVector3f& worldOffset, const CTransform4f& localToWorld,
                                     const CVector3f& scale, int flags)
: x4_weaponDesc(description)
, x10_random(skGlobalSeed)
, x14_localToWorldXf(localToWorld)
, x44_localXf(CTransform4f::Identity())
, x74_worldOffset(worldOffset)
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
, x80_previousLocalOffset(CVector3f::Zero())
#endif
, x80_localOffset(CVector3f::Zero())
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
, x98_interpolationOffset(CVector3f::Zero())
#endif
, x8c_projOffset(CVector3f::Zero())
, x98_scale(CVector3f(1.f, 1.f, 1.f))
, xa4_localOffset2(CVector3f::Zero())
, xb0_velocity(CVector3f::Zero())
, xbc_gravity(CVector3f::Zero())
, xc8_ambientLightColor(CColor::White())
, xd0_curTime(0.0)
, xd8_remainderTime(0.0)
, xe0_maxTurnRate(0.f)
, xe4_flags(flags)
, xe8_lifetime(0)
, xec_childSystemUpdateRate(0)
, xf0_(0)
, xf4_curFrame(0)
, xf8_lastParticleFrame(-1)
, xfc_APSMGen(nullptr)
, x100_APS2Gen(nullptr)
, x104_(nullptr)
, x118_swoosh1(nullptr)
, x11c_swoosh2(nullptr)
, x120_swoosh3(nullptr)
, x124_24_active(true)
, x124_25_APSO(false)
, x124_26_AP11(false)
, x124_27_AP21(false)
, x124_28_AS11(false)
, x124_29_AS12(false)
, x124_30_AS13(false)
, x124_31_VMD2(false) {
  CGlobalRandom __(x10_random);
  x124_31_VMD2 = x4_weaponDesc->x10_VMD2;
  x124_25_APSO = x4_weaponDesc->x28_APSO;
  uint unk = xe4_flags & 1;
  if (x4_weaponDesc->x34_APSM) {
    xfc_APSMGen = rs_new CElementGen(*x4_weaponDesc->x34_APSM, CElementGen::kMOT_Normal,
                                     unk ? CElementGen::kOSF_Two : CElementGen::kOSF_One);
    xfc_APSMGen->SetGlobalScale(scale);
  }

  if (x4_weaponDesc->x44_APS2) {
    x100_APS2Gen = rs_new CElementGen(*x4_weaponDesc->x44_APS2, CElementGen::kMOT_Normal,
                                      unk ? CElementGen::kOSF_Two : CElementGen::kOSF_One);
    x100_APS2Gen->SetGlobalScale(scale);
  }
  if (x4_weaponDesc->x54_ASW1) {
    x118_swoosh1 = rs_new CParticleSwoosh(*x4_weaponDesc->x54_ASW1, 0);
    x118_swoosh1->SetGlobalScale(scale);
  }
  if (x4_weaponDesc->x64_ASW2) {
    x11c_swoosh2 = rs_new CParticleSwoosh(*x4_weaponDesc->x64_ASW2, 0);
    x11c_swoosh2->SetGlobalScale(scale);
  }
  if (x4_weaponDesc->x74_ASW3) {
    x120_swoosh3 = rs_new CParticleSwoosh(*x4_weaponDesc->x74_ASW3, 0);
    x120_swoosh3->SetGlobalScale(scale);
  }

  if (x4_weaponDesc->x14_PSLT) {
    x4_weaponDesc->x14_PSLT->GetValue(0, xe8_lifetime);
  } else {
    xe8_lifetime = 0x7FFFFF;
  }

  if (x4_weaponDesc->x4_IVEC) {
    x4_weaponDesc->x4_IVEC->GetValue(0, xb0_velocity);
  }

  if (x4_weaponDesc->x0_IORN) {
    CTransform4f orient(CTransform4f::Identity());
    CVector3f angle(0.f, 0.f, 0.f);
    x4_weaponDesc->x0_IORN->GetValue(0, angle);
    CRelAngle relAngleX = CRelAngle::FromDegrees(angle.GetX());
    orient.RotateLocalX(relAngleX);
    CRelAngle relAngleY = CRelAngle::FromDegrees(angle.GetY());
    orient.RotateLocalY(relAngleY);
    CRelAngle relAngleZ = CRelAngle::FromDegrees(angle.GetZ());
    orient.RotateLocalZ(relAngleZ);
    SetRelativeOrientation(orient);
  } else {
    SetRelativeOrientation(CTransform4f::Identity());
  }
  if (x4_weaponDesc->GetOHEF()) {
    x108_model = *x4_weaponDesc->GetOHEF();
  }

  x124_26_AP11 = x4_weaponDesc->x2a_AP11;
  x124_27_AP21 = x4_weaponDesc->x2b_AP21;
  x124_28_AS11 = x4_weaponDesc->x2c_AS11;
  x124_29_AS12 = x4_weaponDesc->x2d_AS12;
  x124_30_AS13 = x4_weaponDesc->x2e_AS13;
  UpdateChildParticleSystems(1.f / 60.f);
}

CProjectileWeapon::~CProjectileWeapon() {
  delete xfc_APSMGen;
  delete x100_APS2Gen;
  delete x104_;
  delete x118_swoosh1;
  delete x11c_swoosh2;
  delete x120_swoosh3;
}

bool CProjectileWeapon::Update(float dt) {
  CGlobalRandom __(x10_random);

  double actualTime = xf4_curFrame * (1.0 / 60.0);
  xec_childSystemUpdateRate = 0;
  double useDt = close_enough(dt, 1.f / 60.f, 1.6666666851961054e-5f) ? (1.0 / 60.0) : dt;
  double timeScale = 1.0;
  useDt *= timeScale;
  if (useDt < 0.f) {
    useDt = 0.f;
  }
  xd0_curTime += useDt;
  while (actualTime < xd0_curTime &&
         !close_enough(actualTime, xd0_curTime, 1.6666666666666667e-5)) {
    if (xf4_curFrame < xe8_lifetime) {
      CParticleGlobals::SetEmitterTime(xf4_curFrame);
      CParticleGlobals::SetParticleLifetime(xe8_lifetime);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(xf4_curFrame);
      UpdatePSTranslationAndOrientation();
    }

    actualTime += 1.f / 60.f;
    ++xf4_curFrame;
    ++xec_childSystemUpdateRate;
  }

  if (close_enough(actualTime, xd0_curTime, 1.6666666666666667e-5)) {
    xd0_curTime = actualTime;
  }

  xd8_remainderTime = (float)((actualTime - xd0_curTime) / (1.f / 60.0));
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  x98_interpolationOffset =
      static_cast< float >(xd8_remainderTime) * (x80_previousLocalOffset - x80_localOffset);
#endif

  if (xf4_curFrame < xe8_lifetime) {
    xe0_maxTurnRate = 0.f;
    if (CRealElement* trat = x4_weaponDesc->x30_TRAT) {
      trat->GetValue(0, xe0_maxTurnRate);
    }
  }

  return false;
}

void CProjectileWeapon::UpdateParticleFX() {
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  if (xfc_APSMGen && x4_weaponDesc->x28_30_SPS1) {
    xfc_APSMGen->Update(1.f / 60.f);
  }
  if (x100_APS2Gen && x4_weaponDesc->x29_24_SPS2) {
    x100_APS2Gen->Update(1.f / 60.f);
  }
#endif
  for (int i = 0; i < xec_childSystemUpdateRate; ++i) {
    UpdateChildParticleSystems(1.f / 60.f);
  }
}

const CTransform4f CProjectileWeapon::GetTransform() const {
  return x14_localToWorldXf * x44_localXf;
}

CTransform4f CProjectileWeapon::GetTransform() { return x14_localToWorldXf * x44_localXf; }

const CVector3f CProjectileWeapon::GetTranslation() const {
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  return x14_localToWorldXf *
             (x80_localOffset + x98_interpolationOffset + x44_localXf * x8c_projOffset) +
         x74_worldOffset;
#else
  return x14_localToWorldXf * (x80_localOffset + x44_localXf * x8c_projOffset) + x74_worldOffset;
#endif
}

void CProjectileWeapon::SetRelativeOrientation(const CTransform4f& orient) { x44_localXf = orient; }

void CProjectileWeapon::SetWorldSpaceOrientation(const CTransform4f& orient) {
  x44_localXf = x14_localToWorldXf.GetInverse() * orient;
}

void CProjectileWeapon::UpdatePSTranslationAndOrientation() {
  if (xe8_lifetime >= xf4_curFrame && x124_24_active) {
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    x80_previousLocalOffset = x80_localOffset;
#endif
    if (CModVectorElement* psvm = x4_weaponDesc->xc_PSVM) {
      psvm->GetValue(xf4_curFrame, xb0_velocity, x80_localOffset);
    }

    if (x124_31_VMD2) {
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
      CVector3f velocity = x44_localXf * xb0_velocity;
      x80_localOffset += velocity;
#else
      x80_localOffset += x44_localXf * xb0_velocity;
#endif
    } else {
      x80_localOffset += xb0_velocity;
    }

    xb0_velocity += xbc_gravity / 60.f;

    if (CVectorElement* psov = x4_weaponDesc->x8_PSOV) {
      CVector3f orient(0.f, 0.f, 0.f);
      psov->GetValue(xf4_curFrame, orient);

      CTransform4f xf = x44_localXf;
      xf.RotateLocalX(CRelAngle::FromDegrees(orient.GetX()));
      xf.RotateLocalY(CRelAngle::FromDegrees(orient.GetY()));
      xf.RotateLocalZ(CRelAngle::FromDegrees(orient.GetZ()));
      SetRelativeOrientation(xf);
    }

    if (CVectorElement* pscl = x4_weaponDesc->x18_PSCL) {
      pscl->GetValue(xf4_curFrame, x98_scale);
    }

    if (CColorElement* pcol = x4_weaponDesc->x1c_PCOL) {
      pcol->GetValue(xf4_curFrame, xc8_ambientLightColor);
    }

    if (CVectorElement* pofs = x4_weaponDesc->x20_POFS) {
      pofs->GetValue(xf4_curFrame, xa4_localOffset2);
    }

    if (CVectorElement* ofst = x4_weaponDesc->x24_OFST) {
      ofst->GetValue(xf4_curFrame, x8c_projOffset);
    }
  }
}

void CProjectileWeapon::UpdateChildParticleSystems(float dt) {
  double useDt = (close_enough(dt, 1.f / 60.f, 1.6666666851961054e-5f)) ? 1.0 / 60.0 : dt;

  if (xfc_APSMGen) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        xfc_APSMGen->SetParticleEmission(false);
        xfc_APSMGen->EndLifetime();
      } else {
        if (x124_26_AP11) {
          xfc_APSMGen->SetGlobalTranslation(GetTranslation());
        } else {
          xfc_APSMGen->SetTranslation(GetTranslation());
        }

        if (x124_25_APSO) {
          xfc_APSMGen->SetOrientation(GetTransform());
        }
      }
    }
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    if (!x4_weaponDesc->x28_30_SPS1)
#endif
      xfc_APSMGen->Update(useDt);
    if (xfc_APSMGen->IsSystemDeletable() == TRUE) {
      delete xfc_APSMGen;
      xfc_APSMGen = nullptr;
    }
  }

  if (x100_APS2Gen) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        x100_APS2Gen->SetParticleEmission(false);
        x100_APS2Gen->EndLifetime();
      } else {
        if (x124_27_AP21) {
          x100_APS2Gen->SetGlobalTranslation(GetTranslation());
        } else {
          x100_APS2Gen->SetTranslation(GetTranslation());
        }

        if (x124_25_APSO) {
          x100_APS2Gen->SetOrientation(GetTransform());
        }
      }
    }

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    if (!x4_weaponDesc->x29_24_SPS2)
#endif
      x100_APS2Gen->Update(useDt);
    if (x100_APS2Gen->IsSystemDeletable() == TRUE) {
      delete x100_APS2Gen;
      x100_APS2Gen = nullptr;
    }
  }

  if (x118_swoosh1) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        x118_swoosh1->SetParticleEmission(false);
      } else {
        if (x124_28_AS11) {
          x118_swoosh1->SetGlobalTranslation(GetTranslation());
        } else {
          x118_swoosh1->SetTranslation(GetTranslation());
        }
        x118_swoosh1->SetOrientation(GetTransform());
      }
    }
    x118_swoosh1->SetWarmUp();
    x118_swoosh1->Update(0.0);
    if (x118_swoosh1->IsSystemDeletable() == TRUE) {
      delete x118_swoosh1;
      x118_swoosh1 = nullptr;
    }
  }

  if (x11c_swoosh2) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        x11c_swoosh2->SetParticleEmission(false);
      } else {
        if (x124_29_AS12) {
          x11c_swoosh2->SetGlobalTranslation(GetTranslation());
        } else {
          x11c_swoosh2->SetTranslation(GetTranslation());
        }
        x11c_swoosh2->SetOrientation(GetTransform());
      }
    }
    x11c_swoosh2->SetWarmUp();
    x11c_swoosh2->Update(0.0);
    if (x11c_swoosh2->IsSystemDeletable() == TRUE) {
      delete x11c_swoosh2;
      x11c_swoosh2 = nullptr;
    }
  }

  if (x120_swoosh3) {
    if (xf8_lastParticleFrame != xf4_curFrame) {
      if (xf4_curFrame > xe8_lifetime) {
        x120_swoosh3->SetParticleEmission(false);
      } else {
        if (x124_30_AS13) {
          x120_swoosh3->SetGlobalTranslation(GetTranslation());
        } else {
          x120_swoosh3->SetTranslation(GetTranslation());
        }
        x120_swoosh3->SetOrientation(GetTransform());
      }
    }
    x120_swoosh3->SetWarmUp();
    x120_swoosh3->Update(0.0);
    if (x120_swoosh3->IsSystemDeletable() == TRUE) {
      delete x120_swoosh3;
      x120_swoosh3 = nullptr;
    }
  }

  if (x104_) {
    x104_->Update(useDt);
    if (x104_->IsSystemDeletable() == TRUE) {
      delete x104_;
      x104_ = nullptr;
    }
  }

  xf8_lastParticleFrame = xf4_curFrame;
}

const bool CProjectileWeapon::IsSystemDeletable() const {
  bool ret = true;
  if (xfc_APSMGen && !xfc_APSMGen->IsSystemDeletable()) {
    ret = false;
  } else if (x100_APS2Gen && !x100_APS2Gen->IsSystemDeletable()) {
    ret = false;
  } else if (x118_swoosh1 && !x118_swoosh1->IsSystemDeletable()) {
    ret = false;
  } else if (x11c_swoosh2 && !x11c_swoosh2->IsSystemDeletable()) {
    ret = false;
  } else if (x120_swoosh3 && !x120_swoosh3->IsSystemDeletable()) {
    ret = false;
  } else if (x104_ && !x104_->IsSystemDeletable()) {
    ret = false;
  } else if (x124_24_active) {
    ret = xf4_curFrame >= xe8_lifetime;
  }
  return ret;
}

void CProjectileWeapon::Render() const {
  if (xf4_curFrame <= xe8_lifetime && x124_24_active && x108_model) {
    CTransform4f localXf = CTransform4f::Translate(
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
        x80_localOffset + x98_interpolationOffset + (x44_localXf * x8c_projOffset) +
        xa4_localOffset2);
#else
        x80_localOffset + (x44_localXf * x8c_projOffset) + xa4_localOffset2);
#endif
    CTransform4f worldXf = CTransform4f::Translate(x74_worldOffset);
    CTransform4f scaleXf =
        CTransform4f::Scale(x98_scale.GetX(), x98_scale.GetY(), x98_scale.GetZ());
    CGraphics::SetModelMatrix(worldXf * x14_localToWorldXf * localXf * scaleXf * x44_localXf);
    CLight light = CLight::BuildLocalAmbient(CVector3f::Zero(), xc8_ambientLightColor);
    (*x108_model)->Draw(CModelFlags(CModelFlags::kT_Opaque, 1.f));
  }
}

void CProjectileWeapon::AddToRenderer() const {
  if (xfc_APSMGen)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*xfc_APSMGen);
  if (x100_APS2Gen)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*x100_APS2Gen);
  if (x118_swoosh1)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*x118_swoosh1);
  if (x11c_swoosh2)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*x11c_swoosh2);
  if (x120_swoosh3)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*x120_swoosh3);
  if (x104_)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*x104_);
}

void CProjectileWeapon::RenderParticles() const {
  if (xfc_APSMGen) {
    xfc_APSMGen->Render();
  }
  if (x100_APS2Gen) {
    x100_APS2Gen->Render();
  }
  if (x118_swoosh1) {
    x118_swoosh1->Render();
  }
  if (x11c_swoosh2) {
    x11c_swoosh2->Render();
  }
  if (x120_swoosh3) {
    x120_swoosh3->Render();
  }
  if (x104_) {
    x104_->Render();
  }
}

rstl::optional_object< TLockedToken< CGenDescription > > CProjectileWeapon::CollisionOccured(
    const EWeaponCollisionResponseTypes colType, const bool deflected, const bool useTarget,
    const CVector3f& pos, const CVector3f& normal, const CVector3f& target) {
  x80_localOffset = x14_localToWorldXf.TransposeRotate(pos - x74_worldOffset) - x8c_projOffset;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  x98_interpolationOffset = CVector3f::Zero();
#endif

  if (deflected) {
    CVector3f posToTarget = target - GetTranslation();
    if (useTarget && posToTarget.CanBeNormalized()) {
      SetWorldSpaceOrientation(CTransform4f::LookAt(CVector3f::Zero(), posToTarget.AsNormalized()));
    } else {
      const CVector3f& col = GetTransform().GetColumn(kDY);
      CTransform4f lookXf = CTransform4f::LookAt(
          CVector3f::Zero(), col - ((CVector3f::Dot(normal, col) * 2.f) * normal), normal);
      SetWorldSpaceOrientation(lookXf);
    }
    return rstl::optional_object_null();
  }

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
    return rstl::optional_object_null();
  }
  TToken< CCollisionResponseData > tok = (*x4_weaponDesc->GetCollisionResponse());
  return tok->GetParticleDescription(colType);
}

uint CProjectileWeapon::GetSoundIdForCollision(EWeaponCollisionResponseTypes type) const {
  if (!x4_weaponDesc->GetCollisionResponse()) {
    return -1;
  }
  TToken< CCollisionResponseData > tok = (*x4_weaponDesc->GetCollisionResponse());
  return tok->GetSoundEffectId(type);
}

rstl::optional_object< TLockedToken< CDecalDescription > >
CProjectileWeapon::GetDecalForCollision(EWeaponCollisionResponseTypes type) const {
  if (!x4_weaponDesc->GetCollisionResponse()) {
    return rstl::optional_object_null();
  }
  TToken< CCollisionResponseData > tok = (*x4_weaponDesc->GetCollisionResponse());
  return tok->GetDecalDescription(type);
}

float CProjectileWeapon::GetAudibleRange() const {
  if (!x4_weaponDesc->GetCollisionResponse()) {
    return 0.f;
  }
  TToken< CCollisionResponseData > tok = (*x4_weaponDesc->GetCollisionResponse());
  return tok->GetAudibleRange();
}

float CProjectileWeapon::GetAudibleFallOff() const {
  if (!x4_weaponDesc->GetCollisionResponse()) {
    return 0.f;
  }
  TToken< CCollisionResponseData > tok = (*x4_weaponDesc->GetCollisionResponse());
  return tok->GetAudibleFallOff();
}

float CProjectileWeapon::GetMaxTurnRate() const { return xe0_maxTurnRate; }

void CProjectileWeapon::SetVelocity(const CVector3f& velocity) { xb0_velocity = velocity; }

const CVector3f& CProjectileWeapon::GetVelocity() const { return xb0_velocity; }

void CProjectileWeapon::SetGravity(const CVector3f& gravity) { xbc_gravity = gravity; }

const CVector3f& CProjectileWeapon::GetGravity() const { return xbc_gravity; }

void CProjectileWeapon::SetGlobalSeed(const uint seed) { skGlobalSeed = seed; }

rstl::optional_object< CAABox > CProjectileWeapon::GetBounds() const {
  CAABox ret = CAABox::MakeMaxInvertedBox();
  bool hasBox = false;

  if (xfc_APSMGen) {
    rstl::optional_object< CAABox > bounds = xfc_APSMGen->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (x100_APS2Gen) {
    rstl::optional_object< CAABox > bounds = x100_APS2Gen->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (x118_swoosh1) {
    rstl::optional_object< CAABox > bounds = x118_swoosh1->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (x11c_swoosh2) {
    rstl::optional_object< CAABox > bounds = x11c_swoosh2->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (x120_swoosh3) {
    rstl::optional_object< CAABox > bounds = x120_swoosh3->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (hasBox) {
    return ret;
  }

  return rstl::optional_object_null();
}
