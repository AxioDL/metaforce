#include "MetroidPrime/Enemies/CMetroidBeta.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

const uint CMetroidBetaData::skNumProperties = 23;

static const SSphereJointInfo skSphereJoints[] = {{"Pelvis", 1.5f}};
static const char* const skLeftClaw = "L_Claw_1";
static const char* const skRightClaw = "R_Claw_1";
static const char* const skPelvis = "Pelvis";
static const CVector3f skExtendedTouchBounds(1.f, 1.f, 1.f);
static const CVector3f skTentacleOffset(-0.2f, 0.f, -0.3f);

CMetroidBetaData::CMetroidBetaData(CInputStream& in, int propCount)
: mFrozenVulnerability(in)
, mEnergyDrainVulnerability(in)
, mEnergyDrainPerSec(in.Get< float >())
, mMaxEnergyDrainAllowed(in.Get< float >())
, mBreakLeashEnergyDrain(in.Get< float >())
, mTelegraphAttackTime(in.Get< float >())
, mSpecialAttackTime(in.Get< float >())
, xe4_(in.Get< float >())
, mMaxGrowthScale(in.Get< float >())
, mStage2GrowthEnergy(in.Get< float >())
, mStage3GrowthEnergy(in.Get< float >())
, mElectricParticleId(in.Get< CAssetId >())
, mElectricSwooshEffectId(in.Get< CAssetId >())
, mElectricMorphBallHitEffectId(in.Get< CAssetId >())
, mElectricGunHitEffectId(in.Get< CAssetId >())
, mElectricGunFeedbackId(in.Get< CAssetId >())
, mStartsInWall(in.ReadBool()) {}

CMetroidBeta::CMetroidBeta(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CPatternedInfo& pInfo, const CActorParameters& aParms,
                           const CMetroidBetaData& metroidData)
: CPatterned(kC_MetroidBeta, uid, name, kFT_One, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Flyer, aParms, kCS_Large)
, mProgState(-1)
, mData(metroidData)
, mTeamMgr(kInvalidUniqueId)
, mPathFind(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mLeftClaw(0xff)
, mRightClaw(0xff)
, mCollisionManager(nullptr)
, mColPrim(GetBoundingBox(), GetMaterialList())
, mPelvisCollisionId(kInvalidUniqueId)
, mAttackCooldown(0.f)
, mTelegraphTimer(0.f)
, mDrainedEnergy(0.f)
, mInterferenceTime(0.f)
, mSpecialAttackTime(0.f)
, mTargetPos(CVector3f::Zero())
, mGrowthScale(GetModelData()->ScaleCopy())
, mGenerateStartScale(GetModelData()->ScaleCopy())
, mInitialScale(GetModelData()->ScaleCopy())
, mGenerateDuration(0.f)
, mGrowthEnergy(0.f)
, mPreviousGrowthEnergy(0.f)
, x7e4_(gpSimplePool->GetObj(SObjectTag('PART', metroidData.GetElectricParticleId())))
, x7f0_(gpSimplePool->GetObj(SObjectTag('SWHC', metroidData.GetElectricSwooshEffectId())))
, x7fc_(gpSimplePool->GetObj(SObjectTag('PART', metroidData.GetElectricMorphBallHitEffectId())))
, x808_(gpSimplePool->GetObj(SObjectTag('PART', metroidData.GetElectricGunHitEffectId())))
, x814_(gpSimplePool->GetObj(SObjectTag('PART', metroidData.GetElectricGunFeedbackId())))
, x820_(rs_new CElementGen(x7e4_))
, x824_(rs_new CParticleSwoosh(x7f0_, 0))
, x828_(rs_new CElementGen(x7fc_))
, x82c_(rs_new CElementGen(x808_))
, x830_(rs_new CElementGen(x814_))
, mParticlePhase(0.f)
, mRandom(1469)
, x840_24_(false)
, x840_25_(false)
, x840_26_(false)
, x840_27_(false)
, x840_28_(false)
, x840_29_(false)
, x840_30_(false)
, x840_31_(false) {
  x820_->SetParticleEmission(false);
  x828_->SetParticleEmission(false);
  x82c_->SetParticleEmission(false);
  const rstl::vector< CParticleSwoosh::SSwooshData >& swooshes = x824_->GetSwooshes();
  for (int i = 0; i < swooshes.size() - 1; ++i) {
    x824_->SetWarmUp();
    x824_->Update(0.0);
  }
  const float scale = 0.75f * GetModelData()->ScaleCopy().GetY();
  const CAABox box(CVector3f(-scale, -scale, -2.f * scale), CVector3f(scale, scale, 2.f * scale));
  SetBoundingBox(box);
  mColPrim.Box() = box;
}

CMetroidBeta::~CMetroidBeta() {}

ENTITY_ACCEPT_IMPL(CMetroidBeta)

void CMetroidBeta::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (CTeamAiMgr::GetTeamAiRole(mgr, mTeamMgr, GetUniqueId()) == nullptr) {
      SwarmAdd(mgr);
    }
    CPatterned::Think(dt, mgr);
    mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    UpdateHealthInfo(mgr);
    UpdateAILogicTimers(dt, mgr);
    UpdateParticleEffects(dt, mgr);
  }
}

