#include "MetroidPrime/Enemies/CMetroidPrimeStage2.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "rstl/math.hpp"

static const int skVisorPhases[] = {0, 1, 0, 2};
static const int skStepDirections[] = {1, 2, 3};
static const char* const skLockOnTarget = "lockon_target_LCTR";
static const char* const skEyes = "Eyes";
static const char* const skHead = "Head";
static SSphereJointInfo skJointInfo[] = {{skLockOnTarget, 1.5f}};

template < int N >
static inline uint CountBitGroups(uint bits) {
  const uint mask = 0xffffffffu / ((1u << (1u << N)) + 1u);
  bits = (bits & mask) + ((bits & ~mask) >> (1 << N));
  return CountBitGroups< N + 1 >(bits);
}
template <>
inline uint CountBitGroups< 5 >(uint bits) {
  return bits;
}
static inline uint CountSetBits(uint bits) { return CountBitGroups< 0 >(bits); }

CMetroidPrimeStage2::CMetroidPrimeStage2(TUniqueId uid, const rstl::string& name,
                                         const CEntityInfo& info, const CTransform4f& xf,
                                         const CModelData& mData, const CPatternedInfo& pInfo,
                                         const CActorParameters& actParms, CAssetId particle1,
                                         CDamageInfo dInfo, CAssetId electric, uint sfxId,
                                         CAssetId particle2)
: CPatterned(kC_MetroidPrimeStage2, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Flyer, actParms, kCS_Medium)
, mParticleDesc(gpSimplePool->GetObj(SObjectTag('PART', particle2)))
, mSearchPath(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mCollisionManager(nullptr)
, mElementGen(nullptr)
, mParticle1(particle1)
, mElectric(electric)
, mSpawnXf(CTransform4f::Identity())
, mDamageInfo(dInfo)
, mSpaceWarpPos(xf.GetTranslation())
, mHpThreshold(0.f)
, mHpFraction(0.f)
, mPhaseAlpha(0.f)
, mBaseScale(4.f)
, mMaxScale(0.9f * mBaseScale + mBaseScale)
, mMorphT(0.f)
, mDamagePhase(0)
, mCurrentVisorPhase(0)
, mPreviousVisorPhase(mCurrentVisorPhase)
, mSpawnedAiCount(0)
, mMinAttackInterval(2)
, mMaxAttackInterval(4)
, mAttackCounter(0)
, mNextAttackThreshold(mMinAttackInterval - 1)
, mMaxSpawnedCount(2)
, mMaterialSetIdx(0)
, mStepDirection(1)
, mBossUtilityWaypointId(kInvalidUniqueId)
, mLockOnTargetCollider(kInvalidUniqueId)
, mSfxHandle()
, mSfxId(CSfxManager::TranslateSFXID(sfxId))
, mIsProjectileAttacking(false)
, mCanAttack(true)
, mIsPhaseTransitioning(false)
, mIsSpaceWarping(false)
, mIsVisible(true)
, mCanSpawnAi(false)
, mIsMorphing(false)
, mHasEventStarted(false) {
  SetDrawShadow(false);
  MakeThermalColdAndHot();
}

CMetroidPrimeStage2::~CMetroidPrimeStage2() {}

void CMetroidPrimeStage2::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                          CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered: {
    SetupCollisionManager(mgr);
    mCollisionManager->SetActive(mgr, true);
    mBaseScale = GetModelData()->GetScale().GetX();
    mMaxScale = 0.9f * mBaseScale + mBaseScale;
    const float scale = 1.f / (0.625f * mBaseScale);
    mMoveScale = CVector3f(scale, scale, scale);
    const float hp = HealthInfo(mgr)->GetHP();
    mHpThreshold = 0.3f * hp;
    if (hp > 0.f) {
      mHpFraction = 1.f / hp;
    }
    mBodyController->Activate(mgr);
    break;
  }
  case kSM_InitializedInArea: {
    const TAreaId area = GetCurrentAreaId();
    mSearchPath.SetArea(mgr.GetWorld()->GetArea(area)->GetPostConstructed()->mPathArea);
    mBossUtilityWaypointId = GetConnectedObject(mgr, kSS_Play, kSM_Activate);
    break;
  }
  case kSM_Activate:
    mCollisionManager->SetActive(mgr, true);
    break;
  case kSM_Deactivate:
    mCollisionManager->SetActive(mgr, false);
    break;
  case kSM_Deleted:
    mCollisionManager->Destroy(mgr);
    mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
    break;
  case kSM_Touched:
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      if (actor->GetLastTouchedObject() == mgr.GetPlayer()->GetUniqueId() &&
          mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
    break;
  case kSM_Damage:
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (const CGameProjectile* projectile =
              TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(touched))) {
        if (projectile->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
          const CDamageVulnerability* vulnerability = actor->GetDamageVulnerability();
          const CWeaponMode& mode = projectile->GetCurrentDamageInfo().GetWeaponMode();
          if (vulnerability->WeaponHits(mode, CDamageVulnerability::kRD_No) &&
              mode.GetType() == kWT_Phazon) {
            UpdateSummonType(mgr);
            TakeDamage(CVector3f::Forward(), 1.f);
            if (!mIsProjectileAttacking && !mIsPhaseTransitioning) {
              mBodyController->CommandMgr().DeliverCmd(
                  CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_One));
              PlayPainSound(mgr);
            }
          }
        }
      }
    } else if (const CGameProjectile* projectile =
                   TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      mgr.ApplyDamage(
          uid, mLockOnTargetCollider, projectile->GetOwnerId(),
          projectile->GetCurrentDamageInfo(),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
    break;
  case kSM_Start:
    mCanAttack = true;
    break;
  case kSM_Stop:
    mCanAttack = false;
    break;
  case kSM_InvulnDamage:
    break;
  }
}

