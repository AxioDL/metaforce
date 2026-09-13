#include "MetroidPrime/Enemies/CElitePirate.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

const int CElitePirateData::skMinProperties = 41;

const CElitePirate::SJointInfo CElitePirate::skLeftArmJointList[3] = {
    {"L_shoulder", "L_elbow", 1.f, 1.5f},
    {"L_elbow", "L_wrist", 0.9f, 1.3f},
    {"L_knee", "L_ankle", 0.9f, 1.3f},
};
const CElitePirate::SJointInfo CElitePirate::skRightArmJointList[3] = {
    {"R_shoulder", "R_elbow", 1.f, 1.5f},
    {"R_elbow", "R_wrist", 0.9f, 1.3f},
    {"R_knee", "R_ankle", 0.9f, 1.3f},
};
const CElitePirate::SSphereJointInfo CElitePirate::skSphereJointList[7] = {
    {"Head_1", 1.2f}, {"L_Palm_LCTR", 1.5f}, {"R_Palm_LCTR", 1.5f}, {"Spine_1", 1.5f},
    {"Collar", 1.2f}, {"L_ball", 0.8f},      {"R_ball", 0.8f},
};
const char* const CElitePirate::skpHeadLCTR = "Head_1";
const char* const CElitePirate::skpLauncherLCTR = "grenadeLauncher_LCTR";
const char* const CElitePirate::skpRightClawLCTR = "R_Palm_LCTR";
const char* const CElitePirate::skpLeftClawLCTR = "L_Palm_LCTR";
const char* const CElitePirate::skpGrenadeLauncherLCTR = "lockon_target_LCTR";
const CVector3f CElitePirate::skExtendedClawBounds(2.f, 2.f, 2.f);
const CVector3f CElitePirate::skLocalShieldBounds(4.f, 4.f, 2.f);

CElitePirateData::CElitePirateData(CInputStream& in, int propCount)
: x0_tauntInterval(in.ReadFloat())
, x4_tauntVariance(in.ReadFloat())
, x8_(in.ReadFloat())
, xc_(in.ReadFloat())
, x10_attackChance(in.ReadFloat())
, x14_shotAtTime(in.ReadFloat())
, x18_shotAtTimeVariance(in.ReadFloat())
, x1c_projectileAttractionRadius(in.ReadFloat())
, x20_energyAbsorbParticleDescId(in.ReadLong())
, x24_energyAbsorbSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, x28_launcherActParams(LoadActorParameters(in))
, x90_launcherAnimParams(LoadAnimationParameters(in))
, x9c_launcherParticleGenDescId(in.ReadLong())
, xa0_launcherSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, xa4_grenadeModelId(in.ReadLong())
, xa8_grenadeDamageInfo(in)
, xc4_launcherHp(in.ReadFloat())
, xc8_grenadeElementGenDescId1(in.ReadLong())
, xcc_grenadeElementGenDescId2(in.ReadLong())
, xd0_grenadeElementGenDescId3(in.ReadLong())
, xd4_grenadeElementGenDescId4(in.ReadLong())
, xd8_grenadeVelocityInfo(in)
, xe0_grenadeTrajectoryInfo(in)
, xf0_grenadeNumBounces(in.ReadLong())
, xf4_grenadeBounceSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, xf6_grenadeExplodeSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, xf8_shockwaveParticleDescId(in.ReadLong())
, xfc_shockwaveDamageInfo(in)
, x118_shockwaveWeaponDescId(in.ReadLong())
, x11c_shockwaveElectrocuteSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, x11e_canCallForBackup(in.ReadBool())
, x11f_fastWhenAttractingEnergy(propCount >= 42 ? in.ReadBool() : true) {}

CElitePirate::CElitePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           const CElitePirateData& data)
: CPatterned(kC_ElitePirate, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actParms, kCS_Large)
, x568_state(kState_Invalid)
, x56c_vulnerability(pInfo.GetDamageVulnerability())
, x5d4_collisionActorMgr(nullptr)
, x5d8_data(data)
, x6f8_boneTracking(*GetAnimationData(), rstl::string_l("Head_1"), 80.f * M_PIF / 180.f,
                    CRelAngle::FromDegrees(180.f).AsRadians(), kBTF_None)
, x730_collisionActorMgrHead(nullptr)
, x738_collisionAabb(GetBoundingBox(), GetMaterialList())
, x760_energyAbsorbDesc(data.GetEnergyAbsorbParticleDescId() != kInvalidAssetId
                            ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                  TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                      SObjectTag('PART', data.GetEnergyAbsorbParticleDescId()))))
                            : rstl::optional_object< TLockedToken< CGenDescription > >())
, x770_collisionHeadId(kInvalidUniqueId)
, x772_launcherId(kInvalidUniqueId)
, x79c_energyAttractorId(kInvalidUniqueId)
, x7a0_initialSpeed(x3b4_speed)
, x7a4_steeringSpeed(1.f)
, x7a8_pathShaggedTime(0.f)
, x7ac_energyAbsorbCooldown(0.f)
, x7b0_(1.f)
, x7b4_hp(0.f)
, x7b8_attackTimer(0.f)
, x7bc_tauntTimer(0.f)
, x7c0_shotAtTimer(0.f)
, x7c4_absorbUpdateTimer(0.f)
, x7c8_currAnimId(-1)
, x7cc_activeMaterialSet(0)
, x7d0_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x8b4_targetDestPos(CVector3f::Zero())
, x8c0_positionHistory(5.f)
, x988_24_damageOn(false)
, x988_25_attackingRightClaw(false)
, x988_26_attackingLeftClaw(false)
, x988_27_shotAt(false)
, x988_28_alert(false)
, x988_29_shockWaveAnim(false)
, x988_30_calledForBackup(false)
, x988_31_running(false)
, x989_24_onPath(false) {
  KnockBackCtrl().SetEnableFreeze(false);
  KnockBackCtrl().SetX82_24(false);
  KnockBackCtrl().SetEnableBurnDeath(false);
  KnockBackCtrl().SetEnableExplodeDeath(false);
  KnockBackCtrl().SetEnableLaggedBurnDeath(false);
  SetupPathFindSearch();
}

CElitePirate::~CElitePirate() {}

ENTITY_ACCEPT_IMPL(CElitePirate)

