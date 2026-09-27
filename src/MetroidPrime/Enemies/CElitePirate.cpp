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
: mTauntInterval(in.ReadFloat())
, mTauntVariance(in.ReadFloat())
, x8_(in.ReadFloat())
, xc_(in.ReadFloat())
, mAttackChance(in.ReadFloat())
, mShotAtTime(in.ReadFloat())
, mShotAtTimeVariance(in.ReadFloat())
, mProjectileAttractionRadius(in.ReadFloat())
, mEnergyAbsorbParticleDescId(in.ReadLong())
, mEnergyAbsorbSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, mLauncherActParams(LoadActorParameters(in))
, mLauncherAnimParams(LoadAnimationParameters(in))
, mLauncherParticleGenDescId(in.ReadLong())
, mLauncherSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, mGrenadeModelId(in.ReadLong())
, mGrenadeDamageInfo(in)
, mLauncherHp(in.ReadFloat())
, mGrenadeElementGenDescId1(in.ReadLong())
, mGrenadeElementGenDescId2(in.ReadLong())
, mGrenadeElementGenDescId3(in.ReadLong())
, mGrenadeElementGenDescId4(in.ReadLong())
, mGrenadeVelocityInfo(in)
, mGrenadeTrajectoryInfo(in)
, mGrenadeNumBounces(in.ReadLong())
, mGrenadeBounceSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, mGrenadeExplodeSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, mShockwaveParticleDescId(in.ReadLong())
, mShockwaveDamageInfo(in)
, mShockwaveWeaponDescId(in.ReadLong())
, mShockwaveElectrocuteSfxId(CSfxManager::TranslateSFXID(in.ReadLong()))
, mCanCallForBackup(in.ReadBool())
, mFastWhenAttractingEnergy(propCount >= 42 ? in.ReadBool() : true) {}

CElitePirate::CElitePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           const CElitePirateData& data)
: CPatterned(kC_ElitePirate, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actParms, kCS_Large)
, mState(kState_Invalid)
, mVulnerability(pInfo.GetDamageVulnerability())
, mCollisionActorMgr(nullptr)
, mData(data)
, mBoneTracking(*GetAnimationData(), rstl::string_l("Head_1"), 80.f * M_PIF / 180.f,
                    CRelAngle::FromDegrees(180.f).AsRadians(), kBTF_None)
, mCollisionActorMgrHead(nullptr)
, mCollisionAabb(GetBoundingBox(), GetMaterialList())
, mEnergyAbsorbDesc(data.GetEnergyAbsorbParticleDescId() != kInvalidAssetId
                            ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                  TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                                      SObjectTag('PART', data.GetEnergyAbsorbParticleDescId()))))
                            : rstl::optional_object< TLockedToken< CGenDescription > >())
, mCollisionHeadId(kInvalidUniqueId)
, mLauncherId(kInvalidUniqueId)
, mEnergyAttractorId(kInvalidUniqueId)
, mInitialSpeed(mSpeed)
, mSteeringSpeed(1.f)
, mPathShaggedTime(0.f)
, mEnergyAbsorbCooldown(0.f)
, x7b0_(1.f)
, mHp(0.f)
, mAttackTimer(0.f)
, mTauntTimer(0.f)
, mShotAtTimer(0.f)
, mAbsorbUpdateTimer(0.f)
, mCurrAnimId(-1)
, mActiveMaterialSet(0)
, mPathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mTargetDestPos(CVector3f::Zero())
, mPositionHistory(5.f)
, mDamageOn(false)
, mAttackingRightClaw(false)
, mAttackingLeftClaw(false)
, mShotAt(false)
, mAlert(false)
, mShockWaveAnim(false)
, mCalledForBackup(false)
, mRunning(false)
, mOnPath(false) {
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
    mBoneTracking.Update(dt);
    if (IsUsingBaseCollisionActors()) {
      mCollisionActorMgrHead->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    }
    mCollisionActorMgr->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    if (IsShieldActive() && mData.IsFastWhenAttractingEnergy()) {
      mSpeed = 2.f * mInitialSpeed;
    } else {
      mSpeed = mInitialSpeed;
    }
    UpdateAILogicTimers(dt);
    UpdateBreadCrumbTrail();
    UpdateGrenadeLauncher(mgr, mLauncherId, rstl::string_l(skpLauncherLCTR));
    UpdateHealthInfo(mgr);
    mEnergyAttractor = IsShieldActive();
  }
}

void CElitePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool shouldPass = true;
  switch (msg) {
  case kSM_Registered: {
    mBodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    mLauncherId = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, mLauncherId);
    const float maxSpeed = mBodyController->GetBodyStateInfo().GetMaxSpeed();
    if (maxSpeed > 0.f) {
      mSteeringSpeed =
          (0.99f * mBodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk)) /
          maxSpeed;
    }
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mBodyController->CommandMgr().SetSteeringSpeedRange(mSteeringSpeed, mSteeringSpeed);
    break;
  }
  case kSM_Activate:
    if (IsUsingBaseCollisionActors()) {
      mCollisionActorMgrHead->SetActive(mgr, true);
    }
    if (CEntity* entity = mgr.ObjectById(mLauncherId)) {
      entity->SetActive(true);
    }
    break;
  case kSM_Deactivate:
    if (IsUsingBaseCollisionActors()) {
      mCollisionActorMgrHead->SetActive(mgr, false);
    }
    mCollisionActorMgr->SetActive(mgr, false);
    if (CEntity* entity = mgr.ObjectById(mLauncherId)) {
      entity->SetActive(false);
    }
    break;
  case kSM_Deleted:
    if (IsUsingBaseCollisionActors()) {
      mCollisionActorMgrHead->Destroy(mgr);
    }
    mCollisionActorMgr->Destroy(mgr);
    mgr.DeleteObjectRequest(mLauncherId);
    break;
  case kSM_Alert:
    mAlert = true;
    break;
  case kSM_InitializedInArea: {
    const TAreaId areaId = GetCurrentAreaId();
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mPathArea);
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
        if (mDamageOn) {
          if ((mAttackingRightClaw && IsArmClawCollider(uid, mCollisionRJointIds)) ||
              (mAttackingLeftClaw && IsArmClawCollider(uid, mCollisionLJointIds))) {
            mgr.ApplyDamage(
                GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                CVector3f::Zero());
            mCurDamageRemTime = mDamageWaitTime;
            mDamageOn = false;
          }
        } else if (mCurDamageRemTime <= 0.f) {
          CDamageInfo info = GetContactDamage();
          info.SetDamage(0.5f * info.GetDamage());
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), info,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
        }
      } else if (TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(touchedUid))) {
        SetShotAt(true, mgr);
      }
    } else if (uid == mLauncherId && mLauncherId != kInvalidUniqueId) {
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
        if (uid == mCollisionHeadId) {
          mDamageCooldownTimer = skDamageHitTime;
          KnockBack(projectile->GetVelocity(), mgr, projectile->GetCurrentDamageInfo(),
                    projectile->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
          CPatterned::AcceptScriptMsg(msg, touchedUid, mgr);
        } else if (uid == mEnergyAttractorId && mEnergyAbsorbDesc) {
          StartAbsorbEnergyEffects(mgr, projectile->GetTransform());
        }
        SetShotAt(true, mgr);
      }
    } else if (uid == mLauncherId && mLauncherId != kInvalidUniqueId) {
      mBodyController->CommandMgr().DeliverCmd(
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
  mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), ModelData()->ScaleCopy(),
                              *mBodyController);
  const int numSets = GetModelData()->GetNumMaterialSets();
  SetModelFlags(GetModelFlags().UseShaderSet(
      rstl::min_val(static_cast< int >(mActiveMaterialSet), numSets - 1)));
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
  if (mCollisionActorMgr->GetActive() && playerState->IsFiringComboBeam() &&
      playerState->GetCurrentBeam() == CPlayerState::kBI_Wave) {
    if (const CCollisionActor* actor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mEnergyAttractorId))) {
      return actor->GetTranslation();
    }
  }
  return GetOrbitPosition(mgr);
}

