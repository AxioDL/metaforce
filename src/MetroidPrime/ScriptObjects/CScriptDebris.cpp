#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

static inline float debris_frand(CStateManager& mgr) {
  return (1.f / 16383.5f) * CCast::ToReal32(static_cast< short >(mgr.Random()->Next() % 32767)) -
         1.f;
}

static inline float debris_frand_range(CStateManager& mgr, float min, float max) {
  return (max - min) * mgr.Random()->Float() + min;
}

static CVector3f debris_cone(CStateManager& mgr, float coneAngle, float minMag, float maxMag) {
  const float mag = debris_frand_range(mgr, minMag, maxMag);
  const float cosAngle = CMath::FastCosR((M_PIF / 360.f) * coneAngle);
  const float side = 1.f - (1.f - cosAngle) * mgr.Random()->Float();
  const float sideSquared = side * side;
  const float hyp = mag * CMath::FastSqrtF(CMath::Max(0.f, 1.f - sideSquared));
  const float angle = M_2PIF * mgr.Random()->Float();
  return CVector3f(hyp * CMath::FastCosR(angle), hyp * CMath::FastSinR(angle), mag * side);
}

CScriptDebris::CScriptDebris(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& mData,
                             const CActorParameters& aParams, uint particleId,
                             const CVector3f& particleScale, float zImpulse,
                             const CVector3f& velocity, const CColor& endsColor, float mass,
                             float restitution, float duration, EScaleType scaleType, bool unused,
                             bool randomAngImpulse, bool active)
: CPhysicsActor(uid, active, name, info, xf, mData, CMaterialList(kMT_Solid, kMT_Debris),
                mData.GetBounds(xf.GetRotation()), SMoverData(mass), aParams, 0.3f, 0.1f)
, x258_velocity(velocity)
, x264_color(1.f, 0.5f, 0.5f, 1.f)
, x268_endsColor(endsColor)
, x26c_zImpulse(zImpulse)
, x270_curTime(0.f)
, x274_duration(duration >= 0.f ? duration : 0.5f)
, x278_ooDuration(1.f / x274_duration)
, x27c_restitution(restitution)
, x280_scaleType(scaleType)
, x281_24_randomAngImpulse(randomAngImpulse)
, x281_25_particle1GlobalTranslation(false)
, x281_26_deferDeleteTillParticle1Done(false)
, x281_27_particle2GlobalTranslation(false)
, x281_28_deferDeleteTillParticle2Done(false)
, x281_29_particle3Active(false)
, x281_30_debrisExtended(false)
, x281_31_dieOnProjectile(false)
, x282_24_noBounce(false)
, x283_particleOr0(kOT_NotOriented)
, x284_particleOr1(kOT_NotOriented)
, x285_particleOr2(kOT_NotOriented)
, x288_linConeAngle(0.f)
, x28c_linMinMag(0.f)
, x290_linMaxMag(0.f)
, x294_angMinMag(0.f)
, x298_angMaxMag(0.f)
, x29c_minDuration(0.f)
, x2a0_maxDuration(0.f)
, x2a4_colorInT(0.f)
, x2a8_colorOutT(0.f)
, x2ac_scaleOutStartT(0.f)
, x2b0_scale(CVector3f(mData.GetScale()))
, x2bc_endScale(scaleType == kST_NoScale      ? CVector3f(mData.GetScale())
                : scaleType == kST_EndsToZero ? CVector3f::Zero()
                                              : CVector3f(5.f, 5.f, 5.f))
, x2c8_collisionNormal(CVector3f::Zero())
, x2d4_particleGen0(nullptr)
, x2d8_particleGen1(nullptr)
, x2dc_particleGen2(nullptr)
, x2e0_speedAvg(2.f) {
  SetDoTargetDistanceTest(false);
  if (HasActorLights()) {
    ActorLights()->SetFramesBetweenRecalculation(ActorLights()->GetFramesBetweenRecalculation() *
                                                 2);
  }
  SetUseInSortedLists(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_Debris, kMT_Character, kMT_Player, kMT_Projectile)));
  if (gpResourceFactory->GetResourceTypeById(particleId) != 0) {
    TToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', particleId));
    x2d4_particleGen0 = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    x2d4_particleGen0->SetGlobalScale(particleScale);
  }

  const float m = GetMass();
  const float g = -GravityConstant();
  SetMomentumWR(CVector3f(0.f, 0.f, g * m));
  if (HasActorLights()) {
    ActorLights()->SetAmbienceGenerated(true);
  }
}