void CMetroidBeta::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    SetupCollisionManager(mgr);
    UpdateSoundVolume();
    mLeftClaw = GetAnimationData()->GetLocatorSegId(rstl::string_l(skLeftClaw));
    mRightClaw = GetAnimationData()->GetLocatorSegId(rstl::string_l(skRightClaw));
    break;
  case kSM_Activate:
    mCollisionManager->SetActive(mgr, true);
    break;
  case kSM_Deactivate:
    mCollisionManager->SetActive(mgr, false);
    SwarmRemove(mgr);
    break;
  case kSM_Deleted:
    mCollisionManager->Destroy(mgr);
    SwarmRemove(mgr);
    break;
  case kSM_Damage:
  case kSM_InvulnDamage:
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const TUniqueId touchedId = actor->GetLastTouchedObject();
      if (const CGameProjectile* projectile =
              TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(touchedId))) {
        if (projectile->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
          const CDamageVulnerability& vulnerability = *actor->GetDamageVulnerability();
          const CDamageInfo& damage = projectile->GetCurrentDamageInfo();
          ApplyGrowth(damage.GetDamage(), mgr);
          if (vulnerability.WeaponHits(damage.GetWeaponMode(), CDamageVulnerability::kRD_No)) {
            if (damage.GetWeaponMode().IsCharged() || damage.GetWeaponMode().IsComboed() ||
                damage.GetWeaponMode().GetType() == kWT_Missile) {
              x840_31_ = true;
              mShotSeverity += 1.f;
            }
            const CVector3f pos = projectile->GetTranslation();
            KnockBack(pos - projectile->GetPreviousPos(), mgr, damage, damage.GetKnockBackPower(),
                      true, false);
          }
          if (x840_25_) {
            mShotSeverity += 0.1f;
          }
          x840_26_ = true;
        }
      }
    } else if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(uid))) {
      CDamageInfo damage = weapon->GetCurrentDamageInfo();
      damage.SetRadius(0.f);
      mgr.ApplyDamage(
          uid, mPelvisCollisionId, weapon->GetOwnerId(), damage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
          CVector3f::Zero());
    }
    break;
  case kSM_Alert:
    x840_26_ = true;
    break;
  case kSM_Touched:
    if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
      if (HealthInfo(mgr)->GetHP() > 0.f) {
        const TUniqueId touched = actor->GetLastTouchedObject();
        if (touched == mgr.GetPlayer()->GetUniqueId() && mCurDamageRemTime <= 0.f) {
          CDamageInfo damage = GetContactDamage();
          damage.SetDamage(0.5f * damage.GetDamage());
          if (x840_29_ && x840_30_) {
            damage = GetContactDamage();
          }
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), damage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
              CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
          x840_30_ = false;
        }
      }
    }
    break;
  case kSM_InitializedInArea: {
    if (mTeamMgr == kInvalidUniqueId) {
      mTeamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    }
    const TAreaId areaId = GetCurrentAreaId();
    mPathFind.SetArea(
        mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mPathArea);
    break;
  }
  case kSM_SuspendedMove:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->SetMovable(mgr, false);
    }
    break;
  default:
    break;
  }
}

void CMetroidBeta::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  if (x840_25_) {
    x820_->Render();
    x824_->Render();
    if (mShotSeverity > 0.f) {
      x830_->Render();
    }
  }
}

void CMetroidBeta::RenderHitBallEffect() const {
  if (x840_25_) {
    x828_->Render();
  }
}

void CMetroidBeta::RenderHitGunEffect() const {
  if (x840_25_) {
    x82c_->Render();
  }
}

void CMetroidBeta::AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const {
  if (!GetModelData()->IsNull()) {
    GetModelData()->RenderParticles(planes);
    if (!GetPreRenderClipped() || IsSuckingEnergy()) {
      if (CanRenderUnsorted(mgr)) {
        Render(mgr);
      } else {
        EnsureRendered(mgr);
      }
    }
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_XRay && GetDrawShadow() &&
        GetShadow()->Valid() && planes.BoxInFrustumPlanes(GetShadow()->GetBounds()) == true) {
      gpRender->AddDrawable(GetShadow(), GetShadow()->GetTransform().GetTranslation(),
                            CAABox(GetShadow()->GetTransform().GetTranslation(),
                                   GetShadow()->GetTransform().GetTranslation()),
                            1, IRenderer::kDS_SortedCallback);
    }
  }
}

void CMetroidBeta::Touch(CActor& actor, CStateManager& mgr) {
  if (IsAlive()) {
    if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
      if (projectile->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
        if (projectile->HasAttrib(CWeapon::kPA_Charged) &&
            !projectile->HasAttrib(CWeapon::kPA_Ice)) {
          mDrainedEnergy = mData.GetMaxEnergyDrainAllowed() * GetDamageMultiplier();
          mShotSeverity = 1.f;
          x840_24_ = false;
        } else {
          const CDamageVulnerability* vulnerability = GetDamageVulnerability();
          if (projectile->HasAttrib(CWeapon::kPA_Ice) &&
              vulnerability->WeaponHits(CWeaponMode::Ice(), CDamageVulnerability::kRD_No)) {
            float multiplier = 1.f;
            if (projectile->HasAttrib(CWeapon::kPA_Charged)) {
              multiplier = 2.f;
            }
            const CVector3f relativePos = projectile->GetTranslation() - GetTranslation();
            const CVector3f projectilePos = projectile->GetTranslation();
            const CUnitVector3f direction(
                GetTransform().TransposeRotate(projectilePos - projectile->GetPreviousPos()));
            Freeze(mgr, relativePos, direction, multiplier * GetFreezeDuration());
            x840_24_ = false;
          }
          mShotSeverity += 0.1f;
        }
        x840_27_ = true;
      }
    }
    CPatterned::Touch(actor, mgr);
  }
}