void CMetroidPrimeStage2::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(1);
  AddSphereCollisionList(skJointInfo, 1, joints);
  mCollisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  mCollisionManager->SetActive(mgr, GetActive());
  for (uint i = 0; i < mCollisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mCollisionManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      if (desc.GetName() == rstl::string_l(skLockOnTarget)) {
        mLockOnTargetCollider = id;
      }
    }
  }
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_Player, kMT_Character)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
}

void CMetroidPrimeStage2::AddSphereCollisionList(
    const SSphereJointInfo* joints, int count, rstl::vector< CJointCollisionDescription >& descs) {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId seg = animData->GetLocatorSegId(rstl::string_l(joints[i].name));
    if (seg != CSegId(0xff)) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          seg, joints[i].radius, rstl::string_l(joints[i].name), 1000.f);
      descs.push_back(desc);
    }
  }
}

void CMetroidPrimeStage2::SetupHealthInfo(CStateManager& mgr) {
  const CHealthInfo* health = HealthInfo(mgr);
  if (CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mLockOnTargetCollider))) {
    *actor->HealthInfo(mgr) = *health;
    actor->SetDamageVulnerability(
        *static_cast< const CMetroidPrimeStage2* >(this)->GetDamageVulnerability());
  }
}

void CMetroidPrimeStage2::UpdateHealthInfo(CStateManager& mgr) {
  if (IsAlive()) {
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(mLockOnTargetCollider))) {
      actor->SetDamageVulnerability(
          *static_cast< const CMetroidPrimeStage2* >(this)->GetDamageVulnerability());
      CAi::HealthInfo(mgr)->SetHP(actor->HealthInfo(mgr)->GetHP());
    }
    if (HealthInfo(mgr)->GetHP() <= 0.f) {
      Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
      RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    }
  }
}

void CMetroidPrimeStage2::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    if (IsAlive()) {
      UpdateVisibleSpectrum(dt, mgr);
    }
    mBodyController->FaceDirection(mgr.GetPlayer()->GetTranslation() - GetTranslation(), dt);
    mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    UpdateHealthInfo(mgr);
    UpdateNumActiveMetroids(mgr);
    if (mIsMorphing) {
      mMorphT += 2.f * dt;
      if (mMorphT >= 1.f) {
        mMorphT = 0.f;
      }
      UpdateMaterialSet(-4.f * mMorphT * (mMorphT - 1.f));
    }
  }
}

