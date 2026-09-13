#include "MetroidPrime/Enemies/CBabygoth.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CFlameThrower.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

const int CBabygothData::skMinProperties = 33;

const CVector3f CBabygoth::skAttackTouchBounds(0.2f, 0.2f, 0.2f);

const CBabygoth::SSphereJointInfo CBabygoth::skSphereJointList[5] = {
    {"L_knee", 1.2f}, {"R_knee", 1.2f},    {"LCTR_SHEMOUTH", 1.7f},
    {"Pelvis", 1.2f}, {"butt_LCTR", 0.9f},
};

const char* const CBabygoth::skpMouthDamageJoint = "LCTR_SHEMOUTH";
const char* const CBabygoth::skpPelvisDamageJoint = "Pelvis";
const char* const CBabygoth::skpButtDamageJoint = "butt_LCTR";

CBabygothData::CBabygothData(CInputStream& in, int propCount)
: x0_fireballAttackTime(in.Get< float >())
, x4_fireballAttackTimeVariance(in.Get< float >())
, x8_fireballWeapon(in.Get< CAssetId >())
, xc_fireballDamage(in)
, x28_attackContactDamage(in)
, x44_fireBreathWeapon(in.Get< CAssetId >())
, x48_fireBreathRes(in.Get< CAssetId >())
, x4c_fireBreathDamage(in)
, x68_mouthVulnerabilities(in)
, xd0_shellVulnerabilities(in)
, x138_noShellModel(in.Get< CAssetId >())
, x13c_noShellSkin(in.Get< CAssetId >())
, x140_shellHitPoints(in.Get< float >())
, x144_shellCrackSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, x148_intermediateCrackParticle(in.Get< CAssetId >())
, x14c_crackOneParticle(in.Get< CAssetId >())
, x150_crackTwoParticle(in.Get< CAssetId >())
, x154_destroyShellParticle(in.Get< CAssetId >())
, x158_crackOneSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, x15a_crackTwoSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, x15c_destroyShellSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, x160_timeUntilAttack(in.Get< float >())
, x164_attackCooldownTime(in.Get< float >())
, x168_interestTime(in.Get< float >())
, x16c_flamePlayerSteamTxtr(in.Get< CAssetId >())
, x170_flamePlayerHitSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, x174_flamePlayerIceTxtr(in.Get< CAssetId >()) {}

CBabygoth::CBabygoth(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                     const CActorParameters& actParms, const CBabygothData& babyData)
: CPatterned(kC_Babygoth, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actParms, kCS_Medium)
, x568_stateProg(-1)
, x56c_shellState(kSCS_Default)
, x570_babyData(babyData)
, x6e8_teamMgr(kInvalidUniqueId)
, x6ec_pathSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x7d0_approachPathSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x8b4_pathFindMode(kPFM_Normal)
, x8b8_backupDestPos(CVector3f::Zero())
, x8c4_initialFaceDir(CVector3f::Zero())
, x8d0_initialSpeed(x3b4_speed)
, x8d4_stepBackwardDist(0.f)
, x8d8_attackTimeLeft(0.f)
, x8dc_attackTimer(0.f)
, x8e0_attackCooldownTimeLeft(0.f)
, x8e4_fireballAttackTimeLeft(0.f)
, x8e8_interestTimer(0.f)
, x8ec_bodyHP(0.f)
, x8f0_boneTracking(*GetAnimationData(), rstl::string_l("Head_1"), CMath::Deg2Rad(80.f),
                    CRelAngle::FromDegrees(180.f).AsRadians(), kBTF_None)
, x928_colActMgr(nullptr)
, x930_aabox(GetBoundingBox(), GetMaterialList())
, x958_iceProjectile(babyData.GetFireballResID(), babyData.GetFireballDamage())
, x980_flameThrower(kInvalidUniqueId)
, x984_flameThrowerDesc(
      babyData.GetFireBreathWeapon() != kInvalidAssetId
          ? gpSimplePool->GetObj(SObjectTag('WPSC', babyData.GetFireBreathWeapon()))
          : gpSimplePool->GetObj("FlameThrower"))
, x98c_dVuln(pInfo.GetDamageVulnerability())
, x9f4_mouthLocator(0xff)
, x9f6_mouthCollisionActor(kInvalidUniqueId)
, xa00_shellHitPoints(babyData.GetShellHitPoints())
, xa04_drawMaterialIdx(0)
, xa08_noShellModel(TLockedToken< CSkinnedModel >(rs_new CSkinnedModel(
      gpSimplePool->GetObj(SObjectTag('CMDL', babyData.GetNoShellModel())),
      gpSimplePool->GetObj(SObjectTag('CSKR', babyData.GetNoShellSkin())),
      GetAnimationData()->GetModelData()->GetLayoutInfo(), CSkinnedModel::kDO_Owned)))
, xa14_crackOneParticle(gpSimplePool->GetObj(SObjectTag('PART', babyData.GetCrackOneParticle())))
, xa20_crackTwoParticle(gpSimplePool->GetObj(SObjectTag('PART', babyData.GetCrackTwoParticle())))
, xa2c_destroyShellParticle(
      gpSimplePool->GetObj(SObjectTag('PART', babyData.GetDestroyShellParticle())))
, xa38_intermediateCrackParticle(
      babyData.GetIntermediateCrackParticle() != kInvalidAssetId
          ? rstl::optional_object< TLockedToken< CGenDescription > >(
                TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                    SObjectTag('PART', babyData.GetIntermediateCrackParticle()))))
          : rstl::optional_object< TLockedToken< CGenDescription > >())
, xa48_24_isAlert(false)
, xa48_25_(false)
, xa48_26_inProjectileAttack(false)
, xa48_27_(false)
, xa48_28_pendingAttackContactDamage(false)
, xa48_29_hasBeenEnraged(false)
, xa48_30_heardPlayerFire(false)
, xa48_31_approachNeedsPathSearch(true)
, xa49_24_gettingUp(false)
, xa49_25_shouldStepBackwards(false)
, xa49_26_readyForTeam(false)
, xa49_27_locomotionValid(false)
, xa49_28_onApproachPath(false)
, xa49_29_objectSpaceCollision(false) {
  x958_iceProjectile.Token().Lock();
  UpdateTouchBounds();
  KnockBackCtrl().SetEnableFreeze(false);
  KnockBackCtrl().SetX82_24(false);
  KnockBackCtrl().SetLocomotionDuringElectrocution(true);
  KnockBackCtrl().SetEnableExplodeDeath(false);
  const CPASAnimParmData stepParms(pas::kAS_Step, CPASAnimParm::FromEnum(1),
                                   CPASAnimParm::FromEnum(0));
  x8d4_stepBackwardDist = GetAnimationDistance(stepParms) * GetModelData()->ScaleCopy().GetY();
  xa08_noShellModel->SetLayoutInfo(GetAnimationData()->GetModelData()->GetLayoutInfo());
  MakeThermalColdAndHot();
}