void CElitePirate::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    x6f8_boneTracking.Update(dt);
    if (IsUsingBaseCollisionActors()) {
      x730_collisionActorMgrHead->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    }
    x5d4_collisionActorMgr->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    if (IsShieldActive() && x5d8_data.IsFastWhenAttractingEnergy()) {
      x3b4_speed = 2.f * x7a0_initialSpeed;
    } else {
      x3b4_speed = x7a0_initialSpeed;
    }
    UpdateAILogicTimers(dt);
    UpdateBreadCrumbTrail();
    UpdateGrenadeLauncher(mgr, x772_launcherId, rstl::string_l(skpLauncherLCTR));
    UpdateHealthInfo(mgr);
    x328_31_energyAttractor = IsShieldActive();
  }
}

void CElitePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool shouldPass = true;
  switch (msg) {
  case kSM_Registered: {
    x450_bodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    x772_launcherId = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, x772_launcherId);
    const float maxSpeed = x450_bodyController->GetBodyStateInfo().GetMaxSpeed();
    if (maxSpeed > 0.f) {
      x7a4_steeringSpeed =
          (0.99f * x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk)) /
          maxSpeed;
    }
    x450_bodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    x450_bodyController->CommandMgr().SetSteeringSpeedRange(x7a4_steeringSpeed, x7a4_steeringSpeed);
    break;
  }
  case kSM_Activate:
    if (IsUsingBaseCollisionActors()) {
      x730_collisionActorMgrHead->SetActive(mgr, true);
    }
    if (CEntity* entity = mgr.ObjectById(x772_launcherId)) {
      entity->SetActive(true);
    }
    break;
  case kSM_Deactivate:
    if (IsUsingBaseCollisionActors()) {
      x730_collisionActorMgrHead->SetActive(mgr, false);
    }
    x5d4_collisionActorMgr->SetActive(mgr, false);
    if (CEntity* entity = mgr.ObjectById(x772_launcherId)) {
      entity->SetActive(false);
    }
    break;
  case kSM_Deleted:
    if (IsUsingBaseCollisionActors()) {
      x730_collisionActorMgrHead->Destroy(mgr);
    }
    x5d4_collisionActorMgr->Destroy(mgr);
    mgr.DeleteObjectRequest(x772_launcherId);
    break;
  case kSM_Alert:
    x988_28_alert = true;
    break;
  case kSM_InitializedInArea: {
    const TAreaId areaId = GetCurrentAreaId();
    x7d0_pathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->x10bc_pathArea);
    break;
  }
  case kSM_Touched: {
    if (!(HealthInfo(mgr)->GetHP() > 0.f)) {
      break;
    }
    const CCollisionActor* const actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid));
    if (actor) {
      const TUniqueId touchedUid = actor->GetLastTouchedObject();
      if (touchedUid == mgr.GetPlayer()->GetUniqueId()) {
        if (x988_24_damageOn) {
          if ((x988_25_attackingRightClaw && IsArmClawCollider(uid, x774_collisionRJointIds)) ||
              (x988_26_attackingLeftClaw && IsArmClawCollider(uid, x788_collisionLJointIds))) {
            mgr.ApplyDamage(
                GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                CVector3f::Zero());
            x420_curDamageRemTime = x424_damageWaitTime;
            x988_24_damageOn = false;
          }
        } else if (x420_curDamageRemTime <= 0.f) {
          CDamageInfo info = GetContactDamage();
          info.SetDamage(0.5f * info.GetDamage());
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), info,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      } else if (TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(touchedUid))) {
        SetShotAt(true, mgr);
      }
    } else if (uid == x772_launcherId && x772_launcherId != kInvalidUniqueId) {
      SetShotAt(true, mgr);
    }
    break;
  }
  case kSM_Damage: {
    shouldPass = false;
    CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid));
    if (actor) {
      const TUniqueId touchedUid = actor->GetLastTouchedObject();
      if (const CGameProjectile* projectile =
              TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(touchedUid))) {
        if (uid == x770_collisionHeadId) {
          x428_damageCooldownTimer = skDamageHitTime;
          KnockBack(projectile->GetVelocity(), mgr, projectile->GetCurrentDamageInfo(),
                    projectile->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
          CPatterned::AcceptScriptMsg(msg, touchedUid, mgr);
        } else if (uid == x79c_energyAttractorId && x760_energyAbsorbDesc) {
          StartAbsorbEnergyEffects(mgr, projectile->GetTransform());
        }
        SetShotAt(true, mgr);
      }
    } else if (uid == x772_launcherId && x772_launcherId != kInvalidUniqueId) {
      x450_bodyController->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Eight));
    } else {
      ReDirectDamage(mgr, uid);
    }
    break;
  }
  case kSM_InvulnDamage:
    SetShotAt(true, mgr);
    if (!TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      ReDirectDamage(mgr, uid);
    }
    break;
  }
  if (shouldPass) {
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
  }
}

// TODO: The scale copy still misses a model-pointer reload; shader-set register allocation differs.
void CElitePirate::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CPatterned::PreRender(mgr, frustum);
  x6f8_boneTracking.PreRender(mgr, *AnimationData(), GetTransform(), ModelData()->ScaleCopy(),
                              *x450_bodyController);
  const int numSets = GetModelData()->GetNumMaterialSets();
  SetModelFlags(GetModelFlags().UseShaderSet(
      rstl::min_val(static_cast< int >(x7cc_activeMaterialSet), numSets - 1)));
}

void CElitePirate::KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info,
                             float magnitude, bool direct, const bool inDeferred) {
  if (AllowKnockBack(info)) {
    CPatterned::KnockBack(dir, mgr, info, magnitude, direct, inDeferred);
    if (info.GetWeaponMode().IsComboed() && info.GetWeaponMode().GetType() == kWT_Ice) {
      const CVector3f pos(0.f, 0.f, 0.f);
      const CUnitVector3f localDir(GetTransform().TransposeRotate(dir));
      Freeze(mgr, pos, localDir, 1.5f);
    }
  }
}