void CMetroidPrimeStage2::UpdateVisibleSpectrum(float dt, CStateManager& mgr) {
  const int phase = skVisorPhases[mgr.GetPlayerState()->GetCurrentVisor()];
  if (phase == mCurrentVisorPhase) {
    const float alpha = 1.f - mPhaseAlpha;
    mColor.SetAlpha(alpha);
    const float scale = alpha * (mBaseScale - mMaxScale) + mMaxScale;
    ModelData()->SetScale(CVector3f(scale, scale, scale));
    if (mIsVisible != true) {
      AddMaterial(kMT_Orbit, kMT_Target, mgr);
      ActivateHeadFx(mgr, true);
      mIsVisible = true;
    }
  } else {
    const float alpha = phase == mPreviousVisorPhase ? mPhaseAlpha : 0.f;
    mColor.SetAlpha(alpha);
    const float scale = alpha * (mBaseScale - mMaxScale) + mMaxScale;
    ModelData()->SetScale(CVector3f(scale, scale, scale));
    if (mIsVisible) {
      RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      ActivateHeadFx(mgr, false);
      mIsVisible = false;
    }
  }
  const CTransform4f& xf = GetCreatureTransform(mgr);
  if (mIsPhaseTransitioning) {
    mPhaseAlpha -= 0.5f * dt;
    mSpaceWarpPos = xf.GetTranslation();
    if (mPhaseAlpha < 0.f) {
      mPhaseAlpha = 0.f;
      mIsPhaseTransitioning = false;
      mIsSpaceWarping = false;
    }
  }
  if (mElementGen.get()) {
    if (!mElementGen->IsSystemDeletable()) {
      mElementGen->SetGlobalOrientation(xf.GetRotation());
      mElementGen->SetGlobalTranslation(xf.GetTranslation());
      mElementGen->Update(dt);
    } else {
      mElementGen = nullptr;
    }
  }
}

void CMetroidPrimeStage2::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CPatterned::PreRender(mgr, frustum);
  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
  const int phase = skVisorPhases[visor];
  const bool xray = visor == CPlayerState::kPV_XRay;
  if (mIsPhaseTransitioning) {
    float alpha = 1.f - mPhaseAlpha;
    if (phase != mCurrentVisorPhase) {
      alpha = phase == mPreviousVisorPhase ? mPhaseAlpha : 0.f;
    }
    SetModelFlags(CModelFlags::AlphaBlendedDepthCompareUpdate(alpha, true, false)
                      .UseShaderSet(xray ? 0 : mMaterialSetIdx));
  } else {
    const CModelFlags flags = GetModelFlags().UseShaderSet(xray ? 0 : mMaterialSetIdx);
    if (phase != mCurrentVisorPhase) {
      SetModelFlags(flags.DepthCompareUpdate(true, false));
    } else {
      SetModelFlags(flags);
    }
  }
}

void CMetroidPrimeStage2::Render(const CStateManager& mgr) const {
  if (mIsSpaceWarping) {
    mgr.DrawSpaceWarp(mSpaceWarpPos, 1.f);
  }
  CPatterned::Render(mgr);
}

void CMetroidPrimeStage2::UpdateNumActiveMetroids(CStateManager& mgr) {
  mSpawnedAiCount = 0;
  CObjectList& list = mgr.ObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (CPatterned* actor = TCastToPtr< CPatterned >(list[i])) {
      if (actor != this && actor->GetActive() && actor->GetCurrentAreaId() == GetCurrentAreaId()) {
        ++mSpawnedAiCount;
      }
    }
  }
}

void CMetroidPrimeStage2::KillActiveMetroids(CStateManager& mgr) {
  CObjectList& list = mgr.ObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (CPatterned* actor = TCastToPtr< CPatterned >(list[i])) {
      if (actor != this && actor->GetActive() && actor->GetCurrentAreaId() == GetCurrentAreaId()) {
        actor->MassiveDeath(mgr);
      }
    }
  }
}

