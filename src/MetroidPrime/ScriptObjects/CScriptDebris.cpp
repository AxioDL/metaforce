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
, mVelocity(velocity)
, mColor(1.f, 0.5f, 0.5f, 1.f)
, mEndsColor(endsColor)
, mZImpulse(zImpulse)
, mCurTime(0.f)
, mDuration(duration >= 0.f ? duration : 0.5f)
, mOoDuration(1.f / mDuration)
, mRestitution(restitution)
, mScaleType(scaleType)
, mRandomAngImpulse(randomAngImpulse)
, mParticle1GlobalTranslation(false)
, mDeferDeleteTillParticle1Done(false)
, mParticle2GlobalTranslation(false)
, mDeferDeleteTillParticle2Done(false)
, mParticle3Active(false)
, mDebrisExtended(false)
, mDieOnProjectile(false)
, mNoBounce(false)
, mParticleOr0(kOT_NotOriented)
, mParticleOr1(kOT_NotOriented)
, mParticleOr2(kOT_NotOriented)
, mLinConeAngle(0.f)
, mLinMinMag(0.f)
, mLinMaxMag(0.f)
, mAngMinMag(0.f)
, mAngMaxMag(0.f)
, mMinDuration(0.f)
, mMaxDuration(0.f)
, mColorInT(0.f)
, mColorOutT(0.f)
, mScaleOutStartT(0.f)
, mScale(CVector3f(mData.GetScale()))
, mEndScale(scaleType == kST_NoScale      ? CVector3f(mData.GetScale())
                : scaleType == kST_EndsToZero ? CVector3f::Zero()
                                              : CVector3f(5.f, 5.f, 5.f))
, mCollisionNormal(CVector3f::Zero())
, mParticleGen0(nullptr)
, mParticleGen1(nullptr)
, mParticleGen2(nullptr)
, mSpeedAvg(2.f) {
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
    mParticleGen0 = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen0->SetGlobalScale(particleScale);
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
, mVelocity(CVector3f::Zero())
, mColor(color)
, mEndsColor(endsColor)
, mZImpulse(0.f)
, mCurTime(0.f)
, mDuration(0.f)
, mOoDuration(0.f)
, mRestitution(restitution)
, mScaleType(kST_NoScale)
, mRandomAngImpulse(false)
, mParticle1GlobalTranslation(particle1GlobalTranslation)
, mDeferDeleteTillParticle1Done(deferDeleteTillParticle1Done)
, mParticle2GlobalTranslation(particle2GlobalTranslation)
, mDeferDeleteTillParticle2Done(deferDeleteTillParticle2Done)
, mParticle3Active(false)
, mDebrisExtended(true)
, mDieOnProjectile(dieOnProjectile)
, mNoBounce(noBounce)
, mParticleOr0(particleOr0)
, mParticleOr1(particleOr1)
, mParticleOr2(particleOr2)
, mLinConeAngle(linConeAngle)
, mLinMinMag(linMinMag)
, mLinMaxMag(linMaxMag)
, mAngMinMag(angMinMag)
, mAngMaxMag(angMaxMag)
, mMinDuration(minDuration)
, mMaxDuration(maxDuration)
, mColorInT(colorInT / 100.f)
, mColorOutT(colorOutT / 100.f)
, mScaleOutStartT(scaleOutStartT / 100.f)
, mScale(scale)
, mEndScale(CVector3f::ByElementMultiply(scale, endScale))
, mCollisionNormal(CVector3f::Zero())
, mParticleGen0(nullptr)
, mParticleGen1(nullptr)
, mParticleGen2(nullptr)
, mSpeedAvg(2.f) {
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
    mParticleGen0 = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen0->SetGlobalScale(particle0Scale);
  }
  if (gpResourceFactory->GetResourceTypeById(particle1) != 0) {
    TToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', particle1));
    mParticleGen1 = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen1->SetGlobalScale(particle1Scale);
  }
  if (gpResourceFactory->GetResourceTypeById(particle2) != 0) {
    TToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', particle2));
    mParticleGen2 = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mParticleGen2->SetGlobalScale(particle2Scale);
  }
  const float mass = GetMass();
  SetMomentumWR(CVector3f(0.f, 0.f, -downwardSpeed * mass));
}