CVector3f CElitePirate::GetOrbitPosition(const CStateManager& mgr) const {
  if (mLauncherId != kInvalidUniqueId &&
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal) {
    if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(mLauncherId))) {
      return GetGrenadeLaunchPos(*actor);
    }
  }
  if (IsUsingBaseCollisionActors()) {
    if (const CCollisionActor* actor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mCollisionHeadId))) {
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
    if (mLauncherId != kInvalidUniqueId) {
      if (CEntity* launcher = mgr.ObjectById(mLauncherId)) {
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
    mDamageOn = true;
    handled = true;
    break;
  case kUE_DamageOff:
    mDamageOn = false;
    handled = true;
    break;
  case kUE_ScreenShake:
    ProcessStompGround(mgr);
    handled = true;
    break;
  case kUE_BecomeShootThrough:
    if (IsUsingBaseCollisionActors()) {
      for (uint i = 0; i < mCollisionActorMgrHead->GetNumCollisionActors(); ++i) {
        const CJointCollisionDescription& desc =
            mCollisionActorMgrHead->GetCollisionDescFromIndex(i);
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
  if (mAlert) {
    return true;
  }
  return CPatterned::SpotPlayer(mgr, arg);
}

bool CElitePirate::InDetectionRange(CStateManager& mgr, float arg) {
  if (mAlert) {
    return true;
  }
  return CPatterned::InDetectionRange(mgr, arg);
}

bool CElitePirate::ShouldTurn(CStateManager& mgr, float) {
  const CVector2f dist = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
  const CVector2f forward = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward, dist) > CRelAngle::FromDegrees(15.f).AsRadians();
}

bool CElitePirate::ShouldTaunt(CStateManager&, float) { return mTauntTimer <= 0.f; }

bool CElitePirate::ShouldFire(CStateManager& mgr, float) {
  return ShouldFireLauncher(mgr, mLauncherId);
}

bool CElitePirate::ShouldFireLauncher(CStateManager& mgr, const TUniqueId uid) {
  if (mAttackTimer <= 0.f && uid != kInvalidUniqueId) {
    if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(uid))) {
      const CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
      const CVector3f delta = aim - GetTranslation();
      if (delta.MagSquared() >= mMaxAttackRange * mMaxAttackRange &&
          !ShouldTurn(mgr, 0.f)) {
        const CVector3f origin = GetGrenadeLaunchPos(*actor);
        if (!IsPatternObstructed(mgr, origin, aim)) {
          float angle = mData.GetGrenadeTrajectoryInfo().GetAngleMin();
          float velocity = mData.GetGrenadeTrajectoryInfo().GetVelocityMin();
          const CEPGrenadeLaunchParms& launchInfo = mData.GetGrenadeTrajectoryInfo();
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
  if (dist.MagSquared() <= mMinAttackRange * mMinAttackRange) {
    return !ShouldTurn(mgr, 0.f);
  }
  return false;
}

bool CElitePirate::ShouldSpecialAttack(CStateManager& mgr, float) {
  if (mAttackTimer <= 0.f && GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId()) {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CVector3f dist = aimPos - GetTranslation();
    const float magSquared = dist.MagSquared();
    if (magSquared >= mMinAttackRange * mMinAttackRange &&
        magSquared <= mMaxAttackRange * mMaxAttackRange) {
      return CMath::AbsF(dist.GetZ()) < 3.f;
    }
  }
  return false;
}

bool CElitePirate::ShouldCallForBackup(CStateManager& mgr, float arg) {
  return ShouldCallForBackupForLauncher(mgr, arg, mLauncherId);
}

bool CElitePirate::ShouldCallForBackupForLauncher(CStateManager&, float, TUniqueId uid) const {
  if (!mCalledForBackup && uid == kInvalidUniqueId && mData.CanCallForBackup()) {
    return mPathShaggedTime >= 3.f;
  }
  return false;
}

bool CElitePirate::ShotAt(CStateManager&, float) { return mShotAt; }

bool CElitePirate::InPosition(CStateManager&, float) {
  const CVector3f dist = mTargetDestPos - GetTranslation();
  return dist.MagSquared() < 25.f;
}

bool CElitePirate::TooClose(CStateManager& mgr, float) {
  const CVector3f dist = GetTranslation() - mgr.GetPlayer()->GetTranslation();
  return dist.MagSquared() < mMinAttackRange * mMinAttackRange;
}

bool CElitePirate::AggressionCheck(CStateManager& mgr, float arg) {
  if (mLauncherId == kInvalidUniqueId && !PathShagged(mgr, arg)) {
    if (!mRunning) {
      const CVector3f dist = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      return dist.MagSquared() > 4.f * mMaxAttackRange * mMaxAttackRange;
    }
    return true;
  }
  return false;
}

bool CElitePirate::AnimOver(CStateManager&, float) { return mState == kState_Over; }

void CElitePirate::Run(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mRunning = true;
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mBodyController->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking.SetActive(true);
    UpdatePathDestination(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    break;
  case kStateMsg_Update:
    if (!PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      const CVector3f move =
          mPositionHistory.GetValue(GetTranslation(), GetTransform().GetForward());
      if (move.IsNonZero()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      } else if (ShouldTurn(mgr, 0.f)) {
        const CVector3f aim =
            mgr.GetPlayer()->GetAimPosition(mgr, 0.5f * GetAnimationData()->GetPlaybackRate());
        const CVector3f face = aim - GetTranslation();
        if (face.CanBeNormalized()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), face.AsNormalized(), 1.f));
        }
      }
    }
    break;
  case kStateMsg_Deactivate:
    mRunning = false;
    mBoneTracking.SetActive(false);
    mBodyController->CommandMgr().SetSteeringSpeedRange(mSteeringSpeed, mSteeringSpeed);
    break;
  }
}

void CElitePirate::Halt(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
    mOnPath = false;
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
    mOnPath = true;
    mAlert = false;
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking.SetActive(true);
    UpdatePathDestination(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    mTauntTimer =
        mgr.Random()->Float() * mData.GetTauntVariance() + mData.GetTauntInterval();
    if (TooClose(mgr, 0.f)) {
      mBodyController->CommandMgr().ClearLocomotionCmds();
    }
    break;
  case kStateMsg_Update:
    if (mTauntTimer > 0.f) {
      mTauntTimer -= dt;
    }
    if (!TooClose(mgr, 0.f) && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else if (PathShagged(mgr, 0.f)) {
      const CVector3f move =
          mPositionHistory.GetValue(GetTranslation(), GetTransform().GetForward());
      if (move.IsNonZero()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
    } else if (ShouldTurn(mgr, 0.f)) {
      const CVector3f aim = mgr.GetPlayer()->GetAimPosition(
          mgr, 0.5f * GetModelData()->GetAnimationData()->GetPlaybackRate());
      const CVector3f face = aim - GetTranslation();
      if (face.CanBeNormalized()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), face.AsNormalized(), 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mBoneTracking.SetActive(false);
    break;
  }
}

void CElitePirate::Taunt(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_Zero;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Taunt) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
      }
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Taunt) {
        mState = kState_Over;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
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
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    if (HasPatrolPath(mgr, 0.f)) {
      CPatterned::Patrol(mgr, msg, dt);
      UpdateDest(mgr);
    } else {
      SetDestPos(mLatestLeashPosition);
    }
    mTargetDestPos = mDestPos;
    if (GetSearchPath()) {
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update:
    if (!PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      mBodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
          mSteeringBehaviors.Arrival(*this, mTargetDestPos, 25.f), CVector3f::Zero(), 1.f));
    }
    break;
  case kStateMsg_Deactivate:
    mAlert = false;
    break;
  }
}

void CElitePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mHitByPlayerProjectile = false;
    mOnPath = false;
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CElitePirate::Generate(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_One;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_One:
      if (ShouldTurn(mgr, 0.f)) {
        const CVector3f face = mgr.GetPlayer()->GetTranslation() - GetTranslation();
        if (face.CanBeNormalized()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), face.AsNormalized(), 1.f));
        }
      } else {
        mState = kState_Zero;
      }
      break;
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
      }
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Generate) {
        mState = kState_Over;
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
    mState = kState_Zero;
    break;
  case kStateMsg_Update: {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_One, playerPos, false));
      }
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        mState = kState_Over;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(playerPos - GetTranslation());
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
    mState = kState_Zero;
    break;
  case kStateMsg_Update: {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        mState = kState_Two;
        mShockWaveAnim = true;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Two, playerPos, false));
      }
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        mState = kState_Over;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(playerPos - GetTranslation());
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    mShockWaveAnim = false;
    break;
  }
}