CBabygoth::~CBabygoth() {}
ENTITY_ACCEPT_IMPL(CBabygoth)

void CBabygoth::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    PreventPlayerInterpenetration(mgr, dt);
    if (xa49_26_readyForTeam && !CTeamAiMgr::GetTeamAiRole(mgr, x6e8_teamMgr, GetUniqueId())) {
      AddToTeam(mgr);
    }
    CPatterned::Think(dt, mgr);
    if (x450_bodyController->IsElectrocuting()) {
      x8f0_boneTracking.SetActive(false);
    }
    UpdateAILogicTimers(dt);
    AnimationData()->PreRender();
    x8f0_boneTracking.Update(dt);
    x8f0_boneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(),
                                GetModelData()->ScaleCopy(), *x450_bodyController);
    x928_colActMgr->Update(dt, mgr,
                           xa49_29_objectSpaceCollision ? CCollisionActorManager::kUO_ObjectSpace
                                                        : CCollisionActorManager::kUO_WorldSpace);
    xa49_29_objectSpaceCollision = true;
    UpdateHealthInfo(mgr);
    UpdateParticleEffects(dt, mgr);
    CheckShouldGetUp(mgr);
    CheckShouldWakeUp(mgr, dt);
    if (!x400_25_alive && x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsDying()) {
      SetShootThrough(mgr);
    }
  }
}

void CBabygoth::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool callBase = true;
  switch (msg) {
  case kSM_Registered: {
    if (!HasPatrolPath(mgr, 0.f)) {
      x450_bodyController->SetLocomotionType(pas::kLT_Crouch);
    }
    x450_bodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    CreateFlameThrower(mgr);
    const float maxSpeed = x450_bodyController->GetBodyStateInfo().GetMaxSpeed();
    const float speed =
        maxSpeed > 0.f
            ? x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk) / maxSpeed
            : 1.f;
    x450_bodyController->CommandMgr().SetSteeringBlendMode(kSBM_Clamped);
    x450_bodyController->CommandMgr().SetSteeringSpeedRange(0.f, speed);
    x9f4_mouthLocator = GetAnimationData()->GetLocatorSegId(rstl::string_l(skpMouthDamageJoint));
    break;
  }
  case kSM_Activate:
    x928_colActMgr->SetActive(mgr, true);
    break;
  case kSM_Deactivate:
    x928_colActMgr->SetActive(mgr, false);
    xa49_29_objectSpaceCollision = false;
    RemoveFromTeam(mgr);
    break;
  case kSM_Deleted:
    x928_colActMgr->Destroy(mgr);
    if (x980_flameThrower != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(x980_flameThrower);
      x980_flameThrower = kInvalidUniqueId;
    }
    RemoveFromTeam(mgr);
    break;
  case kSM_Falling: {
    if (!x450_bodyController->IsFrozen()) {
      const float weight = GetGravityConstant() * GetMass();
      x150_momentum = CVector3f(0.f, 0.f, -weight);
      x328_27_onGround = false;
      RemoveMaterial(kMT_GroundCollider, mgr);
    }
    callBase = false;
    break;
  }
  case kSM_OnFloor:
    SetMomentumWR(CVector3f::Zero());
    x328_27_onGround = true;
    AddMaterial(kMT_GroundCollider, mgr);
    callBase = false;
    break;
  case kSM_Alert:
    xa48_24_isAlert = true;
    break;
  case kSM_InitializedInArea: {
    const TAreaId areaId = GetCurrentAreaId();
    const CGameArea* area = mgr.GetWorld()->GetArea(areaId);
    x6ec_pathSearch.SetArea(area->GetPostConstructed()->x10bc_pathArea);
    x7d0_approachPathSearch.SetArea(area->GetPostConstructed()->x10bc_pathArea);
    if (x6e8_teamMgr == kInvalidUniqueId) {
      x6e8_teamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    }
    break;
  }
  case kSM_Touched:
    ApplyContactDamage(uid, mgr);
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (const CWeapon* const weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touched))) {
        if (weapon->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
          xa48_24_isAlert = true;
        }
      }
    }
    break;
  case kSM_Damage:
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (const CWeapon* const weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touched))) {
        if (weapon->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
          if (IsMouthCollisionActor(uid)) {
            TakeDamage(CVector3f::Zero(), 0.f);
          } else if (IsShellCollisionActor(uid)) {
            TakeDamage(CVector3f::Zero(), 0.f);
            if (x56c_shellState != kSCS_Destroyed && xa38_intermediateCrackParticle) {
              CTransform4f xf = weapon->GetTransform();
              xf.RotateLocalZ(CRelAngle::FromRadians(M_PIF));
              StartCrackShellEffect(mgr, *xa38_intermediateCrackParticle, xf,
                                    x570_babyData.GetShellCrackSfx(), false);
            }
          }
          KnockBack(weapon->GetTransform().GetForward(), mgr, weapon->GetCurrentDamageInfo(),
                    weapon->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
        }
      }
      xa48_24_isAlert = true;
      x8e8_interestTimer = 0.f;
      mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    } else {
      ReDirectDamage(mgr, uid);
    }
    x400_24_hitByPlayerProjectile = true;
    break;
  case kSM_InvulnDamage:
    mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    x400_24_hitByPlayerProjectile = true;
    xa48_24_isAlert = true;
    x8e8_interestTimer = 0.f;
    if (!TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      ReDirectDamage(mgr, uid);
    }
    break;
  case kSM_SuspendedMove:
    if (x928_colActMgr.get()) {
      x928_colActMgr->SetMovable(mgr, false);
    }
    break;
  }
  if (callBase) {
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
  }
}

void CBabygoth::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CBabygoth::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CPatterned::PreRender(mgr, frustum);
  CModelFlags flags = GetModelFlags();
  flags = flags.UseShaderSet(xa04_drawMaterialIdx);
  SetModelFlags(flags);
}

void CBabygoth::KnockBack(const CVector3f& direction, CStateManager& mgr, const CDamageInfo& damage,
                          float magnitude, bool direct, const bool deferred) {
  KnockBackCtrl().EnableAnimReaction(kAR_Hurled, x56c_shellState == kSCS_Destroyed);
  CPatterned::KnockBack(direction, mgr, damage, magnitude, direct, deferred);
  if (x400_25_alive) {
    switch (KnockBackCtrl().GetActiveParms().x0_animState) {
    case kAR_Hurled:
      x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("GetUp"));
      break;
    }
  }
}