CScriptDebris::~CScriptDebris() {}

void CScriptDebris::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  mCurTime += dt;
  bool done = mCurTime >= mDuration;

  if (!mParticleGen0.null()) {
    if (mCurTime >= mDuration) {
      mParticleGen0->SetParticleEmission(false);
    } else {
      if (mParticle1GlobalTranslation) {
        mParticleGen0->SetGlobalTranslation(GetTranslation());
      } else {
        mParticleGen0->SetTranslation(GetTranslation());
      }
      if (mParticleOr0 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          mParticleGen0->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
      } else if (mParticleOr0 == kOT_ToObject) {
        mParticleGen0->SetOrientation(GetTransform().GetRotation());
      }
    }
    if (mDeferDeleteTillParticle1Done && mParticleGen0->GetParticleCount() != 0) {
      done = false;
    }
    if (mCurTime < mDuration || mDeferDeleteTillParticle1Done) {
      mParticleGen0->Update(dt);
    }
  }

  if (!mParticleGen1.null()) {
    if (mCurTime >= mDuration) {
      mParticleGen1->SetParticleEmission(false);
    } else {
      if (mParticle2GlobalTranslation) {
        mParticleGen1->SetGlobalTranslation(GetTranslation());
      } else {
        mParticleGen1->SetTranslation(GetTranslation());
      }
      if (mParticleOr1 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          mParticleGen1->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
      } else if (mParticleOr1 == kOT_ToObject) {
        mParticleGen1->SetOrientation(GetTransform().GetRotation());
      }
    }
    if (mDeferDeleteTillParticle2Done && mParticleGen1->GetParticleCount() != 0) {
      done = false;
    }
    if (mCurTime < mDuration || mDeferDeleteTillParticle2Done) {
      mParticleGen1->Update(dt);
    }
  }

  if (!mParticleGen2.null()) {
    if (mCurTime >= mDuration && !mParticle3Active) {
      mParticleGen2->SetGlobalTranslation(GetTranslation());
      if (mParticleOr2 == kOT_AlongVelocity) {
        if (GetVelocityWR().CanBeNormalized()) {
          const CVector3f velocity = GetVelocityWR().AsNormalized();
          const CVector3f up = CMath::AbsF(velocity.GetZ()) < 0.99f ? CVector3f(0.f, 0.f, 1.f)
                                                                    : CVector3f(0.f, 1.f, 0.f);
          mParticleGen2->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), velocity, up));
        }
      } else if (mParticleOr2 == kOT_ToObject) {
        mParticleGen2->SetOrientation(GetTransform().GetRotation());
      } else if (mParticleOr2 == kOT_AlongCollisionNormal) {
        if (mCollisionNormal.MagSquared() == 0.f) {
          mCollisionNormal = CVector3f::Up();
        }
        const CTransform4f orientation = CTransform4f::LookAt(
            CVector3f::Zero(), mCollisionNormal,
            CMath::AbsF(CVector3f::Dot(CVector3f::Up(), mCollisionNormal)) > 0.99f
                ? CVector3f::Right()
                : CVector3f::Up());
        mParticleGen2->SetOrientation(orientation);
      }
      mParticle3Active = true;
    }
    if (mParticle3Active) {
      mParticleGen2->Update(dt);
      if (!mParticleGen2->IsSystemDeletable()) {
        done = false;
      }
    }
  }

  if (HasModelData()) {
    const float t = mCurTime / mDuration > mScaleOutStartT
                        ? (mCurTime - mDuration * mScaleOutStartT) /
                              (mDuration * (1.f - mScaleOutStartT))
                        : 0.f;
    ModelData()->SetScale(CVector3f::Lerp(mScale, mEndScale, t));
  }
  if (mCurTime >= mDuration) {
    SetMomentumWR(CVector3f::Zero());
    SetMaterialFilter(CMaterialFilter::MakeExclude(
        CMaterialList(kMT_Debris, kMT_Character, kMT_Player, kMT_Projectile)));
    if (done) {
      mgr.DeleteObjectRequest(GetUniqueId());
      return;
    }
  }
  if (mMovable) {
    mSpeedAvg.AddValue(GetVelocityWR().Magnitude());
    if (*mSpeedAvg.GetAverage() < 0.1f) {
      mMovable = false;
    }
  }
}