bool CMetroidPrimeStage2::CanSummonToPosition(const CTransform4f& xf, CStateManager& mgr) {
  TEntityList nearList;
  const CMaterialFilter nearFilter =
      CMaterialFilter::MakeInclude(CMaterialList(kMT_Player, kMT_Character, kMT_AIBlock));
  const CVector3f position = xf.GetTranslation();
  const CAABox bounds(position - CVector3f(2.f, 2.f, 2.f), position + CVector3f(2.f, 2.f, 2.f));
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid, kMT_Player, kMT_Character, kMT_AIBlock),
      CMaterialList(kMT_ProjectilePassthrough));
  mgr.BuildNearList(nearList, bounds, nearFilter, this);
  TUniqueId id = kInvalidUniqueId;
  CCollisionInfoList info;
  CCollidableSphere sphere(CSphere(CVector3f::Zero(), 2.f), CMaterialList(kMT_Solid, kMT_AIBlock));
  CGameCollision::DetectCollision(mgr, sphere, xf, filter, nearList, id, info);
  if (info.GetCount() > 0) {
    return false;
  }
  if (CCollisionActor* const actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mLockOnTargetCollider))) {
    const CVector3f delta = position - actor->GetTranslation();
    const float distance = delta.Magnitude();
    const CRayCastResult result = mgr.RayStaticIntersection(
        actor->GetTranslation(), delta.AsNormalized(), distance,
        CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough)));
    if (result.IsInvalid()) {
      return true;
    }
  }
  return false;
}

void CMetroidPrimeStage2::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                          EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_EventStart:
    if (mHasEventStarted != true) {
      SendScriptMsgs(kSS_CameraTarget, mgr, kSM_None);
      mHasEventStarted = true;
    }
    handled = true;
    break;
  case kUE_BeginAction: {
    const ushort sfx = mSfxId;
    CShockWaveInfo info(mParticle1, mDamageInfo, 2.f, mElectric, sfx, 180.f);
    CreateShockWave(mgr, info);
    BlastShake(1.f, mgr);
    handled = true;
    break;
  }
  case kUE_Activate:
    SpawnPhazonPool(mgr);
    handled = true;
    break;
  case kUE_EggLay:
    if (mCanSpawnAi && mDamagePhase != 0 && mSpawnedAiCount < mMaxSpawnedCount) {
      const float angle1 = CRelAngle::FromDegrees(22.5f).AsRadians() * mgr.Random()->Range(-1, 1);
      const float angle2 = CRelAngle::FromDegrees(45.f).AsRadians() * mgr.Random()->Range(-1, 1);
      const float radius = 2.f * CMath::FastCosR(angle1);
      const CVector3f position = mSpawnXf * (2.f * CVector3f(radius * CMath::FastSinR(angle2),
                                                                 -CMath::FastSinR(angle1),
                                                                 radius * CMath::FastCosR(angle2)));
      if (CScriptWaypoint* waypoint =
              TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mBossUtilityWaypointId))) {
        waypoint->SetTransform(CTransform4f::LookAt(
            position, mgr.GetPlayer()->GetAimPosition(mgr, 0.f), CVector3f::Up()));
        if (CanSummonToPosition(waypoint->GetTransform(), mgr)) {
          SendScriptMsgs(kSS_Zero, mgr, kSM_None);
          mSpaceWarpPos = waypoint->GetTranslation();
        }
      }
    }
    handled = true;
    break;
  case kUE_Deactivate:
    mIsSpaceWarping = false;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

CVector3f CMetroidPrimeStage2::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (const CCollisionActor* actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mLockOnTargetCollider))) {
    return actor->GetTranslation();
  }
  return CPatterned::GetAimPosition(mgr, dt);
}

void CMetroidPrimeStage2::AddToRenderer(const CFrustumPlanes& frustum,
                                        const CStateManager& mgr) const {
  if (GetActive() && mElementGen.get()) {
    gpRender->AddParticleGen(*mElementGen);
  }
  CPatterned::AddToRenderer(frustum, mgr);
}