void CBabygoth::Shock(CStateManager& mgr, float duration, float damage) {
  if (!x9f8_shellIds.empty()) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(x9f8_shellIds[0]))) {
      const CWeaponMode weapon(kWT_Wave);
      switch (
          actor->GetDamageVulnerability()->GetVulnerability(weapon, CDamageVulnerability::kRD_No)) {
      case kVN_Weak:
        x450_bodyController->SetElectrocuting(1.5f * duration);
        x3f0_pendingShockDamage = 1.5f * damage;
        break;
      case kVN_Normal:
        x450_bodyController->SetElectrocuting(duration);
        x3f0_pendingShockDamage = damage;
        break;
      }
    }
  }
}

void CBabygoth::TakeDamage(const CVector3f&, float) {
  if (x400_25_alive) {
    x428_damageCooldownTimer = skDamageHitTime;
  }
}

const CDamageVulnerability* CBabygoth::GetDamageVulnerability() const {
  return &CDamageVulnerability::PassThroughVulnerability();
}

const CDamageVulnerability* CBabygoth::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                              const CDamageInfo&) const {
  return &CDamageVulnerability::PassThroughVulnerability();
}

bool CBabygoth::Listen(const CVector3f& origin, EListenNoiseType noiseType) {
  bool heard = false;
  if (x400_25_alive && noiseType == kLNT_PlayerFire) {
    const CVector3f delta = origin - GetTranslation();
    if (delta.MagSquared() < 1600.f) {
      xa48_30_heardPlayerFire = heard = true;
    }
  }
  const bool result = heard;
  return result;
}

CVector3f CBabygoth::GetOrigin(const CStateManager&, const CTeamAiRole&,
                               const CVector3f& aimPos) const {
  CVector3f result = GetTranslation();
  CVector3f delta = GetTranslation() - aimPos;
  delta[kDZ] = 0.f;
  if (delta.CanBeNormalized()) {
    result = aimPos + delta.AsNormalized() * x2fc_minAttackRange;
  }
  return result;
}

void CBabygoth::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f xf = GetLctrTransform(node.GetLocatorName());
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    CVector3f targetPos = ProjectileInfo()->PredictInterceptPos(xf.GetTranslation(), aimPos,
                                                                *mgr.GetPlayer(), false, dt);
    const CVector3f forward = GetTransform().GetForward();
    const CVector3f delta = targetPos - xf.GetTranslation();
    if (CVector3f::GetAngleDiff(forward, delta) > CMath::Deg2Rad(30.f)) {
      if (delta.CanBeNormalized()) {
        const CVector3f direction =
            CVector3f::Slerp(forward, delta.AsNormalized(), CRelAngle::FromDegrees(30.f));
        targetPos = xf.GetTranslation() + direction * delta.Magnitude();
      } else {
        targetPos = xf.GetTranslation() + xf.GetForward() * delta.Magnitude();
      }
    }
    const CTransform4f launchXf =
        CTransform4f::LookAt(xf.GetTranslation(), targetPos, CVector3f::Up());
    LaunchProjectile(launchXf, mgr, 4, CWeapon::kPA_None, false,
                     rstl::optional_object< TLockedToken< CGenDescription > >(),
                     CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
    handled = true;
    break;
  }
  case kUE_ScreenShake:
    handled = true;
    break;
  case kUE_BecomeShootThrough:
    SetShootThrough(mgr);
    handled = true;
    break;
  case kUE_DamageOn:
    if (xa48_26_inProjectileAttack) {
      if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(x980_flameThrower))) {
        flame->Fire(GetTransform(), mgr, false);
      }
    }
    break;
  case kUE_DamageOff:
    if (xa48_26_inProjectileAttack) {
      if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(x980_flameThrower))) {
        flame->Reset(mgr, false);
      }
    }
    break;
  case kUE_BeginAction:
    if (xa48_26_inProjectileAttack) {
      x8f0_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
      x8f0_boneTracking.SetActive(true);
    }
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

CVector3f CBabygoth::GetAimPosition(const CStateManager& mgr, float) const {
  if (x450_bodyController->GetLocomotionType() != pas::kLT_Crouch && !x9f8_shellIds.empty()) {
    CVector3f shellPos = CVector3f::Zero();
    for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
      if (const CCollisionActor* actor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(*it))) {
        shellPos += actor->GetTranslation() + actor->GetSphereRadius() * CVector3f::Up();
      }
    }
    if (shellPos.IsNonZero()) {
      shellPos /= static_cast< float >(x9f8_shellIds.size());
    }
    CVector3f mouthPos = shellPos;
    if (const CCollisionActor* actor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x9f6_mouthCollisionActor))) {
      mouthPos[kDZ] = actor->GetTranslation().GetZ();
    }
    const CVector2f forward = GetTransform().GetForward().ToVec2f();
    const CVector2f direction = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
    const float factor = CVector2f::GetAngleDiff(forward, direction) / M_PIF;
    return CVector3f::Lerp(mouthPos, shellPos, factor);
  }
  return CPatterned::GetAimPosition(mgr, 0.f);
}

bool CBabygoth::TooClose(CStateManager& mgr, float) {
  const CAABox playerBounds = mgr.GetPlayer()->GetBoundingBox();
  const CAABox bounds = x930_aabox.CalculateAABox(GetTransform());
  if (bounds.DoBoundsOverlap(playerBounds)) {
    xa49_25_shouldStepBackwards = true;
    return true;
  }
  return false;
}

bool CBabygoth::AggressionCheck(CStateManager&, float) {
  if (x400_25_alive && !xa48_29_hasBeenEnraged && x56c_shellState == kSCS_Destroyed) {
    return true;
  }
  return false;
}

