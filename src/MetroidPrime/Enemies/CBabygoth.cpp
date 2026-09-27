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
: mFireballAttackTime(in.Get< float >())
, mFireballAttackTimeVariance(in.Get< float >())
, mFireballWeapon(in.Get< CAssetId >())
, mFireballDamage(in)
, mAttackContactDamage(in)
, mFireBreathWeapon(in.Get< CAssetId >())
, mFireBreathRes(in.Get< CAssetId >())
, mFireBreathDamage(in)
, mMouthVulnerabilities(in)
, mShellVulnerabilities(in)
, mNoShellModel(in.Get< CAssetId >())
, mNoShellSkin(in.Get< CAssetId >())
, mShellHitPoints(in.Get< float >())
, mShellCrackSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, mIntermediateCrackParticle(in.Get< CAssetId >())
, mCrackOneParticle(in.Get< CAssetId >())
, mCrackTwoParticle(in.Get< CAssetId >())
, mDestroyShellParticle(in.Get< CAssetId >())
, mCrackOneSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, mCrackTwoSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, mDestroyShellSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, mTimeUntilAttack(in.Get< float >())
, mAttackCooldownTime(in.Get< float >())
, mInterestTime(in.Get< float >())
, mFlamePlayerSteamTxtr(in.Get< CAssetId >())
, mFlamePlayerHitSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, mFlamePlayerIceTxtr(in.Get< CAssetId >()) {}

CBabygoth::CBabygoth(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                     const CActorParameters& actParms, const CBabygothData& babyData)
: CPatterned(kC_Babygoth, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actParms, kCS_Medium)
, mStateProg(-1)
, mShellState(kSCS_Default)
, mBabyData(babyData)
, mTeamMgr(kInvalidUniqueId)
, mPathSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mApproachPathSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mPathFindMode(kPFM_Normal)
, mBackupDestPos(CVector3f::Zero())
, mInitialFaceDir(CVector3f::Zero())
, mInitialSpeed(mSpeed)
, mStepBackwardDist(0.f)
, mAttackTimeLeft(0.f)
, mAttackTimer(0.f)
, mAttackCooldownTimeLeft(0.f)
, mFireballAttackTimeLeft(0.f)
, mInterestTimer(0.f)
, mBodyHP(0.f)
, mBoneTracking(*GetAnimationData(), rstl::string_l("Head_1"), CMath::Deg2Rad(80.f),
                    CRelAngle::FromDegrees(180.f).AsRadians(), kBTF_None)
, mColActMgr(nullptr)
, mAabox(GetBoundingBox(), GetMaterialList())
, mIceProjectile(babyData.GetFireballResID(), babyData.GetFireballDamage())
, mFlameThrower(kInvalidUniqueId)
, mFlameThrowerDesc(
      babyData.GetFireBreathWeapon() != kInvalidAssetId
          ? gpSimplePool->GetObj(SObjectTag('WPSC', babyData.GetFireBreathWeapon()))
          : gpSimplePool->GetObj("FlameThrower"))
, mDVuln(pInfo.GetDamageVulnerability())
, mMouthLocator(0xff)
, mMouthCollisionActor(kInvalidUniqueId)
, mShellHitPoints(babyData.GetShellHitPoints())
, mDrawMaterialIdx(0)
, mNoShellModel(TLockedToken< CSkinnedModel >(rs_new CSkinnedModel(
      gpSimplePool->GetObj(SObjectTag('CMDL', babyData.GetNoShellModel())),
      gpSimplePool->GetObj(SObjectTag('CSKR', babyData.GetNoShellSkin())),
      GetAnimationData()->GetModelData()->GetLayoutInfo(), CSkinnedModel::kDO_Owned)))
, mCrackOneParticle(gpSimplePool->GetObj(SObjectTag('PART', babyData.GetCrackOneParticle())))
, mCrackTwoParticle(gpSimplePool->GetObj(SObjectTag('PART', babyData.GetCrackTwoParticle())))
, mDestroyShellParticle(
      gpSimplePool->GetObj(SObjectTag('PART', babyData.GetDestroyShellParticle())))
, mIntermediateCrackParticle(
      babyData.GetIntermediateCrackParticle() != kInvalidAssetId
          ? rstl::optional_object< TLockedToken< CGenDescription > >(
                TLockedToken< CGenDescription >(gpSimplePool->GetObj(
                    SObjectTag('PART', babyData.GetIntermediateCrackParticle()))))
          : rstl::optional_object< TLockedToken< CGenDescription > >())
, mIsAlert(false)
, xa48_25_(false)
, mInProjectileAttack(false)
, xa48_27_(false)
, mPendingAttackContactDamage(false)
, mHasBeenEnraged(false)
, mHeardPlayerFire(false)
, mApproachNeedsPathSearch(true)
, mGettingUp(false)
, mShouldStepBackwards(false)
, mReadyForTeam(false)
, mLocomotionValid(false)
, mOnApproachPath(false)
, mObjectSpaceCollision(false) {
  mIceProjectile.Token().Lock();
  UpdateTouchBounds();
  KnockBackCtrl().SetEnableFreeze(false);
  KnockBackCtrl().SetX82_24(false);
  KnockBackCtrl().SetLocomotionDuringElectrocution(true);
  KnockBackCtrl().SetEnableExplodeDeath(false);
  const CPASAnimParmData stepParms(pas::kAS_Step, CPASAnimParm::FromEnum(1),
                                   CPASAnimParm::FromEnum(0));
  mStepBackwardDist = GetAnimationDistance(stepParms) * GetModelData()->ScaleCopy().GetY();
  mNoShellModel->SetLayoutInfo(GetAnimationData()->GetModelData()->GetLayoutInfo());
  MakeThermalColdAndHot();
}