CVector3f CElitePirate::GetAimPosition(const CStateManager& mgr, float) const {
  const CPlayerState* playerState = mgr.GetPlayerState();
  if (x5d4_collisionActorMgr->GetActive() && playerState->IsFiringComboBeam() &&
      playerState->GetCurrentBeam() == CPlayerState::kBI_Wave) {
    if (const CCollisionActor* actor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x79c_energyAttractorId))) {
      return actor->GetTranslation();
    }
  }
  return GetOrbitPosition(mgr);
}

CVector3f CElitePirate::GetOrbitPosition(const CStateManager& mgr) const {
  if (x772_launcherId != kInvalidUniqueId &&
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal) {
    if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(x772_launcherId))) {
      return GetGrenadeLaunchPos(*actor);
    }
  }
  if (IsUsingBaseCollisionActors()) {
    if (const CCollisionActor* actor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x770_collisionHeadId))) {
      return actor->GetTranslation();
    }
  }
  return GetLctrTransform("lockon_target_LCTR").GetTranslation();
}

void CElitePirate::TakeDamage(const CVector3f&, float) {}

const CDamageVulnerability* CElitePirate::GetDamageVulnerability() const {
  return &CDamageVulnerability::PassThroughVulnerability();
}

const CDamageVulnerability* CElitePirate::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                                 const CDamageInfo&) const {
  return &CDamageVulnerability::PassThroughVulnerability();
}

void CElitePirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile:
    if (x772_launcherId != kInvalidUniqueId) {
      if (CEntity* launcher = mgr.ObjectById(x772_launcherId)) {
        mgr.DeliverScriptMsg(launcher, GetUniqueId(), kSM_Action);
      }
    }
    handled = true;
    break;
  case kUE_BeginAction: {
    const CTransform4f locator(GetLctrTransform(node.GetLocatorName()));
    const CVector3f front = GetTransform().GetForward();
    const CVector3f delta = locator.GetTranslation() - GetTranslation();
    const float dot = CVector3f::Dot(delta, front);
    CVector3f position = GetTranslation() + dot * front;
    position.SetZ(GetTranslation().GetZ());
    const CTransform4f xf(CTransform4f::Translate(position));
    CShockWave* const wave = rs_new CShockWave(
        mgr.AllocateUniqueId(), rstl::string_l("Shock Wave"),
        CEntityInfo(GetCurrentAreaId(), NullConnectionList), xf, GetUniqueId(), GetShockWaveInfo(),
        IsElitePirate() ? 2.f : 1.3f, IsElitePirate() ? 0.4f : 0.5f);
    if (wave) {
      mgr.AddObject(wave);
    }
    handled = true;
    break;
  }
  case kUE_DamageOn:
    x988_24_damageOn = true;
    handled = true;
    break;
  case kUE_DamageOff:
    x988_24_damageOn = false;
    handled = true;
    break;
  case kUE_ScreenShake:
    ProcessStompGround(mgr);
    handled = true;
    break;
  case kUE_BecomeShootThrough:
    if (IsUsingBaseCollisionActors()) {
      for (uint i = 0; i < x730_collisionActorMgrHead->GetNumCollisionActors(); ++i) {
        const CJointCollisionDescription& desc =
            x730_collisionActorMgrHead->GetCollisionDescFromIndex(i);
        const TUniqueId uid = desc.GetCollisionActorId();
        if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
          actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
        }
      }
    }
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

bool CElitePirate::SpotPlayer(CStateManager& mgr, float arg) {
  if (x988_28_alert) {
    return true;
  }
  return CPatterned::SpotPlayer(mgr, arg);
}

bool CElitePirate::InDetectionRange(CStateManager& mgr, float arg) {
  if (x988_28_alert) {
    return true;
  }
  return CPatterned::InDetectionRange(mgr, arg);
}

bool CElitePirate::ShouldTurn(CStateManager& mgr, float) {
  const CVector2f dist = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
  const CVector2f forward = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward, dist) > CRelAngle::FromDegrees(15.f).AsRadians();
}

bool CElitePirate::ShouldTaunt(CStateManager&, float) { return x7bc_tauntTimer <= 0.f; }

bool CElitePirate::ShouldFire(CStateManager& mgr, float) {
  return ShouldFireLauncher(mgr, x772_launcherId);
}

bool CElitePirate::ShouldFireLauncher(CStateManager& mgr, const TUniqueId uid) {
  if (x7b8_attackTimer <= 0.f && uid != kInvalidUniqueId) {
    if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(uid))) {
      const CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
      const CVector3f delta = aim - GetTranslation();
      if (delta.MagSquared() >= x300_maxAttackRange * x300_maxAttackRange &&
          !ShouldTurn(mgr, 0.f)) {
        const CVector3f origin = GetGrenadeLaunchPos(*actor);
        if (!IsPatternObstructed(mgr, origin, aim)) {
          float angle = x5d8_data.GetGrenadeTrajectoryInfo().GetAngleMin();
          float velocity = x5d8_data.GetGrenadeTrajectoryInfo().GetVelocityMin();
          const CEPGrenadeLaunchParms& launchInfo = x5d8_data.GetGrenadeTrajectoryInfo();
          const CEPGrenadeLaunchParms parms(launchInfo.GetVelocityMin(),
                                            launchInfo.GetVelocityMax(), launchInfo.GetAngleMin(),
                                            launchInfo.GetAngleMax());
          const CVector3f target = CGrenadeLauncher::PredictTargetPosition(mgr);
          CGrenadeLauncher::ComputeLaunchSpeedAndAngle(target, origin, parms, angle, velocity);
          const CVector3f direction(0.f, CMath::FastCosR(angle), CMath::FastSinR(angle));
          const CVector3f destination = origin + 7.5f * GetTransform().Rotate(direction);
          return !IsPatternObstructed(mgr, origin, destination);
        }
      }
    }
  }
  return false;
}

bool CElitePirate::ShouldAttack(CStateManager& mgr, float) {
  const CVector3f dist = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if (dist.MagSquared() <= x2fc_minAttackRange * x2fc_minAttackRange) {
    return !ShouldTurn(mgr, 0.f);
  }
  return false;
}

bool CElitePirate::ShouldSpecialAttack(CStateManager& mgr, float) {
  if (x7b8_attackTimer <= 0.f && GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId()) {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CVector3f dist = aimPos - GetTranslation();
    const float magSquared = dist.MagSquared();
    if (magSquared >= x2fc_minAttackRange * x2fc_minAttackRange &&
        magSquared <= x300_maxAttackRange * x300_maxAttackRange) {
      return CMath::AbsF(dist.GetZ()) < 3.f;
    }
  }
  return false;
}