CScriptDebris::CScriptDebris(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& mData,
                             const CActorParameters& aParams, float linConeAngle, float linMinMag,
                             float linMaxMag, float angMinMag, float angMaxMag, float minDuration,
                             float maxDuration, float colorInT, float colorOutT,
                             const CColor& color, const CColor& endsColor, float scaleOutStartT,
                             const CVector3f& scale, const CVector3f& endScale, float restitution,
                             float downwardSpeed, const CVector3f& localOffset, uint particle0,
                             const CVector3f& particle0Scale, bool particle1GlobalTranslation,
                             bool deferDeleteTillParticle1Done, EOrientationType particleOr0,
                             uint particle1, const CVector3f& particle1Scale,
                             bool particle2GlobalTranslation, bool deferDeleteTillParticle2Done,
                             EOrientationType particleOr1, uint particle2,
                             const CVector3f& particle2Scale, EOrientationType particleOr2,
                             bool solid, bool dieOnProjectile, bool noBounce, bool active)
: CPhysicsActor(uid, active, name, info, xf, mData, CMaterialList(kMT_Solid, kMT_Debris),
                mData.IsNull() ? CAABox(CVector3f(-0.5f, -0.5f, -0.5f), CVector3f(0.5f, 0.5f, 0.5f))
                               : mData.GetBounds(xf.GetRotation()),
                SMoverData(1.f), aParams, 0.3f, 0.1f)
, x258_velocity(CVector3f::Zero())
, x264_color(color)
, x268_endsColor(endsColor)
, x26c_zImpulse(0.f)
, x270_curTime(0.f)
, x274_duration(0.f)
, x278_ooDuration(0.f)
, x27c_restitution(restitution)
, x280_scaleType(kST_NoScale)
, x281_24_randomAngImpulse(false)
, x281_25_particle1GlobalTranslation(particle1GlobalTranslation)
, x281_26_deferDeleteTillParticle1Done(deferDeleteTillParticle1Done)
, x281_27_particle2GlobalTranslation(particle2GlobalTranslation)
, x281_28_deferDeleteTillParticle2Done(deferDeleteTillParticle2Done)
, x281_29_particle3Active(false)
, x281_30_debrisExtended(true)
, x281_31_dieOnProjectile(dieOnProjectile)
, x282_24_noBounce(noBounce)
, x283_particleOr0(particleOr0)
, x284_particleOr1(particleOr1)
, x285_particleOr2(particleOr2)
, x288_linConeAngle(linConeAngle)
, x28c_linMinMag(linMinMag)
, x290_linMaxMag(linMaxMag)
, x294_angMinMag(angMinMag)
, x298_angMaxMag(angMaxMag)
, x29c_minDuration(minDuration)
, x2a0_maxDuration(maxDuration)
, x2a4_colorInT(colorInT / 100.f)
, x2a8_colorOutT(colorOutT / 100.f)
, x2ac_scaleOutStartT(scaleOutStartT / 100.f)
, x2b0_scale(scale)
, x2bc_endScale(CVector3f::ByElementMultiply(scale, endScale))
, x2c8_collisionNormal(CVector3f::Zero())
, x2d4_particleGen0(nullptr)
, x2d8_particleGen1(nullptr)
, x2dc_particleGen2(nullptr)
, x2e0_speedAvg(2.f) {
  SetUseInSortedLists(false);
  SetTranslation(GetTranslation() + GetTransform().Rotate(localOffset));
  if (solid) {
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(kMT_Solid), CMaterialList(kMT_Debris, kMT_Character, kMT_Player)));
  } else {
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(),
        CMaterialList(kMT_Debris, kMT_Character, kMT_Player, kMT_Projectile, kMT_Solid)));
  }
  if (gpResourceFactory->GetResourceTypeById(particle0) != 0) {
    TToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', particle0));
    x2d4_particleGen0 = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    x2d4_particleGen0->SetGlobalScale(particle0Scale);
  }
  if (gpResourceFactory->GetResourceTypeById(particle1) != 0) {
    TToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', particle1));
    x2d8_particleGen1 = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    x2d8_particleGen1->SetGlobalScale(particle1Scale);
  }
  if (gpResourceFactory->GetResourceTypeById(particle2) != 0) {
    TToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', particle2));
    x2dc_particleGen2 = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    x2dc_particleGen2->SetGlobalScale(particle2Scale);
  }
  const float mass = GetMass();
  SetMomentumWR(CVector3f(0.f, 0.f, -downwardSpeed * mass));
}