CBabygoth::~CBabygoth() {}
ENTITY_ACCEPT_IMPL(CBabygoth)

void CBabygoth::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    PreventPlayerInterpenetration(mgr, dt);
    if (mReadyForTeam && !CTeamAiMgr::GetTeamAiRole(mgr, mTeamMgr, GetUniqueId())) {
      AddToTeam(mgr);
    }
    CPatterned::Think(dt, mgr);
    if (mBodyController->IsElectrocuting()) {
      mBoneTracking.SetActive(false);
    }
    UpdateAILogicTimers(dt);
    AnimationData()->PreRender();
    mBoneTracking.Update(dt);
    mBoneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(),
                                GetModelData()->ScaleCopy(), *mBodyController);
    mColActMgr->Update(dt, mgr,
                           mObjectSpaceCollision ? CCollisionActorManager::kUO_ObjectSpace
                                                        : CCollisionActorManager::kUO_WorldSpace);
    mObjectSpaceCollision = true;
    UpdateHealthInfo(mgr);
    UpdateParticleEffects(dt, mgr);
    CheckShouldGetUp(mgr);
    CheckShouldWakeUp(mgr, dt);
    if (!mAlive && mBodyController->GetBodyStateInfo().GetCurrentState()->IsDying()) {
      SetShootThrough(mgr);
    }
  }
}

void CBabygoth::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool callBase = true;
  switch (msg) {
  case kSM_Registered: {
    if (!HasPatrolPath(mgr, 0.f)) {
      mBodyController->SetLocomotionType(pas::kLT_Crouch);
    }
    mBodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    CreateFlameThrower(mgr);
    const float maxSpeed = mBodyController->GetBodyStateInfo().GetMaxSpeed();
    const float speed =
        maxSpeed > 0.f
            ? mBodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk) / maxSpeed
            : 1.f;
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_Clamped);
    mBodyController->CommandMgr().SetSteeringSpeedRange(0.f, speed);
    mMouthLocator = GetAnimationData()->GetLocatorSegId(rstl::string_l(skpMouthDamageJoint));
    break;
  }
  case kSM_Activate:
    mColActMgr->SetActive(mgr, true);
    break;
  case kSM_Deactivate:
    mColActMgr->SetActive(mgr, false);
    mObjectSpaceCollision = false;
    RemoveFromTeam(mgr);
    break;
  case kSM_Deleted:
    mColActMgr->Destroy(mgr);
    if (mFlameThrower != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mFlameThrower);
      mFlameThrower = kInvalidUniqueId;
    }
    RemoveFromTeam(mgr);
    break;
  case kSM_Falling: {
    if (!mBodyController->IsFrozen()) {
      const float weight = GetGravityConstant() * GetMass();
      mMomentum = CVector3f(0.f, 0.f, -weight);
      mOnGround = false;
      RemoveMaterial(kMT_GroundCollider, mgr);
    }
    callBase = false;
    break;
  }
  case kSM_OnFloor:
    SetMomentumWR(CVector3f::Zero());
    mOnGround = true;
    AddMaterial(kMT_GroundCollider, mgr);
    callBase = false;
    break;
  case kSM_Alert:
    mIsAlert = true;
    break;
  case kSM_InitializedInArea: {
    const TAreaId areaId = GetCurrentAreaId();
    const CGameArea* area = mgr.GetWorld()->GetArea(areaId);
    mPathSearch.SetArea(area->GetPostConstructed()->mPathArea);
    mApproachPathSearch.SetArea(area->GetPostConstructed()->mPathArea);
    if (mTeamMgr == kInvalidUniqueId) {
      mTeamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    }
    break;
  }
  case kSM_Touched:
    ApplyContactDamage(uid, mgr);
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (const CWeapon* const weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touched))) {
        if (weapon->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
          mIsAlert = true;
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
            if (mShellState != kSCS_Destroyed && mIntermediateCrackParticle) {
              CTransform4f xf = weapon->GetTransform();
              xf.RotateLocalZ(CRelAngle::FromRadians(M_PIF));
              StartCrackShellEffect(mgr, *mIntermediateCrackParticle, xf,
                                    mBabyData.GetShellCrackSfx(), false);
            }
          }
          KnockBack(weapon->GetTransform().GetForward(), mgr, weapon->GetCurrentDamageInfo(),
                    weapon->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
        }
      }
      mIsAlert = true;
      mInterestTimer = 0.f;
      mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    } else {
      ReDirectDamage(mgr, uid);
    }
    mHitByPlayerProjectile = true;
    break;
  case kSM_InvulnDamage:
    mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    mHitByPlayerProjectile = true;
    mIsAlert = true;
    mInterestTimer = 0.f;
    if (!TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      ReDirectDamage(mgr, uid);
    }
    break;
  case kSM_SuspendedMove:
    if (mColActMgr.get()) {
      mColActMgr->SetMovable(mgr, false);
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
  flags.mMatSetIdx = mDrawMaterialIdx;
  SetModelFlags(flags);
}

CProjectileInfo* CBabygoth::ProjectileInfo() { return &mIceProjectile; }