bool CBabygoth::InMaxRange(CStateManager& mgr, float) {
  return (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
         (1.5f * x300_maxAttackRange) * (1.5f * x300_maxAttackRange);
}

bool CBabygoth::ShotAt(CStateManager&, float) { return x400_24_hitByPlayerProjectile; }

bool CBabygoth::OffLine(CStateManager& mgr, float) {
  SetPathFindMode(kPFM_Normal);
  return PathShagged(mgr, 0.f);
}

bool CBabygoth::LostInterest(CStateManager&, float) {
  if (x8e8_interestTimer >= x570_babyData.GetInterestTime()) {
    return x6ec_pathSearch.OnPath(GetTranslation()) == CPathFindSearch::kR_Success;
  }
  return false;
}

bool CBabygoth::LineOfSight(CStateManager& mgr, float) {
  const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  const CTransform4f mouthXf = GetLctrTransform(x9f4_mouthLocator);
  const CVector3f mouthPos = mouthXf.GetTranslation();
  return !IsPatternObstructed(mgr, mouthPos, aimPos);
}

bool CBabygoth::Leash(CStateManager& mgr, float) {
  const CVector3f delta = x3a0_latestLeashPosition - GetTranslation();
  if (delta.MagSquared() > x3c8_leashRadius * x3c8_leashRadius) {
    const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    return playerDelta.MagSquared() > x3cc_playerLeashRadius * x3cc_playerLeashRadius &&
           x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
  }
  return false;
}

bool CBabygoth::ShouldAttack(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  if (GetCurrentAreaId() == player.GetCurrentAreaId() && !player.GetFrozenState() &&
      x8d8_attackTimeLeft <= 0.f && x8e0_attackCooldownTimeLeft <= 0.f) {
    const CVector3f aimPos = player.GetAimPosition(mgr, 0.f);
    const CVector3f delta = aimPos - GetTranslation();
    const float distance = delta.Magnitude();
    if (x450_bodyController->GetBodyStateInfo().GetMaxSpeed() * x570_babyData.GetTimeUntilAttack() >
            distance &&
        !ShouldTurn(mgr, CRelAngle::FromDegrees(25.f).AsRadians())) {
      const CTransform4f mouthXf = GetLctrTransform(x9f4_mouthLocator);
      const CVector3f mouthPos = mouthXf.GetTranslation();
      if (!IsPatternObstructed(mgr, mouthPos, aimPos)) {
        if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x6e8_teamMgr))) {
          if (team->IsPartOfTeam(GetUniqueId())) {
            return team->AddMeleeAttacker(GetUniqueId());
          }
        }
        return true;
      }
    }
  }
  return false;
}

bool CBabygoth::ShouldSpecialAttack(CStateManager& mgr, float) {
  if (GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId() &&
      x8e4_fireballAttackTimeLeft <= 0.f) {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f mouthXf = GetLctrTransform(x9f4_mouthLocator);
    const CVector3f mouthPos = mouthXf.GetTranslation();
    const CVector3f delta = aimPos - mouthPos;
    if (delta.MagSquared() >= x300_maxAttackRange * x300_maxAttackRange &&
        !ShouldTurn(mgr, CRelAngle::FromDegrees(25.f).AsRadians()) &&
        !IsPatternObstructed(mgr, mouthPos, aimPos)) {
      return true;
    }
  }
  return false;
}

bool CBabygoth::ShouldFire(CStateManager& mgr, float) {
  if (GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId() && x8d8_attackTimeLeft <= 0.f) {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f mouthXf = GetLctrTransform(x9f4_mouthLocator);
    const CVector3f mouthPos = mouthXf.GetTranslation();
    const CVector3f delta = aimPos - mouthPos;
    if (delta.MagSquared() <= x300_maxAttackRange * x300_maxAttackRange &&
        !ShouldTurn(mgr, CMath::Deg2Rad(30.f))) {
      return !IsPatternObstructed(mgr, mouthPos, aimPos);
    }
  }
  return false;
}

bool CBabygoth::ShouldTurn(CStateManager& mgr, float arg) {
  const float angle = arg == 0.f ? CMath::Deg2Rad(45.f) : arg;
  const float speed = GetModelData()->GetAnimationData()->GetPlaybackRate();
  const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, speed > 0.f ? 1.f / speed : 0.f);
  const CVector2f direction = (aimPos - GetTranslation()).ToVec2f();
  const CVector2f forward = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward, direction) > angle;
}

bool CBabygoth::InPosition(CStateManager&, float) {
  const CVector3f delta = x8b8_backupDestPos - GetTranslation();
  return delta.MagSquared() < 9.f;
}

bool CBabygoth::InDetectionRange(CStateManager& mgr, float arg) {
  bool result = xa48_24_isAlert;
  if (!result) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    const float scale = arg > 0.f ? arg : 1.f;
    const float range = scale * x3bc_detectionRange;
    if (delta.MagSquared() < range * range) {
      if (x3c0_detectionHeightRange > 0.f) {
        return delta.GetZ() * delta.GetZ() < x3c0_detectionHeightRange * x3c0_detectionHeightRange;
      }
      return true;
    }
  }
  return result;
}

bool CBabygoth::SpotPlayer(CStateManager& mgr, float arg) {
  return xa48_24_isAlert ? xa48_24_isAlert : CPatterned::SpotPlayer(mgr, arg);
}

bool CBabygoth::AnimOver(CStateManager&, float) { return x568_stateProg == 4; }

void CBabygoth::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    xa48_26_inProjectileAttack = true;
    x8e8_interestTimer = 0.f;
    xa49_27_locomotionValid = false;
    CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x6e8_teamMgr));
    if (teamMgr && teamMgr->HasTeamAiRole(GetUniqueId())) {
      const bool added = teamMgr->AddProjectileAttacker(GetUniqueId());
      int state = 4;
      if (added) {
        state = 0;
      }
      x568_stateProg = state;
    } else {
      x568_stateProg = 0;
    }
    break;
  }
  case kStateMsg_Update: {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() ==
          pas::kAS_ProjectileAttack) {
        x568_stateProg = 3;
        x3b4_speed = 2.f * x8d0_initialSpeed;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Two, aimPos, false));
      }
      break;
    case 3:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() !=
          pas::kAS_ProjectileAttack) {
        x568_stateProg = 4;
      } else if (!mgr.ObjectById(x980_flameThrower)) {
        x450_bodyController->CommandMgr().DeliverTargetVector(aimPos - GetTranslation());
        x8f0_boneTracking.UnsetTarget();
        x8f0_boneTracking.SetTargetPosition(aimPos);
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(x980_flameThrower))) {
      flame->Reset(mgr, false);
    }
    xa48_26_inProjectileAttack = false;
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    x8f0_boneTracking.SetActive(false);
    x3b4_speed = x8d0_initialSpeed;
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, x6e8_teamMgr, GetUniqueId(), false);
    break;
  }
}

void CBabygoth::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    xa48_25_ = xa48_28_pendingAttackContactDamage = true;
    x8e8_interestTimer = 0.f;
    xa49_25_shouldStepBackwards = false;
    x2e0_destPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, msg, dt);
    x8dc_attackTimer = 0.f;
    x450_bodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    x450_bodyController->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    ExtendTouchBounds(mgr, skAttackTouchBounds);
    break;
  case kStateMsg_Update:
    ProcessCharge(mgr, dt);
    break;
  case kStateMsg_Deactivate: {
    UpdateAttackTimeLeft(mgr);
    xa48_25_ = xa48_28_pendingAttackContactDamage = false;
    x8e0_attackCooldownTimeLeft = x570_babyData.GetAttackCooldownTime();
    x8dc_attackTimer = 0.f;
    const float maxSpeed = x450_bodyController->GetBodyStateInfo().GetMaxSpeed();
    const float speed =
        maxSpeed > 0.f
            ? x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk) / maxSpeed
            : 1.f;
    x450_bodyController->CommandMgr().SetSteeringBlendMode(kSBM_Clamped);
    x450_bodyController->CommandMgr().SetSteeringSpeedRange(0.f, speed);
    ExtendTouchBounds(mgr, CVector3f::Zero());
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, x6e8_teamMgr, GetUniqueId(), true);
    break;
  }
  }
}