CScriptDebris::~CScriptDebris() {}

void CScriptDebris::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  x270_curTime += dt;
  bool done = x270_curTime >= x274_duration;

  if (!x2d4_particleGen0.null()) {
    if (x270_curTime >= x274_duration) {
      x2d4_particleGen0->SetParticleEmission(false);
    } else {
      if (x281_25_particle1GlobalTranslation) {
        x2d4_particleGen0->SetGlobalTranslation(GetTranslation());
      } else {
        x2d4_particleGen0->SetTranslation(GetTranslation());
      }
      if (x283_particleOr0 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          x2d4_particleGen0->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
      } else if (x283_particleOr0 == kOT_ToObject) {
        x2d4_particleGen0->SetOrientation(GetTransform().GetRotation());
      }
    }
    if (x281_26_deferDeleteTillParticle1Done && x2d4_particleGen0->GetParticleCount() != 0) {
      done = false;
    }
    if (x270_curTime < x274_duration || x281_26_deferDeleteTillParticle1Done) {
      x2d4_particleGen0->Update(dt);
    }
  }

  if (!x2d8_particleGen1.null()) {
    if (x270_curTime >= x274_duration) {
      x2d8_particleGen1->SetParticleEmission(false);
    } else {
      if (x281_27_particle2GlobalTranslation) {
        x2d8_particleGen1->SetGlobalTranslation(GetTranslation());
      } else {
        x2d8_particleGen1->SetTranslation(GetTranslation());
      }
      if (x284_particleOr1 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          x2d8_particleGen1->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
      } else if (x284_particleOr1 == kOT_ToObject) {
        x2d8_particleGen1->SetOrientation(GetTransform().GetRotation());
      }
    }
    if (x281_28_deferDeleteTillParticle2Done && x2d8_particleGen1->GetParticleCount() != 0) {
      done = false;
    }
    if (x270_curTime < x274_duration || x281_28_deferDeleteTillParticle2Done) {
      x2d8_particleGen1->Update(dt);
    }
  }

  if (!x2dc_particleGen2.null()) {
    if (x270_curTime >= x274_duration && !x281_29_particle3Active) {
      x2dc_particleGen2->SetGlobalTranslation(GetTranslation());
      if (x285_particleOr2 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          x2dc_particleGen2->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
      } else if (x285_particleOr2 == kOT_ToObject) {
        x2dc_particleGen2->SetOrientation(GetTransform().GetRotation());
      } else if (x285_particleOr2 == kOT_AlongCollisionNormal) {
        if (x2c8_collisionNormal.MagSquared() == 0.f) {
          x2c8_collisionNormal = CVector3f::Up();
        }
        const CTransform4f orientation = CTransform4f::LookAt(
            CVector3f::Zero(), x2c8_collisionNormal,
            CMath::AbsF(CVector3f::Dot(CVector3f::Up(), x2c8_collisionNormal)) > 0.99f
                ? CVector3f::Right()
                : CVector3f::Up());
        x2dc_particleGen2->SetOrientation(orientation);
      }
      x281_29_particle3Active = true;
    }
    if (x281_29_particle3Active) {
      x2dc_particleGen2->Update(dt);
      if (!x2dc_particleGen2->IsSystemDeletable()) {
        done = false;
      }
    }
  }

  if (HasModelData()) {
    const float t = x270_curTime / x274_duration > x2ac_scaleOutStartT
                        ? (x270_curTime - x274_duration * x2ac_scaleOutStartT) /
                              (x274_duration * (1.f - x2ac_scaleOutStartT))
                        : 0.f;
    ModelData()->SetScale(CVector3f::Lerp(x2b0_scale, x2bc_endScale, t));
  }
  if (x270_curTime >= x274_duration) {
    SetMomentumWR(CVector3f::Zero());
    SetMaterialFilter(CMaterialFilter::MakeExclude(
        CMaterialList(kMT_Debris, kMT_Character, kMT_Player, kMT_Projectile)));
    if (done) {
      mgr.DeleteObjectRequest(GetUniqueId());
      return;
    }
  }
  if (xf8_24_movable) {
    x2e0_speedAvg.AddValue(GetVelocityWR().Magnitude());
    if (*x2e0_speedAvg.GetAverage() < 0.1f) {
      xf8_24_movable = false;
    }
  }
}