void CElitePirate::Attack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_Zero;
    ExtendTouchBounds(mgr, mCollisionRJointIds, skExtendedClawBounds);
    if (GetModelData()->GetNumMaterialSets() > 1) {
      mActiveMaterialSet = 1;
    }
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        mState = kState_One;
        mAttackingRightClaw = true;
        mCurrAnimId = mBodyController->GetCurrentAnimId();
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
      }
      break;
    case kState_One:
      if (mBodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mState = kState_Over;
      } else if (mCurrAnimId != mBodyController->GetCurrentAnimId()) {
        mState = kState_Two;
        mAttackingRightClaw = false;
        mAttackingLeftClaw = true;
        ExtendTouchBounds(mgr, mCollisionRJointIds, CVector3f::Zero());
        ExtendTouchBounds(mgr, mCollisionLJointIds, skExtendedClawBounds);
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
        if (ShouldAttack(mgr, 0.f)) {
          mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Two));
        }
      }
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mState = kState_Over;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    mAttackingRightClaw = mAttackingLeftClaw = mDamageOn = false;
    mCurrAnimId = -1;
    ExtendTouchBounds(mgr, mCollisionRJointIds, CVector3f::Zero());
    ExtendTouchBounds(mgr, mCollisionLJointIds, CVector3f::Zero());
    mActiveMaterialSet = 0;
    break;
  }
}

void CElitePirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    if (IsUsingBaseCollisionActors()) {
      if (CCollisionActor* actor =
              TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionHeadId))) {
        actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
      }
    }
    mCollisionActorMgr->SetActive(mgr, true);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking.SetActive(true);
    UpdatePathDestination(mgr);
    CPatterned::PathFind(mgr, msg, dt);
    if (TooClose(mgr, 0.f)) {
      mBodyController->CommandMgr().ClearLocomotionCmds();
    }
    break;
  case kStateMsg_Update:
    if (mShotAt) {
      mShotAtTimer -= dt;
      if (mShotAtTimer <= 0.f) {
        mShotAt = false;
      }
    }
    mPathShaggedTime = PathShagged(mgr, 0.f) ? mPathShaggedTime + dt : 0.f;
    if (!TooClose(mgr, 0.f) && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else if (PathShagged(mgr, 0.f)) {
      const CVector3f move =
          mPositionHistory.GetValue(GetTranslation(), GetTransform().GetForward());
      if (move.IsNonZero()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
    } else if (ShouldTurn(mgr, 0.f)) {
      const CVector3f aim = mgr.GetPlayer()->GetAimPosition(
          mgr, 0.5f * GetModelData()->GetAnimationData()->GetPlaybackRate());
      const CVector3f face = aim - GetTranslation();
      if (face.CanBeNormalized()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), face.AsNormalized(), 1.f));
      }
    }
    AttractProjectiles(mgr);
    UpdateBlockPose(dt, mgr);
    break;
  case kStateMsg_Deactivate:
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mBoneTracking.SetActive(false);
    if (IsUsingBaseCollisionActors()) {
      if (CCollisionActor* actor =
              TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionHeadId))) {
        actor->SetDamageVulnerability(mVulnerability);
      }
    }
    mCollisionActorMgr->SetActive(mgr, false);
    break;
  }
}