void CMetroidBeta::CollidedWith(const TUniqueId& uid, const CCollisionInfoList& list,
                                CStateManager& mgr) {
  if (x840_29_) {
    static const CMaterialList stopMaterials(kMT_Ceiling, kMT_Wall, kMT_Character);
    for (int i = 0; i < list.GetCount(); ++i) {
      if (list[i].GetMaterialLeft().SharesMaterials(stopMaterials)) {
        mSpecialAttackTime += 3.f * mPreThinkDt;
        break;
      }
    }
  }
  CPatterned::CollidedWith(uid, list, mgr);
}

void CMetroidBeta::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType event, float dt) {
  bool handled = false;
  switch (event) {
  case kUE_GenerateEnd:
    AddMaterial(kMT_Solid, mgr);
    handled = true;
    break;
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, event, dt);
  }
}

CVector3f CMetroidBeta::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (const CCollisionActor* actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mPelvisCollisionId))) {
    return actor->GetTranslation();
  }
  return CPatterned::GetAimPosition(mgr, dt);
}

CVector3f CMetroidBeta::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                  const CVector3f& aimPos) const {
  const CPlayer& player = *mgr.GetPlayer();
  const CTransform4f& xf = GetTransform();
  const float posZ = xf.Get23();
  const float posY = xf.Get13();
  const CVector3f direction((CVector3f(xf.Get03(), posY, posZ) - player.GetTranslation()).DropZ(),
                            0.f);
  const CVector3f forward =
      direction.CanBeNormalized() ? direction.AsNormalized() : player.GetTransform().GetForward();
  const float range = 0.5f * GetBodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
  CVector3f result = player.GetTranslation() + range * forward;
  result.SetZ(3.5f + player.GetTranslation().GetZ());
  return result;
}

const CDamageVulnerability* CMetroidBeta::GetDamageVulnerability() const {
  if (GetBodyCtrl()->GetPercentageFrozen() > 0.f) {
    return &mData.GetFrozenVulnerability();
  }
  if (x840_25_) {
    return &mData.GetEnergyDrainVulnerability();
  }
  if (x840_28_ && !GetBodyCtrl()->IsFrozen()) {
    return &mData.GetEnergyDrainVulnerability();
  }
  return CAi::GetDamageVulnerability();
}

const CDamageVulnerability* CMetroidBeta::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                                 const CDamageInfo&) const {
  return GetDamageVulnerability();
}

EWeaponCollisionResponseTypes CMetroidBeta::GetCollisionResponseType(const CVector3f&,
                                                                     const CVector3f&,
                                                                     const CWeaponMode& mode,
                                                                     int) const {
  EWeaponCollisionResponseTypes response = kWCR_Unknown33;
  const bool frozen = GetBodyCtrl()->GetPercentageFrozen() > 0.f;
  if (!GetDamageVulnerability()->WeaponHurts(mode, CDamageVulnerability::kRD_No) && !frozen) {
    response = kWCR_Unknown58;
  }
  return response;
}

bool CMetroidBeta::IsSuckingEnergy() const { return x840_25_ && !GetBodyCtrl()->IsFrozen(); }

bool CMetroidBeta::AttackOver(CStateManager&, float) { return false; }

bool CMetroidBeta::PathShagged(CStateManager&, float) {
  if (GetSearchPath() != nullptr) {
    return GetSearchPath()->IsShagged();
  }
  return false;
}

bool CMetroidBeta::Attacked(CStateManager&, float) {
  if (mGrowthEnergy - mPreviousGrowthEnergy > 0.f) {
    if (mPreviousGrowthEnergy < mData.GetStage2GrowthEnergy()) {
      return mGrowthEnergy >= mData.GetStage2GrowthEnergy();
    }
    if (mGrowthEnergy >= mData.GetStage3GrowthEnergy()) {
      return true;
    }
  }
  return false;
}

bool CMetroidBeta::ShotAt(CStateManager&, float) { return x840_27_; }

bool CMetroidBeta::ShouldAttack(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  if (player.IsInsideFluid() || GetCurrentAreaId() != player.GetCurrentAreaId() ||
      mAttackCooldown > 0.f) {
    return false;
  }
  if (player.GetOrbitState() == CPlayer::kOS_ForcedOrbitObject ||
      player.GetOrbitState() == CPlayer::kOS_Grapple ||
      player.GetGrappleState() != CPlayer::kGS_None ||
      player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
    return false;
  }
  const CVector3f source = GetTentacleSourcePos();
  const CVector3f target = ComputeTargetPos(mgr);
  const CVector3f direction = target - source;
  const float maxAttackRange = mMaxAttackRange;
  float minRange = mMinAttackRange;
  const float margin = 0.25f * (maxAttackRange - minRange);
  const float distanceSq = direction.MagSquared();
  minRange = margin + minRange;
  const float maxRange = maxAttackRange - margin;
  const float minRangeSq = minRange * minRange;
  const float maxRangeSq = maxRange * maxRange;
  if (distanceSq < minRangeSq || distanceSq > maxRangeSq) {
    return false;
  }
  if (IsPatternObstructed(mgr, source, target)) {
    return false;
  }
  if (const CTeamAiMgr* aiMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mTeamMgr))) {
    return aiMgr->GetProjectileAttackers().size() < aiMgr->GetProjectileRoleCount();
  }
  return true;
}

