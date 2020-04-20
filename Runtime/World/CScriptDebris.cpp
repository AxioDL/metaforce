#include "Runtime/World/CScriptDebris.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Collision/CCollisionInfoList.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptDebris::CScriptDebris(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             CModelData&& mData, const CActorParameters& aParams, CAssetId particleId,
                             const zeus::CVector3f& particleScale, float zImpulse, const zeus::CVector3f& velocity,
                             const zeus::CColor& endsColor, float mass, float restitution, float duration,
                             EScaleType scaleType, bool b1, bool randomAngImpulse, bool active)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData),
                CMaterialList(EMaterialTypes::Solid, EMaterialTypes::Debris), mData.GetBounds(xf.getRotation()),
                SMoverData(mass), aParams, 0.3f, 0.1f)
, x258_velocity(velocity)
, x264_color(1.f, 0.5f, 0.5f, 1.f)
, x268_endsColor(endsColor)
, x26c_zImpulse(zImpulse)
, x274_duration(duration >= 0.f ? duration : 0.5f)
, x278_ooDuration(1.f / x274_duration)
, x27c_restitution(restitution)
, x280_scaleType(scaleType)
, x281_24_randomAngImpulse(randomAngImpulse)
, x2b0_scale(mData.GetScale())
, x2e0_speedAvg(2.f) {
  if (scaleType == EScaleType::NoScale)
    x2bc_endScale = mData.GetScale();
  else if (scaleType == EScaleType::EndsToZero)
    x2bc_endScale = zeus::skZero3f;
  else
    x2bc_endScale.splat(5.f);

  xe7_30_doTargetDistanceTest = false;

  if (x90_actorLights)
    x90_actorLights->SetAreaUpdateFramePeriod(x90_actorLights->GetAreaUpdateFramePeriod() * 2);

  SetUseInSortedLists(false);

  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid},
      {EMaterialTypes::Debris, EMaterialTypes::Character, EMaterialTypes::Player, EMaterialTypes::Projectile}));

  if (g_ResFactory->GetResourceTypeById(particleId).IsValid()) {
    TToken<CGenDescription> desc = g_SimplePool->GetObj({FOURCC('PART'), particleId});
    x2d4_particleGens[0] = std::make_unique<CElementGen>(desc, CElementGen::EModelOrientationType::Normal,
                                                         CElementGen::EOptionalSystemFlags::One);
    x2d4_particleGens[0]->SetGlobalScale(particleScale);
  }

  x150_momentum = zeus::CVector3f(0.f, 0.f, -CPhysicsActor::GravityConstant() * xe8_mass);

  if (x90_actorLights)
    x90_actorLights->SetAmbienceGenerated(true);
}

CScriptDebris::CScriptDebris(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             CModelData&& mData, const CActorParameters& aParams, float linConeAngle, float linMinMag,
                             float linMaxMag, float angMinMag, float angMaxMag, float minDuration, float maxDuration,
                             float colorInT, float colorOutT, const zeus::CColor& color, const zeus::CColor& endsColor,
                             float scaleOutStartT, const zeus::CVector3f& scale, const zeus::CVector3f& endScale,
                             float restitution, float downwardSpeed, const zeus::CVector3f& localOffset,
                             CAssetId particle1, const zeus::CVector3f& particle1Scale, bool particle1GlobalTranslation,
                             bool deferDeleteTillParticle1Done, EOrientationType particle1Or, CAssetId particle2,
                             const zeus::CVector3f& particle2Scale, bool particle2GlobalTranslation,
                             bool deferDeleteTillParticle2Done, EOrientationType particle2Or, CAssetId particle3,
                             const zeus::CVector3f& particle3Scale, EOrientationType particle3Or, bool solid,
                             bool dieOnProjectile, bool noBounce, bool active)
: CPhysicsActor(
      uid, active, name, info, xf, std::move(mData), CMaterialList(EMaterialTypes::Solid, EMaterialTypes::Debris),
      (mData.HasAnimData() || mData.HasNormalModel() ? mData.GetBounds(xf.getRotation()) : zeus::CAABox{-0.5f, 0.5f}),
      SMoverData(1.f), aParams, 0.3f, 0.1f)
