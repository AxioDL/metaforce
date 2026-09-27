#include "MetroidPrime/Enemies/CGrenadeLauncher.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "rstl/math.hpp"
#include <float.h>

static const CMaterialList skLauncherMaterial(kMT_Character, kMT_Solid);
static const int skLauncherAnims[] = {0, 3};
static const char* const skGrenadeLocator = "grenade_LCTR";
static const char* const skLockOnLocator = "lockon_target_LCTR";
const float CGrenadeLauncher::skMaxWeight = 0.5f;

CGrenadeLauncher::CGrenadeLauncher(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                   const CTransform4f& xf, const CModelData& mData,
                                   const CAABox& bounds, const CHealthInfo& healthInfo,
                                   const CDamageVulnerability& vulnerability,
                                   const CActorParameters& actParams, TUniqueId parentId,
                                   const CEPGrenadeLauncherData& data, float explodePlayerDistance)
: CPhysicsActor(uid, true, name, info, xf, mData, skLauncherMaterial, bounds, SMoverData(1000.f),
                actParams, 0.3f, 0.1f)
, mStarted(0)
, mHealthInfo(healthInfo)
, mVulnerability(vulnerability)
, mParentId(parentId)
, mData(data)
, mCSphere(CSphere(CVector3f(0.f, 0.f, 0.f), mData.ScaleCopy().GetZ()), skLauncherMaterial)
, mShotTimer(-1.f)
, mColor(1.f, 1.f, 1.f, 1.f)
, mGrenadeActorParams(actParams)
, mParticleGenDesc(data.GetShootParticleGenDescId() != kInvalidAssetId
                           ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                 TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                     SObjectTag('PART', data.GetShootParticleGenDescId()))))
                           : rstl::optional_object< TLockedToken< CGenDescription > >())
, mYaw(0.f)
, mYawVelocity(0.f)
, mPitch(0.f)
, mPitchVelocity(0.f)
, mThermalMag(actParams.GetThermalMag())
, mDamageTimer(0.f)
, mDamageColor(0.5f, 0.f, 0.f, 1.f)
, mDamageAddColor(0.f, 0.f, 0.f, 1.f)
, mExplodePlayerDistance(explodePlayerDistance)
, mLaunchGrenade(false)
, mVisible(true)
, mFollowPlayer(true) {
  ModelData()->EnableLooping(true);
  for (int i = 0; i < 4; ++i) {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(i));
    const rstl::pair< float, int > anim =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
    mAnimIds[i] = anim.second;
  }
}

CGrenadeLauncher::~CGrenadeLauncher() {}

void CGrenadeLauncher::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (mLaunchGrenade) {
      LaunchGrenadeProjectile(mgr);
      mLaunchGrenade = false;
    }
    UpdateCollisionPrimitive();
    UpdateGrenadeLauncherDamageTime(dt);
    UpdateGunTracking(dt, mgr);
    UpdateHitDamageTime(dt);
    const CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.GetOffsetDelta(), dt);
    RotateToOR(deltas.GetOrientationDelta(), dt);
    CPatterned* parent = TCastToPtr< CPatterned >(mgr.ObjectById(mParentId));
    if (!parent || !parent->IsAlive() || HealthInfo(mgr)->GetHP() <= 0.f) {
      mgr.DeliverScriptMsg(parent, GetUniqueId(), kSM_Damage);
      StartExplosionEffect(mgr);
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  }
}

void CGrenadeLauncher::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                       CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    UpdateLauncherAnimation();
    break;
  case kSM_Start:
    if (uid == mParentId && mStarted != 1) {
      mStarted = 1;
      UpdateLauncherAnimation();
    }
    break;
  case kSM_Stop:
    if (uid == mParentId && mStarted != 0) {
      mStarted = 0;
      UpdateLauncherAnimation();
    }
    break;
  case kSM_Action:
    if (uid == mParentId && mStarted == 1)
      mLaunchGrenade = true;
    break;
  case kSM_Damage:
    mDamageTimer = 0.33f;
    break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CGrenadeLauncher)