bool CMetroidBeta::ShouldSpecialAttack(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  if (GetCurrentAreaId() == player.GetCurrentAreaId() && mAttackCooldown <= 0.f) {
    const CVector3f target = player.GetAimPosition(mgr, 0.f);
    const CVector3f direction = target - GetTranslation();
    if (direction.MagSquared() < mMinAttackRange * mMinAttackRange &&
        !ShouldTurn(mgr, 0.f) && !IsPatternObstructed(mgr, GetTranslation(), target)) {
      if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
        if (aiMgr->IsPartOfTeam(GetUniqueId())) {
          return aiMgr->AddMeleeAttacker(GetUniqueId());
        }
      }
      return true;
    }
  }
  return false;
}

bool CMetroidBeta::InAttackPosition(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  const CVector3f direction = GetTranslation() - player.GetTranslation();
  const CVector3f playerForward = player.GetTransform().GetForward();
  float maxAngle = CRelAngle::FromDegrees(180.f).AsRadians();
  if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    maxAngle = CRelAngle::FromDegrees(45.f).AsRadians();
  }
  if (CVector2f::GetAngleDiff(direction.ToVec2f(), playerForward.ToVec2f()) < maxAngle &&
      CVector3f::Dot(direction, GetTransform().GetForward()) < 0.f) {
    if ((player.GetTranslation() - GetTranslation()).MagSquared() <
        mMaxAttackRange * mMaxAttackRange) {
      const CVector3f& pos = GetTranslation();
      const CVector3f& playerPos = player.GetTranslation();
      if (pos.GetZ() > playerPos.GetZ() && pos.GetZ() < playerPos.GetZ() + 0.5f) {
        const CVector3f source = GetTentacleSourcePos();
        const CVector3f target = ComputeTargetPos(mgr);
        return !IsPatternObstructed(mgr, source, target);
      }
    }
  }
  return false;
}

bool CMetroidBeta::StartAttack(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  if (player.IsInsideFluid() || GetCurrentAreaId() != player.GetCurrentAreaId() ||
      mAttackCooldown > 0.f) {
    return false;
  }
  if (player.GetOrbitState() == CPlayer::kOS_ForcedOrbitObject ||
      player.GetOrbitState() == CPlayer::kOS_Grapple ||
      player.GetGrappleState() != CPlayer::kGS_None ||
      player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
    return false;
  }
  const CVector3f source = GetTentacleSourcePos();
  const CVector3f target = ComputeTargetPos(mgr);
  const CVector3f direction = target - source;
  const float distanceSq = direction.MagSquared();
  const float minRange = mMinAttackRange;
  const float maxRange = mMaxAttackRange;
  const float minRangeSq = minRange * minRange;
  const float maxRangeSq = maxRange * maxRange;
  if (distanceSq < minRangeSq || distanceSq > maxRangeSq) {
    return false;
  }
  if (IsPatternObstructed(mgr, source, target)) {
    return false;
  }
  if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
    if (aiMgr->IsPartOfTeam(GetUniqueId())) {
      return aiMgr->AddProjectileAttacker(GetUniqueId());
    }
  }
  return true;
}

bool CMetroidBeta::BreakAttack(CStateManager&, float) { return x840_31_; }

bool CMetroidBeta::InDetectionRange(CStateManager& mgr, float arg) {
  if (x840_26_) {
    return x840_26_;
  }
  return CPatterned::InDetectionRange(mgr, arg) && !mgr.GetPlayer()->IsInsideFluid();
}

bool CMetroidBeta::InPosition(CStateManager&, float) {
  const CVector3f direction = mTargetPos - GetTranslation();
  return direction.MagSquared() < 25.f;
}

bool CMetroidBeta::ShouldTurn(CStateManager& mgr, float) {
  const CVector2f direction = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
  const CVector2f forward = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward, direction) > CRelAngle::FromDegrees(15.f).AsRadians();
}

bool CMetroidBeta::ShouldWallHang(CStateManager&, float) { return mData.StartsInWall(); }

bool CMetroidBeta::AnimOver(CStateManager&, float) { return mProgState == 3; }

void CMetroidBeta::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    x840_26_ = false;
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mTeamMgr, GetUniqueId());
    if (role != nullptr) {
      mDestPos = role->GetTeamPosition();
    } else {
      mDestPos = GetOrigin(mgr, CTeamAiRole(GetUniqueId()), mgr.GetPlayer()->GetTranslation());
    }
    mTargetPos = mDestPos;
    CPatterned::PathFind(mgr, kStateMsg_Activate, 0.f);
    break;
  }
  case kStateMsg_Update:
    if (GetSearchPath() != nullptr && !PathShagged(mgr, 0.f) && !PathOver(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mTeamMgr, GetUniqueId());
      if (role != nullptr) {
        mTargetPos = role->GetTeamPosition();
      } else {
        mTargetPos =
            GetOrigin(mgr, CTeamAiRole(GetUniqueId()), mgr.GetPlayer()->GetTranslation());
      }
      ApplyForwardSteering(mgr, mTargetPos);
    }
    ApplySeparationBehavior(mgr);
    break;
  }
  AdjustPathFindSteering(mgr);
}

void CMetroidBeta::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Update: {
    const CVector3f direction = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (ShouldTurn(mgr, 0.f) && direction.CanBeNormalized()) {
      BodyCtrl()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
    } else {
      const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mTeamMgr, GetUniqueId());
      const CVector3f& target = role != nullptr ? role->GetTeamPosition()
                                                : GetOrigin(mgr, CTeamAiRole(GetUniqueId()),
                                                            mgr.GetPlayer()->GetTranslation());
      const CVector3f& position = GetTranslation();
      const CVector3f arrivalPos(position[kDX], position[kDY], target[kDZ]);
      const CVector3f arrival = mSteeringBehaviors.Arrival(*this, arrivalPos, 0.5f);
      if (arrival.MagSquared() > 0.01f) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
      }
    }
    break;
  }
  }
}