void CBabygoth::SpecialAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    xa48_27_ = true;
    x8e8_interestTimer = 0.f;
    xa49_27_locomotionValid = false;
    CTeamAiMgr* const teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x6e8_teamMgr));
    if (teamMgr && teamMgr->HasTeamAiRole(GetUniqueId())) {
      const bool added = teamMgr->AddProjectileAttacker(GetUniqueId());
      int state = 4;
      if (added) {
        state = 0;
      }
      x568_stateProg = state;
    } else {
      x568_stateProg = 0;
    }
    break;
  }
  case kStateMsg_Update: {
    const CVector3f target = mgr.GetPlayer()->GetTranslation();
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() ==
          pas::kAS_ProjectileAttack) {
        x568_stateProg = 3;
        x3b4_speed = 2.f * x8d0_initialSpeed;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_One, target, false));
      }
      break;
    case 3:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() !=
          pas::kAS_ProjectileAttack) {
        x568_stateProg = 4;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    xa48_27_ = false;
    x3b4_speed = x8d0_initialSpeed;
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, x6e8_teamMgr, GetUniqueId(), false);
    break;
  }
}

void CBabygoth::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    xa49_28_onApproachPath = false;
    xa49_26_readyForTeam = true;
    xa48_24_isAlert = false;
    x8e8_interestTimer = 0.f;
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    x8f0_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x8f0_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    x8b8_backupDestPos = x2e0_destPos;
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, msg, dt);
    break;
  case kStateMsg_Update:
    SetPathFindMode(kPFM_Normal);
    if (GetSearchPath() && !PathShagged(mgr, 0.f) &&
        x6ec_pathSearch.GetCurrentWaypoint() < x6ec_pathSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, msg, dt);
      x8e8_interestTimer = 0.f;
      const CVector3f move = x450_bodyController->CommandMgr().GetMoveVector();
      if (move.CanBeNormalized()) {
        const CVector3f arrival =
            x45c_steeringBehaviors.Arrival(*this, mgr.GetPlayer()->GetTranslation(), 15.f);
        const float magnitude = arrival.Magnitude();
        x450_bodyController->CommandMgr().ClearLocomotionCmds();
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(magnitude * move.AsNormalized(), CVector3f::Zero(), 10.f));
      }
      ApplySeparationBehavior(mgr);
      const CVector3f forward = GetTransform().GetForward();
      const CVector3f newMove = x450_bodyController->CommandMgr().GetMoveVector();
      if (CVector3f::Dot(forward, newMove) < 0.f && move.CanBeNormalized()) {
        x450_bodyController->CommandMgr().ClearLocomotionCmds();
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), newMove.AsNormalized(), 1.f));
      }
    } else {
      const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      if (delta.CanBeNormalized()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    x8f0_boneTracking.SetActive(false);
    break;
  }
}

void CBabygoth::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    x8f0_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x8f0_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
    if (!xa49_27_locomotionValid) {
      x450_bodyController->CommandMgr().ClearLocomotionCmds();
    }
    xa48_31_approachNeedsPathSearch = true;
    xa49_28_onApproachPath = true;
    break;
  case kStateMsg_Update:
    SetPathFindMode(kPFM_Approach);
    if (xa48_31_approachNeedsPathSearch) {
      x2e0_destPos = x8b8_backupDestPos;
      if (x7d0_approachPathSearch.FindClosestReachablePoint(GetTranslation(), x2e0_destPos) ==
          CPathFindSearch::kR_Success) {
        const CVector3f delta = x2e0_destPos - GetTranslation();
        if (delta.MagSquared() < 10.f) {
          x2e0_destPos = GetTranslation();
        }
        x8b8_backupDestPos = x2e0_destPos;
        CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
        if (!xa49_27_locomotionValid) {
          x450_bodyController->CommandMgr().ClearLocomotionCmds();
        }
      }
      xa48_31_approachNeedsPathSearch = false;
    }
    xa49_27_locomotionValid &= !IsOtherCharacterNearPathDest(mgr);
    if (xa49_27_locomotionValid && GetSearchPath() && !PathShagged(mgr, 0.f) &&
        x7d0_approachPathSearch.GetCurrentWaypoint() <
            x7d0_approachPathSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, msg, dt);
      ApplySeparationBehavior(mgr);
    } else {
      const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      if (ShouldTurn(mgr, CRelAngle::FromDegrees(25.f).AsRadians()) && delta.CanBeNormalized()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    xa49_27_locomotionValid = true;
    break;
  case kStateMsg_Deactivate:
    x8f0_boneTracking.SetActive(false);
    SetPathFindMode(kPFM_Normal);
    break;
  }
}

void CBabygoth::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Update) {
    ApplySeparationBehavior(mgr);
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CBabygoth::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    xa49_26_readyForTeam = false;
    RemoveFromTeam(mgr);
    x400_24_hitByPlayerProjectile = false;
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    if (HasPatrolPath(mgr, 0.f)) {
      CPatterned::Patrol(mgr, msg, dt);
      UpdateDest(mgr);
    } else {
      SetDestPos(x3a0_latestLeashPosition);
    }
    x8b8_backupDestPos = x2e0_destPos;
    if (GetSearchPath()) {
      SetPathFindMode(kPFM_Normal);
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update:
    if (GetSearchPath() && !PathShagged(mgr, 0.f)) {
      SetPathFindMode(kPFM_Normal);
      CPatterned::PathFind(mgr, msg, dt);
      ApplySeparationBehavior(mgr);
    } else {
      const CVector3f arrival = x45c_steeringBehaviors.Arrival(*this, x8b8_backupDestPos, 9.f);
      x450_bodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
    }
    break;
  }
}

void CBabygoth::Generate(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    x8c4_initialFaceDir = GetTransform().GetForward();
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
        x568_stateProg = 3;
        SendScriptMsgs(kSS_Attack, mgr, kSM_None);
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Zero, CVector3f::Zero()));
      }
      break;
    case 3:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_stateProg = 4;
      }
      break;
    }
    break;
  }
}