void CElitePirate::CallForBackup(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_Zero;
    mCalledForBackup = true;
    SetShotAt(false, mgr);
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Generate) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Five, CVector3f::Zero()));
      }
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Generate) {
        mState = kState_Over;
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
    mCollisionActorMgrHead =
        rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, true);
    mCollisionActorMgrHead->SetActive(mgr, GetActive());
  }
  mCollisionRJointIds.clear();
  mCollisionLJointIds.clear();
  const CSegId segId = GetAnimationData()->GetLocatorSegId(rstl::string_l(skpLeftClawLCTR));
  const CJointCollisionDescription shield = CJointCollisionDescription::OBBCollision(
      segId, skLocalShieldBounds, CVector3f::Zero(), rstl::string_l("Shield"), 10.f);
  joints.clear();
  joints.push_back(shield);
  mCollisionActorMgr =
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
    for (uint i = 0; i < mCollisionActorMgrHead->GetNumCollisionActors(); ++i) {
      const CJointCollisionDescription& desc =
          mCollisionActorMgrHead->GetCollisionDescFromIndex(i);
      const TUniqueId uid = desc.GetCollisionActorId();
      if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
        if (desc.GetName() == rstl::string_l(skpHeadLCTR)) {
          mCollisionHeadId = uid;
        } else if (IsArmClawCollider(desc.GetName(), skpRightClawLCTR, skRightArmJointList, 3)) {
          mCollisionRJointIds.push_back(uid);
        } else if (IsArmClawCollider(desc.GetName(), skpLeftClawLCTR, skLeftArmJointList, 3)) {
          mCollisionLJointIds.push_back(uid);
        }
        if (uid != mCollisionHeadId) {
          actor->SetDamageVulnerability(vulnerability);
        }
      }
    }
    mCollisionActorMgrHead->AddMaterial(
        mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough, kMT_Immovable));
  }
  mEnergyAttractorId =
      mCollisionActorMgr->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (CCollisionActor* const actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mEnergyAttractorId))) {
    actor->SetWeaponCollisionResponseType(kWCR_None);
  }
  mCollisionActorMgr->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
}

void CElitePirate::AddCollisionList(const SJointInfo* joints, int count,
                                    rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId from = animData.GetLocatorSegId(rstl::string_l(joints[i].mFrom));
    const CSegId to = animData.GetLocatorSegId(rstl::string_l(joints[i].mTo));
    if (from.val() != 0xff && to.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereSubdivideCollision(
          from, to, joints[i].mRadius, joints[i].mSeparation,
          CJointCollisionDescription::kOT_One, rstl::string_l(joints[i].mFrom), 10.f);
      list.push_back(desc);
    }
  }
}

void CElitePirate::AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                                          rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId id = animData.GetLocatorSegId(rstl::string_l(joints[i].mName));
    if (id.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          id, joints[i].mRadius, rstl::string_l(joints[i].mName), 10.f);
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
    if (name == rstl::string_l(joints[i].mFrom)) {
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
  mCollisionAabb.Box() = box;
  mPathFindSearch.SetCharacterRadius(scale);
  mPathFindSearch.SetCharacterHeight(3.f * scale);
}

bool CElitePirate::IsShieldActive() const {
  return mBodyController->GetLocomotionType() == pas::kLT_Crouch &&
         (mBodyController->GetCurrentStateId() == pas::kAS_Locomotion ||
          mBodyController->GetCurrentStateId() == pas::kAS_Turn);
}

void CElitePirate::SetupHealthInfo(CStateManager& mgr) {
  const CHealthInfo* health = HealthInfo(mgr);
  mHp = health->GetHP();
  if (IsUsingBaseCollisionActors()) {
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionHeadId))) {
      *actor->HealthInfo(mgr) = *health;
      actor->SetDamageVulnerability(mVulnerability);
    }
  }
  SetupHealthInfoForLauncher(mgr, mLauncherId);
}