void CMetroidBeta::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  CPlayer& player = *mgr.Player();
  switch (msg) {
  case kStateMsg_Activate:
    mProgState = 2;
    mDrainedEnergy = 0.f;
    x840_25_ = true;
    x840_24_ = true;
    mShotSeverity = 0.f;
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal6);
    player.GetPlayerEnergyDrain().AddEnergyDrainSource(GetUniqueId(), 1.f);
    if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      player.SetOrbitState(CPlayer::kOS_ForcedOrbitObject, mgr);
      player.SetOrbitTargetId(GetUniqueId(), mgr);
    }
    break;
  case kStateMsg_Update: {
    SuckEnergyFromTarget(dt, mgr);
    const CVector3f target = ComputeTargetPos(mgr);
    CVector3f direction = GetTranslation() - target;
    direction.SetZ(0.f);
    const float minRange = mMinAttackRange;
    if (direction.MagSquared() >= minRange * minRange) {
      const CVector3f destination = target + minRange * direction.AsNormalized();
      const CVector3f seek = mSteeringBehaviors.Seek(*this, destination);
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(seek, CVector3f::Zero(), 1.f));
    }
    const CVector3f face = -direction;
    if (face.CanBeNormalized()) {
      const CVector3f& normal = face.AsNormalized();
      const CQuaternion rotation = CQuaternion::LookAt(
          CUnitVector3f(GetTransform().GetForward(), CUnitVector3f::kN_No),
          CUnitVector3f(normal, CUnitVector3f::kN_No), CRelAngle::FromDegrees(360.f));
      const CQuaternion orientation = CQuaternion::FromMatrix(GetTransform()) * rotation;
      SetRotation(orientation.BuildNormalized());
    }
    if (ShouldReleaseFromTarget(mgr)) {
      mProgState = 3;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    mAttackCooldown = mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime;
    x840_25_ = false;
    mShotSeverity = 0.f;
    ApplyBreakLeashDamage(mgr);
    if (GetGrowthStage() < 2.f) {
      ApplyGrowth(mDrainedEnergy, mgr);
    }
    CPlayer& player = *mgr.Player();
    player.GetPlayerEnergyDrain().RemoveEnergyDrainSource(GetUniqueId());
    mgr.PlayerState()->StaticInterference().RemoveSource(GetUniqueId());
    if (player.GetOrbitState() == CPlayer::kOS_ForcedOrbitObject &&
        player.GetOrbitTargetId() == GetUniqueId()) {
      player.SetOrbitState(CPlayer::kOS_OrbitObject, mgr);
    }
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, mTeamMgr, GetUniqueId(), true);
    break;
  }
  }
}

void CMetroidBeta::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mProgState = 2;
    x840_29_ = x840_30_ = true;
    x840_31_ = false;
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal10);
    mSpecialAttackTime = 0.f;
    ExtendTouchBounds(mgr, skExtendedTouchBounds);
    break;
  case kStateMsg_Update:
    switch (mProgState) {
    case 2:
      mSpecialAttackTime += dt;
      if (mSpecialAttackTime < mData.GetSpecialAttackTime()) {
        if (!x840_30_ || x840_31_) {
          mProgState = 3;
        } else {
          const CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
          const CVector3f direction = target - GetTranslation();
          const CVector3f forward = GetTransform().GetForward();
          if (CVector3f::Dot(forward, direction) > 0.f) {
            const CVector3f seek = mSteeringBehaviors.Seek(*this, target);
            BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(seek, CVector3f::Zero(), 1.f));
            ApplySeparationBehavior(mgr);
          } else {
            mProgState = 3;
          }
        }
      } else {
        mProgState = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mAttackCooldown = mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime;
    x840_29_ = x840_30_ = false;
    mSpecialAttackTime = 0.f;
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    ExtendTouchBounds(mgr, CVector3f::Zero());
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, mTeamMgr, GetUniqueId(), true);
    break;
  }
}

void CMetroidBeta::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    x840_27_ = false;
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CMetroidBeta::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    if (HasPatrolPath(mgr, 0.f)) {
      CPatterned::Patrol(mgr, msg, dt);
      CPatterned::UpdateDest(mgr);
    } else {
      SetDestPos(mLatestLeashPosition);
    }
    mTargetPos = mDestPos;
    if (GetSearchPath() != nullptr) {
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update:
    if (GetSearchPath() != nullptr && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      const CVector3f arrival = mSteeringBehaviors.Arrival(*this, mTargetPos, 15.f);
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
    }
    ApplySeparationBehavior(mgr);
    break;
  }
}

void CMetroidBeta::SelectTarget(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mProgState = 1;
    break;
  case kStateMsg_Update:
    switch (mProgState) {
    case 1:
      if (GetBodyCtrl()->GetCurrentStateId() == pas::kAS_Generate) {
        mProgState = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Three, CVector3f::Zero()));
      }
      break;
    case 2:
      if (GetBodyCtrl()->GetCurrentStateId() != pas::kAS_Generate) {
        mProgState = 3;
      } else {
        BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                     GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetroidBeta::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mProgState = 2;
    mTelegraphTimer = mData.GetTelegraphAttackTime();
    x840_31_ = false;
    BodyCtrl()->CommandMgr().ClearLocomotionCmds();
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Update:
    mTelegraphTimer -= dt;
    if (mTelegraphTimer > 0.f) {
      const CVector3f direction = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      if (direction.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
      }
    } else {
      mProgState = 3;
    }
    break;
  case kStateMsg_Deactivate:
    x840_31_ = false;
    break;
  }
}