void CBabygoth::KnockBack(const CVector3f& direction, CStateManager& mgr, const CDamageInfo& damage,
                          float magnitude, bool direct, const bool deferred) {
  KnockBackCtrl().EnableAnimReaction(kAR_Hurled, mShellState == kSCS_Destroyed);
  CPatterned::KnockBack(direction, mgr, damage, magnitude, direct, deferred);
  if (mAlive) {
    switch (KnockBackCtrl().GetActiveParms().mAnimState) {
    case kAR_Hurled:
      mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("GetUp"));
      break;
    }
  }
}

void CBabygoth::Shock(CStateManager& mgr, float duration, float damage) {
  if (!mShellIds.empty()) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(mShellIds[0]))) {
      const CWeaponMode weapon(kWT_Wave);
      switch (
          actor->GetDamageVulnerability()->GetVulnerability(weapon, CDamageVulnerability::kRD_No)) {
      case kVN_Weak:
        mBodyController->SetElectrocuting(1.5f * duration);
        mPendingShockDamage = 1.5f * damage;
        break;
      case kVN_Normal:
        mBodyController->SetElectrocuting(duration);
        mPendingShockDamage = damage;
        break;
      }
    }
  }
}

void CBabygoth::TakeDamage(const CVector3f&, float) {
  if (mAlive) {
    mDamageCooldownTimer = skDamageHitTime;
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
  if (mAlive && noiseType == kLNT_PlayerFire) {
    const CVector3f delta = origin - GetTranslation();
    if (delta.MagSquared() < 1600.f) {
      mHeardPlayerFire = heard = true;
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
    result = aimPos + delta.AsNormalized() * mMinAttackRange;
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
    if (mInProjectileAttack) {
      if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(mFlameThrower))) {
        flame->Fire(GetTransform(), mgr, false);
      }
    }
    break;
  case kUE_DamageOff:
    if (mInProjectileAttack) {
      if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(mFlameThrower))) {
        flame->Reset(mgr, false);
      }
    }
    break;
  case kUE_BeginAction:
    if (mInProjectileAttack) {
      mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
      mBoneTracking.SetActive(true);
    }
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

CVector3f CBabygoth::GetAimPosition(const CStateManager& mgr, float) const {
  if (mBodyController->GetLocomotionType() != pas::kLT_Crouch && !mShellIds.empty()) {
    CVector3f shellPos = CVector3f::Zero();
    for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
      if (const CCollisionActor* actor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(*it))) {
        shellPos += actor->GetTranslation() + actor->GetSphereRadius() * CVector3f::Up();
      }
    }
    if (shellPos.IsNonZero()) {
      shellPos /= static_cast< float >(mShellIds.size());
    }
    CVector3f mouthPos = shellPos;
    if (const CCollisionActor* actor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mMouthCollisionActor))) {
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
  const CAABox bounds = mAabox.CalculateAABox(GetTransform());
  if (bounds.DoBoundsOverlap(playerBounds)) {
    mShouldStepBackwards = true;
    return true;
  }
  return false;
}

bool CBabygoth::AggressionCheck(CStateManager&, float) {
  if (mAlive && !mHasBeenEnraged && mShellState == kSCS_Destroyed) {
    return true;
  }
  return false;
}