bool CElitePirate::ShouldCallForBackup(CStateManager& mgr, float arg) {
  return ShouldCallForBackupForLauncher(mgr, arg, x772_launcherId);
}

bool CElitePirate::ShouldCallForBackupForLauncher(CStateManager&, float, TUniqueId uid) const {
  if (!x988_30_calledForBackup && uid == kInvalidUniqueId && x5d8_data.CanCallForBackup()) {
    return x7a8_pathShaggedTime >= 3.f;
  }
  return false;
}

bool CElitePirate::ShotAt(CStateManager&, float) { return x988_27_shotAt; }

bool CElitePirate::InPosition(CStateManager&, float) {
  const CVector3f dist = x8b4_targetDestPos - GetTranslation();
  return dist.MagSquared() < 25.f;
}

bool CElitePirate::TooClose(CStateManager& mgr, float) {
  const CVector3f dist = GetTranslation() - mgr.GetPlayer()->GetTranslation();
  return dist.MagSquared() < x2fc_minAttackRange * x2fc_minAttackRange;
}

bool CElitePirate::AggressionCheck(CStateManager& mgr, float arg) {
  if (x772_launcherId == kInvalidUniqueId && !PathShagged(mgr, arg)) {
    if (!x988_31_running) {
      const CVector3f dist = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      return dist.MagSquared() > 4.f * x300_maxAttackRange * x300_maxAttackRange;
    }
    return true;
  }
  return false;
}

bool CElitePirate::AnimOver(CStateManager&, float) { return x568_state == kState_Over; }

void CElitePirate::Run(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x988_31_running = true;
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    x450_bodyController->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    UpdatePathDestination(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    break;
  case kStateMsg_Update:
    if (!PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      const CVector3f move =
          x8c0_positionHistory.GetValue(GetTranslation(), GetTransform().GetForward());
      if (move.IsNonZero()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      } else if (ShouldTurn(mgr, 0.f)) {
        const CVector3f aim =
            mgr.GetPlayer()->GetAimPosition(mgr, 0.5f * GetAnimationData()->GetPlaybackRate());
        const CVector3f face = aim - GetTranslation();
        if (face.CanBeNormalized()) {
          x450_bodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), face.AsNormalized(), 1.f));
        }
      }
    }
    break;
  case kStateMsg_Deactivate:
    x988_31_running = false;
    x6f8_boneTracking.SetActive(false);
    x450_bodyController->CommandMgr().SetSteeringSpeedRange(x7a4_steeringSpeed, x7a4_steeringSpeed);
    break;
  }
}

void CElitePirate::Halt(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    x450_bodyController->SetLocomotionType(pas::kLT_Lurk);
    x989_24_onPath = false;
    CMaterialFilter filter = GetMaterialFilter();
    filter.ExcludeList().Add(CMaterialList(kMT_Wall, kMT_Ceiling, kMT_AIBlock, kMT_Character));
    SetMaterialFilter(filter);
    break;
  }
  case kStateMsg_Deactivate: {
    CMaterialFilter filter = GetMaterialFilter();
    filter.ExcludeList().Remove(CMaterialList(kMT_Wall, kMT_Ceiling, kMT_AIBlock, kMT_Character));
    SetMaterialFilter(filter);
    break;
  }
  }
}

void CElitePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x989_24_onPath = true;
    x988_28_alert = false;
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    UpdatePathDestination(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    x7bc_tauntTimer =
        mgr.Random()->Float() * x5d8_data.GetTauntVariance() + x5d8_data.GetTauntInterval();
    if (TooClose(mgr, 0.f)) {
      x450_bodyController->CommandMgr().ClearLocomotionCmds();
    }
    break;
  case kStateMsg_Update:
    if (x7bc_tauntTimer > 0.f) {
      x7bc_tauntTimer -= dt;
    }
    if (!TooClose(mgr, 0.f) && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else if (PathShagged(mgr, 0.f)) {
      const CVector3f move =
          x8c0_positionHistory.GetValue(GetTranslation(), GetTransform().GetForward());
      if (move.IsNonZero()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
    } else if (ShouldTurn(mgr, 0.f)) {
      const CVector3f aim = mgr.GetPlayer()->GetAimPosition(
          mgr, 0.5f * GetModelData()->GetAnimationData()->GetPlaybackRate());
      const CVector3f face = aim - GetTranslation();
      if (face.CanBeNormalized()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), face.AsNormalized(), 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    x6f8_boneTracking.SetActive(false);
    break;
  }
}

void CElitePirate::Taunt(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Taunt) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
      }
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Taunt) {
        x568_state = kState_Over;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  }
}

void CElitePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    if (HasPatrolPath(mgr, 0.f)) {
      CPatterned::Patrol(mgr, msg, dt);
      UpdateDest(mgr);
    } else {
      SetDestPos(x3a0_latestLeashPosition);
    }
    x8b4_targetDestPos = x2e0_destPos;
    if (GetSearchPath()) {
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update:
    if (!PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
          x45c_steeringBehaviors.Arrival(*this, x8b4_targetDestPos, 25.f), CVector3f::Zero(), 1.f));
    }
    break;
  case kStateMsg_Deactivate:
    x988_28_alert = false;
    break;
  }
}

void CElitePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    x400_24_hitByPlayerProjectile = false;
    x989_24_onPath = false;
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CElitePirate::Generate(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_One;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_One:
      if (ShouldTurn(mgr, 0.f)) {
        const CVector3f face = mgr.GetPlayer()->GetTranslation() - GetTranslation();
        if (face.CanBeNormalized()) {
          x450_bodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), face.AsNormalized(), 1.f));
        }
      } else {
        x568_state = kState_Zero;
      }
      break;
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Generate) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
      }
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Generate) {
        x568_state = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    SetShotAt(false, mgr);
    ActivateGrenadeLauncher(mgr, true);
    break;
  }
}

void CElitePirate::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    break;
  case kStateMsg_Update: {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_One, playerPos, false));
      }
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        x568_state = kState_Over;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(playerPos - GetTranslation());
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    break;
  }
}