, x264_color(color)
, x268_endsColor(endsColor)
, x27c_restitution(restitution)
, x281_24_randomAngImpulse(false)
, x281_25_particle1GlobalTranslation(particle1GlobalTranslation)
, x281_26_deferDeleteTillParticle1Done(deferDeleteTillParticle1Done)
, x281_27_particle2GlobalTranslation(particle2GlobalTranslation)
, x281_28_deferDeleteTillParticle2Done(deferDeleteTillParticle2Done)
, x281_29_particle3Active(false)
, x281_30_debrisExtended(true)
, x281_31_dieOnProjectile(false)
, x282_24_noBounce(noBounce)
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
, x2bc_endScale(scale * endScale)
, x2e0_speedAvg(2.f) {
  x283_particleOrs[0] = particle1Or;
  x283_particleOrs[1] = particle2Or;
  x283_particleOrs[2] = particle3Or;

  SetUseInSortedLists(false);

  SetTranslation(x34_transform.rotate(localOffset) + x34_transform.origin);

  if (solid) {
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        {EMaterialTypes::Solid}, {EMaterialTypes::Debris, EMaterialTypes::Character, EMaterialTypes::Player}));
  } else {
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude({}, {EMaterialTypes::Debris, EMaterialTypes::Character,
                                                               EMaterialTypes::Player, EMaterialTypes::Projectile,
                                                               EMaterialTypes::Solid}));
  }

  if (g_ResFactory->GetResourceTypeById(particle1).IsValid()) {
    TToken<CGenDescription> desc = g_SimplePool->GetObj({FOURCC('PART'), particle1});
    x2d4_particleGens[0] = std::make_unique<CElementGen>(desc, CElementGen::EModelOrientationType::Normal,
                                                         CElementGen::EOptionalSystemFlags::One);
    x2d4_particleGens[0]->SetGlobalScale(particle1Scale);
  }

  if (g_ResFactory->GetResourceTypeById(particle2).IsValid()) {
    TToken<CGenDescription> desc = g_SimplePool->GetObj({FOURCC('PART'), particle2});
    x2d4_particleGens[1] = std::make_unique<CElementGen>(desc, CElementGen::EModelOrientationType::Normal,
                                                         CElementGen::EOptionalSystemFlags::One);
    x2d4_particleGens[1]->SetGlobalScale(particle2Scale);
  }

  if (g_ResFactory->GetResourceTypeById(particle3).IsValid()) {
    TToken<CGenDescription> desc = g_SimplePool->GetObj({FOURCC('PART'), particle3});
    x2d4_particleGens[2] = std::make_unique<CElementGen>(desc, CElementGen::EModelOrientationType::Normal,
                                                         CElementGen::EOptionalSystemFlags::One);
    x2d4_particleGens[2]->SetGlobalScale(particle3Scale);
  }

  x150_momentum = zeus::CVector3f(0.f, 0.f, -downwardSpeed * xe8_mass);
}

void CScriptDebris::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptDebris::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (x2d4_particleGens[0]) {
    if (x270_curTime < x274_duration || x281_26_deferDeleteTillParticle1Done) {
      g_Renderer->AddParticleGen(*x2d4_particleGens[0]);
    }
  }

  if (x2d4_particleGens[1]) {
    if (x270_curTime < x274_duration || x281_28_deferDeleteTillParticle2Done) {
      g_Renderer->AddParticleGen(*x2d4_particleGens[1]);
    }
  }

  if (x281_29_particle3Active) {
    g_Renderer->AddParticleGen(*x2d4_particleGens[2]);
  }

  if (x64_modelData && !x64_modelData->IsNull()) {
    if (x270_curTime < x274_duration) {
      CActor::AddToRenderer(frustum, mgr);
    }
  }
}

static zeus::CVector3f debris_cone(CStateManager& mgr, float coneAng, float minMag, float maxMag) {
  float mag = mgr.GetActiveRandom()->Float() * (maxMag - minMag) + minMag;
  float side = 1.f - (1.f - std::cos(zeus::degToRad(coneAng * 0.5f))) * mgr.GetActiveRandom()->Float();
  float hyp = std::max(0.f, 1.f - side * side);
  if (hyp != 0.f)
    hyp = std::sqrt(hyp);
  hyp *= mag;
  float ang = mgr.GetActiveRandom()->Float() * 2.f * M_PIF;
  return {std::cos(ang) * hyp, std::sin(ang) * hyp, mag * side};
}