bool CBabygoth::InMaxRange(CStateManager& mgr, float) {
  return (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
         (1.5f * mMaxAttackRange) * (1.5f * mMaxAttackRange);
}

bool CBabygoth::ShotAt(CStateManager&, float) { return mHitByPlayerProjectile; }

bool CBabygoth::OffLine(CStateManager& mgr, float) {
  SetPathFindMode(kPFM_Normal);
  return PathShagged(mgr, 0.f);
}

bool CBabygoth::LostInterest(CStateManager&, float) {
  if (mInterestTimer >= mBabyData.GetInterestTime()) {
    return mPathSearch.OnPath(GetTranslation()) == CPathFindSearch::kR_Success;
  }
  return false;
}

bool CBabygoth::LineOfSight(CStateManager& mgr, float) {
  const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  const CTransform4f mouthXf = GetLctrTransform(mMouthLocator);
  const CVector3f mouthPos = mouthXf.GetTranslation();
  return !IsPatternObstructed(mgr, mouthPos, aimPos);
}

bool CBabygoth::Leash(CStateManager& mgr, float) {
  const CVector3f delta = mLatestLeashPosition - GetTranslation();
  if (delta.MagSquared() > mLeashRadius * mLeashRadius) {
    const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    return playerDelta.MagSquared() > mPlayerLeashRadius * mPlayerLeashRadius &&
           mCurPlayerLeashTime > mPlayerLeashTime;
  }
  return false;
}

bool CBabygoth::ShouldAttack(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  if (GetCurrentAreaId() == player.GetCurrentAreaId() && !player.GetFrozenState() &&
      mAttackTimeLeft <= 0.f && mAttackCooldownTimeLeft <= 0.f) {
    const CVector3f aimPos = player.GetAimPosition(mgr, 0.f);
    const CVector3f delta = aimPos - GetTranslation();
    const float distance = delta.Magnitude();
    if (mBodyController->GetBodyStateInfo().GetMaxSpeed() * mBabyData.GetTimeUntilAttack() >
            distance &&
        !ShouldTurn(mgr, CRelAngle::FromDegrees(25.f).AsRadians())) {
      const CTransform4f mouthXf = GetLctrTransform(mMouthLocator);
      const CVector3f mouthPos = mouthXf.GetTranslation();
      if (!IsPatternObstructed(mgr, mouthPos, aimPos)) {
        if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
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
      mFireballAttackTimeLeft <= 0.f) {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f mouthXf = GetLctrTransform(mMouthLocator);
    const CVector3f mouthPos = mouthXf.GetTranslation();
    const CVector3f delta = aimPos - mouthPos;
    if (delta.MagSquared() >= mMaxAttackRange * mMaxAttackRange &&
        !ShouldTurn(mgr, CRelAngle::FromDegrees(25.f).AsRadians()) &&
        !IsPatternObstructed(mgr, mouthPos, aimPos)) {
      return true;
    }
  }
  return false;
}

bool CBabygoth::ShouldFire(CStateManager& mgr, float) {
  if (GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId() && mAttackTimeLeft <= 0.f) {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f mouthXf = GetLctrTransform(mMouthLocator);
    const CVector3f mouthPos = mouthXf.GetTranslation();
    const CVector3f delta = aimPos - mouthPos;
    if (delta.MagSquared() <= mMaxAttackRange * mMaxAttackRange &&
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
  const CVector3f delta = mBackupDestPos - GetTranslation();
  return delta.MagSquared() < 9.f;
}

bool CBabygoth::InDetectionRange(CStateManager& mgr, float arg) {
  bool result = mIsAlert;
  if (!result) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    const float scale = arg > 0.f ? arg : 1.f;
    const float range = scale * mDetectionRange;
    if (delta.MagSquared() < range * range) {
      if (mDetectionHeightRange > 0.f) {
        return delta.GetZ() * delta.GetZ() < mDetectionHeightRange * mDetectionHeightRange;
      }
      return true;
    }
  }
  return result;
}

bool CBabygoth::SpotPlayer(CStateManager& mgr, float arg) {
  return mIsAlert ? mIsAlert : CPatterned::SpotPlayer(mgr, arg);
}

bool CBabygoth::AnimOver(CStateManager&, float) { return mStateProg == 4; }

void CBabygoth::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    mInProjectileAttack = true;
    mInterestTimer = 0.f;
    mLocomotionValid = false;
    CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr));
    if (teamMgr && teamMgr->HasTeamAiRole(GetUniqueId())) {
      const bool added = teamMgr->AddProjectileAttacker(GetUniqueId());
      int state = 4;
      if (added) {
        state = 0;
      }
      mStateProg = state;
    } else {
      mStateProg = 0;
    }
    break;
  }
  case kStateMsg_Update: {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() ==
          pas::kAS_ProjectileAttack) {
        mStateProg = 3;
        mSpeed = 2.f * mInitialSpeed;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Two, aimPos, false));
      }
      break;
    case 3:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() !=
          pas::kAS_ProjectileAttack) {
        mStateProg = 4;
      } else if (!mgr.ObjectById(mFlameThrower)) {
        mBodyController->CommandMgr().DeliverTargetVector(aimPos - GetTranslation());
        mBoneTracking.UnsetTarget();
        mBoneTracking.SetTargetPosition(aimPos);
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(mFlameThrower))) {
      flame->Reset(mgr, false);
    }
    mInProjectileAttack = false;
    if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    mBoneTracking.SetActive(false);
    mSpeed = mInitialSpeed;
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, mTeamMgr, GetUniqueId(), false);
    break;
  }
}

void CBabygoth::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 0;
    xa48_25_ = mPendingAttackContactDamage = true;
    mInterestTimer = 0.f;
    mShouldStepBackwards = false;
    mDestPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, msg, dt);
    mAttackTimer = 0.f;
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mBodyController->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    ExtendTouchBounds(mgr, skAttackTouchBounds);
    break;
  case kStateMsg_Update:
    ProcessCharge(mgr, dt);
    break;
  case kStateMsg_Deactivate: {
    UpdateAttackTimeLeft(mgr);
    xa48_25_ = mPendingAttackContactDamage = false;
    mAttackCooldownTimeLeft = mBabyData.GetAttackCooldownTime();
    mAttackTimer = 0.f;
    const float maxSpeed = mBodyController->GetBodyStateInfo().GetMaxSpeed();
    const float speed =
        maxSpeed > 0.f
            ? mBodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk) / maxSpeed
            : 1.f;
    mBodyController->CommandMgr().SetSteeringBlendMode(kSBM_Clamped);
    mBodyController->CommandMgr().SetSteeringSpeedRange(0.f, speed);
    ExtendTouchBounds(mgr, CVector3f::Zero());
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, mTeamMgr, GetUniqueId(), true);
    break;
  }
  }
}

void CBabygoth::SpecialAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    xa48_27_ = true;
    mInterestTimer = 0.f;
    mLocomotionValid = false;
    CTeamAiMgr* const teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr));
    if (teamMgr && teamMgr->HasTeamAiRole(GetUniqueId())) {
      const bool added = teamMgr->AddProjectileAttacker(GetUniqueId());
      int state = 4;
      if (added) {
        state = 0;
      }
      mStateProg = state;
    } else {
      mStateProg = 0;
    }
    break;
  }
  case kStateMsg_Update: {
    const CVector3f target = mgr.GetPlayer()->GetTranslation();
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() ==
          pas::kAS_ProjectileAttack) {
        mStateProg = 3;
        mSpeed = 2.f * mInitialSpeed;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_One, target, false));
      }
      break;
    case 3:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() !=
          pas::kAS_ProjectileAttack) {
        mStateProg = 4;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateAttackTimeLeft(mgr);
    xa48_27_ = false;
    mSpeed = mInitialSpeed;
    if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, mTeamMgr, GetUniqueId(), false);
    break;
  }
}