void CElitePirate::SpecialAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    break;
  case kStateMsg_Update: {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        x568_state = kState_Two;
        x988_29_shockWaveAnim = true;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Two, playerPos, false));
      }
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        x568_state = kState_Over;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(playerPos - GetTranslation());
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    x988_29_shockWaveAnim = false;
    break;
  }
}

void CElitePirate::Attack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    ExtendTouchBounds(mgr, x774_collisionRJointIds, skExtendedClawBounds);
    if (GetModelData()->GetNumMaterialSets() > 1) {
      x7cc_activeMaterialSet = 1;
    }
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_state = kState_One;
        x988_25_attackingRightClaw = true;
        x7c8_currAnimId = x450_bodyController->GetCurrentAnimId();
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
      }
      break;
    case kState_One:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_state = kState_Over;
      } else if (x7c8_currAnimId != x450_bodyController->GetCurrentAnimId()) {
        x568_state = kState_Two;
        x988_25_attackingRightClaw = false;
        x988_26_attackingLeftClaw = true;
        ExtendTouchBounds(mgr, x774_collisionRJointIds, CVector3f::Zero());
        ExtendTouchBounds(mgr, x788_collisionLJointIds, skExtendedClawBounds);
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
        if (ShouldAttack(mgr, 0.f)) {
          x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Two));
        }
      }
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_state = kState_Over;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    x988_25_attackingRightClaw = x988_26_attackingLeftClaw = x988_24_damageOn = false;
    x7c8_currAnimId = -1;
    ExtendTouchBounds(mgr, x774_collisionRJointIds, CVector3f::Zero());
    ExtendTouchBounds(mgr, x788_collisionLJointIds, CVector3f::Zero());
    x7cc_activeMaterialSet = 0;
    break;
  }
}

void CElitePirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->SetLocomotionType(pas::kLT_Crouch);
    if (IsUsingBaseCollisionActors()) {
      if (CCollisionActor* actor =
              TCastToPtr< CCollisionActor >(mgr.ObjectById(x770_collisionHeadId))) {
        actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
      }
    }
    x5d4_collisionActorMgr->SetActive(mgr, true);
    x6f8_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x6f8_boneTracking.SetActive(true);
    UpdatePathDestination(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    if (TooClose(mgr, 0.f)) {
      x450_bodyController->CommandMgr().ClearLocomotionCmds();
    }
    break;
  case kStateMsg_Update:
    if (x988_27_shotAt) {
      x7c0_shotAtTimer -= dt;
      if (x7c0_shotAtTimer <= 0.f) {
        x988_27_shotAt = false;
      }
    }
    x7a8_pathShaggedTime = PathShagged(mgr, 0.f) ? x7a8_pathShaggedTime + dt : 0.f;
    if (!TooClose(mgr, 0.f) && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else if (PathShagged(mgr, 0.f)) {
      const CVector3f move =
          x8c0_positionHistory.GetValue(GetTranslation(), GetTransform().GetForward());
      if (move.IsNonZero()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
    } else if (ShouldTurn(mgr, 0.f)) {
      const CVector3f aim = mgr.GetPlayer()->GetAimPosition(
          mgr, 0.5f * GetModelData()->GetAnimationData()->GetPlaybackRate());
      const CVector3f face = aim - GetTranslation();
      if (face.CanBeNormalized()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), face.AsNormalized(), 1.f));
      }
    }
    AttractProjectiles(mgr);
    UpdateBlockPose(dt, mgr);
    break;
  case kStateMsg_Deactivate:
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    x6f8_boneTracking.SetActive(false);
    if (IsUsingBaseCollisionActors()) {
      if (CCollisionActor* actor =
              TCastToPtr< CCollisionActor >(mgr.ObjectById(x770_collisionHeadId))) {
        actor->SetDamageVulnerability(x56c_vulnerability);
      }
    }
    x5d4_collisionActorMgr->SetActive(mgr, false);
    break;
  }
}

void CElitePirate::CallForBackup(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    x988_30_calledForBackup = true;
    SetShotAt(false, mgr);
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Generate) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Five, CVector3f::Zero()));
      }
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Generate) {
        x568_state = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    SendScriptMsgs(kSS_Zero, mgr, kSM_None);
    break;
  }
}

// TODO: The collision-description vector destructor is inlined instead of called.
void CElitePirate::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(13);
  AddCollisionList(skLeftArmJointList, 3, joints);
  AddCollisionList(skRightArmJointList, 3, joints);
  AddSphereCollisionList(skSphereJointList, 7, joints);
  if (IsUsingBaseCollisionActors()) {
    x730_collisionActorMgrHead =
        rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, true);
    x730_collisionActorMgrHead->SetActive(mgr, GetActive());
  }
  x774_collisionRJointIds.clear();
  x788_collisionLJointIds.clear();
  const CSegId segId = GetAnimationData()->GetLocatorSegId(rstl::string_l(skpLeftClawLCTR));
  const CJointCollisionDescription shield = CJointCollisionDescription::OBBCollision(
      segId, skLocalShieldBounds, CVector3f::Zero(), rstl::string_l("Shield"), 10.f);
  joints.clear();
  joints.push_back(shield);
  x5d4_collisionActorMgr =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  SetupCollisionActorInfo(mgr);
  SetupHealthInfo(mgr);
  CMaterialList exclude(kMT_CollisionActor, kMT_AIPassthrough, kMT_Player);
  exclude.Add(kMT_Platform);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), exclude));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
}

void CElitePirate::SetupCollisionActorInfo(CStateManager& mgr) {
  const CDamageVulnerability vulnerability = CDamageVulnerability::ReflectVulnerability();
  if (IsUsingBaseCollisionActors()) {
    for (uint i = 0; i < x730_collisionActorMgrHead->GetNumCollisionActors(); ++i) {
      const CJointCollisionDescription& desc =
          x730_collisionActorMgrHead->GetCollisionDescFromIndex(i);
      const TUniqueId uid = desc.GetCollisionActorId();
      if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
        if (desc.GetName() == rstl::string_l(skpHeadLCTR)) {
          x770_collisionHeadId = uid;
        } else if (IsArmClawCollider(desc.GetName(), skpRightClawLCTR, skRightArmJointList, 3)) {
          x774_collisionRJointIds.push_back(uid);
        } else if (IsArmClawCollider(desc.GetName(), skpLeftClawLCTR, skLeftArmJointList, 3)) {
          x788_collisionLJointIds.push_back(uid);
        }
        if (uid != x770_collisionHeadId) {
          actor->SetDamageVulnerability(vulnerability);
        }
      }
    }
    x730_collisionActorMgrHead->AddMaterial(
        mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough, kMT_Immovable));
  }
  x79c_energyAttractorId =
      x5d4_collisionActorMgr->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (CCollisionActor* const actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(x79c_energyAttractorId))) {
    actor->SetWeaponCollisionResponseType(kWCR_None);
  }
  x5d4_collisionActorMgr->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
}