void CBabygoth::Deactivate(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 1;
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 1: {
      const CVector3f& dest = x3a0_latestLeashPosition;
      const CVector3f delta = dest - GetTranslation();
      if (delta.MagSquared() <= 1.f) {
        x568_stateProg = 2;
      } else {
        const CVector3f arrival = x45c_steeringBehaviors.Arrival(*this, dest, 15.f);
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
      }
      break;
    }
    case 2: {
      const CVector3f forward = GetTransform().GetForward();
      if (CVector3f::GetAngleDiff(forward, x8c4_initialFaceDir) >
          CRelAngle::FromDegrees(5.f).AsRadians()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), x8c4_initialFaceDir, 1.f));
      } else {
        x568_stateProg = 0;
      }
      break;
    }
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        x568_stateProg = 3;
        x450_bodyController->SetLocomotionType(pas::kLT_Crouch);
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_One, CVector3f::Zero()));
      }
      break;
    case 3:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_stateProg = 4;
      }
      break;
    }
    break;
  }
}

void CBabygoth::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
    x450_bodyController->SetLocomotionType(pas::kLT_Crouch);
    xa48_30_heardPlayerFire = xa48_24_isAlert = false;
    x400_24_hitByPlayerProjectile = false;
    x8e8_interestTimer = 0.f;
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  }
}

void CBabygoth::Taunt(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_One));
    x568_stateProg = 3;
    break;
  case kStateMsg_Update:
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Taunt) {
      x568_stateProg = 4;
    }
    break;
  }
}

void CBabygoth::FollowPattern(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    int state = 4;
    if (xa49_25_shouldStepBackwards) {
      state = 0;
    }
    x568_stateProg = state;
    xa49_25_shouldStepBackwards = false;
    x8f0_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x8f0_boneTracking.SetActive(true);
    break;
  }
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      x450_bodyController->CommandMgr().DeliverCmd(
          CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      x568_stateProg = 3;
      break;
    case 3:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Step) {
        x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      } else {
        x568_stateProg = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x8f0_boneTracking.SetActive(false);
    SetPathFindMode(kPFM_Normal);
    break;
  }
}

void CBabygoth::Enraged(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    xa48_29_hasBeenEnraged = true;
    x568_stateProg = 0;
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        x568_stateProg = 3;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Three, CVector3f::Zero()));
      }
      break;
    case 3:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_stateProg = 4;
      }
      break;
    }
    break;
  }
}

void CBabygoth::GetUp(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    xa49_24_gettingUp = true;
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Getup) {
        x568_stateProg = 3;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      }
      break;
    case 3:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Getup) {
        x568_stateProg = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    xa49_24_gettingUp = false;
    break;
  }
}

void CBabygoth::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x8f0_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x8f0_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
    x450_bodyController->CommandMgr().ClearLocomotionCmds();
    break;
  case kStateMsg_Update:
    if (ShouldTurn(mgr, CRelAngle::FromDegrees(25.f).AsRadians())) {
      const float speed = GetAnimationData()->GetPlaybackRate();
      const CVector3f aimPos =
          mgr.GetPlayer()->GetAimPosition(mgr, speed > 0.f ? 1.f / speed : 0.f);
      CVector3f delta = aimPos - GetTranslation();
      delta[kDZ] = 0.f;
      if (delta.CanBeNormalized()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    x8f0_boneTracking.SetActive(false);
    break;
  }
}

void CBabygoth::UpdateTouchBounds() {
  const CAABox bounds(CVector3f(-1.5f, -1.5f, 0.f), CVector3f(1.5f, 1.5f, 2.f));
  SetBoundingBox(bounds);
  x930_aabox.Box() = bounds;
}

void CBabygoth::UpdateAttackPosition(CStateManager& mgr, CVector3f& attackPos) {
  attackPos = GetTranslation();
  if (x8d8_attackTimeLeft <= 0.f) {
    attackPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    CVector3f delta = GetTranslation() - attackPos;
    delta[kDZ] = 0.f;
    if (delta.CanBeNormalized()) {
      attackPos += delta.AsNormalized() * x2fc_minAttackRange;
    }
  }
}

void CBabygoth::ApplyContactDamage(TUniqueId uid, CStateManager& mgr) {
  if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
    if (HealthInfo(mgr)->GetHP() > 0.f) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (touched == mgr.GetPlayer()->GetUniqueId()) {
        if (xa48_28_pendingAttackContactDamage) {
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
              x570_babyData.GetAttackContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          xa48_28_pendingAttackContactDamage = false;
          x420_curDamageRemTime = x424_damageWaitTime;
        } else if (x420_curDamageRemTime <= 0.f) {
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      }
    }
  }
}

void CBabygoth::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(5);
  AddSphereCollisionList(skSphereJointList, 5, joints);
  x928_colActMgr =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  x928_colActMgr->SetActive(mgr, GetActive());
  for (uint i = 0; i < x928_colActMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x928_colActMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      actor->SetDamageVulnerability(x570_babyData.GetMouthVulnerabilities());
      if (desc.GetName() == rstl::string_l(skpMouthDamageJoint)) {
        x9f6_mouthCollisionActor = uid;
      } else if (desc.GetName() == rstl::string_l(skpPelvisDamageJoint) ||
                 desc.GetName() == rstl::string_l(skpButtDamageJoint)) {
        x9f8_shellIds.push_back(uid);
        actor->SetWeaponCollisionResponseType(kWCR_Unknown66);
      }
    }
  }
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_AIPassthrough, kMT_Player)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  x928_colActMgr->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
}

void CBabygoth::AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                                       rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetModelData()->GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId seg = animData.GetLocatorSegId(rstl::string_l(joints[i].x0_name));
    if (seg != CSegId(0xff)) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          seg, joints[i].x4_radius, rstl::string_l(joints[i].x0_name), 1000.f);
      list.push_back(desc);
    }
  }
}

bool CBabygoth::IsMouthCollisionActor(const TUniqueId& uid) const {
  return uid == x9f6_mouthCollisionActor;
}

bool CBabygoth::IsShellCollisionActor(const TUniqueId& uid) const {
  for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
    if (*it == uid) {
      return true;
    }
  }
  return false;
}

void CBabygoth::ExtendTouchBounds(CStateManager& mgr, const CVector3f& bounds) const {
  for (uint i = 0; i < x928_colActMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x928_colActMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      actor->SetExtendedTouchBounds(bounds);
    }
  }
}