void CScriptDebris::Touch(CActor& other, CStateManager& mgr) {
  if (!mDieOnProjectile) {
    return;
  }
  if (TCastToPtr< CGameProjectile >(other)) {
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

rstl::optional_object< CAABox > CScriptDebris::GetTouchBounds() const {
  if (mDieOnProjectile) {
    return GetBoundingBox();
  }
  return rstl::optional_object_null();
}

void CScriptDebris::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                    CStateManager& mgr) {
  switch (msg) {
  case kSM_Activate:
    if (!mDebrisExtended) {
      const float mass = GetMass();
      const float zRand = debris_frand(mgr);
      const float z = mass * mVelocity.GetZ() * CMath::AbsF(zRand) + mZImpulse;
      const float yRand = debris_frand(mgr);
      const float yScale = mass * mVelocity.GetY();
      const float y = yScale * yRand;
      const float xRand = debris_frand(mgr);
      const float xScale = mass * mVelocity.GetX();
      const float x = xScale * xRand;
      const CVector3f linImpulse = GetTransform().GetColumn(kDZ) + CVector3f(x, y, z);
      CAxisAngle angImpulse = CAxisAngle::Identity();
      if (mRandomAngImpulse) {
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
          debris_cone(mgr, mLinConeAngle, mLinMinMag, mLinMaxMag);
      const CAxisAngle angImpulse(debris_cone(mgr, 360.f, mAngMinMag, mAngMaxMag));
      ApplyImpulseOR(linImpulse, angImpulse);
      mDuration = debris_frand_range(mgr, mMinDuration, mMaxDuration);
    }
    if (!mParticleGen0.null()) {
      mParticleGen0->SetParticleEmission(true);
    }
    if (!mParticleGen1.null()) {
      mParticleGen1->SetParticleEmission(true);
    }
    break;
  case kSM_OnFloor:
    if (!mNoBounce) {
      const CVector3f linImpulse = -mRestitution * GetConstantForceWR();
      const CAxisAngle angImpulse = -mRestitution * GetAngularMomentumWR();
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
  float t = mCurTime / mDuration;
  if (t < mColorInT) {
    t = mColorInT > 0.f ? 1.f - mCurTime / (mDuration * mColorInT) : 0.f;
  } else if (t > mColorOutT) {
    t = (mCurTime - mDuration * mColorOutT) / (mDuration * (1.f - mColorOutT));
  } else {
    t = 0.f;
  }
  const CColor color = CColor::Lerp(CColor::White(), mEndsColor, t);
  SetModelFlags(
      CModelFlags::AlphaBlended(color).DepthCompareUpdate(true, color.GetAlphau8() == 255));
}

void CScriptDebris::Render(const CStateManager& mgr) const { CPhysicsActor::Render(mgr); }

void CScriptDebris::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (!mParticleGen0.null()) {
    if (mCurTime < mDuration || mDeferDeleteTillParticle1Done) {
      gpRender->AddParticleGen(*mParticleGen0);
    }
  }
  if (!mParticleGen1.null()) {
    if (mCurTime < mDuration || mDeferDeleteTillParticle2Done) {
      gpRender->AddParticleGen(*mParticleGen1);
    }
  }
  if (mParticle3Active) {
    gpRender->AddParticleGen(*mParticleGen2);
  }
  if (HasModelData() && mCurTime < mDuration) {
    CActor::AddToRenderer(frustum, mgr);
  }
}

ENTITY_ACCEPT_IMPL(CScriptDebris)

void CScriptDebris::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                 CStateManager& mgr) {
  if (list.GetCount() == 0) {
    return;
  }
  if (mNoBounce) {
    mDuration = mCurTime;
    SetVelocityWR(CVector3f::Zero());
  }
  mCollisionNormal = list[0].GetNormalLeft();
}