void CGrenadeLauncher::Render(const CStateManager& mgr) const {
  if (mVisible)
    CPhysicsActor::Render(mgr);
}

void CGrenadeLauncher::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (mDamageAddColor.GetAlphau8() == 255) {
    SetModelFlags(CModelFlags(CModelFlags::kT_Two, CColor(mDamageAddColor.GetRedu8(),
                                                          mDamageAddColor.GetGreenu8(),
                                                          mDamageAddColor.GetBlueu8(), 255)));
  } else {
    SetModelFlags(CModelFlags::AlphaBlendedDepthCompareUpdate(mDamageAddColor, true, true));
  }
  CActor::PreRender(mgr, frustum);
}

void CGrenadeLauncher::AddToRenderer(const CFrustumPlanes& frustum,
                                     const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);
}

void CGrenadeLauncher::Touch(CActor& act, CStateManager& mgr) {
  if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(act)) {
    if (projectile->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
      const CDamageVulnerability* vulnerability = GetDamageVulnerability();
      const CWeaponMode mode(projectile->GetType());
      if (vulnerability->WeaponHurts(mode, CDamageVulnerability::kRD_No)) {
        mShotTimer = 0.5f;
        CEntity* parent = mgr.ObjectById(mParentId);
        if (parent)
          mgr.DeliverScriptMsg(parent, GetUniqueId(), kSM_Touched);
      }
    }
  }
}

rstl::optional_object< CAABox > CGrenadeLauncher::GetTouchBounds() const {
  return mCSphere.CalculateAABox(GetTransform());
}

const CCollisionPrimitive* CGrenadeLauncher::GetCollisionPrimitive() const { return &mCSphere; }

CVector3f CGrenadeLauncher::PredictTargetPosition(const CStateManager& mgr) {
  CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, 1.f);
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    aim -= CVector3f(0.f, 0.f, 0.5f * mgr.GetPlayer()->GetEyeHeight());
  }
  return aim;
}

void CGrenadeLauncher::ComputeLaunchSpeedAndAngle(const CVector3f& target, const CVector3f& origin,
                                                  const CEPGrenadeLaunchParms& parms,
                                                  float& angleOut, float& velocityOut) {
  float angle = parms.GetAngleMin();
  float velocity = parms.GetVelocityMin();
  float bestError = FLT_MAX;
  float step = rstl::max_val(0.01f, 0.1f * (parms.GetAngleMax() - parms.GetAngleMin()));
  CVector3f dist = target - origin;
  const float height = dist.GetZ();
  CVector2f distXY = CVector2f(dist.GetX(), dist.GetY());
  float distance = distXY.Magnitude();
  float minSpeedSq = parms.GetVelocityMin() * parms.GetVelocityMin();
  float maxSpeedSq = parms.GetVelocityMax() * parms.GetVelocityMax();
  const float halfGravity = 0.5f * GravityConstant();
  for (float candidateAngle = parms.GetAngleMin(); candidateAngle <= parms.GetAngleMax();
       candidateAngle += step) {
    float cosine = CMath::FastCosR(candidateAngle);
    float sine = CMath::FastSinR(candidateAngle);
    float divisor = distance * (cosine * sine) - height * (cosine * cosine);
    if (divisor > FLT_EPSILON) {
      float speedSq = (distance * (halfGravity * distance)) / divisor;
      if (speedSq >= minSpeedSq && speedSq <= maxSpeedSq) {
        angle = candidateAngle;
        velocity = CMath::SqrtF(speedSq);
        break;
      }
      float error = speedSq > maxSpeedSq ? speedSq - maxSpeedSq : minSpeedSq - speedSq;
      if (error < bestError) {
        angle = candidateAngle;
        velocity = CMath::SqrtF(speedSq);
        bestError = error;
      }
    }
  }
  angleOut = angle;
  velocityOut = velocity;
}

void CGrenadeLauncher::UpdateLauncherAnimation() {
  if (HasAnimation() && mStarted >= 0 && mStarted <= 1) {
    const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(skLauncherAnims[mStarted]));
    const rstl::pair< float, int > anim =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
    if (anim.first > 0.f) {
      AnimationData()->SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
      ModelData()->EnableLooping(true);
    }
  }
}