void CMetroidBeta::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mGrowthEnergy >= mData.GetStage3GrowthEnergy()) {
      MassiveDeath(mgr);
    }
    mProgState = 1;
    mGenerateStartScale = GetModelData()->ScaleCopy();
    x840_28_ = true;
    break;
  case kStateMsg_Update:
    switch (mProgState) {
    case 1:
      if (GetBodyCtrl()->GetCurrentStateId() == pas::kAS_Generate) {
        mGenerateDuration = GetBodyCtrl()->GetAnimTimeRemaining();
        mProgState = mGenerateDuration > 0.f ? 2 : 3;
      } else if (Attacked(mgr, 0.f)) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, CVector3f::Zero()));
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Seven, CVector3f::Zero()));
      }
      break;
    case 2:
      if (GetBodyCtrl()->GetCurrentStateId() != pas::kAS_Generate) {
        mProgState = 3;
      } else if (Attacked(mgr, 0.f) && !GetBodyCtrl()->IsFrozen()) {
        const float timeRemaining = GetBodyCtrl()->GetAnimTimeRemaining();
        CVector3f scale = mGenerateStartScale;
        const float t = CMath::Clamp(0.f, 1.f - timeRemaining / mGenerateDuration, 1.f);
        if (t < 0.25f) {
          scale = CMath::Clamp(0.f, 1.f - 0.5f * (t / 0.25f), 1.f) * mGenerateStartScale;
        } else {
          const float duration = 0.75f * mGenerateDuration;
          const CVector3f halfScale = 0.5f * mGenerateStartScale;
          const float remaining = duration - timeRemaining;
          scale = halfScale + remaining * ((mGrowthScale - halfScale) / duration);
        }
        UpdateModelScale(scale);
        UpdateSoundVolume();
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mGenerateDuration = 0.f;
    x840_28_ = false;
    if (Attacked(mgr, 0.f)) {
      mPreviousGrowthEnergy = mGrowthEnergy;
      UpdateModelScale(mGrowthScale);
    }
    UpdateSoundVolume();
    break;
  }
}

void CMetroidBeta::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    mProgState = 0;
    RemoveMaterial(kMT_Solid, mgr);
    mCollisionManager->SetActive(mgr, false);
    break;
  case kStateMsg_Update:
    switch (mProgState) {
    case 0:
      if (x840_26_) {
        mProgState = 1;
      }
      break;
    case 1:
      if (GetBodyCtrl()->GetCurrentStateId() == pas::kAS_Generate) {
        mProgState = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, CVector3f::Zero()));
      }
      break;
    case 2:
      if (GetBodyCtrl()->GetCurrentStateId() != pas::kAS_Generate) {
        mProgState = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    mCollisionManager->SetActive(mgr, true);
    break;
  }
}

bool CMetroidBeta::ShouldReleaseFromTarget(CStateManager& mgr) {
  if (GetBodyCtrl()->IsFrozen()) {
    return true;
  }
  if (mShotSeverity >= 1.f) {
    x840_24_ = false;
    return true;
  }
  const CVector3f source = GetTentacleSourcePos();
  const CVector3f target = ComputeTargetPos(mgr);
  const CVector3f direction = target - source;
  if (direction.MagSquared() > mMaxAttackRange * mMaxAttackRange) {
    return true;
  }
  const CVector2f flatDirection = direction.DropZ();
  const float minDistanceSq = 0.25f * mMinAttackRange * mMinAttackRange;
  if (flatDirection.MagSquared() < minDistanceSq) {
    return true;
  }
  if (IsPatternObstructed(mgr, source, target)) {
    return true;
  }
  if (mDrainedEnergy >= mData.GetMaxEnergyDrainAllowed() * GetDamageMultiplier() ||
      mgr.GetPlayer()->IsInsideFluid()) {
    return true;
  }
  return false;
}

void CMetroidBeta::SuckEnergyFromTarget(float dt, CStateManager& mgr) {
  CPlayer& player = *mgr.Player();
  const float damage = dt * mData.GetEnergyDrainPerSec() * GetDamageMultiplier();
  mDrainedEnergy += damage;
  const CDamageInfo info(CWeaponMode(kWT_PoisonWater), damage, 0.f, 0.f, true);
  player.SetNoDamageLoopSfx(true);
  mgr.ApplyDamage(
      GetUniqueId(), player.GetUniqueId(), GetUniqueId(), info,
      CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
      CVector3f::Zero());
  player.SetNoDamageLoopSfx(false);
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    const float magnitude =
        0.1f * CMath::AbsF(CMath::FastSinR((M_PIF / 2.f) * mInterferenceTime));
    mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), magnitude, 0.2f);
    if (mgr.GetPlayer()->GetStaticTimer() < 0.2f) {
      player.SetHudDisable(0.2f);
    }
  }
  mInterferenceTime += dt;
}