void CElitePirate::AddCollisionList(const SJointInfo* joints, int count,
                                    rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId from = animData.GetLocatorSegId(rstl::string_l(joints[i].x0_from));
    const CSegId to = animData.GetLocatorSegId(rstl::string_l(joints[i].x4_to));
    if (from.val() != 0xff && to.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereSubdivideCollision(
          from, to, joints[i].x8_radius, joints[i].xc_separation,
          CJointCollisionDescription::kOT_One, rstl::string_l(joints[i].x0_from), 10.f);
      list.push_back(desc);
    }
  }
}

void CElitePirate::AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                                          rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId id = animData.GetLocatorSegId(rstl::string_l(joints[i].x0_name));
    if (id.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          id, joints[i].x4_radius, rstl::string_l(joints[i].x0_name), 10.f);
      list.push_back(desc);
    }
  }
}

bool CElitePirate::IsArmClawCollider(const rstl::string& name, const char* locator,
                                     const SJointInfo* joints, int count) const {
  if (name == rstl::string_l(locator)) {
    return true;
  }
  for (int i = 0; i < count; ++i) {
    if (name == rstl::string_l(joints[i].x0_from)) {
      return true;
    }
  }
  return false;
}

bool CElitePirate::IsArmClawCollider(TUniqueId uid,
                                     const rstl::reserved_vector< TUniqueId, 7 >& ids) const {
  for (AUTO(it, ids.begin()); it != ids.end(); ++it) {
    if (*it == uid) {
      return true;
    }
  }
  return false;
}

void CElitePirate::ExtendTouchBounds(CStateManager& mgr,
                                     const rstl::reserved_vector< TUniqueId, 7 >& ids,
                                     const CVector3f& bounds) const {
  for (AUTO(it, ids.begin()); it != ids.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      actor->SetExtendedTouchBounds(bounds);
    }
  }
}

void CElitePirate::SetupPathFindSearch() {
  const float scale = 1.5f * GetModelData()->ScaleCopy().GetY();
  const float heightScale = IsElitePirate() ? 5.f : 1.f;
  const CAABox box(CVector3f(-scale, -scale, 0.f), CVector3f(scale, scale, heightScale * scale));
  SetBoundingBox(box);
  x738_collisionAabb.Box() = box;
  x7d0_pathFindSearch.SetCharacterRadius(scale);
  x7d0_pathFindSearch.SetCharacterHeight(3.f * scale);
}

bool CElitePirate::IsShieldActive() const {
  return x450_bodyController->GetLocomotionType() == pas::kLT_Crouch &&
         (x450_bodyController->GetCurrentStateId() == pas::kAS_Locomotion ||
          x450_bodyController->GetCurrentStateId() == pas::kAS_Turn);
}

void CElitePirate::SetupHealthInfo(CStateManager& mgr) {
  const CHealthInfo* health = HealthInfo(mgr);
  x7b4_hp = health->GetHP();
  if (IsUsingBaseCollisionActors()) {
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(x770_collisionHeadId))) {
      *actor->HealthInfo(mgr) = *health;
      actor->SetDamageVulnerability(x56c_vulnerability);
    }
  }
  SetupHealthInfoForLauncher(mgr, x772_launcherId);
}

void CElitePirate::SetupHealthInfoForLauncher(CStateManager& mgr, const TUniqueId uid) const {
  const CHealthInfo* health = GetHealthInfo(mgr);
  if (uid != kInvalidUniqueId) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      *actor->HealthInfo(mgr) =
          CHealthInfo(x5d8_data.GetLauncherHP(), health->GetKnockBackResistance());
      actor->SetDamageVulnerability(x56c_vulnerability);
    }
  }
}

void CElitePirate::UpdateHealthInfo(CStateManager& mgr) {
  const float hp = HealthInfo(mgr)->GetHP();
  if (IsUsingBaseCollisionActors()) {
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(x770_collisionHeadId))) {
      const float damage = hp - actor->HealthInfo(mgr)->GetHP();
      HealthInfo(mgr)->SetHP(hp - damage);
      *actor->HealthInfo(mgr) = *HealthInfo(mgr);
    }
  }
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
  }
}

void CElitePirate::UpdateAILogicTimers(float dt) {
  if (x7b8_attackTimer > 0.f) {
    x7b8_attackTimer -= dt;
  }
  if (x7ac_energyAbsorbCooldown > 0.f) {
    x7ac_energyAbsorbCooldown -= dt;
  }
}