void CMetroidPrimeStage2::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f lookPos = mgr.GetPlayer()->GetTranslation();
    lookPos.SetZ(GetTranslation().GetZ());
    CTransform4f xf = CTransform4f::LookAt(GetTranslation(), lookPos, CVector3f::Up());
    xf.SetTranslation(GetTranslation());
    SetTransform(xf);
    break;
  }
  case kStateMsg_Deactivate:
    mgr.SetBossParams(GetUniqueId(), HealthInfo(mgr)->GetHP(), 91);
    ActivateHeadFx(mgr, true);
    break;
  }
}

void CMetroidPrimeStage2::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    mIsMorphing = true;
    break;
  case kStateMsg_Update:
    if (mHasEventStarted) {
      TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, 5);
    } else {
      TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 2);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    mIsMorphing = false;
    UpdateMaterialSet(1.f);
    break;
  }
}

void CMetroidPrimeStage2::FadeIn(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mMaxSpawnedCount = GetMaxSpawnCount(mgr);
    mAnimState = kAS_Ready;
    mIsProjectileAttacking = true;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
    break;
  case kStateMsg_Deactivate:
    mIsProjectileAttacking = false;
    mIsSpaceWarping = false;
    mCanSpawnAi = false;
    mIsMorphing = false;
    mAnimState = kAS_NotReady;
    break;
  }
}

void CMetroidPrimeStage2::FadeOut(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    StartSpectralFade(mgr);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetroidPrimeStage2::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    mStepDirection = GetAvoidanceStep(mgr, false);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, &CPatterned::TryStep, mStepDirection);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CMetroidPrimeStage2::Halt(CStateManager& mgr, EStateMsg msg, float dt) {}

void CMetroidPrimeStage2::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Update:
    StepToPosition(mgr.GetPlayer()->GetTranslation());
    break;
  }
}

void CMetroidPrimeStage2::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    mIsProjectileAttacking = true;
    break;
  case kStateMsg_Update:
    StepToPosition(mDestPos);
    break;
  case kStateMsg_Deactivate:
    mIsProjectileAttacking = false;
    break;
  }
}

void CMetroidPrimeStage2::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    mStepDirection = GetAvoidanceStep(mgr, true);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, &CPatterned::TryStep, mStepDirection);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CMetroidPrimeStage2::Skid(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, &CPatterned::TryStep, 5);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CMetroidPrimeStage2::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, &CPatterned::TryTaunt, 2);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CMetroidPrimeStage2::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Update:
    if (!GetModelData()->GetAnimationData()->IsAnimTimeRemaining(dt,
                                                                 rstl::string_l("Whole Body"))) {
      DeathDelete(mgr);
    }
    break;
  }
}

bool CMetroidPrimeStage2::ShouldAttack(CStateManager& mgr, float arg) {
  if (mHasEventStarted) {
    return mCanAttack;
  }
  return true;
}

bool CMetroidPrimeStage2::ShouldCrouch(CStateManager& mgr, float arg) {
  if (mAttackCounter < mNextAttackThreshold) {
    ++mAttackCounter;
    return false;
  }
  const float hpFraction = mHpFraction * HealthInfo(mgr)->GetHP();
  mNextAttackThreshold =
      rstl::min_val(mMinAttackInterval + static_cast< int >(3.f * (1.f - hpFraction)),
                    mMaxAttackInterval);
  mAttackCounter = 0;
  return true;
}

bool CMetroidPrimeStage2::ShouldTaunt(CStateManager& mgr, float arg) {
  const CHealthInfo* info = HealthInfo(mgr);
  if (info && info->GetHP() > mHpThreshold) {
    return mgr.Random()->Next() % 100 < 50;
  }
  return false;
}

bool CMetroidPrimeStage2::InPosition(CStateManager& mgr, float arg) {
  return GetTransform().Get23() - mgr.GetPlayer()->GetTransform().Get23() > 0.25f;
}

bool CMetroidPrimeStage2::HasPatrolPath(CStateManager& mgr, float arg) {
  return !mHasEventStarted && CPatterned::HasPatrolPath(mgr, arg);
}