void CGrenadeLauncher::LaunchGrenadeProjectile(CStateManager& mgr) {
  if (HasAnimation()) {
    const CPASAnimParmData parms(pas::kAS_AdditiveFlinch);
    const rstl::pair< float, int > anim =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    if (anim.first > 0.f) {
      AnimationData()->AddAdditiveAnimation(anim.second, 1.f, false, true);
      const CTransform4f locator = GetLocatorTransform(rstl::string_l(skGrenadeLocator));
      const CVector3f origin = GetTranslation() + GetTransform().Rotate(locator.GetTranslation());
      float angle = mData.GetLaunchParms().GetAngleMin();
      float speed = mData.GetLaunchParms().GetVelocityMin();
      const CVector3f target = PredictTargetPosition(mgr);
      ComputeLaunchSpeedAndAngle(target, origin, mData.GetLaunchParms(), angle, speed);
      CVector3f dist = target - origin;
      dist.SetZ(0.f);
      const CVector3f forward = GetTransform().GetForward();
      CVector3f direction = dist.CanBeNormalized() ? dist.AsNormalized() : forward;
      const float maxAngle = M_PIF / 4.f;
      if (CVector3f::GetAngleDiff(forward, direction) > maxAngle) {
        direction = CVector3f::Slerp(forward, direction, CRelAngle(maxAngle));
      }
      const CVector3f look = CVector3f::Slerp(direction, CVector3f::Up(), CRelAngle(angle));
      const CTransform4f xf = CTransform4f::LookAt(origin, origin + look, CVector3f::Up());
      CEntity* grenade = rs_new CBouncyGrenade(
          mgr.AllocateUniqueId(), rstl::string_l("Bouncy Grenade"),
          CEntityInfo(GetCurrentAreaId(), NullConnectionList), xf,
          CModelData(CStaticRes(mData.GetGrenadeModelId(), GetModelData()->ScaleCopy())),
          mGrenadeActorParams, mParentId, mData.GetGrenadeData(), speed,
          mExplodePlayerDistance);
      if (grenade)
        mgr.AddObject(grenade);
    }
  }
}

void CGrenadeLauncher::UpdateCollisionPrimitive() {
  const CTransform4f locator = GetLocatorTransform(rstl::string_l(skLockOnLocator));
  mCSphere.SetSphereCenter(locator.GetTranslation());
}