void CBabygoth::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mOnApproachPath = false;
    mReadyForTeam = true;
    mIsAlert = false;
    mInterestTimer = 0.f;
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking.SetActive(true);
    UpdateAttackPosition(mgr, mDestPos);
    mBackupDestPos = mDestPos;
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, msg, dt);
    break;
  case kStateMsg_Update:
    SetPathFindMode(kPFM_Normal);
    if (GetSearchPath() && !PathShagged(mgr, 0.f) &&
        mPathSearch.GetCurrentWaypoint() < mPathSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, msg, dt);
      mInterestTimer = 0.f;
      const CVector3f move = mBodyController->CommandMgr().GetMoveVector();
      if (move.CanBeNormalized()) {
        const CVector3f arrival =
            mSteeringBehaviors.Arrival(*this, mgr.GetPlayer()->GetTranslation(), 15.f);
        const float magnitude = arrival.Magnitude();
        mBodyController->CommandMgr().ClearLocomotionCmds();
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(magnitude * move.AsNormalized(), CVector3f::Zero(), 10.f));
      }
      ApplySeparationBehavior(mgr);
      const CVector3f forward = GetTransform().GetForward();
      const CVector3f newMove = mBodyController->CommandMgr().GetMoveVector();
      if (CVector3f::Dot(forward, newMove) < 0.f && move.CanBeNormalized()) {
        mBodyController->CommandMgr().ClearLocomotionCmds();
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), newMove.AsNormalized(), 1.f));
      }
    } else {
      const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      if (delta.CanBeNormalized()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mBoneTracking.SetActive(false);
    break;
  }
}

void CBabygoth::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking.SetActive(true);
    UpdateAttackPosition(mgr, mDestPos);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
    if (!mLocomotionValid) {
      mBodyController->CommandMgr().ClearLocomotionCmds();
    }
    mApproachNeedsPathSearch = true;
    mOnApproachPath = true;
    break;
  case kStateMsg_Update:
    SetPathFindMode(kPFM_Approach);
    if (mApproachNeedsPathSearch) {
      mDestPos = mBackupDestPos;
      if (mApproachPathSearch.FindClosestReachablePoint(GetTranslation(), mDestPos) ==
          CPathFindSearch::kR_Success) {
        const CVector3f delta = mDestPos - GetTranslation();
        if (delta.MagSquared() < 10.f) {
          mDestPos = GetTranslation();
        }
        mBackupDestPos = mDestPos;
        CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
        if (!mLocomotionValid) {
          mBodyController->CommandMgr().ClearLocomotionCmds();
        }
      }
      mApproachNeedsPathSearch = false;
    }
    mLocomotionValid &= !IsOtherCharacterNearPathDest(mgr);
    if (mLocomotionValid && GetSearchPath() && !PathShagged(mgr, 0.f) &&
        mApproachPathSearch.GetCurrentWaypoint() <
            mApproachPathSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, msg, dt);
      ApplySeparationBehavior(mgr);
    } else {
      const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      if (ShouldTurn(mgr, CRelAngle::FromDegrees(25.f).AsRadians()) && delta.CanBeNormalized()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    mLocomotionValid = true;
    break;
  case kStateMsg_Deactivate:
    mBoneTracking.SetActive(false);
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
    mReadyForTeam = false;
    RemoveFromTeam(mgr);
    mHitByPlayerProjectile = false;
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    if (HasPatrolPath(mgr, 0.f)) {
      CPatterned::Patrol(mgr, msg, dt);
      UpdateDest(mgr);
    } else {
      SetDestPos(mLatestLeashPosition);
    }
    mBackupDestPos = mDestPos;
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
      const CVector3f arrival = mSteeringBehaviors.Arrival(*this, mBackupDestPos, 9.f);
      mBodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
    }
    break;
  }
}

void CBabygoth::Generate(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 0;
    mInitialFaceDir = GetTransform().GetForward();
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        mBodyController->SetLocomotionType(pas::kLT_Relaxed);
        mStateProg = 3;
        SendScriptMsgs(kSS_Attack, mgr, kSM_None);
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Zero, CVector3f::Zero()));
      }
      break;
    case 3:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        mStateProg = 4;
      }
      break;
    }
    break;
  }
}

void CBabygoth::Deactivate(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 1;
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 1: {
      const CVector3f& dest = mLatestLeashPosition;
      const CVector3f delta = dest - GetTranslation();
      if (delta.MagSquared() <= 1.f) {
        mStateProg = 2;
      } else {
        const CVector3f arrival = mSteeringBehaviors.Arrival(*this, dest, 15.f);
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
      }
      break;
    }
    case 2: {
      const CVector3f forward = GetTransform().GetForward();
      if (CVector3f::GetAngleDiff(forward, mInitialFaceDir) >
          CRelAngle::FromDegrees(5.f).AsRadians()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), mInitialFaceDir, 1.f));
      } else {
        mStateProg = 0;
      }
      break;
    }
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        mStateProg = 3;
        mBodyController->SetLocomotionType(pas::kLT_Crouch);
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_One, CVector3f::Zero()));
      }
      break;
    case 3:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        mStateProg = 4;
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
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    mHeardPlayerFire = mIsAlert = false;
    mHitByPlayerProjectile = false;
    mInterestTimer = 0.f;
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
    mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_One));
    mStateProg = 3;
    break;
  case kStateMsg_Update:
    if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Taunt) {
      mStateProg = 4;
    }
    break;
  }
}

