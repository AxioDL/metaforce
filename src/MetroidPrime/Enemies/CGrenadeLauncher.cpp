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
, x258_started(0)
, x25c_healthInfo(healthInfo)
, x264_vulnerability(vulnerability)
, x2cc_parentId(parentId)
, x2d0_data(data)
, x328_cSphere(CSphere(CVector3f(0.f, 0.f, 0.f), mData.ScaleCopy().GetZ()), skLauncherMaterial)
, x348_shotTimer(-1.f)
, x34c_color(1.f, 1.f, 1.f, 1.f)
, x350_grenadeActorParams(actParams)
, x3b8_particleGenDesc(data.GetShootParticleGenDescId() != kInvalidAssetId
                           ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                 TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                     SObjectTag('PART', data.GetShootParticleGenDescId()))))
                           : rstl::optional_object< TLockedToken< CGenDescription > >())
, x3d8_yaw(0.f)
, x3dc_yawVelocity(0.f)
, x3e0_pitch(0.f)
, x3e4_pitchVelocity(0.f)
, x3e8_thermalMag(actParams.GetThermalMag())
, x3ec_damageTimer(0.f)
, x3f0_damageColor(0.5f, 0.f, 0.f, 1.f)
, x3f4_damageAddColor(0.f, 0.f, 0.f, 1.f)
, x3f8_explodePlayerDistance(explodePlayerDistance)
, x3fc_launchGrenade(false)
, x3fd_visible(true)
, x3fe_followPlayer(true) {
  ModelData()->EnableLooping(true);
  for (int i = 0; i < 4; ++i) {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(i));
    const rstl::pair< float, int > anim =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
    x3c8_animIds[i] = anim.second;
  }
}

CGrenadeLauncher::~CGrenadeLauncher() {}

void CGrenadeLauncher::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (x3fc_launchGrenade) {
      LaunchGrenadeProjectile(mgr);
      x3fc_launchGrenade = false;
    }
    UpdateCollisionPrimitive();
    UpdateGrenadeLauncherDamageTime(dt);
    UpdateGunTracking(dt, mgr);
    UpdateHitDamageTime(dt);
    const CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.GetOffsetDelta(), dt);
    RotateToOR(deltas.GetOrientationDelta(), dt);
    CPatterned* parent = TCastToPtr< CPatterned >(mgr.ObjectById(x2cc_parentId));
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
    if (uid == x2cc_parentId && x258_started != 1) {
      x258_started = 1;
      UpdateLauncherAnimation();
    }
    break;
  case kSM_Stop:
    if (uid == x2cc_parentId && x258_started != 0) {
      x258_started = 0;
      UpdateLauncherAnimation();
    }
    break;
  case kSM_Action:
    if (uid == x2cc_parentId && x258_started == 1)
      x3fc_launchGrenade = true;
    break;
  case kSM_Damage:
    x3ec_damageTimer = 0.33f;
    break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CGrenadeLauncher)

void CGrenadeLauncher::Render(const CStateManager& mgr) const {
  if (x3fd_visible)
    CPhysicsActor::Render(mgr);
}

void CGrenadeLauncher::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (x3f4_damageAddColor.GetAlphau8() == 255) {
    SetModelFlags(CModelFlags(CModelFlags::kT_Two, CColor(x3f4_damageAddColor.GetRedu8(),
                                                          x3f4_damageAddColor.GetGreenu8(),
                                                          x3f4_damageAddColor.GetBlueu8(), 255)));
  } else {
    SetModelFlags(CModelFlags::AlphaBlendedDepthCompareUpdate(x3f4_damageAddColor, true, true));
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
        x348_shotTimer = 0.5f;
        CEntity* parent = mgr.ObjectById(x2cc_parentId);
        if (parent)
          mgr.DeliverScriptMsg(parent, GetUniqueId(), kSM_Touched);
      }
    }
  }
}

rstl::optional_object< CAABox > CGrenadeLauncher::GetTouchBounds() const {
  return x328_cSphere.CalculateAABox(GetTransform());
}

const CCollisionPrimitive* CGrenadeLauncher::GetCollisionPrimitive() const { return &x328_cSphere; }

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
  if (HasAnimation() && x258_started >= 0 && x258_started <= 1) {
    const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(skLauncherAnims[x258_started]));
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
      float angle = x2d0_data.GetLaunchParms().GetAngleMin();
      float speed = x2d0_data.GetLaunchParms().GetVelocityMin();
      const CVector3f target = PredictTargetPosition(mgr);
      ComputeLaunchSpeedAndAngle(target, origin, x2d0_data.GetLaunchParms(), angle, speed);
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
          CModelData(CStaticRes(x2d0_data.GetGrenadeModelId(), GetModelData()->ScaleCopy())),
          x350_grenadeActorParams, x2cc_parentId, x2d0_data.GetGrenadeData(), speed,
          x3f8_explodePlayerDistance);
      if (grenade)
        mgr.AddObject(grenade);
    }
  }
}

void CGrenadeLauncher::UpdateCollisionPrimitive() {
  const CTransform4f locator = GetLocatorTransform(rstl::string_l(skLockOnLocator));
  x328_cSphere.SetSphereCenter(locator.GetTranslation());
}