// TODO: Resource loads and damage-copy stores still differ in scheduling.
void CElitePirate::CreateGrenadeLauncher(CStateManager& mgr, TUniqueId uid) {
  const CAnimationParameters& params = x5d8_data.GetLauncherAnimParams();
  if (params.GetACSFile() != kInvalidAssetId) {
    CModelData model(CAnimRes(params.GetACSFile(), params.GetCharacter(),
                              GetModelData()->ScaleCopy(), params.GetInitialAnimation(), true));
    CBouncyGrenadeData grenadeData(
        SGrenadeVelocityInfo(x5d8_data.GetGrenadeVelocityInfo().GetMass(),
                             x5d8_data.GetGrenadeVelocityInfo().GetSpeed()),
        x5d8_data.GetGrenadeDamageInfo(), x5d8_data.GetGrenadeElementGenDescId1(),
        x5d8_data.GetGrenadeElementGenDescId2(), x5d8_data.GetGrenadeElementGenDescId3(),
        x5d8_data.GetGrenadeElementGenDescId4(), x5d8_data.GetGrenadeNumBounces(),
        x5d8_data.GetGrenadeBounceSfxId(), x5d8_data.GetGrenadeExplodeSfxId());
    const CEPGrenadeLaunchParms& launchInfo = x5d8_data.GetGrenadeTrajectoryInfo();
    CEPGrenadeLaunchParms launchParms(launchInfo.GetVelocityMin(), launchInfo.GetVelocityMax(),
                                      launchInfo.GetAngleMin(), launchInfo.GetAngleMax());
    const CEPGrenadeLauncherData data(grenadeData, x5d8_data.GetGrenadeModelId(),
                                      x5d8_data.GetLauncherParticleGenDescId(),
                                      x5d8_data.GetLauncherSfxId(), launchParms);
    CGrenadeLauncher* const launcher = rs_new CGrenadeLauncher(
        uid, rstl::string_l("Grenade Launcher"),
        CEntityInfo(GetCurrentAreaId(), NullConnectionList), GetTransform(), model,
        model.GetBounds(GetTransform().GetRotation()), CHealthInfo(x5d8_data.GetLauncherHP(), 10.f),
        x56c_vulnerability, x5d8_data.GetLauncherActParams(), GetUniqueId(), data, 0.f);
    if (launcher) {
      mgr.AddObject(launcher);
    }
  }
}

void CElitePirate::UpdateGrenadeLauncher(CStateManager& mgr, TUniqueId& uid,
                                         const rstl::string& locator) const {
  if (uid != kInvalidUniqueId) {
    if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(uid))) {
      actor->SetTransform(CTransform4f(GetLctrTransform(locator)));
    } else {
      uid = kInvalidUniqueId;
    }
  }
}

void CElitePirate::ActivateGrenadeLauncher(CStateManager& mgr, bool active) {
  ActivateGrenadeLauncherById(mgr, active, x772_launcherId);
}

void CElitePirate::ActivateGrenadeLauncherById(CStateManager& mgr, bool active,
                                               const TUniqueId uid) const {
  if (uid != kInvalidUniqueId) {
    if (CEntity* entity = mgr.ObjectById(uid)) {
      mgr.DeliverScriptMsg(entity, GetUniqueId(), active ? kSM_Start : kSM_Stop);
    }
  }
}

CVector3f CElitePirate::GetGrenadeLaunchPos(const CActor& actor) const {
  const CTransform4f locator = actor.GetLocatorTransform(rstl::string_l(skpGrenadeLauncherLCTR));
  const CVector3f position =
      actor.GetTranslation() + actor.GetTransform().Rotate(locator.GetTranslation());
  return position;
}

void CElitePirate::UpdateBreadCrumbTrail() {
  const CVector3f pos = GetTranslation();
  if (x7d0_pathFindSearch.OnPath(pos) == CPathFindSearch::kR_Success) {
    x8c0_positionHistory.Clear();
  }
  x8c0_positionHistory.AddValue(pos);
}

void CElitePirate::UpdatePathDestination(CStateManager& mgr) {
  x8b4_targetDestPos = GetTranslation();
  const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  const CVector3f dist = GetTranslation() - playerPos;
  const float minRangeSquared = x2fc_minAttackRange * x2fc_minAttackRange;
  if (dist.CanBeNormalized() && dist.MagSquared() > minRangeSquared) {
    const CVector3f& dest = playerPos + dist.AsNormalized() * x2fc_minAttackRange;
    x2e0_destPos = dest;
    x8b4_targetDestPos = x2e0_destPos;
  }
}

void CElitePirate::UpdateAttackTimeLeft(CStateManager& mgr) {
  if (mgr.Random()->Float() > x5d8_data.GetAttackChance()) {
    x7b8_attackTimer = x304_averageAttackTime + x308_attackTimeVariation * mgr.Random()->Float();
  }
}

void CElitePirate::ProcessStompGround(CStateManager& mgr) {
  CPlayer& player = *mgr.Player();
  const CVector3f delta = GetTranslation() - player.GetTranslation();
  const float distance = delta.Magnitude();
  const float scale = x988_29_shockWaveAnim ? 1.f : 0.25f;
  const CVector3f modelScale(GetModelData()->GetScale());
  float magnitude = scale * modelScale.Magnitude();
  magnitude -= 0.005f * distance;
  if (magnitude > 0.f && player.GetSurfaceRestraint() != CPlayer::kSR_Air &&
      !player.IsInsideFluid()) {
    if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      if (mgr.GetCameraManager()->GetCurrentCameraId() ==
          mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()) {
        mgr.CameraManager()->AddCameraShaker(CCameraShakeData::HardVertShake(0.5f, magnitude), true);
      }
    } else {
      const float intensity = x988_29_shockWaveAnim ? 20.f : 10.f;
      const CVector3f impulse = player.GetMass() * (intensity * CVector3f::Up());
      player.ApplyImpulseWR(impulse, CAxisAngle::Identity());
      player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
  }
}