void CElitePirate::SetupHealthInfoForLauncher(CStateManager& mgr, const TUniqueId uid) const {
  const CHealthInfo* health = GetHealthInfo(mgr);
  if (uid != kInvalidUniqueId) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      *actor->HealthInfo(mgr) =
          CHealthInfo(mData.GetLauncherHP(), health->GetKnockBackResistance());
      actor->SetDamageVulnerability(mVulnerability);
    }
  }
}

void CElitePirate::UpdateHealthInfo(CStateManager& mgr) {
  const float hp = HealthInfo(mgr)->GetHP();
  if (IsUsingBaseCollisionActors()) {
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionHeadId))) {
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
  if (mAttackTimer > 0.f) {
    mAttackTimer -= dt;
  }
  if (mEnergyAbsorbCooldown > 0.f) {
    mEnergyAbsorbCooldown -= dt;
  }
}

// TODO: Resource loads and damage-copy stores still differ in scheduling.
void CElitePirate::CreateGrenadeLauncher(CStateManager& mgr, TUniqueId uid) {
  const CAnimationParameters& params = mData.GetLauncherAnimParams();
  if (params.GetACSFile() != kInvalidAssetId) {
    CModelData model(CAnimRes(params.GetACSFile(), params.GetCharacter(),
                              GetModelData()->ScaleCopy(), params.GetInitialAnimation(), true));
    CBouncyGrenadeData grenadeData(
        SGrenadeVelocityInfo(mData.GetGrenadeVelocityInfo().GetMass(),
                             mData.GetGrenadeVelocityInfo().GetSpeed()),
        mData.GetGrenadeDamageInfo(), mData.GetGrenadeElementGenDescId1(),
        mData.GetGrenadeElementGenDescId2(), mData.GetGrenadeElementGenDescId3(),
        mData.GetGrenadeElementGenDescId4(), mData.GetGrenadeNumBounces(),
        mData.GetGrenadeBounceSfxId(), mData.GetGrenadeExplodeSfxId());
    const CEPGrenadeLaunchParms& launchInfo = mData.GetGrenadeTrajectoryInfo();
    CEPGrenadeLaunchParms launchParms(launchInfo.GetVelocityMin(), launchInfo.GetVelocityMax(),
                                      launchInfo.GetAngleMin(), launchInfo.GetAngleMax());
    const CEPGrenadeLauncherData data(grenadeData, mData.GetGrenadeModelId(),
                                      mData.GetLauncherParticleGenDescId(),
                                      mData.GetLauncherSfxId(), launchParms);
    CGrenadeLauncher* const launcher = rs_new CGrenadeLauncher(
        uid, rstl::string_l("Grenade Launcher"),
        CEntityInfo(GetCurrentAreaId(), NullConnectionList), GetTransform(), model,
        model.GetBounds(GetTransform().GetRotation()), CHealthInfo(mData.GetLauncherHP(), 10.f),
        mVulnerability, mData.GetLauncherActParams(), GetUniqueId(), data, 0.f);
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
  ActivateGrenadeLauncherById(mgr, active, mLauncherId);
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
  if (mPathFindSearch.OnPath(pos) == CPathFindSearch::kR_Success) {
    mPositionHistory.Clear();
  }
  mPositionHistory.AddValue(pos);
}

void CElitePirate::UpdatePathDestination(CStateManager& mgr) {
  mTargetDestPos = GetTranslation();
  const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  const CVector3f dist = GetTranslation() - playerPos;
  const float minRangeSquared = mMinAttackRange * mMinAttackRange;
  if (dist.CanBeNormalized() && dist.MagSquared() > minRangeSquared) {
    const CVector3f& dest = playerPos + dist.AsNormalized() * mMinAttackRange;
    mDestPos = dest;
    mTargetDestPos = mDestPos;
  }
}

void CElitePirate::UpdateAttackTimeLeft(CStateManager& mgr) {
  if (mgr.Random()->Float() > mData.GetAttackChance()) {
    mAttackTimer = mAverageAttackTime + mAttackTimeVariation * mgr.Random()->Float();
  }
}

void CElitePirate::ProcessStompGround(CStateManager& mgr) {
  CPlayer& player = *mgr.Player();
  const CVector3f delta = GetTranslation() - player.GetTranslation();
  const float distance = delta.Magnitude();
  const float scale = mShockWaveAnim ? 1.f : 0.25f;
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
      const float intensity = mShockWaveAnim ? 20.f : 10.f;
      const CVector3f impulse = player.GetMass() * (intensity * CVector3f::Up());
      player.ApplyImpulseWR(impulse, CAxisAngle::Identity());
      player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
  }
}