void CMetroidBeta::UpdateParticleEffects(float dt, CStateManager& mgr) {
  if (IsSuckingEnergy()) {
    CParticleSwoosh& swoosh = *x824_;
    const int count = swoosh.GetSwooshCount();
    const CVector3f target = ComputeTargetPos(mgr);
    CVector3f source = GetTentacleSourcePos();
    const float countFloat = float(count);
    CVector3f step = target - source;
    step /= countFloat;
    const CTransform4f rotation = CTransform4f::LookAt(CVector3f::Zero(), step);
    x820_->SetParticleEmission(true);
    const int hitIndex =
        mShotSeverity > 0.f ? static_cast< int >(countFloat * (1.f - mShotSeverity)) : -1;
    x830_->SetParticleEmission(hitIndex >= 0);
    const float inverseCount = 1.f / float(count);
    float previousRotation = swoosh.GetSwooshes()[swoosh.GetSwooshCount() - 1].mInitialRot;
    for (int i = 0; i < count; ++i) {
      const float indexFloat = float(i);
      const float phase = float(i) > 0.f ? (2.f * M_PIF) / indexFloat : 0.f;
      const float amplitude = 0.25f * indexFloat * inverseCount;
      const float random = 0.5f * mRandom.Float();
      const float x = random * (amplitude * CMath::FastCosR(phase + mParticlePhase));
      const float z = random * (amplitude * CMath::FastSinR(phase));
      const CVector3f pos =
          source + (float(i) > 0.f ? rotation * CVector3f(x, 0.f, z) : CVector3f::Zero());
      for (int j = 0; j < 3; ++j) {
        const float subIndex = float(j);
        const CVector3f offset = inverseCount * (subIndex * step);
        x820_->SetTranslation(pos + offset);
        x820_->ForceParticleCreation(1);
      }
      CParticleSwoosh::SSwooshData& data = swoosh.Swooshes()[i];
      data.mTranslation = pos;
      const float currentRotation = data.mInitialRot;
      data.mInitialRot = previousRotation;
      previousRotation = currentRotation;
      if (i == hitIndex) {
        x830_->SetTranslation(pos);
        x830_->Update(dt);
      }
      source += step;
    }
    x820_->SetParticleEmission(false);
    x820_->Update(dt);
    mParticlePhase += 0.17f;
    UpdateHitTargetParticleEffect(dt, mgr, target);
  }
}

void CMetroidBeta::UpdateHitTargetParticleEffect(float dt, CStateManager& mgr,
                                                 const CVector3f& target) {
  x828_->SetParticleEmission(false);
  x82c_->SetParticleEmission(false);
  const CPlayer& player = *mgr.GetPlayer();
  switch (player.GetMorphballTransitionState()) {
  case CPlayer::kMS_Morphed: {
    x828_->SetParticleEmission(true);
    const CVector3f pos = target - CVector3f(0.f, 0.f, player.GetMorphBall()->GetBallRadius());
    x828_->SetGlobalTranslation(pos);
    x828_->SetGlobalScale(player.GetModelData()->ScaleCopy());
    break;
  }
  case CPlayer::kMS_Unmorphed: {
    x82c_->SetParticleEmission(true);
    const CTransform4f& gunXf = player.GetPlayerGun()->GetTransform();
    const CVector3f pos = gunXf.GetTranslation() + 0.25f * gunXf.GetForward();
    x82c_->SetOrientation(gunXf.GetRotation());
    x82c_->SetGlobalTranslation(pos);
    x82c_->SetGlobalScale(CVector3f(1.f, 1.f, 1.f) * CPlayerGun::GetGunScale());
    break;
  }
  }
  x828_->Update(dt);
  x82c_->Update(dt);
}

void CMetroidBeta::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(1);
  AddSphereCollisionList(skSphereJoints, 1, joints);
  mCollisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  mCollisionManager->SetActive(mgr, GetActive());
  for (uint i = 0; i < mCollisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mCollisionManager->GetCollisionDescFromIndex(i);
    const TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      if (desc.GetName() == rstl::string_l(skPelvis)) {
        mPelvisCollisionId = uid;
      }
    }
  }
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid, kMT_Wall, kMT_Floor, kMT_Ceiling),
      CMaterialList(kMT_CollisionActor, kMT_Player, kMT_Character)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  mCollisionManager->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
}

void CMetroidBeta::AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                                          rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId id = animData.GetLocatorSegId(rstl::string_l(joints[i].name));
    if (id.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          id, joints[i].radius, rstl::string_l(joints[i].name), 1000.f);
      list.push_back(desc);
    }
  }
}

void CMetroidBeta::SetupHealthInfo(CStateManager& mgr) {
  const CHealthInfo& health = *HealthInfo(mgr);
  if (CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mPelvisCollisionId))) {
    *actor->HealthInfo(mgr) = health;
    actor->SetDamageVulnerability(*GetDamageVulnerability());
  }
}

void CMetroidBeta::UpdateHealthInfo(CStateManager& mgr) {
  if (IsAlive()) {
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(mPelvisCollisionId))) {
      actor->SetDamageVulnerability(*GetDamageVulnerability());
      HealthInfo(mgr)->SetHP(actor->HealthInfo(mgr)->GetHP());
    }
    if (HealthInfo(mgr)->GetHP() <= 0.f) {
      Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
      SwarmRemove(mgr);
      RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    }
  }
}

void CMetroidBeta::ExtendTouchBounds(CStateManager& mgr, const CVector3f& bounds) {
  if (CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(mPelvisCollisionId))) {
    actor->SetExtendedTouchBounds(bounds);
  }
}

void CMetroidBeta::ApplyBreakLeashDamage(CStateManager& mgr) {
  if (x840_24_) {
    const float damage = mData.GetBreakLeashEnergyDrain() * GetDamageMultiplier();
    const CDamageInfo info(CWeaponMode::Power(), damage, 0.f, 0.f, true);
    mgr.ApplyDamage(
        GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), info,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
        CVector3f::Zero());
  }
}