void CElitePirate::StartAbsorbEnergyEffects(CStateManager& mgr, const CTransform4f& xf) {
  if (x7ac_energyAbsorbCooldown <= 0.f) {
    CExplosion* effect = rs_new CExplosion(*x760_energyAbsorbDesc, mgr.AllocateUniqueId(), true,
                                           CEntityInfo(GetCurrentAreaId(), NullConnectionList),
                                           rstl::string_l("Absorb energy Fx"), xf, 0,
                                           GetModelData()->ScaleCopy(), CColor::White());
    if (effect) {
      mgr.AddObject(effect);
      CSfxManager::AddEmitter(x5d8_data.GetEnergyAbsorbSfxId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      x7ac_energyAbsorbCooldown = 0.25f;
    }
  }
}

// TODO: Ten floating-point register operands differ in the Bezier and velocity calculations.
void CElitePirate::AttractProjectiles(CStateManager& mgr) {
  if (IsAlive()) {
    if (const CCollisionActor* actor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x79c_energyAttractorId))) {
      const CVector3f actorPos = actor->GetTranslation();
      const float radius = x5d8_data.GetProjectileAttractionRadius();
      const CVector3f pos = GetTranslation();
      const CVector3f max = pos + CVector3f(radius, radius, radius);
      CVector3f min = pos;
      min -= CVector3f(radius, radius, radius);
      const CAABox projBox((CVector3f(min)), CVector3f(max));
      const CMaterialFilter projFilter =
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile));
      rstl::reserved_vector< TUniqueId, 1024 > projNearList;
      mgr.BuildNearList(projNearList, projBox, projFilter, nullptr);
      if (!projNearList.empty()) {
        const CAABox charBox((CVector3f(min)), CVector3f(max));
        const CMaterialFilter charFilter =
            CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
        rstl::reserved_vector< TUniqueId, 1024 > charNearList;
        mgr.BuildNearList(charNearList, charBox, charFilter, nullptr);
        for (AUTO(it, projNearList.begin()); it != projNearList.end(); ++it) {
          if (CGameProjectile* const projectile =
                  TCastToPtr< CGameProjectile >(mgr.ObjectById(*it))) {
            if (projectile->GetType() != kWT_Missile &&
                projectile->GetOwnerId() == mgr.GetPlayer()->GetUniqueId() &&
                projectile->GetCurrentAreaId() == GetCurrentAreaId()) {
              const CVector3f actorProjDist = actorPos - projectile->GetTranslation();
              if (CVector3f::Dot(GetTransform().GetForward(), actorProjDist) < 0.f) {
                if (projectile->GetVelocity().CanBeNormalized() &&
                    IsClosestEnergyAttractor(mgr, charNearList, projectile->GetTranslation())) {
                  const float dist = actorProjDist.Magnitude();
                  const CVector3f b = projectile->GetTranslation() +
                                      (0.5f * dist) * projectile->GetVelocity().AsNormalized();
                  const CVector3f c = actorPos + CVector3f(0.f, 0.f, 0.4f * (0.4f * dist));
                  CProjectileWeapon& weapon = projectile->Projectile();
                  CVector3f p1 =
                      CMath::GetBezierPoint(projectile->GetTranslation(), b, c, actorPos, 0.333f);
                  const CVector3f p2 =
                      CMath::GetBezierPoint(projectile->GetTranslation(), b, c, actorPos, 0.666f);
                  const float length =
                      (actorPos - p2).Magnitude() +
                      ((p1 - projectile->GetTranslation()).Magnitude() + (p2 - p1).Magnitude());
                  const float step = projectile->GetVelocity().Magnitude();
                  const CVector3f p3 = CMath::GetBezierPoint(projectile->GetTranslation(), b, c,
                                                             actorPos, step / length);
                  const CVector3f look = p3 - projectile->GetTranslation();
                  if (look.CanBeNormalized()) {
                    CTransform4f xf(CTransform4f::LookAt(CVector3f::Zero(), look, CVector3f::Up()));
                    xf.Orthonormalize();
                    weapon.SetWorldSpaceOrientation(xf);
                    const CVector3f normalVelocity = weapon.GetVelocity().AsNormalized();
                    const CVector3f scaledVelocity = normalVelocity * 0.8f;
                    const CVector3f velocity =
                        CVector3f::Lerp(weapon.GetVelocity(), scaledVelocity, 0.6f);
                    weapon.SetVelocity(velocity);
                  }
                }
              }
              SetShotAt(true, mgr);
            }
          }
        }
      }
    }
  }
}

void CElitePirate::UpdateBlockPose(float dt, CStateManager& mgr) {
  if (x988_27_shotAt && !x450_bodyController->IsFrozen()) {
    x7c4_absorbUpdateTimer += dt;
    if (x7c4_absorbUpdateTimer >= 3.f) {
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Turn ||
          !x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsMoving()) {
        bool playerAbove = false;
        if (IsUsingBaseCollisionActors()) {
          if (const CCollisionActor* head =
                  TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x770_collisionHeadId))) {
            const CVector3f& headPos = head->GetTranslation();
            const CVector3f& pos = GetTranslation();
            const CVector3f& playerPos = mgr.GetPlayer()->GetTranslation();
            float height = headPos[kDZ];
            height -= pos[kDZ];
            playerAbove = playerPos[kDZ] >= headPos[kDZ] - height * 0.5f;
          }
        }
        x450_bodyController->CommandMgr().DeliverCmd(CBCAdditiveReactionCmd(
            playerAbove || TooClose(mgr, 0.f) ? pas::kART_Seven : pas::kART_Five, 1.f, false));
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCAdditiveReactionCmd(pas::kART_Six, 1.f, false));
      }
      x7c4_absorbUpdateTimer = 0.f;
    }
  }
}

bool CElitePirate::IsClosestEnergyAttractor(
    CStateManager& mgr, const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
    const CVector3f& pos) const {
  const CVector3f delta = pos - GetTranslation();
  const float dist = delta.MagSquared();
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    if (const CPatterned* actor = TCastToConstPtr< CPatterned >(mgr.GetObjectById(*it))) {
      if (actor->GetUniqueId() != GetUniqueId() && actor->IsEnergyAttractor()) {
        const CVector3f otherDelta = pos - actor->GetTranslation();
        if (otherDelta.MagSquared() < dist) {
          return false;
        }
      }
    }
  }
  return true;
}

void CElitePirate::SetShotAt(bool shotAt, CStateManager& mgr) {
  if (IsElitePirate() && x7b4_hp > 0.f && shotAt) {
    if (HealthInfo(mgr)->GetHP() / x7b4_hp <= x7b0_) {
      x7b0_ -= 0.2f;
      x988_27_shotAt = true;
    }
  } else {
    x988_27_shotAt = shotAt;
  }
  if (x988_27_shotAt) {
    x7c0_shotAtTimer =
        mgr.Random()->Float() * x5d8_data.GetShotAtTimeVariance() + x5d8_data.GetShotAtTime();
  } else {
    x7c0_shotAtTimer = 0.f;
  }
}

void CElitePirate::ReDirectDamage(CStateManager& mgr, TUniqueId uid) {
  if (IsUsingBaseCollisionActors()) {
    if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(uid))) {
      CDamageInfo damage = weapon->GetCurrentDamageInfo();
      damage.SetRadius(0.f);
      mgr.ApplyDamage(
          uid, x770_collisionHeadId, weapon->GetOwnerId(), damage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
  }
}

bool CElitePirate::AllowKnockBack(const CDamageInfo& info) const {
  if (IsAlive() && !info.GetWeaponMode().IsComboed() &&
      info.GetWeaponMode().GetType() == kWT_Plasma) {
    return false;
  }
  return true;
}