void CBabygoth::PreventPlayerInterpenetration(CStateManager& mgr, float dt) {
  if (x450_bodyController->GetLocomotionType() != pas::kLT_Crouch &&
      x450_bodyController->GetCurrentStateId() != pas::kAS_Step) {
    const rstl::optional_object< CVector3f > deviation =
        x928_colActMgr->GetDeviation(mgr, x9f4_mouthLocator);
    if (deviation) {
      const CPlayer& player = *mgr.GetPlayer();
      const CAABox bounds = GetModelData()->GetBounds(GetTransform());
      const CAABox playerBounds = player.GetBoundingBox();
      if (bounds.DoBoundsOverlap(playerBounds)) {
        const float magnitude = deviation->Magnitude();
        const float minimum = 0.9f * GetModelData()->ScaleCopy().GetY();
        if (magnitude > minimum) {
          const float excess = magnitude - minimum;
          const CVector3f delta = GetTranslation() - player.GetTranslation();
          const CVector3f worldOffset =
              (CVector3f::Dot(delta, excess * deviation->AsNormalized()) / delta.MagSquared()) *
              delta;
          const CVector3f localOffset = GetTransform().TransposeRotate(worldOffset);
          const CVector3f impulse = GetMoveToORImpulseWR(localOffset, dt);
          ApplyImpulseWR(impulse, CAxisAngle::Identity());
        }
      }
    }
  }
}

void CBabygoth::SetupHealthInfo(CStateManager& mgr) {
  const CHealthInfo& health = *HealthInfo(mgr);
  x8ec_bodyHP = health.GetHP();
  if (CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(x9f6_mouthCollisionActor))) {
    actor->HealthInfo(mgr)->SetHP(x570_babyData.GetShellHitPoints());
  }
  for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      *actor->HealthInfo(mgr) =
          CHealthInfo(x570_babyData.GetShellHitPoints(), health.GetKnockBackResistance());
      actor->SetDamageVulnerability(x570_babyData.GetShellDamageVulnerability());
    }
  }
  xa00_shellHitPoints = x570_babyData.GetShellHitPoints();
}

void CBabygoth::SetupShellDestroyedHealthInfo(CStateManager& mgr) {
  const CHealthInfo& health = *HealthInfo(mgr);
  if (CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(x9f6_mouthCollisionActor))) {
    *actor->HealthInfo(mgr) = health;
    actor->SetDamageVulnerability(x98c_dVuln);
  }
  for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      *actor->HealthInfo(mgr) = health;
      actor->SetDamageVulnerability(x98c_dVuln);
    }
  }
}

void CBabygoth::UpdateHealthInfo(CStateManager& mgr) {
  if (x400_25_alive) {
    if (x56c_shellState == kSCS_Destroyed) {
      float damage = 0.f;
      if (CCollisionActor* actor =
              TCastToPtr< CCollisionActor >(mgr.ObjectById(x9f6_mouthCollisionActor))) {
        damage = CMath::Max(damage, x8ec_bodyHP - actor->GetHealthInfo(mgr)->GetHP());
      }
      for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
        if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
          damage = CMath::Max(damage, x8ec_bodyHP - actor->GetHealthInfo(mgr)->GetHP());
        }
      }
      HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - damage);
      if (HealthInfo(mgr)->GetHP() <= 0.f) {
        Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
        xa48_26_inProjectileAttack = true;
        xa49_26_readyForTeam = false;
        RemoveFromTeam(mgr);
        RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      } else {
        if (CCollisionActor* actor =
                TCastToPtr< CCollisionActor >(mgr.ObjectById(x9f6_mouthCollisionActor))) {
          actor->HealthInfo(mgr)->SetHP(x8ec_bodyHP);
        }
        for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
          if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
            actor->HealthInfo(mgr)->SetHP(x8ec_bodyHP);
          }
        }
      }
    } else {
      UpdateShellHealthInfo(mgr);
    }
  }
}

void CBabygoth::UpdateShellHealthInfo(CStateManager& mgr) {
  if (xa00_shellHitPoints > 0.f) {
    float damage = 0.f;
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(x9f6_mouthCollisionActor))) {
      damage = CMath::Max(damage,
                          x570_babyData.GetShellHitPoints() - actor->GetHealthInfo(mgr)->GetHP());
    }
    for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
      if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
        damage = CMath::Max(damage,
                            x570_babyData.GetShellHitPoints() - actor->GetHealthInfo(mgr)->GetHP());
      }
    }
    xa00_shellHitPoints -= damage;
    if (xa00_shellHitPoints <= 0.f) {
      x56c_shellState = kSCS_Destroyed;
      SwapSkinnedModel(mgr);
      StartCrackShellEffect(mgr, xa2c_destroyShellParticle, GetTransform(),
                            x570_babyData.GetDestroyShellSfx(), false);
      SetupShellDestroyedHealthInfo(mgr);
    } else {
      if (xa00_shellHitPoints < GetShellStateHP(kSCS_CrackTwo)) {
        if (x56c_shellState != kSCS_CrackTwo) {
          StartCrackShellEffect(mgr, xa20_crackTwoParticle, GetTransform(),
                                x570_babyData.GetCrackTwoSfx(), false);
          x56c_shellState = kSCS_CrackTwo;
          xa04_drawMaterialIdx = 2;
        }
      } else if (xa00_shellHitPoints < GetShellStateHP(kSCS_CrackOne) &&
                 x56c_shellState != kSCS_CrackOne) {
        StartCrackShellEffect(mgr, xa14_crackOneParticle, GetTransform(),
                              x570_babyData.GetCrackOneSfx(), false);
        x56c_shellState = kSCS_CrackOne;
        xa04_drawMaterialIdx = 1;
      }
    }
    const float hp =
        x56c_shellState != kSCS_Destroyed ? x570_babyData.GetShellHitPoints() : x8ec_bodyHP;
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(x9f6_mouthCollisionActor))) {
      actor->HealthInfo(mgr)->SetHP(hp);
    }
    for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
      if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
        actor->HealthInfo(mgr)->SetHP(hp);
      }
    }
  }
}

void CBabygoth::UpdateAttackTimeLeft(CStateManager& mgr) {
  const float scale = x56c_shellState == kSCS_Destroyed ? 0.6f : 1.f;
  x8d8_attackTimeLeft =
      scale * (x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime);
  x8e4_fireballAttackTimeLeft =
      scale * (mgr.Random()->Float() * x570_babyData.GetFireballAttackVariance() +
               x570_babyData.GetFireballAttackTime());
}

void CBabygoth::UpdateAILogicTimers(float dt) {
  const float scale = xa49_28_onApproachPath ? 2.f : 1.f;
  if (x8d8_attackTimeLeft > 0.f) {
    x8d8_attackTimeLeft -= dt * scale;
  }
  if (x8e4_fireballAttackTimeLeft > 0.f) {
    x8e4_fireballAttackTimeLeft -= dt * scale;
  }
  if (x8e0_attackCooldownTimeLeft > 0.f) {
    x8e0_attackCooldownTimeLeft -= dt;
  }
  if (x8e8_interestTimer < x570_babyData.GetInterestTime()) {
    x8e8_interestTimer += dt;
  }
}