void CBabygoth::FollowPattern(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    int state = 4;
    if (mShouldStepBackwards) {
      state = 0;
    }
    mStateProg = state;
    mShouldStepBackwards = false;
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking.SetActive(true);
    break;
  }
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      mBodyController->CommandMgr().DeliverCmd(
          CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      mStateProg = 3;
      break;
    case 3:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Step) {
        mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      } else {
        mStateProg = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mBoneTracking.SetActive(false);
    SetPathFindMode(kPFM_Normal);
    break;
  }
}

void CBabygoth::Enraged(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mHasBeenEnraged = true;
    mStateProg = 0;
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        mStateProg = 3;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_Three, CVector3f::Zero()));
      }
      break;
    case 3:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        mStateProg = 4;
      }
      break;
    }
    break;
  }
}

void CBabygoth::GetUp(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 0;
    mGettingUp = true;
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Getup) {
        mStateProg = 3;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      }
      break;
    case 3:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Getup) {
        mStateProg = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mGettingUp = false;
    break;
  }
}

void CBabygoth::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking.SetActive(true);
    UpdateAttackPosition(mgr, mDestPos);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
    mBodyController->CommandMgr().ClearLocomotionCmds();
    break;
  case kStateMsg_Update:
    if (ShouldTurn(mgr, CRelAngle::FromDegrees(25.f).AsRadians())) {
      const float speed = GetAnimationData()->GetPlaybackRate();
      const CVector3f aimPos =
          mgr.GetPlayer()->GetAimPosition(mgr, speed > 0.f ? 1.f / speed : 0.f);
      CVector3f delta = aimPos - GetTranslation();
      delta[kDZ] = 0.f;
      if (delta.CanBeNormalized()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mBoneTracking.SetActive(false);
    break;
  }
}

void CBabygoth::UpdateTouchBounds() {
  const CAABox bounds(CVector3f(-1.5f, -1.5f, 0.f), CVector3f(1.5f, 1.5f, 2.f));
  SetBoundingBox(bounds);
  mAabox.Box() = bounds;
}

void CBabygoth::UpdateAttackPosition(CStateManager& mgr, CVector3f& attackPos) {
  attackPos = GetTranslation();
  if (mAttackTimeLeft <= 0.f) {
    attackPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    CVector3f delta = GetTranslation() - attackPos;
    delta[kDZ] = 0.f;
    if (delta.CanBeNormalized()) {
      attackPos += delta.AsNormalized() * mMinAttackRange;
    }
  }
}

void CBabygoth::ApplyContactDamage(TUniqueId uid, CStateManager& mgr) {
  if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
    if (HealthInfo(mgr)->GetHP() > 0.f) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (touched == mgr.GetPlayer()->GetUniqueId()) {
        if (mPendingAttackContactDamage) {
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
              mBabyData.GetAttackContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          mPendingAttackContactDamage = false;
          mCurDamageRemTime = mDamageWaitTime;
        } else if (mCurDamageRemTime <= 0.f) {
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
        }
      }
    }
  }
}

void CBabygoth::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(5);
  AddSphereCollisionList(skSphereJointList, 5, joints);
  mColActMgr =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  mColActMgr->SetActive(mgr, GetActive());
  for (uint i = 0; i < mColActMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mColActMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      actor->SetDamageVulnerability(mBabyData.GetMouthVulnerabilities());
      if (desc.GetName() == rstl::string_l(skpMouthDamageJoint)) {
        mMouthCollisionActor = uid;
      } else if (desc.GetName() == rstl::string_l(skpPelvisDamageJoint) ||
                 desc.GetName() == rstl::string_l(skpButtDamageJoint)) {
        mShellIds.push_back(uid);
        actor->SetWeaponCollisionResponseType(kWCR_Unknown66);
      }
    }
  }
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_AIPassthrough, kMT_Player)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  mColActMgr->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
}

void CBabygoth::AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                                       rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetModelData()->GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId seg = animData.GetLocatorSegId(rstl::string_l(joints[i].mName));
    if (seg != CSegId(0xff)) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          seg, joints[i].mRadius, rstl::string_l(joints[i].mName), 1000.f);
      list.push_back(desc);
    }
  }
}

bool CBabygoth::IsMouthCollisionActor(const TUniqueId& uid) const {
  return uid == mMouthCollisionActor;
}

bool CBabygoth::IsShellCollisionActor(const TUniqueId& uid) const {
  for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
    if (*it == uid) {
      return true;
    }
  }
  return false;
}

void CBabygoth::ExtendTouchBounds(CStateManager& mgr, const CVector3f& bounds) const {
  for (uint i = 0; i < mColActMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mColActMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      actor->SetExtendedTouchBounds(bounds);
    }
  }
}

void CBabygoth::PreventPlayerInterpenetration(CStateManager& mgr, float dt) {
  if (mBodyController->GetLocomotionType() != pas::kLT_Crouch &&
      mBodyController->GetCurrentStateId() != pas::kAS_Step) {
    const rstl::optional_object< CVector3f > deviation =
        mColActMgr->GetDeviation(mgr, mMouthLocator);
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
  mBodyHP = health.GetHP();
  if (CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mMouthCollisionActor))) {
    actor->HealthInfo(mgr)->SetHP(mBabyData.GetShellHitPoints());
  }
  for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      *actor->HealthInfo(mgr) =
          CHealthInfo(mBabyData.GetShellHitPoints(), health.GetKnockBackResistance());
      actor->SetDamageVulnerability(mBabyData.GetShellDamageVulnerability());
    }
  }
  mShellHitPoints = mBabyData.GetShellHitPoints();
}