bool CMetroidPrimeStage2::ShouldMove(CStateManager& mgr, float arg) {
  return !mHasEventStarted;
}

bool CMetroidPrimeStage2::CoverFind(CStateManager& mgr, float arg) {
  CVector3f delta = mDestPos - GetTranslation();
  return delta.MagSquared() < 90.f;
}

void CMetroidPrimeStage2::Death(CStateManager& mgr, const CVector3f& direction,
                                EScriptObjectState state) {
  if (IsAlive()) {
    KillActiveMetroids(mgr);
    ActivateHeadFx(mgr, false);
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(mLockOnTargetCollider))) {
      actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
    }
    CPatterned::Death(mgr, direction, state);
  }
}

void CMetroidPrimeStage2::SpawnPhazonPool(CStateManager& mgr) {
  const CRayCastResult result = GetGroundContactPoint(mgr);
  if (result.IsValid()) {
    mSpawnXf = CTransform4f::Translate(result.GetPoint());
    if (CScriptWaypoint* waypoint =
            TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mBossUtilityWaypointId))) {
      waypoint->SetTransform(mSpawnXf);
      SendScriptMsgs(kSS_AboutToMassivelyDie, mgr, kSM_None);
      mCanSpawnAi = true;
    }
  }
}

void CMetroidPrimeStage2::ActivateHeadFx(CStateManager& mgr, const bool active) {
  CAnimData& animData = *ModelData()->AnimationData();
  animData.SetParticleEffectState(rstl::string_l(skEyes), active, mgr);
  animData.SetParticleEffectState(rstl::string_l(skHead), active, mgr);
}

CRayCastResult CMetroidPrimeStage2::GetGroundContactPoint(CStateManager& mgr) {
  return mgr.RayStaticIntersection(GetTranslation(), -CVector3f::Up(), 30.f,
                                   CMaterialFilter::skPassEverything);
}

void CMetroidPrimeStage2::CreateShockWave(CStateManager& mgr, const CShockWaveInfo& info) {
  const CRayCastResult result = GetGroundContactPoint(mgr);
  if (result.IsValid()) {
    CShockWave* const shock = rs_new CShockWave(mgr.AllocateUniqueId(), rstl::string_l("Shockwave"),
                                                CEntityInfo(GetCurrentAreaId(), NullConnectionList),
                                                CTransform4f::Translate(result.GetPoint()),
                                                GetUniqueId(), info, 1.5f, 0.5f);
    if (shock) {
      mgr.AddObject(shock);
    }
  }
}

void CMetroidPrimeStage2::BlastShake(float magnitude, CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  const CVector3f delta = GetTranslation() - player->GetTranslation();
  const float shake = 0.5f - 0.01f * delta.Magnitude();
  if (shake > 0.f && player->GetSurfaceRestraint() != CPlayer::kSR_Air) {
    mgr.CameraManager()->AddCameraShaker(CCameraShakeData::HardVertShake(0.5f, shake), true);
  }
}

int CMetroidPrimeStage2::GetMaxSpawnCount(CStateManager& mgr) { return 2; }