void CGrenadeLauncher::UpdateGunTracking(float dt, CStateManager& mgr) {
  if (HasAnimation() && x258_started == 1 && x3fe_followPlayer) {
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
      float velocity = (0.25f * (yaw - x3d8_yaw)) / dt;
      velocity = CMath::Clamp(-maxSpeed, velocity, maxSpeed);
      float acceleration = (velocity - x3dc_yawVelocity) / dt;
      x3dc_yawVelocity += dt * CMath::Clamp(-maxAcceleration, acceleration, maxAcceleration);
      float pitch =
          atan2f(localTarget.GetZ(), CMath::SqrtF(localTarget.GetY() * localTarget.GetY() +
                                                  localTarget.GetX() * localTarget.GetX()));
      pitch = CMath::Clamp(-maxAngle, pitch, maxAngle);
      pitch = (2.f / M_PIF) * pitch;
      velocity = (0.25f * (pitch - x3e0_pitch)) / dt;
      velocity = CMath::Clamp(-maxSpeed, velocity, maxSpeed);
      acceleration = (velocity - x3e4_pitchVelocity) / dt;
      x3e4_pitchVelocity += dt * CMath::Clamp(-maxAcceleration, acceleration, maxAcceleration);
      const float nextYaw =
          CMath::Clamp(-skMaxWeight, dt * x3dc_yawVelocity + x3d8_yaw, skMaxWeight);
      const float nextPitch =
          CMath::Clamp(-skMaxWeight, dt * x3e4_pitchVelocity + x3e0_pitch, skMaxWeight);
      CAnimData* animData = AnimationData();
      if (nextYaw != x3d8_yaw) {
        float weight = CMath::AbsF(nextYaw);
        if (CMath::AbsF(x3d8_yaw) > 0.f && x3d8_yaw * nextYaw <= 0.f)
          animData->DelAdditiveAnimation(x3c8_animIds[x3d8_yaw < 0.f ? 0 : 1]);
        if (weight > 0.f)
          animData->AddAdditiveAnimation(x3c8_animIds[nextYaw < 0.f ? 0 : 1], weight, false, false);
      }
      if (nextPitch != x3e0_pitch) {
        float weight = CMath::AbsF(nextPitch);
        if (CMath::AbsF(x3e0_pitch) > 0.f && x3e0_pitch * nextPitch <= 0.f)
          animData->DelAdditiveAnimation(x3c8_animIds[x3e0_pitch > 0.f ? 2 : 3]);
        if (weight > 0.f)
          animData->AddAdditiveAnimation(x3c8_animIds[nextPitch > 0.f ? 2 : 3], weight, false,
                                         false);
      }
      x3d8_yaw = nextYaw;
      x3e0_pitch = nextPitch;
    }
  } else {
    CAnimData* animData = AnimationData();
    if (x3d8_yaw != 0.f) {
      animData->DelAdditiveAnimation(x3c8_animIds[x3d8_yaw < 0.f ? 0 : 1]);
      x3d8_yaw = 0.f;
    }
    if (x3e0_pitch != 0.f) {
      animData->DelAdditiveAnimation(x3c8_animIds[x3e0_pitch > 0.f ? 2 : 3]);
      x3e0_pitch = 0.f;
    }
  }
}

void CGrenadeLauncher::UpdateGrenadeLauncherDamageTime(float dt) {
  if (x348_shotTimer > 0.f) {
    x348_shotTimer = CMath::Max(0.f, x348_shotTimer - dt);
    const CColor& color =
        CColor::Lerp(CColor(1.f, 1.f, 1.f, 1.f), CColor(1.f, 0.f, 0.f, 1.f), x348_shotTimer);
    x34c_color.Set(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), 255);
  }
}

void CGrenadeLauncher::StartExplosionEffect(CStateManager& mgr) {
  if (x3b8_particleGenDesc) {
    const CTransform4f xf = GetTransform();
    CExplosion* explosion = rs_new CExplosion(*x3b8_particleGenDesc, mgr.AllocateUniqueId(), true,
                                              CEntityInfo(GetCurrentAreaId(), NullConnectionList),
                                              rstl::string_l("Grenade Launcher Explode Fx"), xf, 0,
                                              GetModelData()->ScaleCopy(), CColor::White());
    if (explosion) {
      mgr.AddObject(explosion);
      CSfxManager::SfxStart(x2d0_data.GetShootSfxId(), CAudioSys::kMaxVolume, 0x40, false,
                            CSfxManager::kMedPriority, false, CSfxManager::kAllAreas);
    }
  }
}

void CGrenadeLauncher::UpdateHitDamageTime(float dt) {
  if (x3ec_damageTimer > 0.f) {
    x3ec_damageTimer = CMath::Max(0.f, x3ec_damageTimer - dt);
    x3f4_damageAddColor = CColor::Lerp(CColor(0.f, 0.f, 0.f, 1.f), x3f0_damageColor,
                                       CMath::Clamp(0.f, x3ec_damageTimer / 0.33f, 1.f));
    SetDamageMag(5.f * x3ec_damageTimer + x3e8_thermalMag);
  } else {
    SetDamageMag(x3e8_thermalMag);
  }
}