void CScriptDebris::Touch(CActor& other, CStateManager& mgr) {
  if (!x281_31_dieOnProjectile) {
    return;
  }
  if (TCastToPtr< CGameProjectile >(other)) {
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

rstl::optional_object< CAABox > CScriptDebris::GetTouchBounds() const {
  if (x281_31_dieOnProjectile) {
    return GetBoundingBox();
  }
  return rstl::optional_object_null();
}

void CScriptDebris::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                    CStateManager& mgr) {
  switch (msg) {
  case kSM_Activate:
    if (!x281_30_debrisExtended) {
      const float mass = GetMass();
      const float zRand = debris_frand(mgr);
      const float z = mass * x258_velocity.GetZ() * CMath::AbsF(zRand) + x26c_zImpulse;
      const float yRand = debris_frand(mgr);
      const float yScale = mass * x258_velocity.GetY();
      const float y = yScale * yRand;
      const float xRand = debris_frand(mgr);
      const float xScale = mass * x258_velocity.GetX();
      const float x = xScale * xRand;
      const CVector3f linImpulse = GetTransform().GetColumn(kDZ) + CVector3f(x, y, z);
      CAxisAngle angImpulse = CAxisAngle::Identity();
      if (x281_24_randomAngImpulse) {
        if (mgr.Random()->Next() % 100 < 50) {
          angImpulse = CAxisAngle(CVector3f(45.f * debris_frand(mgr), 15.f * debris_frand(mgr),
                                            35.f * debris_frand(mgr)));
        } else {
          angImpulse = CAxisAngle::Identity();
        }
      }
      ApplyImpulseWR(linImpulse, angImpulse);
    } else {
      const CVector3f linImpulse =
          debris_cone(mgr, x288_linConeAngle, x28c_linMinMag, x290_linMaxMag);
      const CAxisAngle angImpulse(debris_cone(mgr, 360.f, x294_angMinMag, x298_angMaxMag));
      ApplyImpulseOR(linImpulse, angImpulse);
      x274_duration = debris_frand_range(mgr, x29c_minDuration, x2a0_maxDuration);
    }
    if (!x2d4_particleGen0.null()) {
      x2d4_particleGen0->SetParticleEmission(true);
    }
    if (!x2d8_particleGen1.null()) {
      x2d8_particleGen1->SetParticleEmission(true);
    }
    break;
  case kSM_OnFloor:
    if (!x282_24_noBounce) {
      const CVector3f linImpulse = -x27c_restitution * GetConstantForceWR();
      const CAxisAngle angImpulse = -x27c_restitution * GetAngularMomentumWR();
      ApplyImpulseWR(linImpulse, angImpulse);
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CScriptDebris::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);
  float t = x270_curTime / x274_duration;
  if (t < x2a4_colorInT) {
    t = x2a4_colorInT > 0.f ? 1.f - x270_curTime / (x274_duration * x2a4_colorInT) : 0.f;
  } else if (t > x2a8_colorOutT) {
    t = (x270_curTime - x274_duration * x2a8_colorOutT) / (x274_duration * (1.f - x2a8_colorOutT));
  } else {
    t = 0.f;
  }
  const CColor color = CColor::Lerp(CColor::White(), x268_endsColor, t);
  SetModelFlags(
      CModelFlags::AlphaBlended(color).DepthCompareUpdate(true, color.GetAlphau8() == 255));
}

void CScriptDebris::Render(const CStateManager& mgr) const { CPhysicsActor::Render(mgr); }

void CScriptDebris::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (!x2d4_particleGen0.null()) {
    if (x270_curTime < x274_duration || x281_26_deferDeleteTillParticle1Done) {
      gpRender->AddParticleGen(*x2d4_particleGen0);
    }
  }
  if (!x2d8_particleGen1.null()) {
    if (x270_curTime < x274_duration || x281_28_deferDeleteTillParticle2Done) {
      gpRender->AddParticleGen(*x2d8_particleGen1);
    }
  }
  if (x281_29_particle3Active) {
    gpRender->AddParticleGen(*x2dc_particleGen2);
  }
  if (HasModelData() && x270_curTime < x274_duration) {
    CActor::AddToRenderer(frustum, mgr);
  }
}

ENTITY_ACCEPT_IMPL(CScriptDebris)

void CScriptDebris::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                 CStateManager& mgr) {
  if (list.GetCount() == 0) {
    return;
  }
  if (x282_24_noBounce) {
    x274_duration = x270_curTime;
    SetVelocityWR(CVector3f::Zero());
  }
  x2c8_collisionNormal = list[0].GetNormalLeft();
}