void CMetroidPrimeStage2::StartSpectralFade(CStateManager& mgr) {
  mStateMachineState.SetDelay(2.f);
  mIsPhaseTransitioning = true;
  mIsSpaceWarping = true;
  mPhaseAlpha = 1.f;
  mCanSpawnAi = false;
  const int phase = skVisorPhases[mgr.GetPlayerState()->GetCurrentVisor()];
  const ushort sfx =
      phase == mCurrentVisorPhase ? static_cast< ushort >(0xb7e) : static_cast< ushort >(0xb7d);
  if (phase == mCurrentVisorPhase) {
    mElementGen =
        rs_new CElementGen(mParticleDesc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    if (mElementGen.get()) {
      const CTransform4f& xf = GetCreatureTransform(mgr);
      mElementGen->SetGlobalScale(GetModelData()->ScaleCopy());
      mElementGen->SetGlobalOrientation(xf.GetRotation());
      mElementGen->SetGlobalTranslation(xf.GetTranslation());
    }
  }
  CSfxManager::AddEmitter(sfx, GetTranslation(), CVector3f::Zero(), true, false,
                          CSfxManager::kMedPriority, CSfxManager::kAllAreas);
  mPreviousVisorPhase = mCurrentVisorPhase;
  ++mCurrentVisorPhase;
  if (mCurrentVisorPhase > 2) {
    mCurrentVisorPhase = 0;
  }
}

uint CMetroidPrimeStage2::GetAvoidanceStep(CStateManager& mgr, bool allowBackward) {
  const CTransform4f& xf = GetCreatureTransform(mgr);
  int start = allowBackward ? 0 : 1;
  CVector3f directions[3];
  directions[0] = -xf.GetForward();
  directions[1] = -xf.GetRight();
  directions[2] = xf.GetRight();
  int available = 1 << start;
  for (int i = start; i < 3; ++i) {
    const CRayCastResult result = mgr.RayStaticIntersection(
        xf.GetTranslation(), directions[i], 20.f, CMaterialFilter::skPassEverything);
    if (result.IsInvalid()) {
      available |= 1 << i;
    }
  }
  int choice = 0;
  if (available <= 7) {
    switch (CountSetBits(available)) {
    case 1:
      choice = available >> 1;
      break;
    case 2:
      if ((mgr.Random()->Next() & 1) != 0) {
        choice = (available & 4) != 0 ? 2 : 1;
      } else {
        choice = (available & 1) ^ 1;
      }
      break;
    case 3:
      choice = mgr.Random()->Range(start, 2);
      break;
    }
  }
  return skStepDirections[choice];
}

void CMetroidPrimeStage2::UpdateSummonType(CStateManager& mgr) {
  const float hp = mHpFraction * HealthInfo(mgr)->GetHP();
  if (hp >= 0.f) {
    bool sendMsg = false;
    switch (mDamagePhase) {
    case 0:
      if (hp < 0.75f) {
        mDamagePhase = 1;
      }
      break;
    case 1:
      if (hp < 0.5f) {
        sendMsg = true;
        mDamagePhase = 2;
      }
      break;
    case 2:
      if (hp < 0.25f) {
        sendMsg = true;
        mDamagePhase = 3;
      }
      break;
    }
    if (sendMsg) {
      SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
    }
  }
}

void CMetroidPrimeStage2::UpdateMaterialSet(float t) {
  const float count = GetModelData()->GetNumMaterialSets() - 2;
  const int material = count - count * t;
  if (mMaterialSetIdx != material) {
    mMaterialSetIdx = material;
  }
}

const CTransform4f& CMetroidPrimeStage2::GetCreatureTransform(CStateManager& mgr) const {
  if (const CCollisionActor* actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mLockOnTargetCollider))) {
    return actor->GetTransform();
  }
  return GetTransform();
}

void CMetroidPrimeStage2::PlayPainSound(CStateManager& mgr) {
  if (!CSfxManager::IsPlaying(mSfxHandle)) {
    const int area = GetCurrentAreaId().Value();
    CAudioSys::C3DEmitterParmData parms(1000.f, 0.1f, 1, 127, 20);
    parms.mPos = GetCreatureTransform(mgr).GetTranslation();
    parms.mDir = CVector3f::Zero();
    parms.mSfxId = 0xb67;
    mSfxHandle = CSfxManager::AddEmitter(parms, true, CSfxManager::kMedPriority, false, area);
  }
}

void CMetroidPrimeStage2::StepToPosition(const CVector3f& position) {
  pas::EStepDirection dir =
      FindBestStepDirection(mBodyController->GetCommandMgr().GetMoveVector());
  mBodyController->CommandMgr().ClearLocomotionCmds();
  if (dir == pas::kSD_Forward) {
    const CVector3f delta = mDestPos - GetTranslation();
    if (CVector3f::Dot(GetTransform().GetForward().AsNormalized(), delta.AsNormalized()) <
        -0.258819f) {
      dir = pas::kSD_Backward;
    }
  }
  mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(dir, pas::kStep_Normal));
  mBodyController->CommandMgr().DeliverTargetVector(position - GetTranslation());
}