void CBabygoth::SetupShellDestroyedHealthInfo(CStateManager& mgr) {
  const CHealthInfo& health = *HealthInfo(mgr);
  if (CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mMouthCollisionActor))) {
    *actor->HealthInfo(mgr) = health;
    actor->SetDamageVulnerability(mDVuln);
  }
  for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      *actor->HealthInfo(mgr) = health;
      actor->SetDamageVulnerability(mDVuln);
    }
  }
}

void CBabygoth::UpdateHealthInfo(CStateManager& mgr) {
  if (mAlive) {
    if (mShellState == kSCS_Destroyed) {
      float damage = 0.f;
      if (CCollisionActor* actor =
              TCastToPtr< CCollisionActor >(mgr.ObjectById(mMouthCollisionActor))) {
        damage = CMath::Max(damage, mBodyHP - actor->GetHealthInfo(mgr)->GetHP());
      }
      for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
        if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
          damage = CMath::Max(damage, mBodyHP - actor->GetHealthInfo(mgr)->GetHP());
        }
      }
      HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - damage);
      if (HealthInfo(mgr)->GetHP() <= 0.f) {
        Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
        mInProjectileAttack = true;
        mReadyForTeam = false;
        RemoveFromTeam(mgr);
        RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      } else {
        if (CCollisionActor* actor =
                TCastToPtr< CCollisionActor >(mgr.ObjectById(mMouthCollisionActor))) {
          actor->HealthInfo(mgr)->SetHP(mBodyHP);
        }
        for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
          if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
            actor->HealthInfo(mgr)->SetHP(mBodyHP);
          }
        }
      }
    } else {
      UpdateShellHealthInfo(mgr);
    }
  }
}

void CBabygoth::UpdateShellHealthInfo(CStateManager& mgr) {
  if (mShellHitPoints > 0.f) {
    float damage = 0.f;
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(mMouthCollisionActor))) {
      damage = CMath::Max(damage,
                          mBabyData.GetShellHitPoints() - actor->GetHealthInfo(mgr)->GetHP());
    }
    for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
      if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
        damage = CMath::Max(damage,
                            mBabyData.GetShellHitPoints() - actor->GetHealthInfo(mgr)->GetHP());
      }
    }
    mShellHitPoints -= damage;
    if (mShellHitPoints <= 0.f) {
      mShellState = kSCS_Destroyed;
      SwapSkinnedModel(mgr);
      StartCrackShellEffect(mgr, mDestroyShellParticle, GetTransform(),
                            mBabyData.GetDestroyShellSfx(), false);
      SetupShellDestroyedHealthInfo(mgr);
    } else {
      if (mShellHitPoints < GetShellStateHP(kSCS_CrackTwo)) {
        if (mShellState != kSCS_CrackTwo) {
          StartCrackShellEffect(mgr, mCrackTwoParticle, GetTransform(),
                                mBabyData.GetCrackTwoSfx(), false);
          mShellState = kSCS_CrackTwo;
          mDrawMaterialIdx = 2;
        }
      } else if (mShellHitPoints < GetShellStateHP(kSCS_CrackOne) &&
                 mShellState != kSCS_CrackOne) {
        StartCrackShellEffect(mgr, mCrackOneParticle, GetTransform(),
                              mBabyData.GetCrackOneSfx(), false);
        mShellState = kSCS_CrackOne;
        mDrawMaterialIdx = 1;
      }
    }
    const float hp =
        mShellState != kSCS_Destroyed ? mBabyData.GetShellHitPoints() : mBodyHP;
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(mMouthCollisionActor))) {
      actor->HealthInfo(mgr)->SetHP(hp);
    }
    for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
      if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
        actor->HealthInfo(mgr)->SetHP(hp);
      }
    }
  }
}

void CBabygoth::UpdateAttackTimeLeft(CStateManager& mgr) {
  const float scale = mShellState == kSCS_Destroyed ? 0.6f : 1.f;
  mAttackTimeLeft =
      scale * (mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime);
  mFireballAttackTimeLeft =
      scale * (mgr.Random()->Float() * mBabyData.GetFireballAttackVariance() +
               mBabyData.GetFireballAttackTime());
}

void CBabygoth::UpdateAILogicTimers(float dt) {
  const float scale = mOnApproachPath ? 2.f : 1.f;
  if (mAttackTimeLeft > 0.f) {
    mAttackTimeLeft -= dt * scale;
  }
  if (mFireballAttackTimeLeft > 0.f) {
    mFireballAttackTimeLeft -= dt * scale;
  }
  if (mAttackCooldownTimeLeft > 0.f) {
    mAttackCooldownTimeLeft -= dt;
  }
  if (mInterestTimer < mBabyData.GetInterestTime()) {
    mInterestTimer += dt;
  }
}