void CGrenadeLauncher::UpdateGunTracking(float dt, CStateManager& mgr) {
  if (HasAnimation() && mStarted == 1 && mFollowPlayer) {
    CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    target[kDZ] = GetTransform().GetTranslation()[kDZ];
    const CVector3f position = GetTranslation();
    const CVector3f targetDelta =
        CVector3f(target.GetX() - position.GetX(), target.GetY() - position.GetY(),
                  target.GetZ() - position.GetZ());
    // The explicit copy preserves the original vector temporary layout.
    const CVector3f localTarget = GetTransform().TransposeRotate(CVector3f(targetDelta));
    if (localTarget.CanBeNormalized()) {
      const float maxAngle = M_PIF / 4.f;
      const float maxSpeed = 3.f;
      const float maxAcceleration = 10.f;
      float yaw = atan2f(localTarget.GetX(), localTarget.GetY());
      yaw = CMath::Clamp(-maxAngle, yaw, maxAngle);
      yaw = (2.f / M_PIF) * yaw;
      float velocity = (0.25f * (yaw - mYaw)) / dt;
      velocity = CMath::Clamp(-maxSpeed, velocity, maxSpeed);
      float acceleration = (velocity - mYawVelocity) / dt;
      mYawVelocity += dt * CMath::Clamp(-maxAcceleration, acceleration, maxAcceleration);
      float pitch =
          atan2f(localTarget.GetZ(), CMath::SqrtF(localTarget.GetY() * localTarget.GetY() +
                                                  localTarget.GetX() * localTarget.GetX()));
      pitch = CMath::Clamp(-maxAngle, pitch, maxAngle);
      pitch = (2.f / M_PIF) * pitch;
      velocity = (0.25f * (pitch - mPitch)) / dt;
      velocity = CMath::Clamp(-maxSpeed, velocity, maxSpeed);
      acceleration = (velocity - mPitchVelocity) / dt;
      mPitchVelocity += dt * CMath::Clamp(-maxAcceleration, acceleration, maxAcceleration);
      const float nextYaw =
          CMath::Clamp(-skMaxWeight, dt * mYawVelocity + mYaw, skMaxWeight);
      const float nextPitch =
          CMath::Clamp(-skMaxWeight, dt * mPitchVelocity + mPitch, skMaxWeight);
      CAnimData* animData = AnimationData();
      if (nextYaw != mYaw) {
        float weight = CMath::AbsF(nextYaw);
        if (CMath::AbsF(mYaw) > 0.f && mYaw * nextYaw <= 0.f)
          animData->DelAdditiveAnimation(mAnimIds[mYaw < 0.f ? 0 : 1]);
        if (weight > 0.f)
          animData->AddAdditiveAnimation(mAnimIds[nextYaw < 0.f ? 0 : 1], weight, false, false);
      }
      if (nextPitch != mPitch) {
        float weight = CMath::AbsF(nextPitch);
        if (CMath::AbsF(mPitch) > 0.f && mPitch * nextPitch <= 0.f)
          animData->DelAdditiveAnimation(mAnimIds[mPitch > 0.f ? 2 : 3]);
        if (weight > 0.f)
          animData->AddAdditiveAnimation(mAnimIds[nextPitch > 0.f ? 2 : 3], weight, false,
                                         false);
      }
      mYaw = nextYaw;
      mPitch = nextPitch;
    }
  } else {
    CAnimData* animData = AnimationData();
    if (mYaw != 0.f) {
      animData->DelAdditiveAnimation(mAnimIds[mYaw < 0.f ? 0 : 1]);
      mYaw = 0.f;
    }
    if (mPitch != 0.f) {
      animData->DelAdditiveAnimation(mAnimIds[mPitch > 0.f ? 2 : 3]);
      mPitch = 0.f;
    }
  }
}

void CGrenadeLauncher::UpdateGrenadeLauncherDamageTime(float dt) {
  if (mShotTimer > 0.f) {
    mShotTimer = CMath::Max(0.f, mShotTimer - dt);
    const CColor& color =
        CColor::Lerp(CColor(1.f, 1.f, 1.f, 1.f), CColor(1.f, 0.f, 0.f, 1.f), mShotTimer);
    mColor.Set(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), 255);
  }
}

void CGrenadeLauncher::StartExplosionEffect(CStateManager& mgr) {
  if (mParticleGenDesc) {
    const CTransform4f xf = GetTransform();
    CExplosion* explosion = rs_new CExplosion(*mParticleGenDesc, mgr.AllocateUniqueId(), true,
                                              CEntityInfo(GetCurrentAreaId(), NullConnectionList),
                                              rstl::string_l("Grenade Launcher Explode Fx"), xf, 0,
                                              GetModelData()->ScaleCopy(), CColor::White());
    if (explosion) {
      mgr.AddObject(explosion);
      CSfxManager::SfxStart(mData.GetShootSfxId(), CAudioSys::kMaxVolume, 0x40, false,
                            CSfxManager::kMedPriority, false, CSfxManager::kAllAreas);
    }
  }
}

void CGrenadeLauncher::UpdateHitDamageTime(float dt) {
  if (mDamageTimer > 0.f) {
    mDamageTimer = CMath::Max(0.f, mDamageTimer - dt);
    mDamageAddColor = CColor::Lerp(CColor(0.f, 0.f, 0.f, 1.f), mDamageColor,
                                       CMath::Clamp(0.f, mDamageTimer / 0.33f, 1.f));
    SetDamageMag(5.f * mDamageTimer + mThermalMag);
  } else {
    SetDamageMag(mThermalMag);
  }
}