void CElitePirate::StartAbsorbEnergyEffects(CStateManager& mgr, const CTransform4f& xf) {
  if (mEnergyAbsorbCooldown <= 0.f) {
    CExplosion* effect = rs_new CExplosion(*mEnergyAbsorbDesc, mgr.AllocateUniqueId(), true,
                                           CEntityInfo(GetCurrentAreaId(), NullConnectionList),
                                           rstl::string_l("Absorb energy Fx"), xf, 0,
                                           GetModelData()->ScaleCopy(), CColor::White());
    if (effect) {
      mgr.AddObject(effect);
      CSfxManager::AddEmitter(mData.GetEnergyAbsorbSfxId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      mEnergyAbsorbCooldown = 0.25f;
    }
  }
}

// TODO: Ten floating-point register operands differ in the Bezier and velocity calculations.
void CElitePirate::AttractProjectiles(CStateManager& mgr) {
  if (IsAlive()) {
    if (const CCollisionActor* actor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mEnergyAttractorId))) {
      const CVector3f actorPos = actor->GetTranslation();
      const float radius = mData.GetProjectileAttractionRadius();
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
  if (mShotAt && !mBodyController->IsFrozen()) {
    mAbsorbUpdateTimer += dt;
    if (mAbsorbUpdateTimer >= 3.f) {
      if (mBodyController->GetCurrentStateId() == pas::kAS_Turn ||
          !mBodyController->GetBodyStateInfo().GetCurrentState()->IsMoving()) {
        bool playerAbove = false;
        if (IsUsingBaseCollisionActors()) {
          if (const CCollisionActor* head =
                  TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mCollisionHeadId))) {
            const CVector3f& headPos = head->GetTranslation();
            const CVector3f& pos = GetTranslation();
            const CVector3f& playerPos = mgr.GetPlayer()->GetTranslation();
            float height = headPos[kDZ];
            height -= pos[kDZ];
            playerAbove = playerPos[kDZ] >= headPos[kDZ] - height * 0.5f;
          }
        }
        mBodyController->CommandMgr().DeliverCmd(CBCAdditiveReactionCmd(
            playerAbove || TooClose(mgr, 0.f) ? pas::kART_Seven : pas::kART_Five, 1.f, false));
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCAdditiveReactionCmd(pas::kART_Six, 1.f, false));
      }
      mAbsorbUpdateTimer = 0.f;
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
  if (IsElitePirate() && mHp > 0.f && shotAt) {
    if (HealthInfo(mgr)->GetHP() / mHp <= x7b0_) {
      x7b0_ -= 0.2f;
      mShotAt = true;
    }
  } else {
    mShotAt = shotAt;
  }
  if (mShotAt) {
    mShotAtTimer =
        mgr.Random()->Float() * mData.GetShotAtTimeVariance() + mData.GetShotAtTime();
  } else {
    mShotAtTimer = 0.f;
  }
}

void CElitePirate::ReDirectDamage(CStateManager& mgr, TUniqueId uid) {
  if (IsUsingBaseCollisionActors()) {
    if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(uid))) {
      CDamageInfo damage = weapon->GetCurrentDamageInfo();
      damage.SetRadius(0.f);
      mgr.ApplyDamage(
          uid, mCollisionHeadId, weapon->GetOwnerId(), damage,
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