void CScriptDebris::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate:
    if (!x281_30_debrisExtended) {
      zeus::CVector3f linImpulse;
      linImpulse.z() =
          std::fabs(mgr.GetActiveRandom()->Next() % 32767 / 16383.5f - 1.f) * xe8_mass * x258_velocity.z() +
          x26c_zImpulse;
      linImpulse.y() = (mgr.GetActiveRandom()->Next() % 32767 / 16383.5f - 1.f) * xe8_mass * x258_velocity.y();
      linImpulse.x() = (mgr.GetActiveRandom()->Next() % 32767 / 16383.5f - 1.f) * xe8_mass * x258_velocity.x();
      linImpulse += x34_transform.basis[2];
      zeus::CAxisAngle angImpulse;
      if (x281_24_randomAngImpulse) {
        if (mgr.GetActiveRandom()->Next() % 100 < 50) {
          zeus::CVector3f rotVec;
          rotVec.x() = (mgr.GetActiveRandom()->Next() % 32767 / 16383.5f - 1.f) * 45.f;
          rotVec.y() = (mgr.GetActiveRandom()->Next() % 32767 / 16383.5f - 1.f) * 15.f;
          rotVec.z() = (mgr.GetActiveRandom()->Next() % 32767 / 16383.5f - 1.f) * 35.f;
          angImpulse = zeus::CAxisAngle(rotVec);
        }
      }
      ApplyImpulseWR(linImpulse, angImpulse);
    } else {
      zeus::CVector3f linImpulse = debris_cone(mgr, x288_linConeAngle, x28c_linMinMag, x290_linMaxMag);
      zeus::CVector3f angImpulse = debris_cone(mgr, 360.f, x294_angMinMag, x298_angMaxMag);
      ApplyImpulseOR(linImpulse, angImpulse);
      x274_duration = mgr.GetActiveRandom()->Float() * (x2a0_maxDuration - x29c_minDuration) + x29c_minDuration;
    }

    if (x2d4_particleGens[0])
      x2d4_particleGens[0]->SetParticleEmission(true);
    if (x2d4_particleGens[1])
      x2d4_particleGens[1]->SetParticleEmission(true);
    break;
  case EScriptObjectMessage::OnFloor:
    if (!x282_24_noBounce)
      ApplyImpulseWR(-x27c_restitution * xfc_constantForce, -x27c_restitution * x108_angularMomentum);
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CScriptDebris::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  x270_curTime += dt;
  bool done = x270_curTime >= x274_duration;

  if (x2d4_particleGens[0]) {
    if (x270_curTime >= x274_duration) {
      x2d4_particleGens[0]->SetParticleEmission(false);
    } else {
      if (x281_25_particle1GlobalTranslation)
        x2d4_particleGens[0]->SetGlobalTranslation(GetTranslation());
      else
        x2d4_particleGens[0]->SetTranslation(GetTranslation());

      if (x283_particleOrs[0] == EOrientationType::AlongVelocity) {
        if (x138_velocity.canBeNormalized()) {
          zeus::CVector3f normVel = x138_velocity.normalized();
          zeus::CTransform orient =
              zeus::lookAt(zeus::skZero3f, normVel,
                           std::fabs(normVel.z()) < 0.99f ? zeus::skUp : zeus::skForward);
          x2d4_particleGens[0]->SetOrientation(orient);
        }
      } else if (x283_particleOrs[0] == EOrientationType::ToObject) {
        x2d4_particleGens[0]->SetOrientation(x34_transform.getRotation());
      }
    }

    if (x281_26_deferDeleteTillParticle1Done && x2d4_particleGens[0]->GetParticleCount() != 0)
      done = false;

    if (x270_curTime < x274_duration || x281_26_deferDeleteTillParticle1Done)
      x2d4_particleGens[0]->Update(dt);
  }

  if (x2d4_particleGens[1]) {
    if (x270_curTime >= x274_duration) {
      x2d4_particleGens[1]->SetParticleEmission(false);
    } else {
      if (x281_27_particle2GlobalTranslation)
        x2d4_particleGens[1]->SetGlobalTranslation(GetTranslation());
      else
        x2d4_particleGens[1]->SetTranslation(GetTranslation());

      if (x283_particleOrs[1] == EOrientationType::AlongVelocity) {
        if (x138_velocity.canBeNormalized()) {
          zeus::CVector3f normVel = x138_velocity.normalized();
          zeus::CTransform orient =
              zeus::lookAt(zeus::skZero3f, normVel,
                           std::fabs(normVel.z()) < 0.99f ? zeus::skUp : zeus::skForward);
          x2d4_particleGens[1]->SetOrientation(orient);
        }
      } else if (x283_particleOrs[1] == EOrientationType::ToObject) {
        x2d4_particleGens[1]->SetOrientation(x34_transform.getRotation());
      }
    }

    if (x281_28_deferDeleteTillParticle2Done && x2d4_particleGens[1]->GetParticleCount() != 0)
      done = false;

    if (x270_curTime < x274_duration || x281_28_deferDeleteTillParticle2Done)
      x2d4_particleGens[1]->Update(dt);
  }

  /* End particle */
  if (x2d4_particleGens[2]) {
    if (x270_curTime >= x274_duration && !x281_29_particle3Active) {
      x2d4_particleGens[2]->SetGlobalTranslation(GetTranslation());

      if (x283_particleOrs[2] == EOrientationType::AlongVelocity) {
        if (x138_velocity.canBeNormalized()) {
          zeus::CVector3f normVel = x138_velocity.normalized();
          zeus::CTransform orient =
              zeus::lookAt(zeus::skZero3f, normVel,
                           std::fabs(normVel.z()) < 0.99f ? zeus::skUp : zeus::skForward);
          x2d4_particleGens[2]->SetOrientation(orient);
        }
      } else if (x283_particleOrs[2] == EOrientationType::ToObject) {
        x2d4_particleGens[2]->SetOrientation(x34_transform.getRotation());
      } else if (x283_particleOrs[2] == EOrientationType::AlongCollisionNormal) {
        if (x2c8_collisionNormal.magSquared() == 0.f)
          x2c8_collisionNormal = zeus::skUp;
        zeus::CTransform orient =
            zeus::lookAt(zeus::skZero3f, x2c8_collisionNormal,
                         std::fabs(x2c8_collisionNormal.dot(zeus::skUp)) > 0.99f ? zeus::skRight
                                                                                            : zeus::skUp);
        x2d4_particleGens[2]->SetOrientation(orient);
      }

      x281_29_particle3Active = true;
    }

    if (x281_29_particle3Active) {
      x2d4_particleGens[2]->Update(dt);
      if (!x2d4_particleGens[2]->IsSystemDeletable())
        done = false;
    }
  }

  if (x64_modelData && !x64_modelData->IsNull()) {
    float t = x270_curTime / x274_duration > x2ac_scaleOutStartT
                  ? (x270_curTime - x2ac_scaleOutStartT * x274_duration) / ((1.f - x2ac_scaleOutStartT) * x274_duration)
                  : 0.f;
    x64_modelData->SetScale(zeus::CVector3f::lerp(x2b0_scale, x2bc_endScale, t));
  }

  if (x270_curTime >= x274_duration) {
    x150_momentum = zeus::skZero3f;
    SetMaterialFilter(CMaterialFilter::MakeExclude(
        {EMaterialTypes::Debris, EMaterialTypes::Character, EMaterialTypes::Player, EMaterialTypes::Projectile}));

    if (done) {
      mgr.FreeScriptObject(x8_uid);
      return;
    }
  }

  if (xf8_24_movable) {
    float speed = x138_velocity.magnitude();
    x2e0_speedAvg.AddValue(speed);
    if (x2e0_speedAvg.GetAverage() < 0.1f)
      xf8_24_movable = false;
  }
}