void CBabygoth::CreateFlameThrower(CStateManager& mgr) {
  if (mFlameThrower == kInvalidUniqueId) {
    const CFlameInfo info(6, 4, mBabyData.GetFireBreathResId(), 15, 0.0625f, 20.f, 1.f);
    mFlameThrower = mgr.AllocateUniqueId();
    CFlameThrower* const flame = rs_new CFlameThrower(
        mFlameThrowerDesc, rstl::string_l("IceSheegoth_Flame"), kWT_Plasma, info,
        CTransform4f::Identity(), kMT_CollisionActor, mBabyData.GetFireBreathDamage(),
        mFlameThrower, GetCurrentAreaId(), GetUniqueId(), CWeapon::kPA_None,
        mBabyData.GetFlamePlayerSteamTxtr(), mBabyData.GetFlamePlayerHitSfx(),
        mBabyData.GetFlamePlayerIceTxtr());
    mgr.AddObject(*flame);
  }
}

void CBabygoth::UpdateParticleEffects(float dt, CStateManager& mgr) {
  if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(mFlameThrower))) {
    if (flame->GetActive()) {
      const CTransform4f xf = GetLctrTransform(rstl::string_l("LCTR_SHEMOUTH"));
      flame->SetTransform(xf, mgr, dt);
    }
  }
}

void CBabygoth::SetPathFindMode(EPathFindMode mode) { mPathFindMode = mode; }

EWeaponCollisionResponseTypes CBabygoth::GetCollisionResponseType(const CVector3f& pos,
                                                                  const CVector3f& dir,
                                                                  const CWeaponMode& weapon,
                                                                  int attrib) const {
  if (weapon.GetType() == kWT_Ice) {
    return kWCR_None;
  }
  if (mShellState != kSCS_Destroyed) {
    return kWCR_Unknown66;
  }
  return CPatterned::GetCollisionResponseType(pos, dir, weapon, attrib);
}

bool CBabygoth::IsOtherCharacterNearPathDest(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f delta = mBackupDestPos - ai->GetTranslation();
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
            mSteeringBehaviors.Separation(*this, ai->GetTranslation(), 15.f);
        if (separation.IsNonZero()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CBabygoth::ProcessCharge(CStateManager& mgr, float dt) {
  switch (mStateProg) {
  case 0:
    mAttackTimer += dt;
    if (mAttackTimer < mBabyData.GetTimeUntilAttack()) {
      if (!mPendingAttackContactDamage) {
        mStateProg = 3;
        mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
      } else {
        const CTransform4f mouthXf = GetLctrTransform(mMouthLocator);
        const CVector3f delta = mgr.GetPlayer()->GetTranslation() - mouthXf.GetTranslation();
        const CVector3f forward = GetTransform().GetForward();
        if (CVector3f::Dot(forward, delta) > 0.f) {
          SetPathFindMode(kPFM_Normal);
          if (GetSearchPath() && !PathShagged(mgr, 0.f)) {
            CPatterned::PathFind(mgr, kStateMsg_Update, dt);
            ApplySeparationBehavior(mgr);
          } else {
            mStateProg = 4;
          }
        } else {
          mStateProg = 4;
        }
      }
    } else {
      mStateProg = 4;
    }
    break;
  case 3:
    if (mBodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
      mStateProg = 4;
    } else {
      mShouldStepBackwards = true;
    }
    break;
  }
}

bool CBabygoth::CheckShouldGetUp(CStateManager& mgr) {
  if (mAlive && mBodyController->GetFallState() != pas::kFS_Zero && !mGettingUp) {
    mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("GetUp"));
  }
  return false;
}

bool CBabygoth::CheckShouldWakeUp(CStateManager& mgr, float dt) {
  if (mHeardPlayerFire) {
    mIsAlert = mHitByPlayerProjectile || mgr.Random()->Float() < 0.5f * dt;
    return mIsAlert;
  }
  return false;
}

void CBabygoth::SwapSkinnedModel(CStateManager& mgr) {
  AnimationData()->SubstituteModelData(mNoShellModel);
  for (AUTO(it, mShellIds.begin()); it != mShellIds.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      actor->SetWeaponCollisionResponseType(kWCR_Unknown41);
    }
  }
  mDrawMaterialIdx = 0;
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
    hp = mBabyData.GetShellHitPoints();
    break;
  case kSCS_CrackOne:
    hp = (2.f / 3.f) * mBabyData.GetShellHitPoints();
    break;
  case kSCS_CrackTwo:
    hp = (1.f / 3.f) * mBabyData.GetShellHitPoints();
    break;
  case kSCS_Destroyed:
    hp = 0.f;
    break;
  }
  return hp;
}

void CBabygoth::AddToTeam(CStateManager& mgr) {
  if (mTeamMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
      if (!team->IsPartOfTeam(GetUniqueId())) {
        team->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Projectile,
                               CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CBabygoth::RemoveFromTeam(CStateManager& mgr) {
  if (mTeamMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

void CBabygoth::SetShootThrough(CStateManager& mgr) const {
  for (uint i = 0; i < mColActMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mColActMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
    }
  }
}

void CBabygoth::ReDirectDamage(CStateManager& mgr, TUniqueId uid) {
  if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(uid))) {
    if (!mShellIds.empty()) {
      CDamageInfo damage = weapon->GetCurrentDamageInfo();
      damage.SetRadius(0.f);
      mgr.ApplyDamage(
          uid, mShellIds[0], weapon->GetOwnerId(), damage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
  }
}

float CBabygoth::GetGravityConstant() const { return 10.f * GravityConstant(); }

const CCollisionPrimitive* CBabygoth::GetCollisionPrimitive() const { return &mAabox; }
bool CBabygoth::IsListening() const { return true; }
CPathFindSearch* CBabygoth::GetSearchPath() {
  return mPathFindMode == kPFM_Normal ? &mPathSearch : &mApproachPathSearch;
}