void CBabygoth::CreateFlameThrower(CStateManager& mgr) {
  if (x980_flameThrower == kInvalidUniqueId) {
    const CFlameInfo info(6, 4, x570_babyData.GetFireBreathResId(), 15, 0.0625f, 20.f, 1.f);
    x980_flameThrower = mgr.AllocateUniqueId();
    CFlameThrower* const flame = rs_new CFlameThrower(
        x984_flameThrowerDesc, rstl::string_l("IceSheegoth_Flame"), kWT_Plasma, info,
        CTransform4f::Identity(), kMT_CollisionActor, x570_babyData.GetFireBreathDamage(),
        x980_flameThrower, GetCurrentAreaId(), GetUniqueId(), CWeapon::kPA_None,
        x570_babyData.GetFlamePlayerSteamTxtr(), x570_babyData.GetFlamePlayerHitSfx(),
        x570_babyData.GetFlamePlayerIceTxtr());
    mgr.AddObject(*flame);
  }
}

void CBabygoth::UpdateParticleEffects(float dt, CStateManager& mgr) {
  if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(x980_flameThrower))) {
    if (flame->GetActive()) {
      const CTransform4f xf = GetLctrTransform(rstl::string_l("LCTR_SHEMOUTH"));
      flame->SetTransform(xf, mgr, dt);
    }
  }
}

void CBabygoth::SetPathFindMode(EPathFindMode mode) { x8b4_pathFindMode = mode; }

EWeaponCollisionResponseTypes CBabygoth::GetCollisionResponseType(const CVector3f& pos,
                                                                  const CVector3f& dir,
                                                                  const CWeaponMode& weapon,
                                                                  int attrib) const {
  if (weapon.GetType() == kWT_Ice) {
    return kWCR_None;
  }
  if (x56c_shellState != kSCS_Destroyed) {
    return kWCR_Unknown66;
  }
  return CPatterned::GetCollisionResponseType(pos, dir, weapon, attrib);
}

bool CBabygoth::IsOtherCharacterNearPathDest(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f delta = x8b8_backupDestPos - ai->GetTranslation();
        if (delta.MagSquared() <= 10.f) {
          return true;
        }
      }
    }
  }
  return false;
}

void CBabygoth::ApplySeparationBehavior(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f separation =
            x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), 15.f);
        if (separation.IsNonZero()) {
          x450_bodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CBabygoth::ProcessCharge(CStateManager& mgr, float dt) {
  switch (x568_stateProg) {
  case 0:
    x8dc_attackTimer += dt;
    if (x8dc_attackTimer < x570_babyData.GetTimeUntilAttack()) {
      if (!xa48_28_pendingAttackContactDamage) {
        x568_stateProg = 3;
        x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
      } else {
        const CTransform4f mouthXf = GetLctrTransform(x9f4_mouthLocator);
        const CVector3f delta = mgr.GetPlayer()->GetTranslation() - mouthXf.GetTranslation();
        const CVector3f forward = GetTransform().GetForward();
        if (CVector3f::Dot(forward, delta) > 0.f) {
          SetPathFindMode(kPFM_Normal);
          if (GetSearchPath() && !PathShagged(mgr, 0.f)) {
            CPatterned::PathFind(mgr, kStateMsg_Update, dt);
            ApplySeparationBehavior(mgr);
          } else {
            x568_stateProg = 4;
          }
        } else {
          x568_stateProg = 4;
        }
      }
    } else {
      x568_stateProg = 4;
    }
    break;
  case 3:
    if (x450_bodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
      x568_stateProg = 4;
    } else {
      xa49_25_shouldStepBackwards = true;
    }
    break;
  }
}

bool CBabygoth::CheckShouldGetUp(CStateManager& mgr) {
  if (x400_25_alive && x450_bodyController->GetFallState() != pas::kFS_Zero && !xa49_24_gettingUp) {
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("GetUp"));
  }
  return false;
}

bool CBabygoth::CheckShouldWakeUp(CStateManager& mgr, float dt) {
  if (xa48_30_heardPlayerFire) {
    xa48_24_isAlert = x400_24_hitByPlayerProjectile || mgr.Random()->Float() < 0.5f * dt;
    return xa48_24_isAlert;
  }
  return false;
}

void CBabygoth::SwapSkinnedModel(CStateManager& mgr) {
  AnimationData()->SubstituteModelData(xa08_noShellModel);
  for (AUTO(it, x9f8_shellIds.begin()); it != x9f8_shellIds.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      actor->SetWeaponCollisionResponseType(kWCR_Unknown41);
    }
  }
  xa04_drawMaterialIdx = 0;
}

void CBabygoth::StartCrackShellEffect(CStateManager& mgr,
                                      const TLockedToken< CGenDescription >& particle,
                                      const CTransform4f& xf, const ushort sfx, bool nonEmitter) {
  CExplosion* explosion =
      rs_new CExplosion(particle, mgr.AllocateUniqueId(), true,
                        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList),
                        rstl::string_l("Babygoth Shell Crack Fx"), xf, 0,
                        GetModelData()->ScaleCopy(), CColor::White());
  if (explosion) {
    mgr.AddObject(explosion);
    if (nonEmitter) {
      CSfxManager::SfxStart(sfx, CAudioSys::kMaxVolume, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else {
      CSfxManager::AddEmitter(sfx, GetTranslation(), CVector3f::Up(), false, false,
                              CSfxManager::kMedPriority, GetCurrentAreaId().Value());
    }
  }
}

float CBabygoth::GetShellStateHP(EShellCrackState state) const {
  float hp = 0.f;
  switch (state) {
  case kSCS_Default:
    hp = x570_babyData.GetShellHitPoints();
    break;
  case kSCS_CrackOne:
    hp = (2.f / 3.f) * x570_babyData.GetShellHitPoints();
    break;
  case kSCS_CrackTwo:
    hp = (1.f / 3.f) * x570_babyData.GetShellHitPoints();
    break;
  case kSCS_Destroyed:
    hp = 0.f;
    break;
  }
  return hp;
}

void CBabygoth::AddToTeam(CStateManager& mgr) {
  if (x6e8_teamMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x6e8_teamMgr))) {
      if (!team->IsPartOfTeam(GetUniqueId())) {
        team->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Projectile,
                               CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CBabygoth::RemoveFromTeam(CStateManager& mgr) {
  if (x6e8_teamMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x6e8_teamMgr))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

void CBabygoth::SetShootThrough(CStateManager& mgr) const {
  for (uint i = 0; i < x928_colActMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x928_colActMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
    }
  }
}

void CBabygoth::ReDirectDamage(CStateManager& mgr, TUniqueId uid) {
  if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(uid))) {
    if (!x9f8_shellIds.empty()) {
      CDamageInfo damage = weapon->GetCurrentDamageInfo();
      damage.SetRadius(0.f);
      mgr.ApplyDamage(
          uid, x9f8_shellIds[0], weapon->GetOwnerId(), damage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
  }
}