void CScriptDebris::Touch(CActor& other, CStateManager& mgr) {
  if (x281_31_dieOnProjectile) {
    if (TCastToPtr<CGameProjectile>(other)) {
      SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
      mgr.FreeScriptObject(x8_uid);
    }
  }
}

std::optional<zeus::CAABox> CScriptDebris::GetTouchBounds() const {
  if (x281_31_dieOnProjectile) {
    return GetBoundingBox();
  }
  return std::nullopt;
}

void CScriptDebris::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CActor::PreRender(mgr, frustum);

  float t = x270_curTime / x274_duration;
  if (t < x2a4_colorInT)
    t = x2a4_colorInT > 0.f ? 1.f - x270_curTime / (x274_duration * x2a4_colorInT) : 0.f;
  else if (t > x2a8_colorOutT)
    t = (x270_curTime - x274_duration * x2a8_colorOutT) / (x274_duration * (1.f - x2a8_colorOutT));
  else
    t = 0.f;

  xb4_drawFlags = CModelFlags(5, 0, 3, zeus::CColor::lerp(zeus::skWhite, x268_endsColor, t));
}

void CScriptDebris::Render(CStateManager& mgr) { CPhysicsActor::Render(mgr); }

void CScriptDebris::CollidedWith(TUniqueId, const CCollisionInfoList& colList, CStateManager&) {
  if (colList.GetCount() == 0)
    return;

  if (x282_24_noBounce) {
    x274_duration = x270_curTime;
    SetVelocityWR(zeus::skZero3f);
  } else {
    x2c8_collisionNormal = colList.GetItem(0).GetNormalLeft();
  }
}

} // namespace urde