CVector3f CMetroidBeta::GetTentacleSourcePos() const {
  const CTransform4f leftXf = GetLctrTransform(mLeftClaw);
  const CVector3f left = leftXf.GetTranslation();
  const CTransform4f rightXf = GetLctrTransform(mRightClaw);
  const CVector3f center = 0.5f * (left + rightXf.GetTranslation());
  return center + CVector3f::ByElementMultiply(GetModelData()->GetScale(),
                                               GetTransform().Rotate(skTentacleOffset));
}

CVector3f CMetroidBeta::ComputeTargetPos(CStateManager& mgr) const {
  const CPlayer& player = *mgr.GetPlayer();
  CVector3f target = player.GetTranslation();
  switch (player.GetMorphballTransitionState()) {
  case CPlayer::kMS_Unmorphed:
    target = player.GetPlayerGun()->GetTransform().GetTranslation();
    break;
  case CPlayer::kMS_Morphed:
  case CPlayer::kMS_Morphing:
  case CPlayer::kMS_Unmorphing:
    target = player.GetAimPosition(mgr, 0.f);
    break;
  default:
    break;
  }
  return target;
}

float CMetroidBeta::GetDamageMultiplier() const {
  float result = 0.5f * (GetGrowthStage() - 1.f) + 1.f;
  return result;
}

float CMetroidBeta::GetGrowthStage() const {
  if (mGrowthEnergy < mData.GetStage2GrowthEnergy()) {
    return 1.f + mGrowthEnergy / mData.GetStage2GrowthEnergy();
  }
  if (mGrowthEnergy < mData.GetStage3GrowthEnergy()) {
    return 2.f + (mGrowthEnergy - mData.GetStage2GrowthEnergy()) /
                     (mData.GetStage3GrowthEnergy() - mData.GetStage2GrowthEnergy());
  }
  return 3.f;
}

void CMetroidBeta::ApplyGrowth(float amount, CStateManager& mgr) {
  mGrowthEnergy += amount;
  const float amountScale = mData.GetMaxGrowthScale() - mInitialScale.GetY();
  const float growth = CMath::Clamp(0.f, mGrowthEnergy / mData.GetGrowthEnergy(), 1.f);
  const float scale = growth * amountScale + mInitialScale.GetY();
  mGrowthScale = CVector3f(scale, scale, scale);
  TakeDamage(CVector3f::Zero(), 0.f);
}

void CMetroidBeta::UpdateSoundVolume() {
  const float growth = CMath::Clamp(0.f, GetGrowthStage() - 1.f, 1.f);
  SetVolume(
      CMath::Clamp< uchar >(0, CCast::ToUint8(127.f * ((1.f - 0.67f) * growth + 0.67f)), 127));
  const int pitch = CMath::Clamp(0, 4096 - CCast::ToInt32(4096.f * growth), 16383);
  SetSoundEventPitchBend(pitch);
}

void CMetroidBeta::UpdateModelScale(const CVector3f& scale) { ModelData()->SetScale(scale); }

void CMetroidBeta::SwarmAdd(CStateManager& mgr) {
  if (mTeamMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
      if (!aiMgr->IsPartOfTeam(GetUniqueId())) {
        aiMgr->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Invalid,
                                CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CMetroidBeta::SwarmRemove(CStateManager& mgr) {
  if (mTeamMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
      if (aiMgr->IsPartOfTeam(GetUniqueId())) {
        aiMgr->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

void CMetroidBeta::ApplySeparationBehavior(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f separation =
            mSteeringBehaviors.Separation(*this, ai->GetTranslation(), 5.f);
        if (separation.IsNonZero()) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CMetroidBeta::ApplyForwardSteering(CStateManager& mgr, const CVector3f& destination) {
  const CVector3f position = GetTranslation();
  const CVector3f delta = destination - position;
  if (delta.MagSquared() > 25.f) {
    const CVector3f arrivalPos(GetTranslation().GetX(), GetTranslation().GetY(),
                               destination.GetZ());
    const CVector3f arrival = mSteeringBehaviors.Arrival(*this, arrivalPos, 0.5f);
    if (arrival.MagSquared() > 0.01f) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(arrival, CVector3f::Zero(), 3.f));
    }
    const CVector3f seek = mSteeringBehaviors.Seek(*this, destination);
    BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(seek, CVector3f::Zero(), 1.f));
  } else if (ShouldTurn(mgr, 0.f)) {
    const CVector3f direction = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (direction.CanBeNormalized()) {
      BodyCtrl()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
    }
  }
}

void CMetroidBeta::AdjustPathFindSteering(CStateManager& mgr) {
  const CVector3f move = GetBodyCtrl()->GetCommandMgr().GetMoveVector();
  const CVector3f forward = GetTransform().GetForward();
  if (CVector3f::Dot(forward, move) < 0.f && move.CanBeNormalized()) {
    BodyCtrl()->CommandMgr().ClearLocomotionCmds();
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f::Zero(), move.AsNormalized(), 1.f));
  }
}

void CMetroidBeta::UpdateAILogicTimers(float dt, CStateManager& mgr) {
  if (mAttackCooldown > 0.f) {
    const float delta =
        mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed ? 2.f * dt : dt;
    mAttackCooldown -= delta;
  }
  mShotSeverity = mShotSeverity > 0.3f * dt ? mShotSeverity - 0.3f * dt : 0.f;
}
